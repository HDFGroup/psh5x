#pragma once

#include "H5ArrayT.h"
#include "HDF5Exception.h"
#include "PSH5XException.h"

extern "C" {
#include "H5Dpublic.h"
#include "H5Spublic.h"
#include "H5Tpublic.h"
}

namespace PSH5X
{
    template <typename T>
    public ref class DatasetReaderT
        : System::Management::Automation::Provider::IContentReader
    {
    public:

        DatasetReaderT(hid_t dset, hid_t ftype, hid_t fspace, hid_t mspace)
            : m_array(nullptr), m_ienum(nullptr), m_position(0)
        {
            hid_t mtype = -1; //, mspace = H5S_ALL;

            try
			{
				if (H5Tget_class(ftype) == H5T_BITFIELD) {
					mtype = H5Tget_native_type(ftype, H5T_DIR_DESCEND);
				}
				else {
					mtype = H5Tget_native_type(ftype, H5T_DIR_ASCEND);
				}
				if (mtype < 0) {
					throw gcnew HDF5Exception("H5Tget_native_type failed!");
				}

				hssize_t npoints = 1;

				H5S_class_t cls = H5Sget_simple_extent_type(fspace);
				array<hsize_t>^ dims = gcnew array<hsize_t>(1);
				dims[0] = 1;
				
				if (cls == H5S_SIMPLE)
				{ 
					int rank = H5Sget_simple_extent_ndims(fspace);
					if (rank < 0) {
						throw gcnew HDF5Exception("H5Sget_simple_extent_ndims failed!");
					}
					dims = gcnew array<hsize_t>(rank);
					pin_ptr<hsize_t> dims_ptr = &dims[0];
					if (mspace == H5S_ALL) {
						rank = H5Sget_simple_extent_dims(fspace, dims_ptr, NULL);
					}
					else {
						rank = H5Sget_simple_extent_dims(mspace, dims_ptr, NULL);
					}
					if (rank < 0) {
						throw gcnew HDF5Exception("H5Sget_simple_extent_dims failed!");
					}
				}

				H5Array<T>^ h5a = gcnew H5Array<T>(dims);
				m_array = h5a->GetArray();
				pin_ptr<T> ptr = h5a->GetHandle();

				if (H5Dread(dset, mtype, mspace, fspace, H5P_DEFAULT, ptr) < 0) {
					throw gcnew HDF5Exception("H5Dread failed!");
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

        ~DatasetReaderT() { this->!DatasetReaderT(); }

        !DatasetReaderT() {}

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
            System::Console::WriteLine("DatasetReaderT::Seek()");
        }

    private:

        System::Array^ m_array;

        System::Collections::IEnumerator^ m_ienum;

        long long m_position;

    };

}