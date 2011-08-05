
#include "ProviderUtils.h"

extern "C" {
#include "H5Gpublic.h"
#include "H5Lpublic.h"
}

#include <string>
#include <vector>

using namespace std;

using namespace System;
using namespace System::Management::Automation;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
    bool ProviderUtils::IsH5Object(hid_t file, String^ h5path)
    {
        bool result = false;

        if (ProviderUtils::IsValidH5Path(file, h5path) &&
            !ProviderUtils::IsH5SymLink(file, h5path))
        {
            char* name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();
            hid_t obj_id = H5Oopen(file, name, H5P_DEFAULT);
            if (obj_id >= 0)
            {
                H5O_info_t info;
                if (H5Oget_info(obj_id, &info) >= 0)
                {
                    result = (info.type == H5O_TYPE_DATASET ||
                        info.type == H5O_TYPE_GROUP ||
                        info.type == H5O_TYPE_NAMED_DATATYPE);
                }
                else { // TODO
                }

                if (H5Oclose(obj_id) < 0) { // TODO
                }
            }
            else { // TODO
            }
        }
        else {
            result = false;
        }

        return result;
    }

    bool ProviderUtils::IsH5Group(hid_t file, String^ h5path)
    {
        bool result = false;

        if (ProviderUtils::IsH5Object(file, h5path))
        {
            char* name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();
            hid_t obj_id = H5Oopen(file, name, H5P_DEFAULT);
            if (obj_id >= 0)
            {
                H5O_info_t info;
                if (H5Oget_info(obj_id, &info) >= 0)
                {
                    result = (info.type == H5O_TYPE_GROUP);
                }
                else { //TODO
                }

                if (H5Oclose(obj_id) < 0) { // TODO
                }
            }
            else { // TODO
            }
        }

        return result;
    }

    bool ProviderUtils::IsH5Dataset(hid_t file, String^ h5path)
    {
        bool result = false;

        if (ProviderUtils::IsH5Object(file, h5path))
        {
            char* name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();
            hid_t obj_id = H5Oopen(file, name, H5P_DEFAULT);
            if (obj_id >= 0)
            {
                H5O_info_t info;
                if (H5Oget_info(obj_id, &info) >= 0) {
                    result = (info.type == H5O_TYPE_DATASET);
                }
                else { // TODO
                }

                if (H5Oclose(obj_id) < 0) { // TODO
                }
            }
            else { // TODO
            }
        }
        else {
            result = false;
        }

        return result;
    }

    bool ProviderUtils::IsH5DatatypeObject(hid_t file, String^ h5path)
    {
        bool result = false;

        if (ProviderUtils::IsH5Object(file, h5path))
        {
            char* name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();
            hid_t obj_id = H5Oopen(file, name, H5P_DEFAULT);
            if (obj_id >= 0)
            {
                H5O_info_t info;
                if (H5Oget_info(obj_id, &info) >= 0) {
                    result = (info.type == H5O_TYPE_NAMED_DATATYPE);
                }
                else { // TODO
                }

                if (H5Oclose(obj_id) < 0) { // TODO
                }
            }
            else { // TODO
            }
        }
        else {
            result = false;
        }

        return result;
    }

    bool ProviderUtils::IsH5SymLink(hid_t file, String^ h5path)
    {
        if (ProviderUtils::IsH5RootPathName(h5path)) { return false; }

        bool result = false;

        if (ProviderUtils::IsValidH5Path(file, h5path))
        {
            char* name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();
            H5L_info_t info;
            if (H5Lget_info(file, name, &info, H5P_DEFAULT) >= 0)
            {
                result = (info.type == H5L_TYPE_SOFT || info.type == H5L_TYPE_EXTERNAL);
            }
            else { // TODO
            }
        }
        else {
            result = false;
        }

        return result;
    }

    bool ProviderUtils::IsH5SoftLink(hid_t file, String^ h5path)
    {
        bool result = false;

        if (ProviderUtils::IsValidH5Path(file, h5path))
        {
            char* name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();
            H5L_info_t info;
            if (H5Lget_info(file, name, &info, H5P_DEFAULT) >= 0)
            {
                result = (info.type == H5L_TYPE_SOFT);
            }
            else { // TODO
            }
        }
        else {
            result = false;
        }

        return result;
    }

    bool ProviderUtils::IsH5ExternalLink(hid_t file, String^ h5path)
    {
        bool result = false;

        if (ProviderUtils::IsValidH5Path(file, h5path))
        {
            char* name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();
            H5L_info_t info;
            if (H5Lget_info(file, name, &info, H5P_DEFAULT) >= 0)
            {
                result = (info.type == H5L_TYPE_EXTERNAL);
            }
            else { // TODO
            }
        }
        else {
            result = false;
        }

        return result;
    }
}