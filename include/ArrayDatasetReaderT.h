#pragma once

#include "ArrayUtils.h"
#include "H5ArrayT.h"
#include "HDF5Exception.h"
#include "PSH5XException.h"

extern "C" {
#include "H5Dpublic.h"
#include "H5Spublic.h"
#include "H5Tpublic.h"
}

#include <vector>

namespace PSH5X
{
    template <typename T>
    public ref class ArrayDatasetReaderT
        : System::Management::Automation::Provider::IContentReader
    {
    public:

        ArrayDatasetReaderT(hid_t dset, hid_t ftype, hid_t fspace, hid_t mspace)
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

				// get the array dimensions of the data elements

				int arank = H5Tget_array_ndims(ftype);
				if (arank < 0) {
					throw gcnew HDF5Exception("H5Tget_array_ndims failed!");
				}

				adims = gcnew array<hsize_t>(arank);
				pin_ptr<hsize_t> adims_ptr = &adims[0];

				if (H5Tget_array_dims2(ftype, adims_ptr) < 0) {
					throw gcnew HDF5Exception("H5Tget_array_dims2 failed!");
				}
				hsize_t arrayLength = 1;
				for (int i = 0; i < arank; ++i) {
					arrayLength *= adims[i];
				}

				array<T>^ rdata = gcnew array<T>(npoints*arrayLength);
				pin_ptr<T> rdata_ptr = &rdata[0];

				if (H5Dread(dset, mtype, mspace, fspace, H5P_DEFAULT, rdata_ptr) < 0) {
					throw gcnew HDF5Exception("H5Dread failed!");
				}

				H5Array<T>^ dummy = gcnew H5Array<T>(adims);
				m_array = Array::CreateInstance(dummy->GetArray()->GetType(), (array<long long>^) dims);

				if (rank > 1)
				{
					array<long long>^ index = gcnew array<long long>(rank);
					for (long long i = 0; i < npoints; ++i)
					{
						H5Array<T>^ arr = gcnew H5Array<T>(adims);
						interior_ptr<T> arr_ptr = arr->GetHandle();
						for (hsize_t j = 0; j < arrayLength; ++j) {
							arr_ptr[j] = rdata[i*arrayLength+j];
						}
						index = ArrayUtils::GetIndex((array<long long>^)dims, i);
						m_array->SetValue(arr->GetArray(), index);
					}
				}
				else
				{
					for (long long i = 0; i < npoints; ++i)
					{
						H5Array<T>^ arr = gcnew H5Array<T>(adims);
						interior_ptr<T> arr_ptr = arr->GetHandle();
						for (hsize_t j = 0; j < arrayLength; ++j) {
							arr_ptr[j] = rdata[i*arrayLength+j];
						}
						m_array->SetValue(arr->GetArray(), i);
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

        ~ArrayDatasetReaderT() { this->!ArrayDatasetReaderT(); }

        !ArrayDatasetReaderT() {}

        virtual void Close() {}

        virtual System::Collections::IList^ Read(long long readCount)
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

        virtual void Seek(long long offset, System::IO::SeekOrigin origin)
        {
            throw gcnew PSH5XException("ArrayDatasetReaderT::Seek() not implemented!");
        }

    private:

        System::Array^ m_array;

        System::Collections::IEnumerator^ m_ienum;

        long long m_position;

    };

}