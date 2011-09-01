#pragma once

#include "DatasetInfo.h"

namespace PSH5X
{

    public ref class SimpleDatasetInfo : DatasetInfo
    {
    public:

        property array<hsize_t>^ Dimensions
        {
            array<hsize_t>^ get() { return m_dims; }
        }

        property array<long long>^ MaxDimensions
        {
            array<long long>^ get() { return m_maxdims; }
        }

        SimpleDatasetInfo(hid_t dset);

        ~SimpleDatasetInfo()
        {
            delete m_dims;
            delete m_maxdims;
        }

    private:

        array<hsize_t>^   m_dims;

        array<long long>^ m_maxdims;

    };

}
