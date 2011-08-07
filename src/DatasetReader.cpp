
#include "DatasetReader.h"
#include "Providerutils.h"

extern "C" {
#include "H5Dpublic.h"
#include "H5Spublic.h"
#include "H5Tpublic.h"
}

#include <cstdlib>
#include <cstring>

using namespace System;
using namespace System::Collections;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Provider;
using namespace System::Reflection;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
    DatasetReader::DatasetReader(hid_t h5file, String^ h5path)
        : m_array(nullptr), m_position(0)
    {
        if (ProviderUtils::IsH5Dataset(h5file, h5path))
        {
            char* name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();
            hid_t dset = H5Dopen2(h5file, name, H5P_DEFAULT);
            if (dset < 0) {
                throw gcnew ArgumentException("H5Dopen2 failed!");
            }

            hid_t file_type = H5Dget_type(dset);
            if (file_type < 0) {
                throw gcnew ArgumentException("H5Dget_type failed!");
            }
            hid_t mem_type = H5Tget_native_type(file_type, H5T_DIR_ASCEND);
            if (mem_type < 0) {
                throw gcnew ArgumentException("H5Tget_native_type failed!");
            }

            Hashtable^ type = ProviderUtils::ParseH5Type(mem_type);
            bool isCompound = false, isArray = false, isVlen = false;
            if (((String^)type["Class"]) == "COMPOUND") {
                isCompound = true;
            }
            else if (((String^)type["Class"]) == "ARRAY") {
                isArray = true;
            }
            else if (((String^)type["Class"]) == "VLEN") {
                isVlen = true;
            }

            size_t size = H5Tget_size(mem_type);

            hid_t file_space = H5Dget_space(dset);
            if (file_space < 0) {
                throw gcnew ArgumentException("H5Dget_space failed!");
            }

            hssize_t npoints = H5Sget_simple_extent_npoints(file_space);
            if (npoints > 0)
            {
                hsize_t dims[1];
                dims[0] = static_cast<hsize_t>(npoints);
                hid_t mem_space = H5Screate_simple(1, dims, NULL);

                array<unsigned char>^ mbuf = gcnew array<unsigned char>(npoints*size);

                pin_ptr<unsigned char> p = &mbuf[0];
                unsigned char* buf = p;

                if (H5Dread(dset, mem_type, mem_space, H5S_ALL, H5P_DEFAULT, buf) < 0)
                {
                    throw gcnew ArgumentException("H5Dread failed!");
                }

                m_array = gcnew array<PSObject^>(dims[0]);

                array<unsigned char>^ row = gcnew array<unsigned char>(size);

                if (isCompound)
                {   
                    int mcount = H5Tget_nmembers(mem_type);
                    array<String^>^ mname = gcnew array<String^>(mcount);
                    array<int>^ msize = gcnew array<int>(mcount);
                    array<int>^ moffset = gcnew array<int>(mcount);
                    array<MethodInfo^>^ minfo = gcnew array<MethodInfo^>(mcount);

                    Type^ magicType = System::BitConverter::typeid;

                    for (int i = 0; i < mcount; ++i)
                    {
                        moffset[i] = safe_cast<int>(H5Tget_member_offset(mem_type, i));
                        char* name = H5Tget_member_name(mem_type, safe_cast<unsigned>(i));
                        mname[i] = gcnew String(name);
                        hid_t mtype = H5Tget_member_type(mem_type, safe_cast<unsigned>(i));
                        msize[i] = safe_cast<int>(H5Tget_size(mtype));
                        minfo[i] = ProviderUtils::BitConverterMethod(mtype);
                        H5Tclose(mtype);
                    }

                    for (long long i = 0; i < m_array->LongLength; ++i)
                    {
                        Array::Copy(mbuf, size*i, row, 0, size);

                        m_array[i] = gcnew PSObject();

                        for (int m = 0; m < mcount; ++m)
                        {
                            if (minfo[m] != nullptr) {
                                m_array[i]->Properties->Add(
                                    gcnew PSNoteProperty(mname[m],
                                        minfo[m]->Invoke(nullptr,
                                            gcnew array<Object^>{row, moffset[m]})));
                            }
                            else {
                                m_array[i]->Properties->Add(
                                    gcnew PSNoteProperty(mname[m],
                                        BitConverter::ToString(row, moffset[m],
                                            msize[m])));
                            }
                        }
                    }
                }
                else if (isArray)
                {
                    hid_t base_type = H5Tget_super(mem_type);
                    if (ProviderUtils::H5NativeType2DotNet(base_type) != nullptr)
                    {
                        int rank = H5Tget_array_ndims(mem_type);
                        hsize_t* dims = new hsize_t [rank];
                        rank = H5Tget_array_dims2(mem_type, dims);


                        delete [] dims;
                    }
                    else {
                        throw gcnew ArgumentException("Unsupported base datatype for ARRAY!");
                    }
                }
                else if (isVlen)
                {
                    hid_t base_type = H5Tget_super(mem_type);
                    if (ProviderUtils::H5NativeType2DotNet(base_type) != nullptr)
                    {
                    }
                    else {
                        throw gcnew ArgumentException("Unsupported base datatype for VLEN!");
                    }
                }
                else // atomic type
                {
                    // TODO: parse out types
                    if (ProviderUtils::H5NativeType2DotNet(mem_type) != nullptr)
                    {
                    }
                    else {
                        throw gcnew ArgumentException("Unsupported datatype!");
                    }

                    for (long long i = 0; i < m_array->LongLength; ++i)
                    {    
                        Array::Copy(mbuf, size*i, row, 0, size);
                        m_array[i] = gcnew PSObject(BitConverter::ToString(row, 0, size));
                    }
                }
            }
            else
            {
                m_array = gcnew array<PSObject^>(0);
            }

            if (H5Sclose(file_space) < 0) { // TODO
            }
            if (H5Tclose(mem_type) < 0) { // TODO
            }
            if (H5Tclose(file_type) < 0) { // TODO
            }
            if (H5Dclose(dset) < 0) { // TODO
            }
        }
        else {
            throw gcnew ArgumentException(
                String::Format("'{0}' is not an HDF5 dataset.", h5path));
        }
    }

    DatasetReader::DatasetReader(hid_t h5file, String^ h5path,
        array<hsize_t>^ start, array<hsize_t>^ stride,
        array<hsize_t>^ count, array<hsize_t>^ block)
        : m_array(nullptr), m_position(0)
    {
    }

    DatasetReader::DatasetReader(hid_t h5file, String^ h5path, array<hsize_t>^ coord)
        : m_array(nullptr), m_position(0)
    {
    }


    IList^ DatasetReader::Read(long long readCount)
    {
        array<PSObject^>^ result = nullptr;

        long long remaining = m_array->LongLength - m_position;
        if (remaining > 0)
        {
            long long length = 0;
            if (readCount > remaining) { length = remaining; }
            else { length = readCount; }

            result = gcnew array<PSObject^>(length);
    
            long long pos = m_position;
            for (long long i = 0; i < length; ++i)
            {
                result[i] = m_array[pos++];
            }

            m_position += length;
        }

        return result;
    }
}
