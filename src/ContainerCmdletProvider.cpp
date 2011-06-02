
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
#include "H5Gpublic.h"
#include "H5Ppublic.h"
}

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Collections::ObjectModel;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Provider;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
    void Provider::CopyItem(String^ path, String^ copyPath, bool recurse)
    {
        WriteVerbose(String::Format(
            "HDF5Provider::CopyItem(Path = '{0}', CopyPath = '{1}', Recurse = {2})",
            path, copyPath, recurse));
        WriteWarning("The HDF5Provider::CopyItem() method has not (yet) been implemented.");
        return;
    }

    Object^ Provider::CopyItemDynamicParameters(String^ path, String^ copyPath, bool recurse)
    {
        WriteVerbose(String::Format(
            "HDF5Provider::CopyItemDynamicParameters(Path = '{0}', CopyPath = '{1}', Recurse = {2})",
            path, copyPath, recurse));
        return nullptr;
    }

    void Provider::GetChildItems(System::String^ path, bool recurse)
    {
        WriteVerbose(String::Format("HDF5Provider::GetChildItems(Path = '{0}', recurse = '{1}')",
            path, recurse));
        
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

        if (ProviderUtils::IsH5Group(drive->FileHandle, h5path))
        {
            char* gpath = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();
            hid_t gid = H5Gopen2(drive->FileHandle, gpath, H5P_DEFAULT);
            if (gid >= 0)
            {
                array<String^>^ linkNames = ProviderUtils::GetGroupLinkNames(gid, recurse);

                for each (String^ linkName in linkNames)
                {
                    String^ childPath = path;
                    if (path != (drive->Name + ":\\"))
                        childPath += "\\";
                    childPath += linkName->Replace('/','\\');

                    char* link_name = (char*)(Marshal::StringToHGlobalAnsi(linkName)).ToPointer();
                    H5L_info_t linfo;
                    if (H5Lget_info(gid, link_name, &linfo, H5P_DEFAULT) >= 0)
                    {
                        hid_t obj_id;

                        switch (linfo.type)
                        {
                        case H5L_TYPE_HARD:

                            obj_id = H5Oopen(gid, link_name, H5P_DEFAULT);
                            if (obj_id >= 0)
                            {
                                H5O_info_t oinfo;
                                if (H5Oget_info(obj_id, &oinfo) >= 0)
                                {
                                    switch (oinfo.type)
                                    {
                                    case H5O_TYPE_GROUP:
                                        if (!detailed)
                                        {
                                            WriteItemObject(gcnew GroupInfoLite(obj_id), childPath,
                                                true);
                                        }
                                        else
                                        {
                                            WriteItemObject(gcnew GroupInfo(obj_id), childPath, true);
                                        }
                                        break;
                                    case H5O_TYPE_DATASET:
                                        if (!detailed)
                                        {
                                            WriteItemObject(gcnew DatasetInfoLite(obj_id), childPath,
                                                false);
                                        }
                                        else
                                        {
                                            WriteItemObject(gcnew DatasetInfo(obj_id), childPath,
                                                false);
                                        }
                                        break;
                                    case H5O_TYPE_NAMED_DATATYPE:
                                        WriteItemObject(gcnew DatatypeInfo(obj_id), childPath, false);
                                        break;
                                    default:
                                        WriteItemObject(gcnew ObjectInfo(obj_id), childPath, false);
                                        break;
                                    }
                                }
                                if (H5Oclose(obj_id) < 0) { // TODO
                                }
                            }
                            else { // TODO
                            }

                            break;

                        case H5L_TYPE_SOFT:
                            WriteItemObject(gcnew LinkInfo(gid, linkName, "SoftLink"),
                                childPath, false);
                            break;
                        case H5L_TYPE_EXTERNAL:
                            WriteItemObject(gcnew LinkInfo(gid, linkName, "ExtLink"),
                                childPath, false);
                            break;
                        default:
                            ErrorRecord^ error = gcnew ErrorRecord(
                                gcnew InvalidProgramException("Unable to determine the item type for this path"),
                                "H5L_TYPE", ErrorCategory::InvalidData, nullptr);
                            WriteError(error);
                            break;
                        }
                    }
                    else { // TODO
                    }
                }

                if (H5Gclose(gid) < 0) { // TODO
                }
            }
        }

        return;
    }

    Object^ Provider::GetChildItemsDynamicParameters(System::String^ path, bool recurse)
    {
        WriteVerbose(String::Format("HDF5Provider::GetChildItemsDynamicParameters(Path = '{0}', recurse = '{1}')",
            path, recurse));

        RuntimeDefinedParameterDictionary^ dynamicParameters =
            gcnew RuntimeDefinedParameterDictionary();

        Collection<Attribute^>^ atts = gcnew Collection<Attribute^>();
        ParameterAttribute^ paramAttr = gcnew ParameterAttribute();
        paramAttr->Mandatory = false;
        atts->Add(paramAttr);
        dynamicParameters->Add("ItemType",
            gcnew RuntimeDefinedParameter("ItemType", String::typeid, atts));

        Collection<Attribute^>^ atts1 = gcnew Collection<Attribute^>();
        ParameterAttribute^ paramAttr1 = gcnew ParameterAttribute();
        paramAttr1->Mandatory = false;
        paramAttr1->ValueFromPipeline = false;
        atts1->Add(paramAttr1);
        dynamicParameters->Add("Detailed",
            gcnew RuntimeDefinedParameter("Detailed", SwitchParameter::typeid, atts1));
        
        return dynamicParameters;
    }

    void Provider::GetChildNames(System::String^ path, ReturnContainers returnContainers)
    {
        WriteVerbose(String::Format("HDF5Provider::GetChildNames(Path = '{0}', ReturnContainers = '{1}')",
            path, returnContainers));

        DriveInfo^ drive = nullptr;
        String^ h5path = nullptr;
        if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path))
        {
            ErrorRecord^ error = gcnew ErrorRecord(
                gcnew ArgumentException("Ill-formed HDF5 path name and/or unable to obtain drive name!"),
                "InvalidData", ErrorCategory::InvalidData, nullptr);
            ThrowTerminatingError(error);
        }

        if (ProviderUtils::IsH5Group(drive->FileHandle, h5path))
        {
            char* gpath = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();
            hid_t gid = H5Gopen2(drive->FileHandle, gpath, H5P_DEFAULT);
            if (gid >= 0)
            {
                array<String^>^ linkNames = ProviderUtils::GetGroupLinkNames(gid, false);

                for each (String^ linkName in linkNames)
                {
                    String^ childPath = path + "\\" + linkName->Replace('/','\\');

                    char* link_name = (char*)(Marshal::StringToHGlobalAnsi(linkName)).ToPointer();

                    H5L_info_t info;
                    if (H5Lget_info(gid, link_name, &info, H5P_DEFAULT) >= 0)
                    {
                        switch (info.type)
                        {
                        case H5L_TYPE_HARD:

                            switch (info.type)
                            {
                            case H5O_TYPE_GROUP:
                                WriteItemObject(linkName, childPath, true);
                                break;
                            default:
                                WriteItemObject(linkName, childPath, false);
                                break;
                            }
                            break;

                        default:
                            WriteItemObject(linkName, childPath, false);
                            break;
                        }
                    }
                    else { // TODO
                    }
                }

                if (H5Gclose(gid) < 0) { // TODO
                }
            }
            else { // TODO
            }
        }
        
        return;
    }

    /*
    Object^ Provider::GetChildNamesDynamicParameters(System::String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::GetChildNamesDynamicParameters(Path = '{0}')",
            path));
        WriteWarning("The HDF5Provider::GetChildNamesDynamicParameters() method has not (yet) been implemented.");
        return nullptr;
    }
    */

    bool Provider::HasChildItems(System::String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::HasChildItems(Path = '{0}')", path));

        DriveInfo^ drive = nullptr;
        String^ h5path = nullptr;
        if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path))
        {
            ErrorRecord^ error = gcnew ErrorRecord(
                gcnew ArgumentException("Ill-formed HDF5 path name and/or unable to obtain drive name!"),
                "InvalidData", ErrorCategory::InvalidData, nullptr);
            ThrowTerminatingError(error);
        }

        bool result = false;

        if (ProviderUtils::IsH5Group(drive->FileHandle, h5path))
        {
            char* gpath = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();
            hid_t gid = H5Gopen2(drive->FileHandle, gpath, H5P_DEFAULT);
            if (gid >= 0)
            {
                H5G_info_t info;
                if (H5Gget_info(gid, &info) >= 0)
                {
                    result = (info.nlinks > 0);
                }
                else { // TODO
                }

                if (H5Gclose(gid) < 0) { // TODO
                }
            }
            else { // TODO
            }
        }

        return result;
    }

    void Provider::RemoveItem(String^ path, bool recurse)
    {
        WriteVerbose(String::Format(
            "HDF5Provider::RemoveItem(Path = '{0}', Recurse = {1})", path, recurse));
        if (recurse)
        {
            WriteWarning("The '-Recurse' option has no effect at the moment.");
        }

        DriveInfo^ drive = nullptr;
        String^ h5path = nullptr;
        if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path))
        {
            ErrorRecord^ error = gcnew ErrorRecord(
                gcnew ArgumentException("Ill-formed HDF5 path name and/or unable to obtain drive name!"),
                "InvalidData", ErrorCategory::InvalidData, nullptr);
            ThrowTerminatingError(error);
        }

        if (ProviderUtils::IsH5RootPathName(h5path)) // root group, TODO: refine with address check!
        {
            ErrorRecord^ error = gcnew ErrorRecord(
                gcnew ArgumentException(String::Format("Cannot remove root group '{0}'", h5path)),
                "h5path", ErrorCategory::InvalidData, nullptr);
            ThrowTerminatingError(error);
        }

        String^ groupPath = ProviderUtils::ParentPath(h5path);
        char* group_path = (char*)(Marshal::StringToHGlobalAnsi(groupPath)).ToPointer();
        hid_t group = H5Gopen2(drive->FileHandle, group_path, H5P_DEFAULT);
        if (group >= 0)
        {
            String^ linkName = ProviderUtils::ChildName(h5path);
            char* link_name = (char*)(Marshal::StringToHGlobalAnsi(linkName)).ToPointer();
            if (H5Lexists(group, link_name, H5P_DEFAULT) > 0)
            {
                if (this->ShouldProcess(h5path,
                    String::Format("Removing HDF5 item '{0}'", path)))
                {
                    if (H5Ldelete(group, link_name, H5P_DEFAULT) >= 0)
                    {
                        if (H5Fflush(group, H5F_SCOPE_LOCAL) < 0) { // TODO
                        }
                    }
                    else { // TODO
                    }
                }
            }
            else { // TODO
            }

            if (H5Gclose(group) < 0) { // TODO
            }
        }

        return;
    }

    Object^ Provider::RemoveItemDynamicParameters(String^ path, bool recurse)
    {
        WriteVerbose(String::Format(
            "HDF5Provider::RemoveItemDynamicParameters(Path = '{0}', Recurse = {1})",
            path, recurse));
        return nullptr;
    }

    void Provider::RenameItem(String^ path, String^ newName)
    {
        WriteVerbose(String::Format(
            "HDF5Provider::RenameItem(Path = '{0}', NewName = '{1}')", path, newName));
        WriteWarning("The HDF5Provider::RenameItem() method has not (yet) been implemented.");
        return;
    }

    Object^ Provider::RenameItemDynamicParameters(String^ path, String^ newName)
    {
        WriteVerbose(String::Format(
            "HDF5Provider::RenameItemDynamicParameters(Path = '{0}', NewName = '{1}')",
            path, newName));
        return nullptr;
    }

}
