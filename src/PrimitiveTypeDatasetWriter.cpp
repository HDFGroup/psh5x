
#include "HDF5Exception.h"
#include "PrimitiveTypeDatasetWriter.h"
#include "PSH5XException.h"
#include "ProviderUtils.h"

extern "C" {
#include "H5Dpublic.h"
#include "H5Spublic.h"
#include "H5Tpublic.h"
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
                fspace = H5Dget_space(dset);
                if (fspace < 0) {
                    throw gcnew HDF5Exception("H5Dget_space failed!");
                }

                hssize_t npoints = 1;
				if (H5Sget_simple_extent_type(fspace) == H5S_SIMPLE) {
				    npoints = H5Sget_simple_extent_npoints(fspace);
				}

                if (content->Count != safe_cast<int>(npoints)) {
                    throw gcnew PSH5XException("Size mismatch!");
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
					while (ienum->MoveNext()) {
						writer->Write((Byte) ienum->Current);
					}
				}
				else if (t == SByte::typeid) {
					while (ienum->MoveNext()) {
						writer->Write((SByte) ienum->Current);
					}
				}
				else if (t == UInt16::typeid) {
					while (ienum->MoveNext()) {
						writer->Write((UInt16) ienum->Current);
					}
				}
				else if (t == Int16::typeid) {
					while (ienum->MoveNext()) {
						writer->Write((Int16) ienum->Current);
					}
				}
				else if (t == UInt32::typeid) {
					while (ienum->MoveNext()) {
						writer->Write((UInt32) ienum->Current);
					}
				}
				else if (t == Int32::typeid) {
					while (ienum->MoveNext()) {
						writer->Write((Int32) ienum->Current);
					}
				}
				else if (t == UInt64::typeid) {
					while (ienum->MoveNext()) {
						writer->Write((UInt64) ienum->Current);
					}
				}
				else if (t == Int64::typeid) {
					while (ienum->MoveNext()) {
						writer->Write((Int64) ienum->Current);
					}
				}
				else if (t == Single::typeid) {
					while (ienum->MoveNext()) {
						writer->Write((Single) ienum->Current);
					}
				}
				else if (t == Double::typeid) {
					while (ienum->MoveNext()) {
						writer->Write((Double) ienum->Current);
					}
				}
				else {
					throw gcnew PSH5XException("Unsupported primitive .NET type!");
				}

				array<unsigned char>^ a = ms->ToArray();
				pin_ptr<unsigned char> ptr = &a[0];

                if (H5Dwrite(dset, mtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, ptr) < 0) {
                    throw gcnew HDF5Exception("H5Dwrite failed!");
                }

                if (H5Fflush(dset, H5F_SCOPE_LOCAL) < 0) {
                    throw gcnew ArgumentException("H5Fflush failed!");
                }
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

		void PrimitiveTypeDatasetWriter::Seek(long long offset, System::IO::SeekOrigin origin)
		{
			throw gcnew PSH5XException("PrimitiveTypeDatasetWriter::Seek() not implemented!");
		}
}