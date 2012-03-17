
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
    VlenDatasetReader::VlenDatasetReader(hid_t dset, hid_t ftype, hid_t fspace, hid_t mspace)
        : m_array(nullptr), m_position(0)
    {
        hvl_t* rdata = NULL;

        hid_t mtype = -1, base_type = -1;

        try
        {
			hssize_t npoints = 1;
			int rank = 1;

			H5S_class_t cls = H5Sget_simple_extent_type(fspace);
			array<hsize_t>^ dims = gcnew array<hsize_t>(1);
			dims[0] = 1;

			if (cls == H5S_SIMPLE)
			{ 
				rank = H5Sget_simple_extent_ndims(fspace);
				if (rank < 0) {
					throw gcnew HDF5Exception("H5Sget_simple_extent_ndims failed!");
				}
				dims = gcnew array<hsize_t>(rank);
				pin_ptr<hsize_t> dims_ptr = &dims[0];
				if (mspace == H5S_ALL)
				{
					rank = H5Sget_simple_extent_dims(fspace, dims_ptr, NULL);
					npoints = H5Sget_simple_extent_npoints(fspace);
				}
				else
				{
					rank = H5Sget_simple_extent_dims(mspace, dims_ptr, NULL);
					npoints = H5Sget_simple_extent_npoints(mspace);
				}
				if (rank < 0) {
					throw gcnew HDF5Exception("H5Sget_simple_extent_dims failed!");
				}
			}

			base_type = H5Tget_super(ftype);
			if (base_type < 0) {
				throw gcnew HDF5Exception("H5Tget_super failed!");
			}

			m_type = ProviderUtils::H5Type2DotNet(ftype);
			if (m_type != nullptr)
			{
				mtype = ProviderUtils::GetH5MemoryType(m_type, ftype);

				rdata = new hvl_t [npoints];

				if(H5Dread(dset, mtype, mspace, fspace, H5P_DEFAULT, rdata) < 0) {
					throw gcnew HDF5Exception("H5Dread failed!");
				}

				m_array = Array::CreateInstance(m_type, (array<long long>^) dims);

				if (rank > 1)
				{
					array<long long>^ index = gcnew array<long long>(rank);
					for (long long i = 0; i < npoints; ++i)
					{
						index = ArrayUtils::GetIndex((array<long long>^)dims, i);
						m_array->SetValue(ProviderUtils::GetArray(rdata[i].p, rdata[i].len, base_type), index);
					}
				}
				else
				{
					for (long long i = 0; i < npoints; ++i) {
						m_array->SetValue(ProviderUtils::GetArray(rdata[i].p, rdata[i].len, base_type), i);
					}
				}

				if (mspace == H5S_ALL) {
					if (H5Dvlen_reclaim(mtype, fspace, H5P_DEFAULT, rdata) < 0) {
						throw gcnew HDF5Exception("H5Dvlen_reclaim failed!");
					}
				}
				else {
					if (H5Dvlen_reclaim(mtype, mspace, H5P_DEFAULT, rdata) < 0) {
						throw gcnew HDF5Exception("H5Dvlen_reclaim failed!");
					}
				}

				m_ienum = m_array->GetEnumerator();
				m_ienum->MoveNext();
			}
			else {
				throw gcnew PSH5XException("Unsupported base type in array type!");
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