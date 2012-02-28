
#include "DatasetInfo.h"
#include "DatasetInfoLite.h"
#include "DatatypeInfo.h"
#include "GroupInfo.h"
#include "GroupInfoLite.h"
#include "HDF5Exception.h"
#include "LinkInfo.h"
#include "ObjectInfo.h"
#include "Provider.h"
#include "ProviderUtils.h"
#include "PSH5XException.h"

extern "C" {
#include "H5Apublic.h"
#include "H5Gpublic.h"
#include "H5Lpublic.h"
#include "H5Opublic.h"
#include "H5Spublic.h"
}

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Collections::ObjectModel;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Provider;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{

    void Provider::GetItem(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::GetItem(Path = '{0}')", path));

        hid_t obj_id = -1, group_id = -1;

        char *root_name = NULL, *group_path = NULL, *link_name = NULL, *path_name = NULL;

        try
        {
            DriveInfo^ drive = nullptr;
            String^ h5path = nullptr;
            if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path))
            {
                throw gcnew PSH5XException("Ill-formed HDF5 path name and/or unable to obtain drive name!");
            }

            bool detailed = false;
            RuntimeDefinedParameterDictionary^ dynamicParameters =
                (RuntimeDefinedParameterDictionary^) DynamicParameters;
            if (dynamicParameters != nullptr && dynamicParameters->ContainsKey("Detailed"))
            {
                detailed = dynamicParameters["Detailed"]->IsSet;
            }

            if (ProviderUtils::IsH5RootPathName(h5path)) // root group
            {
#pragma region root group

                String^ rootName = "/";
                root_name =  (char*)(Marshal::StringToHGlobalAnsi(rootName)).ToPointer();

                obj_id = H5Oopen(drive->FileHandle, root_name, H5P_DEFAULT);
                if (obj_id < 0) {
                    throw gcnew HDF5Exception("H5Oopen failed!!!");
                }

                if (!detailed) {
                    WriteItemObject(gcnew GroupInfoLite(obj_id), path, true);
                }
                else {
                    WriteItemObject(gcnew GroupInfo(obj_id), path, true);
                }

#pragma endregion
            }
            else
            {
                String^ groupPath = ProviderUtils::ParentPath(h5path);
                group_path = (char*)(Marshal::StringToHGlobalAnsi(groupPath)).ToPointer();

                group_id = H5Gopen2(drive->FileHandle, group_path, H5P_DEFAULT);
                if (group_id < 0) {
                    throw gcnew HDF5Exception("H5Gopen2 failed!");
                }

                String^ linkName = ProviderUtils::ChildName(h5path);
                link_name = (char*)(Marshal::StringToHGlobalAnsi(linkName)).ToPointer();

                if (H5Lexists(group_id, link_name, H5P_DEFAULT) > 0)
                {
                    H5L_info_t linfo;
                    if (H5Lget_info(group_id, link_name, &linfo, H5P_DEFAULT) < 0) {
                        throw gcnew Exception("H5Lget_info failed!");
                    }

                    switch (linfo.type)
                    {
                    case H5L_TYPE_HARD:

#pragma region HDF5 hard link

                        path_name =  (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();
                        obj_id = H5Oopen(drive->FileHandle, path_name, H5P_DEFAULT);
                        if (obj_id < 0) {
                            throw gcnew Exception("H5Oopen failed!!!");
                        }

                        H5O_info_t oinfo;
                        if (H5Oget_info(obj_id, &oinfo) >= 0)
                        {
                            switch (oinfo.type)
                            {
                            case H5O_TYPE_GROUP:

                                if (!detailed) {
                                    WriteItemObject(gcnew GroupInfoLite(obj_id), path, true);
                                }
                                else {
                                    WriteItemObject(gcnew GroupInfo(obj_id), path, true);
                                }
                                break;

                            case H5O_TYPE_DATASET:

                                if (!detailed) {
                                    WriteItemObject(ProviderUtils::GetDatasetInfoLite(obj_id), path, false);
                                }
                                else {
                                    WriteItemObject(ProviderUtils::GetDatasetInfo(obj_id), path, false);
                                }
                                break;

                            case H5O_TYPE_NAMED_DATATYPE:

                                WriteItemObject(gcnew DatatypeInfo(obj_id), path, false);
                                break;

                            default:

                                WriteItemObject(gcnew ObjectInfo(obj_id), path, false);
                                break;
                            }
                        }
#pragma endregion
                        break;

                    case H5L_TYPE_SOFT:

                        WriteItemObject(gcnew LinkInfo(group_id, linkName, "SoftLink"),
                            path, false);
                        break;

                    case H5L_TYPE_EXTERNAL:

                        WriteItemObject(gcnew LinkInfo(group_id, linkName, "ExtLink"),
                            path, false);
                        break;

                    default:

                        throw gcnew PSH5XException("Unable to determine the item type for this path!!!");
                        break;
                    }
                }
                else
                {
                    WriteWarning(String::Format("Item not found at Path = '{0}'", path));
                }
            }
        }
        finally
        {
            if (group_id >= 0) {
                H5Gclose(group_id);
            }

            if (obj_id >= 0) {
                H5Oclose(obj_id);
            }

            if (path_name != NULL) {
                Marshal::FreeHGlobal(IntPtr(path_name));
            }

            if (link_name != NULL) {
                Marshal::FreeHGlobal(IntPtr(link_name));
            }

            if (group_path != NULL) {
                Marshal::FreeHGlobal(IntPtr(group_path));
            }

            if (root_name != NULL) {
                Marshal::FreeHGlobal(IntPtr(root_name));
            }
        }

        return;
    }

    Object^ Provider::GetItemDynamicParameters(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::GetItemDynamicParameters(Path = '{0}')", path));

        RuntimeDefinedParameterDictionary^ dynamicParameters =
            gcnew RuntimeDefinedParameterDictionary();

        Collection<Attribute^>^ atts = gcnew Collection<Attribute^>();
        ParameterAttribute^ paramAttr = gcnew ParameterAttribute();
        paramAttr->Mandatory = false;
        paramAttr->ValueFromPipeline = false;
        atts->Add(paramAttr);
        dynamicParameters->Add("Detailed",
            gcnew RuntimeDefinedParameter("Detailed", SwitchParameter::typeid, atts));

        return dynamicParameters;
    }

}