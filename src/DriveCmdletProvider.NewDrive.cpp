
#include "DriveInfo.h"
#include "HDF5Exception.h"
#include "Provider.h"
#include "PSH5XException.h"

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
            if (dynamicParameters->ContainsKey("Mode") && dynamicParameters["Mode"]->Value != nullptr) {
                readonly = (dynamicParameters["Mode"]->Value->ToString()->ToUpper() != "RW");
			}

			H5F_libver_t ver = H5F_LIBVER_EARLIEST;

			if (dynamicParameters["Latest"]->IsSet && dynamicParameters["V18"]->IsSet) {
				throw gcnew PSH5XException("The -Latest and the -V18 options are incompatible. Use one of them!");
			}
			if (dynamicParameters["V18"]->IsSet) {
				ver = H5F_LIBVER_18;
			}
			if (dynamicParameters["Latest"]->IsSet) {
				ver = H5F_LIBVER_LATEST;
			}
			
            return gcnew DriveInfo(path, readonly, drive,
				dynamicParameters["Force"]->IsSet,
				dynamicParameters["Core"]->IsSet,
				ver);
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

        ParameterAttribute^ atts2 = gcnew ParameterAttribute();
        atts2->Mandatory = false;
        atts2->ValueFromPipeline = false;
        RuntimeDefinedParameter^ paramForce = gcnew RuntimeDefinedParameter();
        paramForce->Name = "Force";
        paramForce->ParameterType = SwitchParameter::typeid;
        paramForce->Attributes->Add(atts2);
        dynamicParameters->Add("Force", paramForce);
        
		ParameterAttribute^ atts3 = gcnew ParameterAttribute();
        atts3->Mandatory = false;
        atts3->ValueFromPipeline = false;
        RuntimeDefinedParameter^ paramCore = gcnew RuntimeDefinedParameter();
        paramCore->Name = "Core";
        paramCore->ParameterType = SwitchParameter::typeid;
        paramCore->Attributes->Add(atts3);
        dynamicParameters->Add("Core", paramCore);

		ParameterAttribute^ atts4 = gcnew ParameterAttribute();
        atts4->Mandatory = false;
        atts4->ValueFromPipeline = false;
        RuntimeDefinedParameter^ paramLatest = gcnew RuntimeDefinedParameter();
        paramLatest->Name = "Latest";
        paramLatest->ParameterType = SwitchParameter::typeid;
        paramLatest->Attributes->Add(atts4);
        dynamicParameters->Add("Latest", paramLatest);
        
		ParameterAttribute^ atts5 = gcnew ParameterAttribute();
        atts5->Mandatory = false;
        atts5->ValueFromPipeline = false;
        RuntimeDefinedParameter^ paramV18 = gcnew RuntimeDefinedParameter();
        paramV18->Name = "V18";
        paramV18->ParameterType = SwitchParameter::typeid;
        paramV18->Attributes->Add(atts5);
        dynamicParameters->Add("V18", paramV18);
        
        return dynamicParameters;
    }
}