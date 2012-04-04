
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
	// TODO: RemoveItem, at the moment, is an unlink. Add an option that let's one remove ALL
	//       links to a given object. I'd have to iterate the entire file to pick up
	//       ALL pathnames and then carefully unlink them one-by-one.

    void Provider::RemoveItem(String^ path, bool recurse)
    {
        WriteVerbose(String::Format(
            "HDF5Provider::RemoveItem(Path = '{0}', Recurse = {1})", path, recurse));

        hid_t gid = -1;

        char *group_path = NULL, *link_name = NULL;

        try
        {
			/*
            if (recurse) {
                WriteWarning("The '-Recurse' option has no effect.");
            }
			*/

            DriveInfo^ drive = nullptr;
            String^ h5path = nullptr;
            if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path))
            {
                throw gcnew PSH5XException("Ill-formed HDF5 path name and/or unable to obtain drive name!");
            }

            if (drive->ReadOnly) {
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
                                throw gcnew HDF5Exception("H5Fflush failed!");
                            }
                        }
                        else {
                            throw gcnew HDF5Exception("H5Ldelete failed!!!");
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
}
