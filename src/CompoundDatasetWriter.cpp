
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
using namespace System::IO;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Provider;
using namespace System::Reflection;
using namespace System::Runtime::InteropServices;
using namespace System::Text;

namespace PSH5X
{
    CompoundDatasetWriter::CompoundDatasetWriter(hid_t h5file, System::String^ h5path)
        : m_h5file(h5file), m_h5path(h5path), m_array(nullptr)
    {
    }

    IList^ CompoundDatasetWriter::Write(IList^ content)
    {
		char* name = (char*)(Marshal::StringToHGlobalAnsi(m_h5path)).ToPointer();

        hid_t dset = -1, fspace = -1, ftype = -1, mtype = -1, cmtype = -1, base_type = -1;

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


#pragma region peek at the first object and determine the in-memory type

			IEnumerator^ ienum = content->GetEnumerator();
			ienum->MoveNext();

			Object^ obj = ProviderUtils::GetDotNetObject(ienum->Current);
			mtype = ProviderUtils::GetH5MemoryType(obj->GetType(), ftype);

			array<FieldInfo^>^ fields = ProviderUtils::GetDotNetObject(obj)->GetType()->GetFields();

			ienum->Reset();

#pragma endregion

#pragma region parse the compound type

			int mcount = H5Tget_nmembers(mtype);

			array<H5T_class_t>^ member_class = gcnew array<H5T_class_t>(mcount);

			array<MethodInfo^>^ member_info = gcnew array<MethodInfo^>(mcount);

			array<bool>^ member_is_vlen_str = gcnew array<bool>(mcount);

			array<int>^ member_strlen = gcnew array<int>(mcount);

			Type^ magicType = System::IO::BinaryWriter::typeid;

			for (int m = 0; m < mcount; ++m)
			{
				cmtype = H5Tget_member_type(mtype, safe_cast<unsigned>(m));
				if (cmtype < 0) {
					throw gcnew HDF5Exception("H5Tget_member_type failed!");
				}

				H5T_class_t cls = H5Tget_class(cmtype);
				member_class[m] = cls;

				switch (cls)
				{
				case H5T_BITFIELD:
				case H5T_ENUM:
				case H5T_FLOAT:
				case H5T_INTEGER:
					{
						member_info[m] = magicType->GetMethod("Write",
							gcnew array<Type^>{ ProviderUtils::H5Type2DotNet(cmtype) });

						if (member_info[m] == nullptr) {
							throw gcnew PSH5XException("Unable to serialize type!");
						}
					}
					break;

				case H5T_STRING:
					{
						member_is_vlen_str[m] = (H5Tis_variable_str(cmtype) > 0);
						if (!member_is_vlen_str[m]) {
							member_strlen[m] = safe_cast<int>(H5Tget_size(cmtype));
						}
					}
					break;

				case H5T_ARRAY:
					{
						base_type = H5Tget_super(cmtype);
						if (base_type < 0) {
							throw gcnew HDF5Exception("H5Tget_member_type failed!");
						}

						member_info[m] = magicType->GetMethod("Write",
							gcnew array<Type^>{ ProviderUtils::H5Type2DotNet(base_type) });

						if (member_info[m] == nullptr) {
							throw gcnew PSH5XException("Unable to serialize type!");
						}

						H5Tclose(base_type);
						base_type = -1;
					}
					break;

				default:

					throw gcnew PSH5XException("Unsupported member type!");
					break;
				}

				if (H5Tclose(cmtype) < 0) {
					throw gcnew HDF5Exception("H5Tclose failed!");
				}
				cmtype = -1;
			}

#pragma endregion

			MemoryStream^ ms = gcnew MemoryStream();
			ASCIIEncoding^ ascii = gcnew ASCIIEncoding();
			BinaryWriter^ writer = gcnew BinaryWriter(ms, ascii);

			while (ienum->MoveNext())
			{
				if (ienum->Current != nullptr)
				{
					Object^ obj = ProviderUtils::GetDotNetObject(ienum->Current);
					for (int m = 0; m < fields->Length; m++)
					{
						Object^ value = fields[m]->GetValue(obj);
						switch (member_class[m])
						{
						case H5T_BITFIELD:
						case H5T_ENUM:
						case H5T_FLOAT:
						case H5T_INTEGER:
							{
								member_info[m]->Invoke(writer, gcnew array<Object^>{value});
							}
							break;

						case H5T_STRING:
							{
								if (member_is_vlen_str[m]) {
									writer->Write(Marshal::StringToHGlobalAnsi((String^) value).ToInt64());
								}
								else{
									array<unsigned char>^ bytes = Encoding::ASCII->GetBytes(
										((String^) value)->PadRight(member_strlen[m])->Substring(0, member_strlen[m]));
									writer->Write(bytes);
								}
							}
							break;

						case H5T_ARRAY:
							{
								IEnumerator^ aenum = ((Array^) value)->GetEnumerator();
								while (aenum->MoveNext()) {
									member_info[m]->Invoke(
										writer, gcnew array<Object^>{aenum->Current});
								}
							}
							break;

						default:

							break;
						}
					}
				}
				else {
					throw gcnew PSH5XException(
						String::Format("Uninitialized array found at position {0}!", npoints));
				}
			}

			m_array = ms->ToArray();
			pin_ptr<unsigned char> ptr = &((array<unsigned char>^)m_array)[0];

			if (H5Dwrite(dset, mtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, ptr) < 0) {
				throw gcnew HDF5Exception("H5Dwrite failed!");
			}

			if (H5Fflush(dset, H5F_SCOPE_LOCAL) < 0) {
				throw gcnew ArgumentException("H5Fflush failed!");
			}

			m_position += content->Count;
		}
		finally
		{
			if (base_type >= 0) {
                H5Tclose(base_type);
            }
			if (cmtype >= 0) {
                H5Tclose(cmtype);
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
            if (dset >= 0) {
                H5Dclose(dset);
            }

            Marshal::FreeHGlobal(IntPtr(name));
		}

		return nullptr;
	}
}