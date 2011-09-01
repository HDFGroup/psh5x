#pragma once

#include "SimpleDatasetInfo.h"

namespace PSH5X
{

    public ref class SimpleChunkedDatasetInfo : SimpleDatasetInfo
    {
    public:

        property array<hsize_t>^ Chunk
        {
            array<hsize_t>^ get() { return m_chunk; }
        }

        SimpleChunkedDatasetInfo(hid_t dset);

        ~SimpleChunkedDatasetInfo()
        {
            delete m_chunk;
        }

    private:

        array<hsize_t>^ m_chunk;

    };

}
