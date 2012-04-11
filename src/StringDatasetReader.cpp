
#include "ArrayUtils.h"
#include "HDF5Exception.h"
#include "Providerutils.h"
#include "PSH5XException.h"
#include "StringDatasetReader.h"


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
    StringDatasetReader::StringDatasetReader(hid_t dset, hid_t ftype, hid_t fspace, hid_t mspace)
        : m_array(nullptr), m_position(0)
    {
        char** rdata = NULL;
        char** vrdata = NULL;

        hid_t mtype = -1;

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

			m_array = Array::CreateInstance(String::typeid, (array<long long>^) dims);

			htri_t is_vlen = H5Tis_variable_str(ftype);
			if (is_vlen > 0)
			{
				mtype = H5Tcreate(H5T_STRING, H5T_VARIABLE);
				if (mtype < 0) {
					throw gcnew HDF5Exception("H5Tcreate failed!");
				}
				if (H5Tset_cset(mtype, H5Tget_cset(ftype)) < 0) {
					throw gcnew HDF5Exception("H5Tset_cset failed!");
				}

				vrdata = new char* [npoints];

				if (H5Dread (dset, mtype, mspace, fspace, H5P_DEFAULT, vrdata) < 0) {
					throw gcnew HDF5Exception("H5Dread failed!");
				}

				if (rank > 1)
				{
					array<long long>^ index = gcnew array<long long>(rank);
					for (long long i = 0; i < npoints; ++i)
					{
						index = ArrayUtils::GetIndex((array<long long>^)dims, i);
						m_array->SetValue(Marshal::PtrToStringAnsi(IntPtr(vrdata[i])), index);
					}
				}
				else
				{
					for (long long i = 0; i < npoints; ++i)
					{
						m_array->SetValue(Marshal::PtrToStringAnsi(IntPtr(vrdata[i])), i);
					}
				}

				if (mspace == H5S_ALL) {
					if (H5Dvlen_reclaim(mtype, fspace, H5P_DEFAULT, vrdata) < 0) {
						throw gcnew HDF5Exception("H5Dvlen_reclaim failed!");
					}
				}
				else {
					if (H5Dvlen_reclaim(mtype, mspace, H5P_DEFAULT, vrdata) < 0) {
						throw gcnew HDF5Exception("H5Dvlen_reclaim failed!");
					}
				}
			}
			else if (is_vlen == 0)
			{
				size_t size = H5Tget_size(ftype);

				if (H5Tget_strpad(ftype) == H5T_STR_SPACEPAD) { // FORTRAN
					++size;
				}

				mtype = H5Tcreate(H5T_STRING, size);
				if (mtype < 0) {
					throw gcnew HDF5Exception("H5Tcreate failed!");
				}
				if (H5Tset_cset(mtype, H5Tget_cset(ftype)) < 0) {
					throw gcnew HDF5Exception("H5Tset_cset failed!");
				}

				rdata = new char* [npoints];
				rdata[0] = new char [npoints*size];
				for (hssize_t i = 1; i < npoints; ++i) {
					rdata[i] = rdata[0] + i*size;
				}

				if (H5Dread (dset, mtype, mspace, fspace, H5P_DEFAULT, rdata[0]) < 0) {
					throw gcnew HDF5Exception("H5Dread failed!");
				}

				if (rank > 1)
				{
					array<long long>^ index = gcnew array<long long>(rank);
					for (long long i = 0; i < npoints; ++i)
					{
						index = ArrayUtils::GetIndex((array<long long>^)dims, i);
						m_array->SetValue(Marshal::PtrToStringAnsi(IntPtr(rdata[i])), index);
					}
				}
				else {
					for (long long i = 0; i < npoints; ++i) {
						m_array->SetValue(Marshal::PtrToStringAnsi(IntPtr(rdata[i])), i);
					}
				}
			}
			else {
				throw gcnew HDF5Exception("H5Tis_variable_str failed!");
			}

			m_ienum = m_array->GetEnumerator();
			m_ienum->MoveNext();
        }
        finally
        {
            if (rdata != NULL) {
                delete [] rdata[0];
                delete [] rdata;
            }

            if (vrdata != NULL) {
                delete [] vrdata;
            }
			
			if (mtype >= 0) {
                H5Tclose(mtype);
            }
        }
    }

    IList^ StringDatasetReader::Read(long long readCount)
    {
        Array^ result = nullptr;

        long long remaining = m_array->LongLength - m_position;
        if (remaining > 0)
        {
            long long length = 0;

            if (readCount > remaining) {
                length = remaining;
            }
            else {
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

            result = Array::CreateInstance(String::typeid, safe_cast<int>(length));

            for (long long i = 0; i < length; ++i) {
                result->SetValue(m_ienum->Current, i);
                m_ienum->MoveNext();
            }

            m_position += length;
        }

        return result;
    }
}