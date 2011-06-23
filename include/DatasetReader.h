#pragma once

extern "C" {
#include "H5Dpublic.h"
}

namespace PSH5X
{

    public ref class DatasetReader
        : System::Management::Automation::Provider::IContentReader
    {
    public:

        DatasetReader(hid_t h5file, System::String^ h5path);

        DatasetReader(hid_t h5file, System::String^ h5path,
            array<hsize_t>^ start, array<hsize_t>^ stride,
            array<hsize_t>^ count, array<hsize_t>^ block);
        
        DatasetReader(hid_t h5file, System::String^ h5path, array<hsize_t>^ coord);

        ~DatasetReader() { this->!DatasetReader(); }

        !DatasetReader() {}

        virtual void Close() {}

        virtual System::Collections::IList^ Read(long long readCount);

        virtual void Seek(long long offset, System::IO::SeekOrigin origin)
        {
            System::Console::WriteLine("DatasetReader->Seek()");
        }

    private:

        array<System::Management::Automation::PSObject^>^ m_array;

        long long m_position;
        
    };

}
