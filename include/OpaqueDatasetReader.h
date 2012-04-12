#pragma once

#include "PSH5XException.h"

extern "C" {
#include "H5Ipublic.h"
}

namespace PSH5X
{
    public ref class OpaqueDatasetReader
        : System::Management::Automation::Provider::IContentReader
    {
    public:

        OpaqueDatasetReader(hid_t dset, hid_t ftype, hid_t fspace, hid_t mspace);

        ~OpaqueDatasetReader() { this->!OpaqueDatasetReader(); }

        !OpaqueDatasetReader() {}

        virtual void Close() {}

        virtual System::Collections::IList^ Read(long long readCount);

        virtual void Seek(long long offset, System::IO::SeekOrigin origin);

    private:

        System::Array^ m_array;

        System::Collections::IEnumerator^ m_ienum;

        long long m_position;

    };

}