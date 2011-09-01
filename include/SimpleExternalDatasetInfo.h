#pragma once

#include "SimpleDatasetInfo.h"

namespace PSH5X
{

    public ref class SimpleExternalDatasetInfo : SimpleDatasetInfo
    {
    public:

        property int ExternalFileCount
        {
            int get() { return m_external_count; }
        }

        SimpleExternalDatasetInfo(hid_t dset);

        ~SimpleExternalDatasetInfo() {}

    private:

        int m_external_count;

    };

}
