
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
#include "H5Apublic.h"
#include "H5Gpublic.h"
#include "H5Lpublic.h"
#include "H5Opublic.h"
#include "H5Spublic.h"
}

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Collections::ObjectModel;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Provider;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
    void Provider::ClearItem(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::ClearItem(Path = '{0}')", path));
        WriteWarning("The HDF5Provider::ClearItem() not implemented!!!");
        return;
    }

    Object^ Provider::ClearItemDynamicParameters(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::ClearItemDynamicParameters(Path = '{0}')", path));
        return nullptr;
    }

    bool Provider::IsValidPath(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::IsValidPath(Path = '{0}')", path));
        
        String^ npath = ProviderUtils::NormalizePath(path);
        String^ h5path = ProviderUtils::PathNoDrive(npath);

        return ProviderUtils::IsWellFormedH5Path(h5path);
    }

    void Provider::InvokeDefaultAction(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::InvokeDefaultAction(Path = '{0}')", path));

        throw gcnew NotImplementedException("HDF5Provider::InvokeDefaultAction not implemented!");

        return;
    }

    Object^ Provider::InvokeDefaultActionDynamicParameters(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::InvokeDefaultActionDynamicParameters(Path = '{0}')", path));
        return nullptr;
    }

	// ItemExists means HDF5 path is in use
	// Use the -Resolvable to determine if it can be reolved to and HDF5 object

    bool Provider::ItemExists(System::String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::ItemExists(Path = '{0}')",path));

		DriveInfo^ drive = nullptr;
        String^ h5path = nullptr;

        if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path)) {
            throw gcnew PSH5XException("Ill-formed HDF5 path name and/or unable to obtain drive name!");
        }

		bool resolvableCheck = false;
		RuntimeDefinedParameterDictionary^ dynamicParameters =
			(RuntimeDefinedParameterDictionary^) DynamicParameters;
		if (dynamicParameters != nullptr && dynamicParameters->ContainsKey("Resolvable")) {
			resolvableCheck = dynamicParameters["Resolvable"]->IsSet;
		}

		if (!resolvableCheck) {
			return ProviderUtils::IsValidH5Path(drive->FileHandle, h5path);
		}
		else {
			return ProviderUtils::IsResolvableH5Path(drive->FileHandle, h5path);
		}
    }

    Object^ Provider::ItemExistsDynamicParameters(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::ItemExistsDynamicParameters(Path = '{0}')", path));
        
        // add -itemType optional parameter so that existence and item type can be tested
        // e.g., Test-Path HDF5::h5:/foo/bar -itemType Group

        RuntimeDefinedParameterDictionary^ dynamicParameters =
            gcnew RuntimeDefinedParameterDictionary();

        Collection<Attribute^>^ atts = gcnew Collection<Attribute^>();
        ParameterAttribute^ paramAttr = gcnew ParameterAttribute();
        paramAttr->Mandatory = false;
        paramAttr->ValueFromPipeline = false;
        atts->Add(paramAttr);
        dynamicParameters->Add("Resolvable",
            gcnew RuntimeDefinedParameter("Resolvable", SwitchParameter::typeid, atts));
        
        return dynamicParameters;
    }

}