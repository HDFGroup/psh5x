
#include "HDF5Exception.h"
#include "ProviderUtils.h"
#include "PSH5XException.h"
#include "SimpleChunkedDatasetInfo.h"
#include "SimpleChunkedDatasetInfoLite.h"
#include "SimpleExternalDatasetInfo.h"
#include "SimpleExternalDatasetInfoLite.h"

extern "C" {

#include "H5Dpublic.h"
#include "H5Ipublic.h"
#include "H5Ppublic.h"
#include "H5Spublic.h"

}

using namespace System;
using namespace System::Collections;
using namespace System::Management::Automation;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{

    DatasetInfoLite^ ProviderUtils::GetDatasetInfoLite(hid_t obj_id)
    {
        DatasetInfoLite^ result = nullptr;

        if (H5Iget_type(obj_id) != H5I_DATASET) {
            throw gcnew PSH5XException("This is not a dataset!");
        }

        hid_t dspace = -1, plist = -1;

        try
        {
            dspace = H5Dget_space(obj_id);
            if (dspace < 0) {
                throw gcnew HDF5Exception("H5Dget_space failed!");
            }

            if (H5Sget_simple_extent_type(dspace) == H5S_SIMPLE)
            {
                plist = H5Dget_create_plist(obj_id);
                if (plist < 0) {
                    throw gcnew HDF5Exception("H5Dget_create_plist failed!");
                }

                if (H5Pget_layout(plist) == H5D_CHUNKED) {
                    result = gcnew SimpleChunkedDatasetInfoLite(obj_id);
                }
                else if (H5Pget_external_count(plist) > 0) {
                    result = gcnew SimpleExternalDatasetInfoLite(obj_id);
                }
                else {
                    result = gcnew SimpleDatasetInfoLite(obj_id);
                }
            }
            else {
                result = gcnew DatasetInfoLite(obj_id);
            }
        }
        finally
        {
            if (dspace >= 0) {
                H5Sclose(dspace);
            }

            if (plist >= 0) {
                H5Pclose(plist);
            }
        }

        return result;
    }

    DatasetInfo^ ProviderUtils::GetDatasetInfo(hid_t obj_id)
    {
        DatasetInfo^ result = nullptr;

        if (H5Iget_type(obj_id) != H5I_DATASET) {
            throw gcnew PSH5XException("This is not a dataset!");
        }

        hid_t dspace = -1, plist = -1;

        try
        {
            dspace = H5Dget_space(obj_id);
            if (dspace < 0) {
                throw gcnew HDF5Exception("H5Dget_space failed!");
            }

            if (H5Sget_simple_extent_type(dspace) == H5S_SIMPLE)
            {
                plist = H5Dget_create_plist(obj_id);
                if (plist < 0) {
                    throw gcnew HDF5Exception("H5Dget_create_plist failed!");
                }

                if (H5Pget_layout(plist) == H5D_CHUNKED) {
                    result = gcnew SimpleChunkedDatasetInfo(obj_id);
                }
                else if (H5Pget_external_count(plist) > 0) {
                    result = gcnew SimpleExternalDatasetInfo(obj_id);
                }
                else {
                    result = gcnew SimpleDatasetInfo(obj_id);
                }
            }
            else {
                result = gcnew DatasetInfo(obj_id);
            }
        }
        finally
        {
            if (dspace >= 0) {
                H5Sclose(dspace);
            }

            if (plist >= 0) {
                H5Pclose(plist);
            }
        }

        return result;
    }
}