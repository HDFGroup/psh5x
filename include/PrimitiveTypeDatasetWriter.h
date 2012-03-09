#pragma once

extern "C" {
#include "H5Ipublic.h"
}

namespace PSH5X
{
    public ref class PrimitiveTypeDatasetWriter
        : System::Management::Automation::Provider::IContentWriter
    {
    public:

        PrimitiveTypeDatasetWriter(hid_t h5file, System::String^ h5path)
            : m_h5file(h5file), m_h5path(h5path)
        {
        }

        // TODO: deal with hyperslabs and point sets

        /*

        DatasetWriterT(hid_t h5file, System::String^ h5path,
        array<hsize_t>^ start, array<hsize_t>^ stride,
        array<hsize_t>^ count, array<hsize_t>^ block);

        DatasetWriterT(hid_t h5file, System::String^ h5path, array<hsize_t>^ coord);
        */

        ~PrimitiveTypeDatasetWriter() { this->!PrimitiveTypeDatasetWriter(); }

        !PrimitiveTypeDatasetWriter() {}

        virtual void Close() {}

		virtual System::Collections::IList^ Write(System::Collections::IList^ content);

		virtual void Seek(long long offset, System::IO::SeekOrigin origin);

private:

    hid_t m_h5file;

    System::String^ m_h5path;
};

}
