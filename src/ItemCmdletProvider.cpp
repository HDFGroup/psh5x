
#include "DatasetInfo.h"
#include "DatasetInfoLite.h"
#include "DatatypeInfo.h"
#include "GroupInfo.h"
#include "GroupInfoLite.h"
#include "LinkInfo.h"
#include "ObjectInfo.h"
#include "Provider.h"
#include "ProviderUtils.h"

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
    void Provider::ClearItem(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::ClearItem(Path = '{0}')", path));
        WriteWarning("The HDF5Provider::ClearItem() not implemented!!!");
        return;
    }

    Object^ Provider::ClearItemDynamicParameters(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::ClearItemDynamicParameters(Path = '{0}')", path));
        return nullptr;
    }

    bool Provider::IsValidPath(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::IsValidPath(Path = '{0}')", path));
        
        String^ npath = ProviderUtils::NormalizePath(path);
        String^ h5path = ProviderUtils::PathNoDrive(npath);

        return ProviderUtils::IsWellFormedH5Path(h5path);
    }

    void Provider::GetItem(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::GetItem(Path = '{0}')", path));
        
        Exception^ ex = nullptr;

        hid_t oid = -1, gid = -1;

        char *root_name = NULL, *group_path = NULL, *link_name = NULL, *path_name = NULL;

        DriveInfo^ drive = nullptr;
        String^ h5path = nullptr;
        if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path))
        {
            ErrorRecord^ error = gcnew ErrorRecord(
                gcnew ArgumentException("Ill-formed HDF5 path name and/or unable to obtain drive name!"),
                "InvalidData", ErrorCategory::InvalidData, nullptr);
            ThrowTerminatingError(error);
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
            
            oid = H5Oopen(drive->FileHandle, root_name, H5P_DEFAULT);
            if (oid < 0) {
                ex = gcnew Exception("H5Oopen failed!!!");
                goto error;
            }

            if (!detailed)
            {
                WriteItemObject(gcnew GroupInfoLite(oid), path, true);
            }
            else
            {
                WriteItemObject(gcnew GroupInfo(oid), path, true);
            }

#pragma endregion
        }
        else
        {
            String^ groupPath = ProviderUtils::ParentPath(h5path);
            group_path = (char*)(Marshal::StringToHGlobalAnsi(groupPath)).ToPointer();

            gid = H5Gopen2(drive->FileHandle, group_path, H5P_DEFAULT);
            if (gid < 0) {
                ex = gcnew Exception("H5Gopen2 failed!!!");
                goto error;
            }

            String^ linkName = ProviderUtils::ChildName(h5path);
            link_name = (char*)(Marshal::StringToHGlobalAnsi(linkName)).ToPointer();

            if (H5Lexists(gid, link_name, H5P_DEFAULT) > 0)
            {
                H5L_info_t linfo;
                if (H5Lget_info(gid, link_name, &linfo, H5P_DEFAULT) < 0) {
                    ex = gcnew Exception("H5Lget_info failed!!!");
                    goto error;
                }

                switch (linfo.type)
                {
                case H5L_TYPE_HARD:
#pragma region HDF5 hard link

                    path_name =  (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();
                    oid = H5Oopen(drive->FileHandle, path_name, H5P_DEFAULT);
                    if (oid < 0) {
                        ex = gcnew Exception("H5Oopen failed!!!");
                        goto error;
                    }

                    H5O_info_t oinfo;
                    if (H5Oget_info(oid, &oinfo) >= 0)
                    {
                        switch (oinfo.type)
                        {
                        case H5O_TYPE_GROUP:
                            if (!detailed) {
                                WriteItemObject(gcnew GroupInfoLite(oid), path, true);
                            }
                            else {
                                WriteItemObject(gcnew GroupInfo(oid), path, true);
                            }
                            break;
                        case H5O_TYPE_DATASET:
                            if (!detailed) {
                                WriteItemObject(gcnew DatasetInfoLite(oid), path, false);
                            }
                            else {
                                WriteItemObject(gcnew DatasetInfo(oid), path, false);
                            }
                            break;
                        case H5O_TYPE_NAMED_DATATYPE:
                            WriteItemObject(gcnew DatatypeInfo(oid), path, false);
                            break;
                        default:
                            WriteItemObject(gcnew ObjectInfo(oid), path, false);
                            break;
                        }
                    }
#pragma endregion
                    break;

                case H5L_TYPE_SOFT:
                    WriteItemObject(gcnew LinkInfo(gid, linkName, "SoftLink"),
                        path, false);
                    break;
                case H5L_TYPE_EXTERNAL:
                    WriteItemObject(gcnew LinkInfo(gid, linkName, "ExtLink"),
                        path, false);
                    break;
                default:

                    ex = gcnew Exception("Unable to determine the item type for this path!!!");
                    goto error;
                    break;
                }
            }
            else
            {
                WriteWarning(String::Format("Item not found at Path = '{0}'", path));
            }
        }

error:

        if (gid >= 0) {
            H5Gclose(gid);
        }

        if (oid >= 0) {
            H5Oclose(oid);
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

        if (ex != nullptr) {
            throw ex;
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

    void Provider::InvokeDefaultAction(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::InvokeDefaultAction(Path = '{0}')", path));

        ErrorRecord^ error = gcnew ErrorRecord(
            gcnew NotImplementedException("HDF5Provider::InvokeDefaultAction not implemented!!!"),
            "NotImplemented", ErrorCategory::NotImplemented, nullptr);
        WriteError(error);

        return;
    }

    Object^ Provider::InvokeDefaultActionDynamicParameters(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::InvokeDefaultActionDynamicParameters(Path = '{0}')", path));
        return nullptr;
    }

    bool Provider::ItemExists(System::String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::ItemExists(Path = '{0}')",path));

        DriveInfo^ drive = nullptr;
        String^ h5path = nullptr;
        if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path))
        {
            ErrorRecord^ error = gcnew ErrorRecord(
                gcnew ArgumentException("Ill-formed HDF5 path name and/or unable to obtain drive name!"),
                "InvalidData", ErrorCategory::InvalidData, nullptr);
            ThrowTerminatingError(error);
        }

        return ProviderUtils::IsValidH5Path(drive->FileHandle, h5path);
    }

    Object^ Provider::ItemExistsDynamicParameters(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::ItemExistsDynamicParameters(Path = '{0}')", path));
        
        // add -itemType optional parameter so that existence and item type can be tested
        // e.g., Test-Path HDF5::h5:/foo/bar -itemType Group

        RuntimeDefinedParameterDictionary^ dynamicParameters =
            gcnew RuntimeDefinedParameterDictionary();

        Collection<Attribute^>^ atts = gcnew Collection<Attribute^>();
        ParameterAttribute^ paramAttr = gcnew ParameterAttribute();
        paramAttr->Mandatory = false;
        atts->Add(paramAttr);
        dynamicParameters->Add("itemType",
            gcnew RuntimeDefinedParameter("itemType", String::typeid, atts));
        
        return dynamicParameters;
    }

}