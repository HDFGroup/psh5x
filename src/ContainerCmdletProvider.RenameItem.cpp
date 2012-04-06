
#include "HDF5Exception.h"
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
                                throw gcnew HDF5Exception("H5Fflush failed!");
                            }

							// support -PassThru

							String^ newpath = nullptr;
							if (path->EndsWith(linkName))
							{
								int idx = path->LastIndexOf(linkName);
								newpath = String::Concat(path->Substring(0, idx), newName);
							}
							else {
								throw gcnew PSH5XException("Can't find link name in path!");
							}

							bool isContainer = false;
							ItemInfo^ iinfo = passThru(gid, newName, isContainer);
							WriteItemObject(iinfo, newpath, isContainer);
                        }
                        else {
                            throw gcnew HDF5Exception("H5Lmove failed!!!");
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
