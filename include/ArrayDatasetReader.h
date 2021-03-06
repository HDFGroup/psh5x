#pragma once

extern "C" {
#include "H5Dpublic.h"
}

namespace PSH5X
{

    public ref class ArrayDatasetReader
        : System::Management::Automation::Provider::IContentReader
    {
    public:

        ArrayDatasetReader(hid_t dset, hid_t ftype, hid_t fspace);

        /*
        TODO: implement this

        ArrayDatasetReader(hid_t h5file, System::String^ h5path,
            array<hsize_t>^ start, array<hsize_t>^ stride,
            array<hsize_t>^ count, array<hsize_t>^ block);
        
        ArrayDatasetReader(hid_t h5file, System::String^ h5path, array<hsize_t>^ coord);
        */

        ~ArrayDatasetReader()
        {
            delete m_array;
            this->!ArrayDatasetReader();
        }

        !ArrayDatasetReader() {}

        virtual void Close() {}

        virtual System::Collections::IList^ Read(long long readCount);

        virtual void Seek(long long offset, System::IO::SeekOrigin origin)
        {
            System::Console::WriteLine("ArrayDatasetReader->Seek()");
        }

    private:

        System::Array^ m_array;

        System::Type^ m_type;

        System::Collections::IEnumerator^ m_ienum;

        long long m_position;
        
    };

}
