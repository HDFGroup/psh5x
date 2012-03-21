#pragma once

#include "PSH5XException.h"

extern "C" {
#include "H5Ipublic.h"
}

namespace PSH5X
{
    public ref class ReferenceDatasetWriter
        : System::Management::Automation::Provider::IContentWriter
    {
    public:

        ReferenceDatasetWriter(hid_t h5file, System::String^ h5path,
			System::Management::Automation::RuntimeDefinedParameterDictionary^ dict);

        ~ReferenceDatasetWriter() { this->!ReferenceDatasetWriter(); }

        !ReferenceDatasetWriter() {}

        virtual void Close() {}

        virtual System::Collections::IList^ ReferenceDatasetWriter::Write(System::Collections::IList^ content);

    virtual void Seek(long long offset, System::IO::SeekOrigin origin)
    {
        throw gcnew PSH5XException("ReferenceDatasetWriter::Seek() not implemented!");
    }

private:

    hid_t m_h5file;

    System::String^ m_h5path;

	System::Management::Automation::RuntimeDefinedParameterDictionary^ m_dict;

    System::Array^ m_array;

    long long m_position;

};

}
