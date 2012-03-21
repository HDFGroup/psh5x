#pragma once

#include "PSH5XException.h"

extern "C" {
#include "H5Ipublic.h"
}

namespace PSH5X
{
    public ref class ReferenceDatasetReader
        : System::Management::Automation::Provider::IContentReader
    {
    public:

        ReferenceDatasetReader(hid_t dset, hid_t ftype, hid_t fspace, hid_t mspace);

        ~ReferenceDatasetReader() { this->!ReferenceDatasetReader(); }

        !ReferenceDatasetReader() {}

        virtual void Close() {}

        virtual System::Collections::IList^ Read(long long readCount);

        virtual void Seek(long long offset, System::IO::SeekOrigin origin)
        {
            throw gcnew PSH5XException("ReferenceDatasetReader::Seek() not implemented!");
        }

    private:

        System::Array^ m_array;

        System::Collections::IEnumerator^ m_ienum;

        long long m_position;

		bool m_isObjectReference;

    };

}