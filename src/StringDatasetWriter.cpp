
#include "HDF5Exception.h"
#include "StringDatasetWriter.h"
#include "PSH5XException.h"
#include "ProviderUtils.h"

extern "C" {
#include "H5Dpublic.h"
#include "H5Spublic.h"
#include "H5Tpublic.h"
}

#include <cstring>

using namespace System;
using namespace System::Collections;
using namespace System::Collections::ObjectModel;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Provider;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
    StringDatasetWriter::StringDatasetWriter(hid_t h5file, System::String^ h5path,
		RuntimeDefinedParameterDictionary^ dict)
        : m_h5file(h5file), m_h5path(h5path), m_dict(dict), m_array(nullptr)
    {
    }

    IList^ StringDatasetWriter::Write(IList^ content)
    {
        char* name = (char*)(Marshal::StringToHGlobalAnsi(m_h5path)).ToPointer();

        hid_t dset = -1, ftype = -1, mtype = -1, fspace = -1, mspace = H5S_ALL;

        htri_t is_vlen = -1;

        char** wdata = NULL;
        char** vwdata = NULL;

		bool sel_flag = false, same_value_flag = false;

		hssize_t npoints = 0;

		int i = 0;

        try
        {
            dset = H5Dopen2(m_h5file, name, H5P_DEFAULT);
            if (dset < 0) {
                throw gcnew HDF5Exception("H5Dopen2 failed!");
            }
			ftype = H5Dget_type(dset);
            if (ftype < 0) {
                throw gcnew HDF5Exception("H5Dget_type failed!");
            }
            fspace = H5Dget_space(dset);
            if (fspace < 0) {
                throw gcnew HDF5Exception("H5Dget_space failed!");
            }

			sel_flag = ProviderUtils::WriterCheckSelection(fspace, mspace, safe_cast<hsize_t>(content->Count), m_dict);

			npoints = 1;
			if (H5Sget_simple_extent_type(fspace) == H5S_SIMPLE) {
				if (sel_flag) {
					npoints = H5Sget_select_npoints(fspace);
				}
				else {
					npoints = H5Sget_simple_extent_npoints(fspace);
				}
			}

			if (content->Count == 1 &&
				H5Sget_simple_extent_type(fspace) != H5S_SCALAR) // assign the same value to all (selected) elements (or scalar)
			{
				same_value_flag = true;
			}
			else
			{
				if (content->Count != safe_cast<int>(npoints)) {
					throw gcnew PSH5XException("Size mismatch!");
				}
			}

			is_vlen = H5Tis_variable_str(ftype);

			array<String^>^ astring = nullptr;

			if (is_vlen > 0)
			{
				vwdata = new char* [npoints];

				astring = gcnew array<String^>(safe_cast<int>(npoints));

				if (ProviderUtils::TryGetValue(content, astring))
				{
					if (!same_value_flag) {
						for (i = 0; i < npoints; ++i) {
							vwdata[i] = (char*) Marshal::StringToHGlobalAnsi(astring[i]).ToPointer();
						}
					}
					else
					{
						char* ptr = (char*) Marshal::StringToHGlobalAnsi(astring[0]).ToPointer();
						for (i = 0; i < npoints; ++i) {
							vwdata[i] = ptr;
						}
					}

					mtype = ProviderUtils::GetH5MemoryType(String::typeid, ftype);

					if (H5Dwrite(dset, mtype, mspace, fspace, H5P_DEFAULT, vwdata) < 0) {
						throw gcnew HDF5Exception("H5Dwrite failed!");
					}
					if (H5Fflush(dset, H5F_SCOPE_LOCAL) < 0) {
						throw gcnew HDF5Exception("H5Fflush failed!");
					}
				}
				else {
					throw gcnew PSH5XException("Value type mismatch!");
				}
			}
			else if (is_vlen == 0)
			{
				mtype = ProviderUtils::GetH5MemoryType(String::typeid, ftype);
				size_t size = H5Tget_size(mtype);

				wdata = new char* [npoints];
				wdata[0] = new char [npoints*size];
				for (i = 1; i < npoints; ++i) {
					wdata[i] = wdata[0] + i*size;
				}

				astring = gcnew array<String^>(safe_cast<int>(npoints));

				if (ProviderUtils::TryGetValue(content, astring))
				{
					size_t length = 0;

					if (!same_value_flag)
					{
						for (i = 0; i < npoints; ++i)
						{
							length = safe_cast<size_t>(astring[i]->Length);
							if (length >= size) {
								throw gcnew PSH5XException(String::Format("String >{0}< too long!", astring[i]));
							}
						
							char* buf = (char*) Marshal::StringToHGlobalAnsi(astring[i]).ToPointer();
							memcpy((void*) wdata[i], (void*) buf, length+1);
							Marshal::FreeHGlobal(IntPtr(buf));
						}
					}
					else // same value everywhere
					{
						length = safe_cast<size_t>(astring[0]->Length);
						if (length >= size) {
							throw gcnew PSH5XException(String::Format("String >{0}< too long!", astring[0]));
						}

						char* buf = (char*) Marshal::StringToHGlobalAnsi(astring[0]).ToPointer();
						for (i = 0; i < npoints; ++i) {
							memcpy((void*) wdata[i], (void*) buf, length+1);
						}
						Marshal::FreeHGlobal(IntPtr(buf));
					}

					if (H5Dwrite(dset, mtype, mspace, fspace, H5P_DEFAULT, wdata[0]) < 0) {
						throw gcnew HDF5Exception("H5Dwrite failed!");
					}
					if (H5Fflush(dset, H5F_SCOPE_LOCAL) < 0) {
						throw gcnew HDF5Exception("H5Fflush failed!");
					}
				}
				else {
					throw gcnew PSH5XException("Value size or type mismatch!");
				}
			}
			else {
				throw gcnew PSH5XException("Unknown STRING type found!!!");
			}

			m_position += content->Count;
        }
        finally
        {
            if (vwdata != NULL) {
				if (!same_value_flag) {
					for (i = 0; i < npoints; ++i) {   
						Marshal::FreeHGlobal(IntPtr(vwdata[i]));
					}
				}
				else {
					Marshal::FreeHGlobal(IntPtr(vwdata[0]));
				}
                delete [] vwdata;
            }

            if (wdata != NULL) {
                delete [] wdata[0];
                delete [] wdata;
            }

			if (mspace != H5S_ALL) {
				H5Sclose(mspace);
			}
			if (fspace >= 0) {
				H5Sclose(fspace);
			}
            if (mtype >= 0) {
                H5Tclose(mtype);
            }
            if (ftype >= 0) {
                H5Tclose(ftype);
            }
            if (dset >= 0) {
                H5Dclose(dset);
            }

            Marshal::FreeHGlobal(IntPtr(name));
        }

        return content;
    }

	void StringDatasetWriter::Seek(long long offset, System::IO::SeekOrigin origin)
	{
		offset = 0;
		origin = System::IO::SeekOrigin::End;

		throw gcnew PSH5XException("StringDatasetWriter::Seek() not implemented!");
	}
}
