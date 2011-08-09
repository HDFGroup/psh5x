#pragma once

extern "C" {
#include "H5Dpublic.h"
}

namespace PSH5X
{
    template <typename T>
    public ref class DatasetWriterT
        : System::Management::Automation::Provider::IContentWriter
    {
    public:

        DatasetWriterT(hid_t h5file, System::String^ h5path)
            : m_h5file(h5file), m_h5path(h5path), m_array(nullptr)
        {
        }

        // TODO: deal with strings, enum, and opaque, and implement hyperslabs and point sets

        /*

        DatasetWriterT(hid_t h5file, System::String^ h5path,
            array<hsize_t>^ start, array<hsize_t>^ stride,
            array<hsize_t>^ count, array<hsize_t>^ block);
        
        DatasetWriterT(hid_t h5file, System::String^ h5path, array<hsize_t>^ coord);
        */

        ~DatasetWriterT() { this->!DatasetWriterT(); }

        !DatasetWriterT() {}

        virtual void Close() {}

        virtual System::Collections::IList^ Write(System::Collections::IList^ content)
        {
            System::Exception^ ex = nullptr;

            char* name = (char*)(Marshal::StringToHGlobalAnsi(m_h5path)).ToPointer();

#pragma region HDF5 handles

            hid_t dset = -1, fspace = -1, ftype = -1, ntype = -1;

            dset = H5Dopen2(m_h5file, name, H5P_DEFAULT);
            if (dset < 0) {
                ex = gcnew ArgumentException("H5Dopen2 failed!");
                goto error;
            }

            fspace = H5Dget_space(dset);
            if (fspace < 0) {
                ex = gcnew ArgumentException("H5Dget_space failed!");
                goto error;
            }

            ftype = H5Dget_type(dset);
            if (ftype < 0) {
                ex = gcnew ArgumentException("H5Dget_type failed!");
                goto error;
            }

            ntype = H5Tget_native_type(ftype, H5T_DIR_ASCEND);
            if (ntype < 0) {
                ex = gcnew ArgumentException("H5Tget_native_type failed!");
                goto error;
            }

#pragma endregion

            hssize_t npoints = H5Sget_simple_extent_npoints(fspace);

            if (content->Count != safe_cast<int>(npoints))
            {
                ex = gcnew ArgumentException("Size mismatch!");
                goto error;
            }

            try
            {
                m_array = gcnew array<T>(content->Count);
                Array^ a = safe_cast<Array^>(content);
                Array::Copy(a, m_array, a->Length);
            }
            catch (Exception^ lex) {
                ex = lex;
                goto error;
            }

            pin_ptr<T> ptr = &m_array[0];

            if (H5Dwrite(dset, ntype, H5S_ALL, H5S_ALL, H5P_DEFAULT, ptr) < 0) {
                ex = gcnew ArgumentException("H5Dwrite failed!");
                goto error;
            }

            if (H5Fflush(dset, H5F_SCOPE_LOCAL) < 0) {
                ex = gcnew ArgumentException("H5Fflush failed!");
                goto error;
            }

error:

            if (ntype >= 0) {
                H5Tclose(ntype);
            }
            if (ftype >= 0) {
                H5Tclose(ftype);
            }
            if (fspace >= 0) {
                H5Sclose(fspace);
            }
            if (dset >= 0) {
                H5Dclose(dset);
            }

            if (ex != nullptr) {
                throw ex;
            }

            return nullptr;
        }

        virtual void Seek(long long offset, System::IO::SeekOrigin origin)
        {
            throw gcnew ArgumentException("DatasetWriter::Seek() not implemented!");
        }

    private:

        hid_t m_h5file;

        System::String^ m_h5path;

        array<T>^ m_array;

    };

}
