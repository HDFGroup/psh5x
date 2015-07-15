
#include "HDF5Exception.h"
#include "PrimitiveTypeDatasetWriter.h"
#include "PSH5XException.h"
#include "ProviderUtils.h"

extern "C" {
#include "H5Dpublic.h"
#include "H5Spublic.h"
#include "H5Tpublic.h"
#include "hdf5_hl.h"
}

using namespace System;
using namespace System::Collections;
using namespace System::Collections::ObjectModel;
using namespace System::IO;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Provider;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
	    IList^ PrimitiveTypeDatasetWriter::Write(IList^ content)
        {
            char* name = (char*)(Marshal::StringToHGlobalAnsi(m_h5path)).ToPointer();

            hid_t dset = -1, ftype = -1, mtype = -1, fspace = -1, mspace = H5S_ALL, ptable = -1;

			bool sel_flag = false, same_value_flag = false;

			hssize_t npoints = 1;

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

				if (!m_is_pkttable)
				{
					fspace = H5Dget_space(dset);
					if (fspace < 0) {
						throw gcnew HDF5Exception("H5Dget_space failed!");
					}

					H5S_class_t scls = H5Sget_simple_extent_type(fspace);

					sel_flag = ProviderUtils::WriterCheckSelection(fspace, mspace, safe_cast<hsize_t>(content->Count), m_dict);

					if (scls == H5S_SIMPLE) {
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
				}
				else {
					ptable = H5PTopen(m_h5file, name);
				}

				// peek at the element type of the first element
				IEnumerator^ ienum = content->GetEnumerator();
				ienum->MoveNext();
				Object^ obj = ProviderUtils::GetDotNetObject(ienum->Current);
				Type^ t = obj->GetType();
				if (!t->IsPrimitive) {
					throw gcnew PSH5XException("This is not a primitive .NET type!");
				}
				mtype = ProviderUtils::GetH5MemoryType(t, ftype);
				ienum->Reset();
			    
				MemoryStream^ ms = gcnew MemoryStream();
				BinaryWriter^ writer = gcnew BinaryWriter(ms);

				if (t == Byte::typeid) {
					if (!same_value_flag) {
						while (ienum->MoveNext()) {
							writer->Write((Byte) ienum->Current);
						}
					}
					else {
						ienum->MoveNext();
						Byte x = safe_cast<Byte>(ienum->Current);
						for (hssize_t i = 0; i < npoints; ++i) {
							writer->Write(x);
						}
					}
				}
				else if (t == SByte::typeid) {
					if (!same_value_flag) {
						while (ienum->MoveNext()) {
							writer->Write((SByte) ienum->Current);
						}
					}
					else {
						ienum->MoveNext();
						SByte x = safe_cast<SByte>(ienum->Current);
						for (hssize_t i = 0; i < npoints; ++i) {
							writer->Write(x);
						}
					}

				}
				else if (t == UInt16::typeid) {
					if (!same_value_flag) {
						while (ienum->MoveNext()) {
							writer->Write((UInt16) ienum->Current);
						}
					}
					else {
						ienum->MoveNext();
						UInt16 x = safe_cast<UInt16>(ienum->Current);
						for (hssize_t i = 0; i < npoints; ++i) {
							writer->Write(x);
						}
					}
				}
				else if (t == Int16::typeid) {
					if (!same_value_flag) {
						while (ienum->MoveNext()) {
							writer->Write((Int16) ienum->Current);
						}
					}
					else {
						ienum->MoveNext();
						Int16 x = safe_cast<Int16>(ienum->Current);
						for (hssize_t i = 0; i < npoints; ++i) {
							writer->Write(x);
						}
					}
				}
				else if (t == UInt32::typeid) {
					if (!same_value_flag) {
						while (ienum->MoveNext()) {
							writer->Write((UInt32) ienum->Current);
						}
					}
					else {
						ienum->MoveNext();
						UInt32 x = safe_cast<UInt32>(ienum->Current);
						for (hssize_t i = 0; i < npoints; ++i) {
							writer->Write(x);
						}
					}
				}
				else if (t == Int32::typeid) {
					if (!same_value_flag) {
						while (ienum->MoveNext()) {
							writer->Write((Int32) ienum->Current);
						}
					}
					else {
						ienum->MoveNext();
						Int32 x = safe_cast<Int32>(ienum->Current);
						for (hssize_t i = 0; i < npoints; ++i) {
							writer->Write(x);
						}
					}
				}
				else if (t == UInt64::typeid) {
					if (!same_value_flag) {
						while (ienum->MoveNext()) {
							writer->Write((UInt64) ienum->Current);
						}
					}
					else {
						ienum->MoveNext();
						UInt64 x = safe_cast<UInt64>(ienum->Current);
						for (hssize_t i = 0; i < npoints; ++i) {
							writer->Write(x);
						}
					}
				}
				else if (t == Int64::typeid) {
					if (!same_value_flag) {
						while (ienum->MoveNext()) {
							writer->Write((Int64) ienum->Current);
						}
					}
					else {
						ienum->MoveNext();
						Int64 x = safe_cast<Int64>(ienum->Current);
						for (hssize_t i = 0; i < npoints; ++i) {
							writer->Write(x);
						}
					}
				}
				else if (t == Single::typeid) {
					if (!same_value_flag) {
						while (ienum->MoveNext()) {
							writer->Write((Single) ienum->Current);
						}
					}
					else {
						ienum->MoveNext();
						Single x = safe_cast<Single>(ienum->Current);
						for (hssize_t i = 0; i < npoints; ++i) {
							writer->Write(x);
						}
					}
				}
				else if (t == Double::typeid) {
					if (!same_value_flag) {
						while (ienum->MoveNext()) {
							writer->Write((Double) ienum->Current);
						}
					}
					else {
						ienum->MoveNext();
						Double x = safe_cast<Double>(ienum->Current);
						for (hssize_t i = 0; i < npoints; ++i) {
							writer->Write(x);
						}
					}
				}
				else {
					throw gcnew PSH5XException("Unsupported primitive .NET type!");
				}

				array<unsigned char>^ a = ms->ToArray();
				pin_ptr<unsigned char> ptr = &a[0];

				if (!m_is_pkttable)
				{	
					if (H5Dwrite(dset, mtype, mspace, fspace, H5P_DEFAULT, ptr) < 0) {
						throw gcnew HDF5Exception("H5Dwrite failed!");
					}

					if (H5Fflush(dset, H5F_SCOPE_LOCAL) < 0) {
						throw gcnew HDF5Exception("H5Fflush failed!");
					}
				}
				else
				{
					if (H5PTappend(ptable, safe_cast<size_t>(content->Count), ptr) < 0) {
						throw gcnew HDF5Exception("H5PTappend failed!");
					}

					if (H5Fflush(m_h5file, H5F_SCOPE_LOCAL) < 0) {
						throw gcnew HDF5Exception("H5Fflush failed!");
					}
				}
            }
            finally
            {
				if (ptable >= 0) {
					H5PTclose(ptable);
				}
                if (mtype >= 0) {
                    H5Tclose(mtype);
                }
                if (ftype >= 0) {
                    H5Tclose(ftype);
                }
                if (fspace >= 0) {
                    H5Sclose(fspace);
                }
				if (mspace != H5S_ALL) {
                    H5Sclose(mspace);
                }
                if (dset >= 0) {
                    H5Dclose(dset);
                }

                Marshal::FreeHGlobal(IntPtr(name));
            }

        return content;
	}

	void PrimitiveTypeDatasetWriter::Seek(long long offset, System::IO::SeekOrigin origin)
	{
		offset = 0;
		origin = System::IO::SeekOrigin::End;

		if (ProviderUtils::IsH5PacketTable(m_h5file, m_h5path)) {
			m_is_pkttable = true;
		}
		else {
			throw gcnew PSH5XException("Add-Content is only available for HDF5 packet tables!");
		}
	}
}