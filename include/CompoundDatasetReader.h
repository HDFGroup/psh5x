#pragma once

extern "C" {
#include "H5Dpublic.h"
}

namespace PSH5X
{

    public ref class CompoundDatasetReader
        : System::Management::Automation::Provider::IContentReader
    {
    public:

        CompoundDatasetReader(hid_t h5file, System::String^ h5path);

        /*
        TODO: implement this

        DatasetReader(hid_t h5file, System::String^ h5path,
            array<hsize_t>^ start, array<hsize_t>^ stride,
            array<hsize_t>^ count, array<hsize_t>^ block);
        
        DatasetReader(hid_t h5file, System::String^ h5path, array<hsize_t>^ coord);
        */

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
