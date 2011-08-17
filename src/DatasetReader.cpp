
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
        Exception^ ex = nullptr;

        char* name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

#pragma region HDF5 handles

        hid_t dset = -1, ftype = -1, ntype = -1, fspace = -1, mspace = -1;

        dset = H5Dopen2(h5file, name, H5P_DEFAULT);
        if (dset < 0) {
            ex = gcnew ArgumentException("H5Dopen2 failed!");
            goto error;
        }

        ftype = H5Dget_type(dset);
        if (ftype < 0) {
            ex = gcnew ArgumentException("H5Dget_type failed!");
            goto error;
        }

        ntype = H5Tget_native_type(ftype, H5T_DIR_ASCEND);
        if (ntype < 0) {
            ex = gcnew ArgumentException("H5Tget_native_type failed!");
            goto error;
        }

        fspace = H5Dget_space(dset);
        if (fspace < 0) {
            ex = gcnew ArgumentException("H5Dget_space failed!");
            goto error;
        }

#pragma endregion

        size_t size = H5Tget_size(ntype);

        hssize_t npoints = H5Sget_simple_extent_npoints(fspace);
        if (npoints > 0)
        {
            hsize_t dims[1];
            dims[0] = static_cast<hsize_t>(npoints);
            mspace = H5Screate_simple(1, dims, NULL);

            array<unsigned char>^ mbuf = gcnew array<unsigned char>(npoints*size);

            pin_ptr<unsigned char> p = &mbuf[0];
            unsigned char* buf = p;

            if (H5Dread(dset, ntype, mspace, H5S_ALL, H5P_DEFAULT, buf) < 0) {
                ex = gcnew ArgumentException("H5Dread failed!");
                goto error;
            }

            m_array = gcnew array<PSObject^>(dims[0]);

            array<unsigned char>^ row = gcnew array<unsigned char>(size);

            if (ProviderUtils::IsH5CompoundType(ntype))
            {
#pragma region HDF5 compound type

                m_array = gcnew array<PSObject^>(dims[0]);

                int mcount = H5Tget_nmembers(ntype);
                array<String^>^ mname = gcnew array<String^>(mcount);
                array<int>^ msize = gcnew array<int>(mcount);
                array<int>^ moffset = gcnew array<int>(mcount);
                array<MethodInfo^>^ minfo = gcnew array<MethodInfo^>(mcount);

                Type^ magicType = System::BitConverter::typeid;

                for (int i = 0; i < mcount; ++i)
                {
                    moffset[i] = safe_cast<int>(H5Tget_member_offset(ntype, i));
                    char* name = H5Tget_member_name(ntype, safe_cast<unsigned>(i));
                    mname[i] = gcnew String(name);

                    hid_t mtype = H5Tget_member_type(ntype, safe_cast<unsigned>(i));
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

#pragma endregion
            }
            else if (ProviderUtils::IsH5ArrayType(ntype))
            {
#pragma region HDF5 array type

                m_array = gcnew array<PSObject^>(dims[0]);

                hid_t base_type = H5Tget_super(ntype);
                if (ProviderUtils::H5NativeType2DotNet(base_type) != nullptr)
                {
                    int rank = H5Tget_array_ndims(ntype);
                    hsize_t* dims = new hsize_t [rank];
                    rank = H5Tget_array_dims2(ntype, dims);


                    delete [] dims;
                }
                else {
                    throw gcnew ArgumentException("Unsupported base datatype for ARRAY!");
                }

#pragma endregion
            }
            else if (ProviderUtils::IsH5VlenType(ntype))
            {
#pragma region HDF5 variable length type

                m_array = gcnew array<PSObject^>(dims[0]);

                hid_t base_type = H5Tget_super(ntype);
                if (ProviderUtils::H5NativeType2DotNet(base_type) != nullptr)
                {
                }
                else {
                    throw gcnew ArgumentException("Unsupported base datatype for VLEN!");
                }

#pragma endregion
            }
            else // atomic type
            {
                ex = gcnew ArgumentException("Unsupported datatype!");
                goto error;
            }
        }
        else
        {
            m_array = gcnew array<PSObject^>(0);
        }

error:

        if (mspace >= 0) {
            H5Sclose(mspace);
        }
        if (fspace >= 0) {
            H5Sclose(fspace);
        }
        if (ntype >= 0) {
            H5Tclose(ntype);
        }
        if (ftype >= 0) {
            H5Tclose(ftype);
        }
        if (dset >= 0) {
            H5Dclose(dset);
        }

        if (name != NULL) {
            Marshal::FreeHGlobal(IntPtr(name));
        }

        if (ex != nullptr) {
            throw ex;
        }
    }

    /*
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
    */

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
