
#include "ArrayDatasetReader.h"
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
using namespace System::Management::Automation;
using namespace System::Management::Automation::Provider;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
    ArrayDatasetReader::ArrayDatasetReader(hid_t h5file, String^ h5path)
        : m_array(nullptr), m_position(0)
    {
        char* name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

        unsigned char* rdata = NULL;

        hid_t dset = -1, ftype = -1, ntype = -1, mtype = -1, base_type = -1, fspace = -1;

        hsize_t* dims = NULL;

        try
        {
            dset = H5Dopen2(h5file, name, H5P_DEFAULT);
            if (dset < 0) {
                throw gcnew HDF5Exception("H5Dopen2 failed!");
            }

            fspace = H5Dget_space(dset);
            if (fspace < 0) {
                throw gcnew HDF5Exception("H5Dget_space failed!");
            }

            hssize_t npoints = H5Sget_simple_extent_npoints(fspace);
            if (npoints > 0)
            {
                m_array = gcnew array<Array^>(npoints);

                ftype = H5Dget_type(dset);
                if (ftype < 0) {
                    throw gcnew HDF5Exception("H5Dget_type failed!");
                }

                int rank = H5Tget_array_ndims(ftype);
                if (rank < 0) {
                    throw gcnew HDF5Exception("H5Tget_array_ndims failed!");
                }

                dims = new hsize_t [rank];
                if (H5Tget_array_dims2(ftype, dims) < 0) {
                    throw gcnew HDF5Exception("H5Tget_array_dims2 failed!");
                }
                hsize_t count = 1;
                for (int i = 0; i < rank; ++i)
                {
                    count *= dims[i];
                }

                base_type = H5Tget_super(ftype);
                if (base_type < 0) {
                    throw gcnew HDF5Exception("H5Tget_super failed!");
                }

                Type^ t = ProviderUtils::H5Type2DotNet(base_type);
                if (t != nullptr)
                {
                    size_t size = H5Tget_size(base_type);
                    rdata = new unsigned char [npoints*count*size];

                    if (H5Tget_class(base_type) == H5T_BITFIELD) {
                        ntype = H5Tget_native_type(base_type, H5T_DIR_DESCEND);
                    }
                    else {
                        ntype = H5Tget_native_type(base_type, H5T_DIR_ASCEND);
                    }

                    mtype = H5Tarray_create(ntype, rank, dims);
                    if (mtype < 0) {
                        throw gcnew HDF5Exception("H5Tarray_create failed!");
                    }

                    if(H5Dread(dset, mtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, rdata) < 0) {
                        throw gcnew HDF5Exception("H5Dread failed!");
                    }

                    for (size_t i = 0; i < npoints; ++i)
                    {
                        void* buffer = (void*) (rdata+i*count*size);
                        m_array[i] = ProviderUtils::GetArray(buffer, count, base_type);
                    }
                }
                else
                {
                    throw gcnew PSH5XException("Unsupported base type in array type!");
                }
            }
            else
            {
                m_array = gcnew array<Array^>(0);
            }
        }
        finally
        {
            if (rdata != NULL) {
                delete [] rdata;
            }
            if (dims != NULL) {
                delete [] dims;
            }
            if (fspace >= 0) {
                H5Sclose(fspace);
            }
            if (ftype >= 0) {
                H5Tclose(ftype);
            }
            if (base_type >= 0) {
                H5Tclose(base_type);
            }
            if (ntype >= 0) {
                H5Tclose(ntype);
            }
            if (mtype >= 0) {
                H5Tclose(mtype);
            }
            if (dset >= 0) {
                H5Dclose(dset);
            }
            if (name != NULL) {
                Marshal::FreeHGlobal(IntPtr(name));
            }
        }
    }

    IList^ ArrayDatasetReader::Read(long long readCount)
    {
        array<Object^>^ result = nullptr;

        long long remaining = m_array->LongLength - m_position;
        if (remaining > 0)
        {
            long long length = 0;

            if (readCount > remaining)
            {
                length = remaining;
            }
            else
            {
                if (readCount > 0)
                {
                    length = readCount;
                }
                else
                {
                    length = m_array->LongLength;
                }
            }

            result = gcnew array<Object^>(length);

            Array::Copy((Array^) m_array, m_position, (Array^) result, (long long) 0, length);

            m_position += length;
        }

        return result;
    }
}