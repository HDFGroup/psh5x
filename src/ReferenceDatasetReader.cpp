
#include "ArrayUtils.h"
#include "H5ArrayT.h"
#include "HDF5Exception.h"
#include "Providerutils.h"
#include "PSH5XException.h"
#include "ReferenceDatasetReader.h"


extern "C" {
#include "H5Dpublic.h"
#include "H5Rpublic.h"
#include "H5Spublic.h"
#include "H5Tpublic.h"
}

using namespace System;
using namespace System::Collections;
using namespace System::Collections::Generic;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Provider;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
	ReferenceDatasetReader::ReferenceDatasetReader(hid_t dset, hid_t ftype, hid_t fspace, hid_t mspace)
		: m_array(nullptr), m_ienum(nullptr), m_position(0), m_isObjectReference(false)
	{
		hid_t mtype = -1, sel = -1;

		array<hsize_t>^ adims = nullptr;

		IntPtr name = IntPtr::Zero;

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

			// sort out what kind of reference we are dealing with

			bool isObjectReference = (H5Tequal(ftype, H5T_STD_REF_OBJ) > 0);
			m_isObjectReference = isObjectReference;
			bool isRegionReference = (H5Tequal(ftype, H5T_STD_REF_DSETREG) > 0);

			if (!(isObjectReference || isRegionReference) || (isObjectReference && isRegionReference)) {
				throw gcnew PSH5XException("Unknown reference type found!");
			}

			int refsize = safe_cast<int>(H5Tget_size(ftype));
			if (refsize == 0) {
				throw gcnew PSH5XException("Zero size refer type found!");
			}

			array<unsigned char>^ rdata = gcnew array<unsigned char>(npoints*refsize);
			pin_ptr<unsigned char> rdata_ptr = &rdata[0];

			if (H5Dread(dset, mtype, mspace, fspace, H5P_DEFAULT, rdata_ptr) < 0) {
				throw gcnew HDF5Exception("H5Dread failed!");
			}

			if (isObjectReference)
			{
				// path name
				m_array = Array::CreateInstance(String::typeid, (array<long long>^) dims);
			}
			else
			{
				// (path name, tuple array)
				KeyValuePair<String^,Array^> dummy(nullptr, nullptr);
			    m_array = Array::CreateInstance(dummy.GetType(), (array<long long>^) dims);
			}

			name = Marshal::AllocHGlobal(256);
			ssize_t ret_val;

			if (rank > 1)
			{
				array<long long>^ index = gcnew array<long long>(rank);

				if (isObjectReference)
				{
					for (long long i = 0; i < npoints; ++i)
					{
						pin_ptr<unsigned char> ptr = &rdata[i*refsize];
						index = ArrayUtils::GetIndex((array<long long>^)dims, i);
						m_array->SetValue(ProviderUtils::GetObjectReference(dset, ptr), index);
					}
				}
				else // region reference
				{
					for (long long i = 0; i < npoints; ++i)
					{
						pin_ptr<unsigned char> ptr = &rdata[i*refsize];
						index = ArrayUtils::GetIndex((array<long long>^)dims, i);
						m_array->SetValue(ProviderUtils::GetRegionReference(dset, ptr), index);
					}
				}
			}
			else
			{
				if (isObjectReference)
				{
					for (long long i = 0; i < npoints; ++i)
					{
						pin_ptr<unsigned char> ptr = &rdata[i*refsize];
						m_array->SetValue(ProviderUtils::GetObjectReference(dset, ptr), i);
					}
				}
				else // region reference
				{
					for (long long i = 0; i < npoints; ++i)
					{
						pin_ptr<unsigned char> ptr = &rdata[i*refsize];
						m_array->SetValue(ProviderUtils::GetRegionReference(dset, ptr), i);
					}
				}
			}

			m_ienum = m_array->GetEnumerator();
			m_ienum->MoveNext();
		}
		finally
		{
			if (name != IntPtr::Zero) {
				Marshal::FreeHGlobal(name);
			}
			if (sel != -1) {
				H5Sclose(sel);
			}
			if (mtype >= 0) {
				H5Tclose(mtype);
			}
		}
	}

	System::Collections::IList^ ReferenceDatasetReader::Read(long long readCount)
	{
		if (m_isObjectReference)
		{
			array<String^>^ result = nullptr;

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

				result = gcnew array<String^>(safe_cast<int>(length));

				for (long long i = 0; i < length; ++i)
				{
					result[i] = safe_cast<String^>(m_ienum->Current);
					m_ienum->MoveNext();
				}

				m_position += length;
			}

			return result;
		}
		else // region reference
		{
			array< KeyValuePair<String^,Array^> >^ result = nullptr;

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

				result = gcnew array< KeyValuePair<String^,Array^> >(safe_cast<int>(length));

				for (long long i = 0; i < length; ++i)
				{
					result[i] = safe_cast< KeyValuePair<String^,Array^> >(m_ienum->Current);
					m_ienum->MoveNext();
				}

				m_position += length;
			}

			return result;
		}
	}
}