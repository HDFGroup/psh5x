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

        DatasetReaderT(hid_t dset, hid_t ftype, hid_t fspace)
            : m_array(nullptr), m_ienum(nullptr), m_position(0)
        {
            hid_t ntype = -1;

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

                Type^ t = ProviderUtils::H5Type2DotNet(ntype);

                if (t != nullptr)
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

                        H5Array<T>^ h5a = gcnew H5Array<T>(dims);
                        m_array = h5a->GetArray();
                        pin_ptr<T> ptr = h5a->GetHandle();

                        if (H5Dread(dset, ntype, H5S_ALL, H5S_ALL, H5P_DEFAULT, ptr) < 0) {
                            throw gcnew HDF5Exception("H5Dread failed!");
                        }

                        m_ienum = m_array->GetEnumerator();
                        m_ienum->MoveNext();
                    }
                    else {
                        m_array = nullptr;
                    }
                }
                else {
                    throw gcnew HDF5Exception("Unsupported HDF5 atomic type!");
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
        DatasetReaderT(hid_t h5file, System::String^ h5path,
            array<hsize_t>^ start, array<hsize_t>^ stride,
            array<hsize_t>^ count, array<hsize_t>^ block);
        
        DatasetReaderT(hid_t h5file, System::String^ h5path, array<hsize_t>^ coord);
        */

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