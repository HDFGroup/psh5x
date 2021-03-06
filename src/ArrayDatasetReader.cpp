
#include "ArrayDatasetReader.h"
#include "ArrayUtils.h"
#include "HDF5Exception.h"
#include "ProviderUtils.h"
#include "PSH5XException.h"

extern "C" {
#include "H5Dpublic.h"
#include "H5Spublic.h"
#include "H5Tpublic.h"
}

#include <vector>

using namespace std;

using namespace System;
using namespace System::Collections;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Provider;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
    ArrayDatasetReader::ArrayDatasetReader(hid_t dset, hid_t ftype, hid_t fspace)
        : m_array(nullptr), m_position(0)
    {
        vector<unsigned char> rdata;

        hid_t ntype = -1, mtype = -1, base_type = -1;

        vector<hsize_t> mdims;

        try
        {
			hssize_t npoints = 1;
			if (H5Sget_simple_extent_type(fspace) == H5S_SIMPLE) { 
				npoints = H5Sget_simple_extent_npoints(fspace);
			}

            if (npoints > 0)
            {
                int rank = H5Sget_simple_extent_ndims(fspace);
                array<hsize_t>^ dims = gcnew array<hsize_t>(rank);
                pin_ptr<hsize_t> dims_ptr = &dims[0];
                rank = H5Sget_simple_extent_dims(fspace, dims_ptr, NULL);

                m_array = gcnew array<Array^>(safe_cast<int>(npoints));

                rank = H5Tget_array_ndims(ftype);
                if (rank < 0) {
                    throw gcnew HDF5Exception("H5Tget_array_ndims failed!");
                }

                mdims = vector<hsize_t>(rank);
                if (H5Tget_array_dims2(ftype, &mdims[0]) < 0) {
                    throw gcnew HDF5Exception("H5Tget_array_dims2 failed!");
                }
                hsize_t count = 1;
                for (int i = 0; i < rank; ++i)
                {
                    count *= mdims[i];
                }

                base_type = H5Tget_super(ftype);
                if (base_type < 0) {
                    throw gcnew HDF5Exception("H5Tget_super failed!");
                }

                Type^ t = ProviderUtils::H5Type2DotNet(base_type);
                if (t != nullptr)
                {
                    m_type = ProviderUtils::GetArrayType(base_type);
                    m_array = Array::CreateInstance(m_type, (array<long long>^) dims);

                    size_t size = H5Tget_size(base_type);

                    if (H5Tget_class(base_type) == H5T_BITFIELD) {
                        ntype = H5Tget_native_type(base_type, H5T_DIR_DESCEND);
                    }
                    else {
                        ntype = H5Tget_native_type(base_type, H5T_DIR_ASCEND);
                    }

                    mtype = H5Tarray_create(ntype, rank, &mdims[0]);
                    if (mtype < 0) {
                        throw gcnew HDF5Exception("H5Tarray_create failed!");
                    }

                    rdata = vector<unsigned char>(npoints*count*size);

                    if(H5Dread(dset, mtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, &rdata[0]) < 0) {
                        throw gcnew HDF5Exception("H5Dread failed!");
                    }

                    array<long long>^ index = gcnew array<long long>(rank);
                    for (size_t i = 0; i < safe_cast<size_t>(npoints); ++i)
                    {
                        void* buffer = (void*) (&rdata[0]+i*count*size);
                        index = ArrayUtils::GetIndex((array<long long>^)dims, i);
                        m_array->SetValue(ProviderUtils::GetArray(buffer, count, base_type), index);
                    }

                    m_ienum = m_array->GetEnumerator();
                    m_ienum->MoveNext();
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
            if (base_type >= 0) {
                H5Tclose(base_type);
            }
            if (ntype >= 0) {
                H5Tclose(ntype);
            }
            if (mtype >= 0) {
                H5Tclose(mtype);
            }
        }
    }

    IList^ ArrayDatasetReader::Read(long long readCount)
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
                else
                {
                    // return the full array w/o copying
                    m_position = m_array->LongLength;
                    return m_array;
                }
            }

            result = Array::CreateInstance(m_type, safe_cast<int>(length));

            for (long long i = 0; i < length; ++i) {
                result->SetValue(m_ienum->Current, i);
                m_ienum->MoveNext();
            }

            m_position += length;
        }

        return result;
    }
}