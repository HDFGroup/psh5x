#pragma once

#include "SimpleDatasetInfoLite.h"

namespace PSH5X
{

    public ref class SimpleExternalDatasetInfoLite : SimpleDatasetInfoLite
    {
    public:

        property int ExternalFileCount
        {
            int get() { return m_external_count; }
        }

        SimpleExternalDatasetInfoLite(hid_t dset);

        ~SimpleExternalDatasetInfoLite() {}

    private:

        int m_external_count;

    };

}
