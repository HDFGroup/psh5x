#pragma once

extern "C" {
#include "H5Ipublic.h"
}

namespace PSH5X
{

    public ref class CompoundDatasetReader
        : System::Management::Automation::Provider::IContentReader
    {
    public:

        CompoundDatasetReader(hid_t dset, hid_t ftype, hid_t fspace, hid_t mspace);

        ~CompoundDatasetReader()
        {
            delete m_array;
            delete m_type;
            this->!CompoundDatasetReader();
        }

        !CompoundDatasetReader() {}

        virtual void Close() {}

        virtual System::Collections::IList^ Read(long long readCount);

        virtual void Seek(long long offset, System::IO::SeekOrigin origin)
        {
            System::Console::WriteLine("CompoundDatasetReader->Seek()");
        }

    private:

        System::Array^ m_array;

        System::Type^ m_type;

        System::Collections::IEnumerator^ m_ienum;

        long long m_position;

    };

}
