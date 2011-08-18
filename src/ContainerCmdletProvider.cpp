
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

        throw gcnew PSH5XException("The HDF5Provider::CopyItem() method has not (yet) been implemented.");
        
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

        hid_t gid = -1, oid = -1;

        char *gpath = NULL, *link_name = NULL;

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

            if (ProviderUtils::IsH5Group(drive->FileHandle, h5path))
            {
                gpath = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

                gid = H5Gopen2(drive->FileHandle, gpath, H5P_DEFAULT);
                if (gid < 0) {
                    throw gcnew HDF5Exception("H5Gopen2 failed!!!");
                }

                array<String^>^ linkNames = ProviderUtils::GetGroupLinkNames(gid, recurse);

                for each (String^ linkName in linkNames)
                {
                    String^ childPath = path;
                    if (path != (drive->Name + ":\\"))
                        childPath += "\\";
                    childPath += linkName->Replace('/','\\');

                    link_name = (char*)(Marshal::StringToHGlobalAnsi(linkName)).ToPointer();
                    H5L_info_t linfo;
                    if (H5Lget_info(gid, link_name, &linfo, H5P_DEFAULT) >= 0)
                    {
                        switch (linfo.type)
                        {
                        case H5L_TYPE_HARD:

#pragma region HDF5 hard link

                            oid = H5Oopen(gid, link_name, H5P_DEFAULT);
                            if (oid < 0) {
                                throw gcnew HDF5Exception("H5Oopen failed!");
                            }

                            H5O_info_t oinfo;
                            if (H5Oget_info(oid, &oinfo) >= 0)
                            {
                                switch (oinfo.type)
                                {
                                case H5O_TYPE_GROUP:
                                    if (!detailed)
                                    {
                                        WriteItemObject(gcnew GroupInfoLite(oid), childPath,
                                            true);
                                    }
                                    else
                                    {
                                        WriteItemObject(gcnew GroupInfo(oid), childPath, true);
                                    }
                                    break;
                                case H5O_TYPE_DATASET:
                                    if (!detailed)
                                    {
                                        WriteItemObject(gcnew DatasetInfoLite(oid), childPath,
                                            false);
                                    }
                                    else
                                    {
                                        WriteItemObject(gcnew DatasetInfo(oid), childPath,
                                            false);
                                    }
                                    break;
                                case H5O_TYPE_NAMED_DATATYPE:
                                    WriteItemObject(gcnew DatatypeInfo(oid), childPath, false);
                                    break;
                                default:
                                    WriteItemObject(gcnew ObjectInfo(oid), childPath, false);
                                    break;
                                }
                            }
                            else {
                                throw gcnew HDF5Exception("H5Oget_info failed!!!");
                            }

#pragma endregion

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

                            throw gcnew PSH5XException("Unable to determine the item type for this path!!!");
                            break;
                        }
                    }
                    else {
                        throw gcnew HDF5Exception("H5Lget_info failed!!!");
                    }
                }
            }
        }
        finally
        {
            if (oid >= 0) {
                H5Oclose(oid);
            }

            if (gid >= 0) {
                H5Gclose(gid);
            }

            if (link_name != NULL) {
                Marshal::FreeHGlobal(IntPtr(link_name));
            }

            if (gpath != NULL) {
                Marshal::FreeHGlobal(IntPtr(gpath));
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

        hid_t gid = -1;

        char *gpath = NULL, *link_name = NULL;

        try
        {
            DriveInfo^ drive = nullptr;
            String^ h5path = nullptr;
            if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path))
            {
                throw gcnew PSH5XException("Ill-formed HDF5 path name and/or unable to obtain drive name!");
            }

            if (ProviderUtils::IsH5Group(drive->FileHandle, h5path))
            {
                gpath = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

                gid = H5Gopen2(drive->FileHandle, gpath, H5P_DEFAULT);
                if (gid < 0) {
                    throw gcnew HDF5Exception("H5Gopen2 failed!");
                }

                array<String^>^ linkNames = ProviderUtils::GetGroupLinkNames(gid, false);

                for each (String^ linkName in linkNames)
                {
                    String^ childPath = path + "\\" + linkName->Replace('/','\\');

                    link_name = (char*)(Marshal::StringToHGlobalAnsi(linkName)).ToPointer();

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
                    else {
                        throw gcnew HDF5Exception("H5Lget_info failed!");
                    }
                }
            }
        }
        finally
        {
            if (gid >= 0) {
                H5Gclose(gid);
            }

            if (link_name != NULL) {
                Marshal::FreeHGlobal(IntPtr(link_name));
            }

            if (gpath != NULL) {
                Marshal::FreeHGlobal(IntPtr(gpath));
            }
        }

        return;
    }

    /*
    Object^ Provider::GetChildNamesDynamicParameters(System::String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::GetChildNamesDynamicParameters(Path = '{0}')",
            path));
        return nullptr;
    }
    */

    bool Provider::HasChildItems(System::String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::HasChildItems(Path = '{0}')", path));

        bool result = false;

        hid_t gid = -1;

        char* gpath = NULL;

        try
        {
            DriveInfo^ drive = nullptr;
            String^ h5path = nullptr;
            if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path))
            {
                throw gcnew PSH5XException("Ill-formed HDF5 path name and/or unable to obtain drive name!");
            }

            if (ProviderUtils::IsH5Group(drive->FileHandle, h5path))
            {
                gpath = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

                gid = H5Gopen2(drive->FileHandle, gpath, H5P_DEFAULT);
                if (gid < 0) {
                    throw gcnew Exception("H5Gopen2 failed!");
                }

                H5G_info_t info;
                if (H5Gget_info(gid, &info) >= 0)
                {
                    result = (info.nlinks > 0);
                }
                else {
                    throw gcnew Exception("H5Gget_info failed!");
                }
            }
        }
        finally
        {
            if (gid >= 0) {
                H5Gclose(gid);
            }

            if (gpath != NULL) {
                Marshal::FreeHGlobal(IntPtr(gpath));
            }
        }

        return result;
    }

    void Provider::RemoveItem(String^ path, bool recurse)
    {
        WriteVerbose(String::Format(
            "HDF5Provider::RemoveItem(Path = '{0}', Recurse = {1})", path, recurse));

        hid_t gid = -1;

        char *group_path = NULL, *link_name = NULL;

        try
        {

            if (recurse)
            {
                WriteWarning("The '-Recurse' option has no effect at the moment.");
            }

            DriveInfo^ drive = nullptr;
            String^ h5path = nullptr;
            if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path))
            {
                throw gcnew PSH5XException("Ill-formed HDF5 path name and/or unable to obtain drive name!");
            }

            if (drive->ReadOnly)
            {
                throw gcnew PSH5XException("The drive is read-only and cannot be modified!");
            }

            if (ProviderUtils::IsH5RootPathName(h5path)) // root group, TODO: refine with address check!
            {
                throw gcnew PSH5XException(String::Format("Cannot remove root group '{0}'", h5path));
            }
            else
            {

                String^ groupPath = ProviderUtils::ParentPath(h5path);
                group_path = (char*)(Marshal::StringToHGlobalAnsi(groupPath)).ToPointer();

                gid = H5Gopen2(drive->FileHandle, group_path, H5P_DEFAULT);
                if (gid < 0) {
                    throw gcnew HDF5Exception("H5Gopen2 failed!");
                }

                String^ linkName = ProviderUtils::ChildName(h5path);
                link_name = (char*)(Marshal::StringToHGlobalAnsi(linkName)).ToPointer();

                if (H5Lexists(gid, link_name, H5P_DEFAULT) > 0)
                {
                    if (this->ShouldProcess(h5path,
                        String::Format("Removing HDF5 item '{0}'", path)))
                    {
                        if (H5Ldelete(gid, link_name, H5P_DEFAULT) >= 0)
                        {
                            if (H5Fflush(gid, H5F_SCOPE_LOCAL) < 0) {
                                throw gcnew Exception("H5Fflush failed!");
                            }
                        }
                        else {
                            throw gcnew Exception("H5Ldelete failed!!!");
                        }
                    }
                }
            }
        }
        finally
        {
            if (gid >= 0) {
                H5Gclose(gid);
            }

            if (link_name != NULL) {
                Marshal::FreeHGlobal(IntPtr(link_name));
            }

            if (group_path != NULL) {
                Marshal::FreeHGlobal(IntPtr(group_path));
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

        throw gcnew PSH5XException("HDF5Provider::RenameItem not implemented!");
            
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
