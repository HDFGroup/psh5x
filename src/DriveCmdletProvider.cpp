
#include "DriveInfo.h"
#include "Provider.h"

extern "C" {
#include "H5Fpublic.h"
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

        Collection<System::Management::Automation::PSDriveInfo^>^ coll =
            gcnew Collection<System::Management::Automation::PSDriveInfo^>();

        // create a writeable sandbox drive h5tmp:

        String^ tmpFile = System::IO::Path::GetTempFileName();
        
        char* tmp_file = (char*)(Marshal::StringToHGlobalAnsi(tmpFile)).ToPointer();
        hid_t status = H5Fcreate(tmp_file, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
        if (status < 0)
        {
            String^ msg = String::Format(
                "H5Fcreate failed with status {0} for file '{1}'",
                status, tmpFile);
            throw gcnew Exception(msg); 
        }
        else
        {
            if (H5Fclose(status) >= 0)
            {
                System::Management::Automation::PSDriveInfo^ info =
                    gcnew System::Management::Automation::PSDriveInfo("h5tmp",
                    __super::ProviderInfo, "h5tmp:\\", "HDF5 sandbox drive (no dogs allowed)",
                    __super::Credential);

                DriveInfo^ drive = gcnew DriveInfo(tmpFile, false, info, false);
                coll->Add(drive);
                Environment::SetEnvironmentVariable("PSH5XTmpFile", tmpFile);
            }
            else
            {
                String^ msg = String::Format(
                    "H5Fclose failed with for name {0}", tmpFile);
                throw gcnew Exception(msg);
            }
        }

        return coll;
    }

    PSDriveInfo^ Provider::NewDrive(
        System::Management::Automation::PSDriveInfo^ drive)
    {
        WriteVerbose("HDF5Provider::NewDrive()");
        
        // dynamicParameters is initialized ONLY when invoked via New-PSDrive

        RuntimeDefinedParameterDictionary^ dynamicParameters =
            (RuntimeDefinedParameterDictionary^) DynamicParameters;

        if (dynamicParameters != nullptr)
        {
            String^ path = dynamicParameters["Path"]->Value->ToString();

            bool readonly = true;
            if (dynamicParameters["Mode"]->Value != nullptr)
                readonly = (dynamicParameters["Mode"]->Value->ToString()->ToUpper() == "RW");

            return gcnew DriveInfo(path, readonly, drive, dynamicParameters["Force"]->IsSet);
        }
        else
        {
            return drive;
        }
    }

    Object^ Provider::NewDriveDynamicParameters()
    {
        WriteVerbose("HDF5Provider::NewDriveDynamicParameters()");
        
        RuntimeDefinedParameterDictionary^ dynamicParameters =
            gcnew RuntimeDefinedParameterDictionary();

        Collection<Attribute^>^ atts = gcnew Collection<Attribute^>();
        ParameterAttribute^ paramAttr = gcnew ParameterAttribute();
        paramAttr->Mandatory = true;
        atts->Add(paramAttr);
        dynamicParameters->Add("Path",
            gcnew RuntimeDefinedParameter("Path", String::typeid, atts));

        Collection<Attribute^>^ atts1 = gcnew Collection<Attribute^>();
        ParameterAttribute^ paramAttr1 = gcnew ParameterAttribute();
        paramAttr1->Mandatory = false;
        atts1->Add(paramAttr1);
        dynamicParameters->Add("Mode",
            gcnew RuntimeDefinedParameter("Mode", String::typeid, atts1));

        ParameterAttribute^ attr5 = gcnew ParameterAttribute();
        attr5->Mandatory = false;
        attr5->ValueFromPipeline = false;

        RuntimeDefinedParameter^ paramForce = gcnew RuntimeDefinedParameter();
        paramForce->Name = "Force";
        paramForce->ParameterType = SwitchParameter::typeid;
        paramForce->Attributes->Add(attr5);

        dynamicParameters->Add("Force", paramForce);
        
        return dynamicParameters;
    }

    PSDriveInfo^ Provider::RemoveDrive(
        System::Management::Automation::PSDriveInfo^ drive)
    {
        WriteVerbose("HDF5Provider::RemoveDrive()");

        DriveInfo^ h5drive = (DriveInfo^) drive;
        herr_t status = H5Fclose(h5drive->FileHandle);
        if (status < 0)
        {
            String^ msg = String::Format("H5close failed with status {0}", status);
            ErrorRecord^ error = gcnew ErrorRecord(gcnew Exception(msg),
                "CloseError", ErrorCategory::CloseError, status);
            WriteError(error);
        }
        h5drive->FileHandle = -1;
        
        return __super::RemoveDrive(drive);
    }
}