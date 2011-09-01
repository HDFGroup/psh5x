
#include "HDF5Exception.h"
#include "PSH5XException.h"
#include "SimpleExternalDatasetInfoLite.h"

extern "C" {
#include "H5Dpublic.h"
#include "H5Ppublic.h"
#include "H5Spublic.h"
#include "H5Tpublic.h"
}

#include <vector>

using namespace std;

using namespace System;
using namespace System::IO;
using namespace System::Management::Automation;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
    SimpleExternalDatasetInfoLite::SimpleExternalDatasetInfoLite(hid_t dset) : SimpleDatasetInfoLite(dset)
    {
        hid_t plist = -1;

        try
        {
            plist = H5Dget_create_plist(dset);
            if (plist < 0) {
                throw gcnew HDF5Exception("H5Dget_create_plist failed!");
            }

            m_external_count = H5Pget_external_count(plist);
            if (m_external_count < 0) {
                throw gcnew HDF5Exception("H5Pget_external_count failed!");
            }
        }
        finally
        {
            if (plist >= 0) {
                H5Pclose(plist);
            }
        }
    }
}
