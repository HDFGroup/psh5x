
#include "HDF5Exception.h"
#include "CompoundDatasetWriter.h"
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
    CompoundDatasetWriter::CompoundDatasetWriter(hid_t h5file, System::String^ h5path)
        : m_h5file(h5file), m_h5path(h5path), m_array(nullptr)
    {
    }

    IList^ CompoundDatasetWriter::Write(IList^ content)
    {
		char* name = (char*)(Marshal::StringToHGlobalAnsi(m_h5path)).ToPointer();

        hid_t dset = -1, fspace = -1, ftype = -1, mtype = -1;

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
			mtype = H5Tget_native_type(ftype, H5T_DIR_ASCEND);
			if (mtype < 0) {
				throw gcnew ArgumentException("H5Tget_native_type failed!");
			}
			fspace = H5Dget_space(dset);
			if (fspace < 0) {
				throw gcnew HDF5Exception("H5Dget_space failed!");
			}

			hssize_t npoints = 1;
			if (H5Sget_simple_extent_type(fspace) == H5S_SIMPLE) {
				npoints = H5Sget_simple_extent_npoints(fspace);
			}

			if (content->Count != safe_cast<int>(npoints))
			{
				throw gcnew PSH5XException("Size mismatch!");
			}

			if (npoints > 0)
			{
				size_t size = H5Tget_size(ftype);

				int mcount = H5Tget_nmembers(ftype);

				// parse the compound type

				m_array = gcnew array<unsigned char>(content->Count*size);

				size_t count = 0;

				IEnumerator^ ienum = content->GetEnumerator();
				ienum->MoveNext();
				
				for (int i = 0; i < npoints; ++i)
				{
					if (ienum->Current != nullptr)
					{
						Object^ obj = ProviderUtils::GetDotNetObject(ienum->Current);

						

						// do something

						


					}
					else {
						throw gcnew PSH5XException(String::Format("Uninitialized array found at position {0}!", npoints));
					}

					ienum->MoveNext();
				}

				pin_ptr<unsigned char> ptr = &((array<unsigned char>^)m_array)[0];

				if (H5Dwrite(dset, mtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, ptr) < 0) {
					throw gcnew HDF5Exception("H5Dwrite failed!");
				}

				if (H5Fflush(dset, H5F_SCOPE_LOCAL) < 0) {
					throw gcnew ArgumentException("H5Fflush failed!");
				}
			}

			m_position += content->Count;
		}
		finally
		{
			
            if (mtype >= 0) {
                H5Tclose(mtype);
            }
            if (ftype >= 0) {
                H5Tclose(ftype);
            }
            if (fspace >= 0) {
                H5Sclose(fspace);
            }
            if (dset >= 0) {
                H5Dclose(dset);
            }

            Marshal::FreeHGlobal(IntPtr(name));
		}

		return nullptr;
	}
}