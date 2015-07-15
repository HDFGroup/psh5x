#pragma once

extern "C" {
#include "H5Ipublic.h"
}

namespace PSH5X
{

    public ref class VlenDatasetReader
        : System::Management::Automation::Provider::IContentReader
    {
    public:

        VlenDatasetReader(hid_t dset, hid_t ftype, hid_t fspace, hid_t mspace);

        ~VlenDatasetReader()
        {
            delete m_array;
            this->!VlenDatasetReader();
        }

        !VlenDatasetReader() {}

        virtual void Close() {}

        virtual System::Collections::IList^ Read(long long readCount);

        virtual void Seek(long long offset, System::IO::SeekOrigin origin);

    private:

        System::Array^ m_array;

        System::Type^ m_type;

        System::Collections::IEnumerator^ m_ienum;

        long long m_position;
        
    };

}
