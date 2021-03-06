#pragma once

#include "PSH5XException.h"

extern "C" {
#include "H5Dpublic.h"
}

namespace PSH5X
{

    public ref class ArrayDatasetWriter
        : System::Management::Automation::Provider::IContentWriter
    {
    public:

        ArrayDatasetWriter(hid_t h5file, System::String^ h5path);

        /*
        ArrayDatasetWriter(hid_t h5file, System::String^ h5path,
            array<hsize_t>^ start, array<hsize_t>^ stride,
            array<hsize_t>^ count, array<hsize_t>^ block);
        
        ArrayDatasetWriter(hid_t h5file, System::String^ h5path, array<hsize_t>^ coord);
        */

        ~ArrayDatasetWriter() { this->!ArrayDatasetWriter(); }

        !ArrayDatasetWriter() {}

        virtual void Close() {}

        virtual System::Collections::IList^ Write(System::Collections::IList^ content);

        virtual void Seek(long long offset, System::IO::SeekOrigin origin)
        {
            throw gcnew PSH5XException("ArrayDatasetWriter::Seek() not implemented!");
        }

    private:

        hid_t m_h5file;

        System::String^ m_h5path;

        System::Array^ m_array;

    };

}