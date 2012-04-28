
#include "DriveInfo.h"
#include "HDF5Exception.h"
#include "Provider.h"
#include "PSH5XException.h"

extern "C" {
#include "H5Fpublic.h"
#include "H5FDcore.h"
#include "H5Ppublic.h"
}

using namespace System;
using namespace System::Collections::ObjectModel;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Provider;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
    Collection<PSDriveInfo^>^ Provider::InitializeDefaultDrives()
    {
        WriteVerbose("HDF5Provider::InitializeDefaultDrives()");

        char* file_name = NULL;

        hid_t file_id = -1, fapl_id = -1;

        Collection<System::Management::Automation::PSDriveInfo^>^ coll =
            gcnew Collection<System::Management::Automation::PSDriveInfo^>();

        try
        {
            // create a writable sandbox drive h5tmp:

            String^ tmpFile = System::IO::Path::GetTempFileName();

            file_name = (char*)(Marshal::StringToHGlobalAnsi(tmpFile)).ToPointer();

			if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) {
			    throw gcnew HDF5Exception("H5Pcreate failed!");
			}
			if (H5Pset_fapl_core(fapl_id, 4096, 1) < 0) {
				throw gcnew HDF5Exception("H5Pset_fapl_core failed!");
			}

            file_id = H5Fcreate(file_name, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
            if (file_id < 0) {
                String^ msg = String::Format(
                    "H5Fcreate failed with status {0} for file '{1}'", file_id, tmpFile);
                throw gcnew HDF5Exception(msg);
            }
            else {
                if (H5Fclose(file_id) < 0) {
                    throw gcnew HDF5Exception("H5Fclose failed!"); 
                }
            }

            System::Management::Automation::PSDriveInfo^ info =
                gcnew System::Management::Automation::PSDriveInfo("h5tmp",
                __super::ProviderInfo, "h5tmp:\\", "HDF5 sandbox drive (no dogs allowed)",
                __super::Credential);

            if (info != nullptr)
            {
                DriveInfo^ drive = gcnew DriveInfo(tmpFile, false, info, false, true);
                if (drive != nullptr)
                {
                    coll->Add(drive);
                    Environment::SetEnvironmentVariable("PSH5XTmpFile", tmpFile);

                    ProviderInfo->Home = "h5tmp:\\";
                }
                else {
                    throw gcnew PSH5XException("Unable to create DriveInfo!");
                }
            }
            else {
                throw gcnew PSH5XException("Failed to construct System::Management::Automation::PSDriveInfo!");
            }
        }
        finally
        {
            if (file_name != NULL) {
                Marshal::FreeHGlobal(IntPtr(file_name));
            }

			if (fapl_id >= 0) {
				H5Pclose(fapl_id);
			}
        }

        return coll;
    }

    PSDriveInfo^ Provider::RemoveDrive(
        System::Management::Automation::PSDriveInfo^ drive)
    {
        WriteVerbose("HDF5Provider::RemoveDrive()");

        DriveInfo^ h5drive = (DriveInfo^) drive;

        try
        {
            herr_t status = H5Fclose(h5drive->FileHandle);
            if (status < 0)
            {
                String^ msg = String::Format("H5close failed with status {0}", status);
                throw gcnew HDF5Exception(msg);
            }
        }
        finally
        {
            h5drive->FileHandle = -1;
        }

        return __super::RemoveDrive(drive);
    }
}