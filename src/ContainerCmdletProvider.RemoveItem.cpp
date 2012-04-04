
#include "HDF5Exception.h"
#include "Provider.h"
#include "ProviderUtils.h"
#include "PSH5XException.h"

extern "C" {
#include "H5Gpublic.h"
#include "H5Lpublic.h"
#include "H5Ppublic.h"
}

#include <iostream>
#include <string>
#include <vector>

using namespace std;

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

	herr_t H5LVisitCallback1(
        hid_t             group,
        const char*       name,
        const H5L_info_t* info,
        void*             op_data)
    {
        pair<haddr_t, vector<string> >* p = static_cast< pair<haddr_t,vector<string> >* >(op_data);
		if (info->u.address == p->first) {
			p->second.push_back(string(name));
		}
        return 0;
    }

    void Provider::RemoveItem(String^ path, bool recurse)
    {
        WriteVerbose(String::Format(
            "HDF5Provider::RemoveItem(Path = '{0}', Recurse = {1})", path, recurse));

        hid_t gid = -1, obj_id = -1;

        char *opath = NULL, *group_path = NULL, *link_name = NULL;

        try
        {
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

				// check if symbolic link -> easy

				// check if -Force and -Recurse (must be group then...)

				if (!recurse)
				{
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
				else 
				{
					// if this was a symlink, then there's nothing to do

					if (!ProviderUtils::IsH5SymLink(drive->FileHandle, h5path))
					{
						opath = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();
						if ((obj_id = H5Oopen(drive->FileHandle, opath, H5P_DEFAULT)) < 0) {
							throw gcnew HDF5Exception("H5Oopen failed!");
						}
						H5O_info_t info;
						if (H5Oget_info(obj_id, &info) < 0) {
							throw gcnew HDF5Exception("H5Oget_info failed!");
						}
						H5Oclose(obj_id);
						obj_id = -1;

						unsigned long fileno = info.fileno;
						haddr_t addr = info.addr;

						pair<haddr_t, vector<string> > op_data;
						op_data.first = info.addr;

						do
						{
							op_data.second.clear();

							if(H5Lvisit(drive->FileHandle, H5_INDEX_NAME, H5_ITER_NATIVE, &H5LVisitCallback1, (void*) &op_data) >= 0)
							{
								for (size_t i = 0; i < op_data.second.size(); ++i)
								{
									if (H5Ldelete(drive->FileHandle, op_data.second[i].c_str(), H5P_DEFAULT) >= 0)
									{
										if (H5Fflush(drive->FileHandle, H5F_SCOPE_LOCAL) < 0) {
											throw gcnew HDF5Exception("H5Fflush failed!");
										}
									}
									else {
										throw gcnew HDF5Exception("H5Ldelete failed!!!");
									}
								}
							}
							else {
								throw gcnew HDF5Exception("H5Lvisit failed!");
							}
						}
						while (op_data.second.size() > 0);
					}
				}
            }
        }
        finally
        {
			if (obj_id >= 0) {
                H5Oclose(obj_id);
            }
            if (gid >= 0) {
                H5Gclose(gid);
            }
            if (link_name != NULL) {
                Marshal::FreeHGlobal(IntPtr(link_name));
            }
            if (group_path != NULL) {
                Marshal::FreeHGlobal(IntPtr(group_path));
            }
			if (opath != NULL) {
                Marshal::FreeHGlobal(IntPtr(opath));
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
