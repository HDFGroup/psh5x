#pragma once

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

        DatasetReaderT(hid_t h5file, System::String^ h5path)
        {
            char* name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

            hid_t dset = -1, ftype = -1, ntype = -1, fspace = -1, mspace = -1;

            try
            {
                dset = H5Dopen2(h5file, name, H5P_DEFAULT);
                if (dset < 0) {
                    throw gcnew HDF5Exception("H5Dopen2 failed!");
                }

                ftype = H5Dget_type(dset);
                if (ftype < 0) {
                    throw gcnew HDF5Exception("H5Dget_type failed!");
                }

                if (H5Tget_class(ftype) == H5T_BITFIELD) {
                    ntype = H5Tget_native_type(ftype, H5T_DIR_DESCEND);
                }
                else {
                    ntype = H5Tget_native_type(ftype, H5T_DIR_ASCEND);
                }
                if (ntype < 0) {
                    throw gcnew HDF5Exception("H5Tget_native_type failed!");
                }

                fspace = H5Dget_space(dset);
                if (fspace < 0) {
                    throw gcnew HDF5Exception("H5Dget_space failed!");
                }

                Type^ t = ProviderUtils::H5NativeType2DotNet(ntype);

                // TODO: deal with strings

                if (t != nullptr)
                {
                    size_t size = H5Tget_size(ntype);

                    hssize_t npoints = H5Sget_simple_extent_npoints(fspace);
                    if (npoints > 0)
                    {
                        hsize_t dims[1];
                        dims[0] = static_cast<hsize_t>(npoints);

                        mspace = H5Screate_simple(1, dims, NULL);
                        if (mspace < 0) {
                            throw gcnew HDF5Exception("H5Screate_simple failed!");
                        }

                        m_array = gcnew array<T>(dims[0]);
                        pin_ptr<T> ptr = &m_array[0];

                        if (H5Dread(dset, ntype, mspace, H5S_ALL, H5P_DEFAULT, ptr) < 0) {
                            H5Sclose(mspace);
                            throw gcnew HDF5Exception("H5Dread failed!");
                        }
                    }
                    else
                    {
                        m_array = gcnew array<T>(0);
                    }
                }
                else {
                    throw gcnew HDF5Exception("Unsupported HDF5 atomic type!");
                }
            }
            finally
            {
                if (mspace >= 0) {
                    H5Sclose(mspace);
                }
                if (fspace >= 0) {
                    H5Sclose(fspace);
                }
                if (ntype >= 0) {
                    H5Tclose(ntype);
                }
                if (ftype >= 0) {
                    H5Tclose(ftype);
                }

                if (dset >= 0) {
                    H5Dclose(dset);
                }

                Marshal::FreeHGlobal(IntPtr(name));
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
                if (readCount > remaining) { length = remaining; }
                else { length = readCount; }

                result = gcnew array<T>(length);

                long long pos = m_position;
                for (long long i = 0; i < length; ++i)
                {
                    result[i] = m_array[pos++];
                }

                m_position += length;
            }

            return result;
        }

        virtual void Seek(long long offset, System::IO::SeekOrigin origin)
        {
            System::Console::WriteLine("DatasetReader::Seek()");
        }

    private:

        array<T>^ m_array;

        long long m_position;
        
    };

}
