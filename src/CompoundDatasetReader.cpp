
#include "ArrayUtils.h"
#include "CompoundDatasetReader.h"
#include "HDF5Exception.h"
#include "ProviderUtils.h"
#include "PSH5XException.h"

extern "C" {
#include "H5Dpublic.h"
#include "H5Spublic.h"
#include "H5Tpublic.h"
}

using namespace System;
using namespace System::Collections;
using namespace System::IO;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Provider;
using namespace System::Reflection;
using namespace System::Runtime::InteropServices;
using namespace System::Text;

namespace PSH5X
{
    CompoundDatasetReader::CompoundDatasetReader(hid_t dset, hid_t ftype, hid_t fspace)
        : m_array(nullptr), m_position(0)
    {
        hid_t mtype = -1, cmtype = -1, base_type = -1;

        try
        {
            hssize_t npoints = 1;

			H5S_class_t cls = H5Sget_simple_extent_type(fspace);
			if (cls == H5S_SIMPLE) { 
				npoints = H5Sget_simple_extent_npoints(fspace);
			}

			int rank = 1;
			if (cls == H5S_SIMPLE) {
				rank = H5Sget_simple_extent_ndims(fspace);
			}

			array<hsize_t>^ dims = gcnew array<hsize_t>(rank);
			dims[0] = 1;
			if (cls == H5S_SIMPLE)
			{
				pin_ptr<hsize_t> dims_ptr = &dims[0];
				rank = H5Sget_simple_extent_dims(fspace, dims_ptr, NULL);
			}

#pragma region get a .NET representation and the in-memory type

			m_type = ProviderUtils::GetCompundDotNetType(ftype);

			array<ConstructorInfo^>^ cinfo = m_type->GetConstructors();
			if (cinfo->Length != 2) {
				throw gcnew PSH5XException("Constructor code generation error!");
			}

			mtype = ProviderUtils::GetH5MemoryType(m_type, ftype);
			size_t size = H5Tget_size(mtype);

#pragma endregion


#pragma region parse the compound type

			int mcount = H5Tget_nmembers(mtype);

			array<H5T_class_t>^ member_class = gcnew array<H5T_class_t>(mcount);

			array<MethodInfo^>^ member_info = gcnew array<MethodInfo^>(mcount);

			array<bool>^ member_is_vlen_str = gcnew array<bool>(mcount);

			array<int>^ member_strlen = gcnew array<int>(mcount);

			array<bool>^ member_is_array = gcnew array<bool>(mcount);

			array<array<hsize_t>^>^ member_array_dims = gcnew array<array<hsize_t>^>(mcount);

			array<size_t>^ member_array_length = gcnew array<size_t>(mcount);

			for (int i = 0; i < mcount; ++i)
			{
				cmtype = H5Tget_member_type(mtype, safe_cast<unsigned>(i));
				if (cmtype < 0) {
					throw gcnew HDF5Exception("H5Tget_member_type failed!");
				}
				member_class[i] = H5Tget_class(cmtype);

				if (member_class[i] == H5T_STRING)
				{
					member_is_vlen_str[i] = (H5Tis_variable_str(cmtype) > 0);

					if (!member_is_vlen_str[i]) {
						member_strlen[i] = safe_cast<int>(H5Tget_size(cmtype));
					}
				}

				if (member_class[i] == H5T_ARRAY)
				{
					member_is_array[i] = true;

					int arank = H5Tget_array_ndims(cmtype);
					if (arank < 0) {
						throw gcnew HDF5Exception("H5Tget_array_ndims failed!");
					}

					member_array_dims[i] = gcnew array<hsize_t>(arank);
					pin_ptr<hsize_t> adims_ptr = &member_array_dims[i][0];
					if (H5Tget_array_dims2(cmtype, adims_ptr) < 0) {
						throw gcnew HDF5Exception("H5Tget_array_dims2 failed!");
					}

					member_array_length[i] = 1;
					for (int d = 0; d < arank; ++d) {
						member_array_length[i] *= member_array_dims[i][d];
					}
				}

				switch(member_class[i])
				{
				case H5T_BITFIELD:
				case H5T_ENUM:
				case H5T_FLOAT:
				case H5T_INTEGER:
					{
						member_info[i] = ProviderUtils::BinaryReaderMethod(cmtype);
					}
					break;

				case H5T_STRING:

					break;

				case H5T_ARRAY:
					{
						base_type = H5Tget_super(cmtype);
						if (base_type < 0) {
							throw gcnew HDF5Exception("H5Tget_member_type failed!");
						}

						member_info[i] = ProviderUtils::BinaryReaderMethod(base_type);
						if (member_info[i] == nullptr) {
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


#pragma region create a buffer and read the dataset as a byte array

			array<unsigned char>^ buf = gcnew array<unsigned char>(safe_cast<size_t>(npoints*size));
			pin_ptr<unsigned char> buf_ptr = &buf[0];

			if (H5Dread(dset, mtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf_ptr) < 0) {
				throw gcnew HDF5Exception("H5Dread failed!");
			}

#pragma endregion

#pragma region create array of .NET objects and initialize its elements from the read buffer

			m_array = Array::CreateInstance(m_type, (array<long long>^) dims);

			// argument array for object constructor
			array<Object^>^ args = gcnew array<Object^>(mcount);

			// multidimensional index in the dataset
			array<long long>^ index = gcnew array<long long>(rank);

			MemoryStream^ ms = gcnew MemoryStream(buf);
			ASCIIEncoding^ ascii = gcnew ASCIIEncoding();
			BinaryReader^ reader = gcnew BinaryReader(ms, ascii);

			for (long long i = 0; i < npoints; ++i)
			{
				for (int m = 0; m < mcount; ++m)
				{
					switch (member_class[m])
					{
					case H5T_BITFIELD:
					case H5T_ENUM:
					case H5T_FLOAT:
					case H5T_INTEGER:
						{
							if (member_info[m] != nullptr) {
								args[m] = member_info[m]->Invoke(reader, nullptr);
							}
							else {
								throw gcnew PSH5XException("How did we get here???");
							}
						}
						break;

					case H5T_STRING:
						{
#pragma region string

							if (member_is_vlen_str[m])
							{
								if (IntPtr::Size == 8) {
									args[m] = Marshal::PtrToStringAnsi(IntPtr(reader->ReadInt64()));
								}
								else {
									args[m] = Marshal::PtrToStringAnsi(IntPtr(reader->ReadInt32()));
								}
							}
							else {
								args[m] = Encoding::ASCII->GetString(reader->ReadBytes(member_strlen[m]));
							}

#pragma endregion
						}
						break;

					case H5T_ARRAY:
						{
#pragma region array

							cmtype = H5Tget_member_type(mtype, safe_cast<unsigned>(m));
							if (cmtype < 0) {
								throw gcnew HDF5Exception("H5Tget_member_type failed!");
							}
							base_type = H5Tget_super(cmtype);
							if (base_type < 0) {
								throw gcnew HDF5Exception("H5Tget_member_type failed!");
							}

							args[m] = ProviderUtils::GetArray(reader, member_array_dims[m], base_type);

							H5Tclose(base_type);
							base_type = -1;
							H5Tclose(cmtype);
							cmtype = -1;

#pragma endregion
						}
						break;

					default:

						throw gcnew PSH5XException("Unsupported member in compound type!");
						break;
					}
				}

				if (rank > 1) {
					index = ArrayUtils::GetIndex((array<long long>^)dims, i);
					m_array->SetValue(cinfo[1]->Invoke(args), index);
				}
				else {
					m_array->SetValue(cinfo[1]->Invoke(args), i);
				}
			}

			m_ienum = m_array->GetEnumerator();
			m_ienum->MoveNext();

#pragma endregion

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
        }
    }

    IList^ CompoundDatasetReader::Read(long long readCount)
    {
        Array^ result = nullptr;

        long long remaining = m_array->LongLength - m_position;
        if (remaining > 0)
        {
            long long length = 0;

            if (readCount > remaining) {
                length = remaining;
            }
            else
            {
                if (readCount > 0) {
                    length = readCount;
                }
                else {
                    // return the full array w/o copying
                    m_position = m_array->LongLength;
                    return m_array;
                }
            }

            result = Array::CreateInstance(m_type, length);

            for (long long i = 0; i < length; ++i) {
                result->SetValue(m_ienum->Current, i);
                m_ienum->MoveNext();
            }

            m_position += length;
        }

        return result;
    }
}