#pragma once

extern "C" {
#include "H5Dpublic.h"
}

namespace PSH5X
{

    public ref class DatasetWriter
        : System::Management::Automation::Provider::IContentWriter
    {
    public:

        DatasetWriter(hid_t h5file, System::String^ h5path);

        /*
        DatasetWriter(hid_t h5file, System::String^ h5path,
            array<hsize_t>^ start, array<hsize_t>^ stride,
            array<hsize_t>^ count, array<hsize_t>^ block);
        
        DatasetWriter(hid_t h5file, System::String^ h5path, array<hsize_t>^ coord);
        */

        ~DatasetWriter() { this->!DatasetWriter(); }

        !DatasetWriter() {}

        virtual void Close() {}

        virtual System::Collections::IList^ Write(
            System::Collections::IList^ content);

        virtual void Seek(long long offset, System::IO::SeekOrigin origin)
        {
            throw gcnew System::ArgumentException("DatasetWriter::Seek() not implemented!");
        }

    private:

        hid_t m_h5file;

        System::String^ m_h5path;

        System::Array^ m_array;

    };

}
