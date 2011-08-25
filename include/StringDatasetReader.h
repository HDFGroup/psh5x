#pragma once

extern "C" {
#include "H5Dpublic.h"
}

namespace PSH5X
{

    public ref class StringDatasetReader
        : System::Management::Automation::Provider::IContentReader
    {
    public:

        StringDatasetReader(hid_t h5file, System::String^ h5path);

        /*

        TODO: implement this

        DatasetReader(hid_t h5file, System::String^ h5path,
            array<hsize_t>^ start, array<hsize_t>^ stride,
            array<hsize_t>^ count, array<hsize_t>^ block);
        
        DatasetReader(hid_t h5file, System::String^ h5path, array<hsize_t>^ coord);
        */

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
