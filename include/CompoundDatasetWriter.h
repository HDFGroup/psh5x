#pragma once

#include "PSH5XException.h"

extern "C" {
#include "H5Dpublic.h"
}

namespace PSH5X
{

    public ref class CompoundDatasetWriter
        : System::Management::Automation::Provider::IContentWriter
    {
    public:

        CompoundDatasetWriter(hid_t h5file, System::String^ h5path);

        /*
        CompoundDatasetWriter(hid_t h5file, System::String^ h5path,
            array<hsize_t>^ start, array<hsize_t>^ stride,
            array<hsize_t>^ count, array<hsize_t>^ block);
        
        CompoundDatasetWriter(hid_t h5file, System::String^ h5path, array<hsize_t>^ coord);
        */

        ~CompoundDatasetWriter() { this->!CompoundDatasetWriter(); }

        !CompoundDatasetWriter() {}

        virtual void Close() {}

        virtual System::Collections::IList^ Write(System::Collections::IList^ content);

        virtual void Seek(long long offset, System::IO::SeekOrigin origin)
        {
            throw gcnew PSH5XException("CompoundDatasetWriter::Seek() not implemented!");
        }

    private:

        hid_t m_h5file;

        System::String^ m_h5path;

        System::Array^ m_array;

    };

}