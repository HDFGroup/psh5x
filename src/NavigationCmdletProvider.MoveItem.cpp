
#include "HDF5Exception.h"
#include "Provider.h"
#include "ProviderUtils.h"
#include "PSH5XException.h"

extern "C" {
#include "H5Gpublic.h"
#include "H5Ppublic.h"
}

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Provider;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
    void Provider::MoveItem(String^ path, String^ destination)
    {
        WriteVerbose(String::Format(
            "HDF5Provider::MoveItem(Path = '{0}', Destination = '{1}')", path, destination));
        
		hid_t lcpl = H5Pcreate(H5P_LINK_CREATE), sgid = -1, dgid = -1;

		char *src_path = NULL, *dst_path = NULL;
		
		try
		{
#pragma region sanity check

			DriveInfo^ drive = nullptr;
			String^ h5path = nullptr;
			if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path)) {
				throw gcnew PSH5XException("Ill-formed HDF5 path name and/or unable to obtain drive name!");
			}

			DriveInfo^ destDrive = nullptr;
			String^ destH5Path = nullptr;
			if (!ProviderUtils::TryGetDriveEtH5Path(destination, ProviderInfo, destDrive, destH5Path)) {
				throw gcnew PSH5XException("Ill-formed HDF5 path name and/or unable to obtain drive name!");
			}

			if (drive != destDrive) {
				throw gcnew PSH5XException("The source and destination must be in the same HDF5 file/H5Drive!");
			}

			// Does the source object exist?
			if (!ProviderUtils::IsResolvableH5Path(drive->FileHandle, h5path)) { 
				throw gcnew PSH5XException(String::Format("The source object '{0}' does not exist", path));
			}
			// Is the destination drive writable?
			if (destDrive->ReadOnly) {
				throw gcnew PSH5XException("The destination drive is read-only and cannot be modified!");
			}
			// Can we create the destination object(s)?
			if (ProviderUtils::CanCreateItemAt(destDrive->FileHandle, destH5Path) ||
				(Force && ProviderUtils::CanForceCreateItemAt(destDrive->FileHandle, destH5Path)))
			{
				// we are good
			}
			else if (ProviderUtils::IsResolvableH5Path(destDrive->FileHandle, destH5Path) &&
				ProviderUtils::IsH5Group(destDrive->FileHandle, destH5Path))
			{
				String^ checkH5path = destH5Path + "/" + ProviderUtils::ChildName(h5path);
				if (ProviderUtils::CanCreateItemAt(destDrive->FileHandle, checkH5path)) {
					destH5Path = checkH5path;
				}
				else {
					throw gcnew PSH5XException(String::Format("Unable to create the destination object '{0}'!", destination));
				}
			}
			else {
				throw gcnew PSH5XException(String::Format("Unable to create the destination object '{0}'!", destination));
			}

#pragma endregion

			if (Force) {
				if (H5Pset_create_intermediate_group(lcpl, 1) < 0) {
					throw gcnew HDF5Exception("H5Pset_create_intermediate_group failed!!!");
				}
			}

			src_path = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();
			dst_path = (char*)(Marshal::StringToHGlobalAnsi(destH5Path)).ToPointer();
			
			if (this->ShouldProcess(h5path, String::Format("Moving HDF5 item '{0}'", path)))
			{
				if (H5Lmove(drive->FileHandle, src_path, destDrive->FileHandle, dst_path,
					lcpl, H5P_DEFAULT) >= 0)
				{
					if (H5Fflush(drive->FileHandle, H5F_SCOPE_LOCAL) < 0) {
						throw gcnew Exception("H5Fflush failed!");
					}
				}
				else {
					throw gcnew Exception("H5Lmove failed!!!");
				}
			}
		}
		finally
		{
			if (dgid >= 0) {
				H5Gclose(dgid);
			}
			if (sgid >= 0) {
				H5Gclose(sgid);
			}
			if (dst_path != NULL) {
                Marshal::FreeHGlobal(IntPtr(dst_path));
            }
			if (src_path != NULL) {
                Marshal::FreeHGlobal(IntPtr(src_path));
            }
			if (lcpl >= 0) {
                H5Pclose(lcpl);
            }
		}

        return;
    }

    Object^ Provider::MoveItemDynamicParameters(String^ path, String^ destination)
    {
        WriteVerbose(String::Format(
            "HDF5Provider::MoveItemDynamicParameters(Path = '{0}', Destination = '{1}')",
            path, destination));
        return nullptr;
    }

}