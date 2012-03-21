
#include "ArrayUtils.h"
#include "H5ArrayT.h"
#include "HDF5Exception.h"
#include "Providerutils.h"
#include "PSH5XException.h"
#include "OpaqueDatasetReader.h"


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
	OpaqueDatasetReader::OpaqueDatasetReader(hid_t dset, hid_t ftype, hid_t fspace, hid_t mspace)
		: m_array(nullptr), m_ienum(nullptr), m_position(0)
	{
		hid_t mtype = -1;

		array<hsize_t>^ adims = nullptr;

		try
		{
			mtype = H5Tget_native_type(ftype, H5T_DIR_ASCEND);
			if (mtype < 0) {
				throw gcnew HDF5Exception("H5Tget_native_type failed!");
			}

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

			// get the size of the opaque type

			size_t osize = H5Tget_size(ftype);
			if (osize == 0) {
				throw gcnew PSH5XException("Zero size opaque type found!");
			}

			array<unsigned char>^ rdata = gcnew array<unsigned char>(npoints*safe_cast<int>(osize));
			pin_ptr<unsigned char> rdata_ptr = &rdata[0];

			if (H5Dread(dset, mtype, mspace, fspace, H5P_DEFAULT, rdata_ptr) < 0) {
				throw gcnew HDF5Exception("H5Dread failed!");
			}

			array<unsigned char>^ dummy = gcnew array<unsigned char>(safe_cast<int>(osize));
			m_array = Array::CreateInstance(dummy->GetType(), (array<long long>^) dims);

			if (rank > 1)
			{
				array<long long>^ index = gcnew array<long long>(rank);
				for (long long i = 0; i < npoints; ++i)
				{
					array<unsigned char>^ arr = gcnew array<unsigned char>(safe_cast<int>(osize));
					interior_ptr<unsigned char> arr_ptr = &arr[0];
					for (hsize_t j = 0; j < osize; ++j) {
						arr_ptr[j] = rdata[i*osize+j];
					}
					index = ArrayUtils::GetIndex((array<long long>^)dims, i);
					m_array->SetValue(arr, index);
				}
			}
			else
			{
				for (long long i = 0; i < npoints; ++i)
				{
					array<unsigned char>^ arr = gcnew array<unsigned char>(safe_cast<int>(osize));
					interior_ptr<unsigned char> arr_ptr = &arr[0];
					for (hsize_t j = 0; j < osize; ++j) {
						arr_ptr[j] = rdata[i*osize+j];
					}
					m_array->SetValue(arr, i);
				}
			}

			m_ienum = m_array->GetEnumerator();
			m_ienum->MoveNext();
		}
		finally
		{
			if (mtype >= 0) {
				H5Tclose(mtype);
			}
		}
	}

	IList^ OpaqueDatasetReader::Read(long long readCount)
	{
		array<Array^>^ result = nullptr;

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

			result = gcnew array<Array^>(safe_cast<int>(length));

			// I have no idea how to efficiently copy a multidimensional array
			// into a onedimensional array

			for (long long i = 0; i < length; ++i) {
				result[i] = safe_cast<Array^>(m_ienum->Current);
				m_ienum->MoveNext();
			}

			m_position += length;
		}

		return result;
	}
}