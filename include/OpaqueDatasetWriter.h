#pragma once

#include "PSH5XException.h"

extern "C" {
#include "H5Ipublic.h"
}

namespace PSH5X
{
    public ref class OpaqueDatasetWriter
        : System::Management::Automation::Provider::IContentWriter
    {
    public:

        OpaqueDatasetWriter(hid_t h5file, System::String^ h5path,
			System::Management::Automation::RuntimeDefinedParameterDictionary^ dict);

        ~OpaqueDatasetWriter() { this->!OpaqueDatasetWriter(); }

        !OpaqueDatasetWriter() {}

        virtual void Close() {}

        virtual System::Collections::IList^ OpaqueDatasetWriter::Write(System::Collections::IList^ content);

    virtual void Seek(long long offset, System::IO::SeekOrigin origin);

private:

    hid_t m_h5file;

    System::String^ m_h5path;

	System::Management::Automation::RuntimeDefinedParameterDictionary^ m_dict;

    System::Array^ m_array;

    long long m_position;

};

}
