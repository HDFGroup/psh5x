
#include "ProviderUtils.h"
#include "ObjectInfoLite.h"

extern "C" {
#include "H5Opublic.h"
}

using namespace System;
using namespace System::Collections;
using namespace System::IO;
using namespace System::Management::Automation;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
    ObjectInfoLite::ObjectInfoLite(hid_t obj)
    {
        H5O_info_t info;

        if (H5Oget_info(obj, &info) >= 0)
        {
            switch (info.type)
            {
            case H5O_TYPE_GROUP:
                __super::m_item_type = "Group";
                break;
            case H5O_TYPE_DATASET:
                __super::m_item_type = "Dataset";
                break;
            case H5O_TYPE_NAMED_DATATYPE:
                __super::m_item_type = "DatatypeObject";
                break;
            default:
                __super::m_item_type = "UNKNOWN";
                break;
            }

            m_atime = info.atime;
            m_mtime = info.mtime;
            m_ctime = info.ctime;
            m_btime = info.btime;
            m_num_attrs = info.num_attrs;
            m_attribute_names = ProviderUtils::GetObjectAttributeNames(obj);
        }
        else { // TODO
        }
    }
}
