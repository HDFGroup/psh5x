#pragma once

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

        ArrayDatasetReaderT(hid_t dset, hid_t ftype, hid_t fspace)
            : m_array(nullptr), m_ienum(nullptr), m_position(0)
        {
            hid_t ntype = -1;

			array<hsize_t>^ adims = nullptr;

            try
			{
				if (H5Tget_class(ftype) == H5T_BITFIELD) {
					ntype = H5Tget_native_type(ftype, H5T_DIR_DESCEND);
				}
				else {
					ntype = H5Tget_native_type(ftype, H5T_DIR_ASCEND);
				}
				if (ntype < 0) {
					throw gcnew HDF5Exception("H5Tget_native_type failed!");
				}

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

					H5Array<T>^ dummy = gcnew H5Array<T>(adims);

					array<T>^ rdata = gcnew array<T>(npoints*arrayLength);
					pin_ptr<T> rdata_ptr = &rdata[0];

					if (H5Dread(dset, ntype, H5S_ALL, H5S_ALL, H5P_DEFAULT, rdata_ptr) < 0) {
						throw gcnew HDF5Exception("H5Dread failed!");
					}

					m_array = Array::CreateInstance(dummy->GetType(), npoints);
					for (hssize_t i = 0; i < npoints; ++i) {
						// TODO: finish this

						m_array->SetValue(dummy, i);
					}

					m_ienum = m_array->GetEnumerator();
					m_ienum->MoveNext();
				}
			}
            finally
            {
                if (ntype >= 0) {
                    H5Tclose(ntype);
                }
            }

        }

        // TODO: implement hyperslabs and point sets

        /*
        ArrayDatasetReaderT(hid_t h5file, System::String^ h5path,
            array<hsize_t>^ start, array<hsize_t>^ stride,
            array<hsize_t>^ count, array<hsize_t>^ block);
        
        ArrayDatasetReaderT(hid_t h5file, System::String^ h5path, array<hsize_t>^ coord);
        */

        ~ArrayDatasetReaderT() { this->!ArrayDatasetReaderT(); }

        !ArrayDatasetReaderT() {}

        virtual void Close() {}

        virtual System::Collections::IList^ Read(long long readCount)
        {
            array<T>^ result = nullptr;

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

                result = gcnew array<T>(safe_cast<int>(length));

                // I have no idea how to efficiently copy a multidimensional array
                // into a onedimensional array

                for (long long i = 0; i < length; ++i) {
                    result[i] = safe_cast<T>(m_ienum->Current);
                    m_ienum->MoveNext();
                }

                m_position += length;
            }

            return result;
        }

        virtual void Seek(long long offset, System::IO::SeekOrigin origin)
        {
            System::Console::WriteLine("ArrayDatasetReaderT::Seek()");
        }

    private:

        System::Array^ m_array;

        System::Collections::IEnumerator^ m_ienum;

        long long m_position;

    };

}