
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
using namespace System::Collections;
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
        
		hid_t ocpypl= H5Pcreate(H5P_OBJECT_COPY), lcpl = H5Pcreate(H5P_LINK_CREATE), oid = -1;

		char *path_str = NULL, *copy_path_str = NULL;
		
		try
		{
#pragma region sanity check

			DriveInfo^ drive = nullptr;
            String^ h5path = nullptr;
            if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path)) {
                throw gcnew PSH5XException("Ill-formed HDF5 path name and/or unable to obtain drive name!");
            }

			DriveInfo^ copyDrive = nullptr;
            String^ copyH5Path = nullptr;
            if (!ProviderUtils::TryGetDriveEtH5Path(copyPath, ProviderInfo, copyDrive, copyH5Path)) {
                throw gcnew PSH5XException("Ill-formed HDF5 path name and/or unable to obtain drive name!");
            }

			// Does the source object exist?
			if (!ProviderUtils::IsResolvableH5Path(drive->FileHandle, h5path)) {
				throw gcnew PSH5XException(String::Format("The source object '{0}' does not exist", path));
			}
			// Is the destination drive writable?
			if (copyDrive->ReadOnly) {
                throw gcnew PSH5XException("The destination drive is read-only and cannot be modified!");
            }
			// Can we create the destination object(s)?
			if (!(ProviderUtils::CanCreateItemAt(copyDrive->FileHandle, copyH5Path) ||
				(Force && ProviderUtils::CanForceCreateItemAt(copyDrive->FileHandle, copyH5Path))))	{
				throw gcnew PSH5XException(String::Format("The destination object '{0}' cannot be created!", copyPath));
			}

#pragma endregion

			if (Force) {
				if (H5Pset_create_intermediate_group(lcpl, 1) < 0) {
					throw gcnew HDF5Exception("H5Pset_create_intermediate_group failed!!!");
				}
			}

			if (!recurse) {
				if(H5Pset_copy_object(ocpypl, H5O_COPY_SHALLOW_HIERARCHY_FLAG) < 0) {
					throw gcnew HDF5Exception("H5Pset_copy_object failed!!!");
				}
			}

			bool ignoreAttributes = false;
            RuntimeDefinedParameterDictionary^ dynamicParameters =
                (RuntimeDefinedParameterDictionary^) DynamicParameters;
            if (dynamicParameters != nullptr && dynamicParameters->ContainsKey("IgnoreAttributes")) {
                ignoreAttributes = dynamicParameters["IgnoreAttributes"]->IsSet;
            }
			if (ignoreAttributes) {
				if(H5Pset_copy_object(ocpypl, H5O_COPY_WITHOUT_ATTR_FLAG) < 0) {
					throw gcnew HDF5Exception("H5Pset_copy_object failed!!!");
				}
			}

			path_str = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();
			copy_path_str = (char*)(Marshal::StringToHGlobalAnsi(copyH5Path)).ToPointer();

			if (H5Ocopy(drive->FileHandle, path_str, copyDrive->FileHandle, copy_path_str, ocpypl, lcpl) >= 0)
			{
				if (H5Fflush(copyDrive->FileHandle, H5F_SCOPE_LOCAL) >= 0)
				{
					oid = H5Oopen(copyDrive->FileHandle, copy_path_str, H5P_DEFAULT);
					if (oid >= 0) {
						WriteItemObject(gcnew ObjectInfo(oid), copyPath,
							ProviderUtils::IsH5Group(copyDrive->FileHandle, copyH5Path));
					}
					else {
						throw gcnew HDF5Exception("H5Oopen failed!!!");
					}
				}
				else {
					throw gcnew HDF5Exception("H5Fflush failed!!!");
				}
			}
			else {
				throw gcnew HDF5Exception("H5Ocopy failed!!!");
			}
		}
		finally
		{
			if (ocpypl >= 0) {
                H5Pclose(ocpypl);
            }

			if (lcpl >= 0) {
                H5Pclose(lcpl);
            }

			if (path_str != NULL) {
                Marshal::FreeHGlobal(IntPtr(path_str));
            }

			if (copy_path_str != NULL) {
                Marshal::FreeHGlobal(IntPtr(copy_path_str));
            }

			if (oid >= 0) {
                H5Oclose(oid);
            }
		}

        return;
    }

    Object^ Provider::CopyItemDynamicParameters(String^ path, String^ copyPath, bool recurse)
    {
        WriteVerbose(String::Format(
            "HDF5Provider::CopyItemDynamicParameters(Path = '{0}', CopyPath = '{1}', Recurse = {2})",
            path, copyPath, recurse));

        RuntimeDefinedParameterDictionary^ dynamicParameters =
            gcnew RuntimeDefinedParameterDictionary();

        Collection<Attribute^>^ atts1 = gcnew Collection<Attribute^>();
        ParameterAttribute^ paramAttr1 = gcnew ParameterAttribute();
        paramAttr1->Mandatory = false;
        paramAttr1->ValueFromPipeline = false;
        atts1->Add(paramAttr1);
        dynamicParameters->Add("IgnoreAttributes",
            gcnew RuntimeDefinedParameter("IgnoreAttributes", SwitchParameter::typeid, atts1));
        
        return dynamicParameters;
    }

    void Provider::GetChildItems(System::String^ path, bool recurse)
    {
        WriteVerbose(String::Format("HDF5Provider::GetChildItems(Path = '{0}', recurse = '{1}')",
            path, recurse));

        hid_t gid = -1, oid = -1;

        char *gpath = NULL, *link_name = NULL;

#pragma region item type filter

        Hashtable dropItem = gcnew Hashtable();
        dropItem["D"] = false;
        dropItem["EL"] = false;
        dropItem["G"] = false;
        dropItem["SL"] = false;
        dropItem["T"] = false;

        if (Filter != nullptr)
        {
            String^ s = String::Copy(Filter)->Trim()->ToUpper();

            if (!s->StartsWith("-") && s->Contains("-")) {
                throw gcnew PSH5XException("Invalid filter expression!");
            }

            if (s->StartsWith("-"))
            {                
                array<wchar_t>^ a = s->Substring(1)->ToCharArray();
                for (int i = 0; i < a->Length; ++i)
                {
                    if (a[i] == 'D') {
                        dropItem["D"] = true;
                    }
                    else if (a[i] == 'E') {
                        dropItem["EL"] = true;
                    }
                    else if (a[i] == 'G') {
                        dropItem["G"] = true;
                    }
                    else if (a[i] == 'S') {
                        dropItem["SL"] = true;
                    }
                    else if (a[i] == 'T') {
                        dropItem["T"] = true;
                    }
                }
            }
            else
            {
                dropItem["D"] = true;
                dropItem["EL"] = true;
                dropItem["G"] = true;
                dropItem["SL"] = true;
                dropItem["T"] = true;

                array<wchar_t>^ a = s->ToCharArray();
                for (int i = 0; i < a->Length; ++i)
                {
                    if (a[i] == 'D') {
                        dropItem["D"] = false;
                    }
                    else if (a[i] == 'E') {
                        dropItem["EL"] = false;
                    }
                    else if (a[i] == 'G') {
                        dropItem["G"] = false;
                    }
                    else if (a[i] == 'S') {
                        dropItem["SL"] = false;
                    }
                    else if (a[i] == 'T') {
                        dropItem["T"] = false;
                    }
                }
            }
        }

#pragma endregion

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
                                    
                                    if ((bool) dropItem["G"]) {
                                        break;
                                    }

                                    if (!detailed) {
                                        WriteItemObject(gcnew GroupInfoLite(oid), childPath,
                                            true);
                                    }
                                    else {
                                        WriteItemObject(gcnew GroupInfo(oid), childPath, true);
                                    }
                                    break;

                                case H5O_TYPE_DATASET:

                                    if ((bool) dropItem["D"]) {
                                        break;
                                    }

                                    if (!detailed) {
                                        WriteItemObject(gcnew DatasetInfoLite(oid), childPath,
                                            false);
                                    }
                                    else {
                                        WriteItemObject(gcnew DatasetInfo(oid), childPath,
                                            false);
                                    }
                                    break;

                                case H5O_TYPE_NAMED_DATATYPE:

                                    if ((bool) dropItem["T"]) {
                                        break;
                                    }

                                    WriteItemObject(gcnew DatatypeInfo(oid), childPath, false);
                                    break;

                                default:

                                    WriteItemObject(gcnew ObjectInfo(oid), childPath, false);
                                    break;
                                }
                            }
                            else {
                                throw gcnew HDF5Exception("H5Oget_info failed!");
                            }

                            if (H5Oclose(oid) < 0) {
                                throw gcnew HDF5Exception("H5Oclose failed!");
                            }
                            oid = -1;

#pragma endregion

                            break;

                        case H5L_TYPE_SOFT:

                            if ((bool) dropItem["SL"]) {
                                break;
                            }

                            WriteItemObject(gcnew LinkInfo(gid, linkName, "SoftLink"),
                                childPath, false);
                            
                            break;

                        case H5L_TYPE_EXTERNAL:
                            
                            if ((bool) dropItem["EL"]) {
                                break;
                            }

                            WriteItemObject(gcnew LinkInfo(gid, linkName, "ExtLink"),
                                childPath, false);
                            
                            break;

                        default:

                            throw gcnew PSH5XException("Unable to determine the item type for this path!");
                            break;
                        }
                    }
                    else {
                        throw gcnew HDF5Exception("H5Lget_info failed!!!");
                    }

                    Marshal::FreeHGlobal(IntPtr(link_name));
                    link_name = NULL;
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


	// TODO: RemoveItem, at the moment, is an unlink. Add an option that let's one remove ALL
	//       links to a given object.

    void Provider::RemoveItem(String^ path, bool recurse)
    {
        WriteVerbose(String::Format(
            "HDF5Provider::RemoveItem(Path = '{0}', Recurse = {1})", path, recurse));

        hid_t gid = -1;

        char *group_path = NULL, *link_name = NULL;

        try
        {
            if (recurse) {
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
        
        if (newName->Contains("/") || newName->Contains("\\")) {
            throw gcnew PSH5XException("The new link name must not contain forward- or backslashes!");
        }

        hid_t gid = -1;

        char *group_path = NULL, *link_name = NULL, *new_name = NULL;

        try
        {
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
                throw gcnew PSH5XException(String::Format("Cannot rename the root group '{0}'", h5path));
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
                new_name = (char*)(Marshal::StringToHGlobalAnsi(newName)).ToPointer();

                if (H5Lexists(gid, link_name, H5P_DEFAULT) > 0)
                {
                    if (this->ShouldProcess(h5path,
                        String::Format("Renaming HDF5 item '{0}'", path)))
                    {
                        if (H5Lmove(gid, link_name, gid, new_name, H5P_DEFAULT, H5P_DEFAULT) >= 0)
                        {
                            if (H5Fflush(gid, H5F_SCOPE_LOCAL) < 0) {
                                throw gcnew Exception("H5Fflush failed!");
                            }
                        }
                        else {
                            throw gcnew Exception("H5Lmove failed!!!");
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

            if (new_name != NULL) {
                Marshal::FreeHGlobal(IntPtr(new_name));
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

    Object^ Provider::RenameItemDynamicParameters(String^ path, String^ newName)
    {
        WriteVerbose(String::Format(
            "HDF5Provider::RenameItemDynamicParameters(Path = '{0}', NewName = '{1}')",
            path, newName));
        return nullptr;
    }

}
