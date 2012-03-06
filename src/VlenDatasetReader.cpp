
#include "ArrayUtils.h"
#include "HDF5Exception.h"
#include "ProviderUtils.h"
#include "PSH5XException.h"
#include "VlenDatasetReader.h"

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
    VlenDatasetReader::VlenDatasetReader(hid_t dset, hid_t ftype, hid_t fspace)
        : m_array(nullptr), m_position(0)
    {
        hvl_t* rdata = NULL;

        hid_t mtype = -1, base_type = -1;

        hssize_t npoints = -1;

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
                    rdata = new hvl_t [npoints];

					mtype = H5Tget_native_type(ftype, H5T_DIR_ASCEND);
                    if (mtype < 0) {
                        throw gcnew HDF5Exception("H5Tget_native_type failed!");
                    }

                    if(H5Dread(dset, mtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, rdata) < 0) {
                        throw gcnew HDF5Exception("H5Dread failed!");
                    }

                    array<long long>^ index = gcnew array<long long>(rank);
                    for (long long i = 0; i < npoints; ++i)
                    {
                        index = ArrayUtils::GetIndex((array<long long>^)dims, i);
                        m_array->SetValue(ProviderUtils::GetArray(rdata[i].p, rdata[i].len, base_type), index);
                    }

                    if (H5Dvlen_reclaim(mtype, fspace, H5P_DEFAULT, rdata) < 0) {
                        throw gcnew HDF5Exception("H5Dvlen_reclaim failed!");
                    }

                    m_ienum = m_array->GetEnumerator();
                    m_ienum->MoveNext();
                }
                else
                {
                    throw gcnew PSH5XException("Unsupported base type in array type!");
                }
            }
        }
        finally
        {
            if (rdata != NULL) {
                delete [] rdata;
            }
            if (base_type >= 0) {
                H5Tclose(base_type);
            }
            if (mtype >= 0) {
                H5Tclose(mtype);
            }
        }
    }

    IList^ VlenDatasetReader::Read(long long readCount)
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