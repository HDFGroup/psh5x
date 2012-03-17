#pragma once

extern "C" {
#include "H5Ipublic.h"
}

namespace PSH5X
{

    public ref class StringDatasetReader
        : System::Management::Automation::Provider::IContentReader
    {
    public:

        StringDatasetReader(hid_t dset, hid_t ftype, hid_t fspace, hid_t mspace);

        ~StringDatasetReader()
        {
            delete m_array;
            this->!StringDatasetReader();
        }

        !StringDatasetReader() {}

        virtual void Close() {}

        virtual System::Collections::IList^ Read(long long readCount);

        virtual void Seek(long long offset, System::IO::SeekOrigin origin)
        {
            System::Console::WriteLine("StringDatasetReader->Seek()");
        }

    private:

        System::Array^ m_array;

        System::Collections::IEnumerator^ m_ienum;

        long long m_position;
        
    };

}
