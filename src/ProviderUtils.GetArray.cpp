
#include "HDF5Exception.h"
#include "ProviderUtils.h"
#include "PSH5XException.h"

extern "C" {
#include "H5Tpublic.h"
}

using namespace System;
using namespace System::CodeDom::Compiler;
using namespace System::Collections;
using namespace System::Management::Automation;
using namespace System::Reflection;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
    Array^ ProviderUtils::GetArray(void* buffer, size_t nelems, hid_t type_id)
    {
        Array^ result = nullptr;

        unsigned char* ptr = (unsigned char*) buffer;

        hid_t ntype = -1;

        htri_t is_vlen = -1;

        try
        {
            size_t size = H5Tget_size(type_id);

            array<unsigned char>^ tmp = gcnew array<unsigned char>(size);

            H5T_class_t cls = H5Tget_class(type_id);

            switch (cls)
            {
            case H5T_INTEGER:

#pragma region HDF5 integer

                ntype = H5Tget_native_type(type_id, H5T_DIR_ASCEND);
                if (ntype < 0) {
                    throw gcnew HDF5Exception("H5Tget_native_type failed!");
                }

                if (H5Tequal(ntype, H5T_NATIVE_CHAR) > 0)
                {
                    array<char>^ a = gcnew array<char>(nelems);
                    Marshal::Copy(IntPtr(ptr), tmp, 0, nelems);
                    result = a;
                }
                else if (H5Tequal(ntype, H5T_NATIVE_SHORT) > 0)
                {
                    array<short>^ a = gcnew array<short>(nelems);
                    for (size_t i = 0; i < nelems; ++i)
                    {
                        Marshal::Copy(IntPtr(ptr+i*size), tmp, 0, size);
                        a[i] = BitConverter::ToInt16(tmp, 0);
                    }
                    result = a;
                }
                else if (H5Tequal(ntype, H5T_NATIVE_INT) > 0)
                {
                    array<int>^ a = gcnew array<int>(nelems);
                    for (size_t i = 0; i < nelems; ++i)
                    {
                        Marshal::Copy(IntPtr(ptr+i*size), tmp, 0, size);
                        a[i] = BitConverter::ToInt32(tmp, 0);
                    }
                    result = a;
                }
                else if (H5Tequal(ntype, H5T_NATIVE_LONG) > 0)
                {
                    array<int>^ a = gcnew array<int>(nelems);
                    for (size_t i = 0; i < nelems; ++i)
                    {
                        Marshal::Copy(IntPtr(ptr+i*size), tmp, 0, size);
                        a[i] = BitConverter::ToInt32(tmp, 0);
                    }
                    result = a;
                }
                else if (H5Tequal(ntype, H5T_NATIVE_LLONG) > 0)
                {
                    array<long long>^ a = gcnew array<long long>(nelems);
                    for (size_t i = 0; i < nelems; ++i)
                    {
                        Marshal::Copy(IntPtr(ptr+i*size), tmp, 0, size);
                        a[i] = BitConverter::ToInt64(tmp, 0);
                    }
                    result = a;
                }
                else if (H5Tequal(ntype, H5T_NATIVE_UCHAR) > 0)
                {
                    array<unsigned char>^ a = gcnew array<unsigned char>(nelems);
                    Marshal::Copy(IntPtr(ptr), tmp, 0, nelems);
                    result = a;
                }
                else if (H5Tequal(ntype, H5T_NATIVE_USHORT) > 0)
                {
                    array<unsigned short>^ a = gcnew array<unsigned short>(nelems);
                    for (size_t i = 0; i < nelems; ++i)
                    {
                        Marshal::Copy(IntPtr(ptr+i*size), tmp, 0, size);
                        a[i] = BitConverter::ToUInt16(tmp, 0);
                    }
                    result = a;
                }
                else if (H5Tequal(ntype, H5T_NATIVE_UINT) > 0)
                {
                    array<unsigned int>^ a = gcnew array<unsigned int>(nelems);
                    for (size_t i = 0; i < nelems; ++i)
                    {
                        Marshal::Copy(IntPtr(ptr+i*size), tmp, 0, size);
                        a[i] = BitConverter::ToUInt32(tmp, 0);
                    }
                    result = a;
                }
                else if (H5Tequal(ntype, H5T_NATIVE_ULONG) > 0)
                {
                    array<unsigned int>^ a = gcnew array<unsigned int>(nelems);
                    for (size_t i = 0; i < nelems; ++i)
                    {
                        Marshal::Copy(IntPtr(ptr+i*size), tmp, 0, size);
                        a[i] = BitConverter::ToUInt32(tmp, 0);
                    }
                    result = a;
                }
                else if (H5Tequal(ntype, H5T_NATIVE_ULLONG) > 0)
                {
                    array<unsigned long long>^ a = gcnew array<unsigned long long>(nelems);
                    for (size_t i = 0; i < nelems; ++i)
                    {
                        Marshal::Copy(IntPtr(ptr+i*size), tmp, 0, size);
                        a[i] = BitConverter::ToUInt64(tmp, 0);
                    }
                    result = a;
                }
                else {
                    throw gcnew PSH5XException("Unknown integer type!");
                }

#pragma endregion

                break;

            case H5T_FLOAT:

#pragma region HDF5 float

                ntype = H5Tget_native_type(type_id, H5T_DIR_ASCEND);
                if (ntype < 0) {
                    throw gcnew HDF5Exception("H5Tget_native_type failed!");
                }

                if (H5Tequal(ntype, H5T_NATIVE_FLOAT) > 0)
                {
                    array<float>^ a = gcnew array<float>(nelems);
                    for (size_t i = 0; i < nelems; ++i)
                    {
                        Marshal::Copy(IntPtr(ptr+i*size), tmp, 0, size);
                        a[i] = BitConverter::ToSingle(tmp, 0);
                    }
                    result = a;
                }
                else if (H5Tequal(ntype, H5T_NATIVE_DOUBLE) > 0)
                {
                    array<double>^ a = gcnew array<double>(nelems);
                    for (size_t i = 0; i < nelems; ++i)
                    {
                        Marshal::Copy(IntPtr(ptr+i*size), tmp, 0, size);
                        a[i] = BitConverter::ToDouble(tmp, 0);
                    }
                    result = a;
                }
                else if (H5Tequal(ntype, H5T_NATIVE_LDOUBLE) > 0)
                {
                    array<double>^ a = gcnew array<double>(nelems);
                    for (size_t i = 0; i < nelems; ++i)
                    {
                        Marshal::Copy(IntPtr(ptr+i*size), tmp, 0, size);
                        a[i] = BitConverter::ToDouble(tmp, 0);
                    }
                    result = a;
                }
                else {
                    throw gcnew PSH5XException("Unknown float type!");
                }

#pragma endregion

                break;


            case H5T_STRING:

#pragma region HDF5 string

                if (true)
                {
                    array<String^>^ a = gcnew array<String^>(nelems);
                    for (size_t i = 0; i < nelems; ++i)
                    {
                        a[i] = Marshal::PtrToStringAnsi(IntPtr(ptr[i]));
                    }
                    result = a;
                }

#pragma endregion

                break;

            case H5T_BITFIELD:

#pragma region HDF5 bitfield

                ntype = H5Tget_native_type(type_id, H5T_DIR_DESCEND);
                if (ntype < 0) {
                    throw gcnew HDF5Exception("H5Tget_native_type failed!");
                }

                if (H5Tequal(ntype, H5T_NATIVE_B8) > 0)
                {
                    array<unsigned char>^ a = gcnew array<unsigned char>(nelems);
                    Marshal::Copy(IntPtr(ptr), tmp, 0, nelems);
                    result = a;
                }
                else if (H5Tequal(ntype, H5T_NATIVE_B16) > 0)
                {
                    array<unsigned short>^ a = gcnew array<unsigned short>(nelems);
                    for (size_t i = 0; i < nelems; ++i)
                    {
                        Marshal::Copy(IntPtr(ptr+i*size), tmp, 0, size);
                        a[i] = BitConverter::ToUInt16(tmp, 0);
                    }
                    result = a;
                }
                else if (H5Tequal(ntype, H5T_NATIVE_B32) > 0)
                {
                    array<unsigned int>^ a = gcnew array<unsigned int>(nelems);
                    for (size_t i = 0; i < nelems; ++i)
                    {
                        Marshal::Copy(IntPtr(ptr+i*size), tmp, 0, size);
                        a[i] = BitConverter::ToUInt32(tmp, 0);
                    }
                    result = a;
                }
                else if (H5Tequal(ntype, H5T_NATIVE_B64) > 0)
                {
                    array<unsigned long long>^ a = gcnew array<unsigned long long>(nelems);
                    for (size_t i = 0; i < nelems; ++i)
                    {
                        Marshal::Copy(IntPtr(ptr+i*size), tmp, 0, size);
                        a[i] = BitConverter::ToUInt64(tmp, 0);
                    }
                    result = a;
                }
                else
                {
                    throw gcnew PSH5XException("Unknown bitfield type!");
                }

#pragma endregion

                break;

            default:

                throw gcnew PSH5XException("Unsupported datatype class!");
                break;
            }


        }
        finally
        {
            if (ntype >= 0) {
                H5Tclose(ntype);
            }
        }

        return result;
    }

    Type^ ProviderUtils::GetArrayType(hid_t type_id)
    {
        Type^ result = nullptr;

        hid_t ntype = -1;

        htri_t is_vlen = -1;

        try
        {
            size_t size = H5Tget_size(type_id);

            H5T_class_t cls = H5Tget_class(type_id);

            switch (cls)
            {
            case H5T_INTEGER:

#pragma region HDF5 integer

                ntype = H5Tget_native_type(type_id, H5T_DIR_ASCEND);
                if (ntype < 0) {
                    throw gcnew HDF5Exception("H5Tget_native_type failed!");
                }

                if (H5Tequal(ntype, H5T_NATIVE_CHAR) > 0) {
                    result = array<char>::typeid;
                }
                else if (H5Tequal(ntype, H5T_NATIVE_SHORT) > 0) {
                    result = array<short>::typeid;
                }
                else if (H5Tequal(ntype, H5T_NATIVE_INT) > 0) {
                    result = array<int>::typeid;
                }
                else if (H5Tequal(ntype, H5T_NATIVE_LONG) > 0) {
                    result = array<int>::typeid;
                }
                else if (H5Tequal(ntype, H5T_NATIVE_LLONG) > 0) {
                    result = array<long long>::typeid;
                }
                else if (H5Tequal(ntype, H5T_NATIVE_UCHAR) > 0) {
                    result = array<unsigned char>::typeid;
                }
                else if (H5Tequal(ntype, H5T_NATIVE_USHORT) > 0) {
                    result = array<unsigned short>::typeid;
                }
                else if (H5Tequal(ntype, H5T_NATIVE_UINT) > 0) {
                    result = array<unsigned int>::typeid;
                }
                else if (H5Tequal(ntype, H5T_NATIVE_ULONG) > 0) {
                    result = array<unsigned int>::typeid;
                }
                else if (H5Tequal(ntype, H5T_NATIVE_ULLONG) > 0) {
                    result = array<unsigned long long>::typeid;
                }
                else {
                    throw gcnew PSH5XException("Unknown integer type!");
                }

#pragma endregion

                break;

            case H5T_FLOAT:

#pragma region HDF5 float

                ntype = H5Tget_native_type(type_id, H5T_DIR_ASCEND);
                if (ntype < 0) {
                    throw gcnew HDF5Exception("H5Tget_native_type failed!");
                }

                if (H5Tequal(ntype, H5T_NATIVE_FLOAT) > 0) {
                    result = array<float>::typeid;
                }
                else if (H5Tequal(ntype, H5T_NATIVE_DOUBLE) > 0) {
                    result = array<double>::typeid;
                }
                else if (H5Tequal(ntype, H5T_NATIVE_LDOUBLE) > 0)
                {
                    result = array<double>::typeid;
                }
                else {
                    throw gcnew PSH5XException("Unknown float type!");
                }

#pragma endregion

                break;


            case H5T_STRING:

#pragma region HDF5 string

                if (true)
                {
                    result = array<String^>::typeid;
                }

#pragma endregion

                break;

            case H5T_BITFIELD:

#pragma region HDF5 bitfield

                ntype = H5Tget_native_type(type_id, H5T_DIR_DESCEND);
                if (ntype < 0) {
                    throw gcnew HDF5Exception("H5Tget_native_type failed!");
                }

                if (H5Tequal(ntype, H5T_NATIVE_B8) > 0) {
                    result = array<unsigned char>::typeid;
                }
                else if (H5Tequal(ntype, H5T_NATIVE_B16) > 0) {
                    result = array<unsigned short>::typeid;
                }
                else if (H5Tequal(ntype, H5T_NATIVE_B32) > 0) {
                    result = array<unsigned int>::typeid;
                }
                else if (H5Tequal(ntype, H5T_NATIVE_B64) > 0) {
                    result = array<unsigned long long>::typeid;
                }
                else
                {
                    throw gcnew PSH5XException("Unknown bitfield type!");
                }

#pragma endregion

                break;

            default:

                throw gcnew PSH5XException("Unsupported datatype class!");
                break;
            }


        }
        finally
        {
            if (ntype >= 0) {
                H5Tclose(ntype);
            }
        }

        return result;
    }
}