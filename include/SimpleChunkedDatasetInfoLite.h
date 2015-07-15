#pragma once

#include "SimpleDatasetInfoLite.h"

namespace PSH5X
{

    public ref class SimpleChunkedDatasetInfoLite : SimpleDatasetInfoLite
    {
    public:

        property array<hsize_t>^ Chunk
        {
            array<hsize_t>^ get() { return m_chunk; }
        }

        SimpleChunkedDatasetInfoLite(hid_t dset);

        ~SimpleChunkedDatasetInfoLite()
        {
            delete m_chunk;
        }

    private:

        array<hsize_t>^ m_chunk;

    };

}
