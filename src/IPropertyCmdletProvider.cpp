
#include "Provider.h"
#include "ProviderUtils.h"

extern "C" {
#include "H5Apublic.h"
#include "H5Gpublic.h"
}

#include <cstdlib>

using namespace System;
using namespace System::Collections;
using namespace System::Collections::ObjectModel;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Provider;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
    void Provider::ClearProperty(String^ path, Collection<String^>^ propertyToClear)
    {
        WriteVerbose(String::Format("HDF5Provider::ClearProperty(Path = '{0}')", path));
        WriteWarning("The HDF5Provider::ClearProperty() method has not (yet) been implemented.");
        return;
    }

    Object^ Provider::ClearPropertyDynamicParameters(String^ path,
        Collection<String^>^ propertyToClear)
    {
        WriteVerbose(String::Format("HDF5Provider::ClearPropertyDynamicParameters(Path = '{0}')", path));
        return nullptr;
    }

    void Provider::GetProperty(String^ path, Collection<String^>^ providerSpecificPickList)
    {
        WriteVerbose(String::Format("HDF5Provider::GetProperty(Path = '{0}')", path));

        String^ npath = ProviderUtils::NormalizePath(path);
        String^ h5path = ProviderUtils::PathNoDrive(npath);
        if (!ProviderUtils::IsWellFormedH5Path(h5path)) { return; }
        DriveInfo^ drive = ProviderUtils::GetDriveFromPath(path, ProviderInfo);
        if (drive == nullptr) { return; }

        String^ groupPath = ProviderUtils::ParentPath(h5path);
        Console::WriteLine(groupPath);

        char* group_path = (char*)(Marshal::StringToHGlobalAnsi(groupPath)).ToPointer();
        hid_t group_id = H5Gopen2(drive->FileHandle, group_path, H5P_DEFAULT);

        String^ linkName = ProviderUtils::ChildName(h5path);
        Console::WriteLine(linkName);
        char* link_name = (char*)(Marshal::StringToHGlobalAnsi(linkName)).ToPointer();

        if (ProviderUtils::IsH5RootPathName(groupPath) || H5Lexists(group_id, link_name, H5P_DEFAULT) > 0)
        {
            H5L_info_t info;
            info.type = H5L_TYPE_ERROR;
            if (ProviderUtils::IsH5RootPathName(groupPath) ||
                H5Lget_info(group_id, link_name, &info, H5P_DEFAULT) >= 0)
            {
                if (ProviderUtils::IsH5RootPathName(groupPath) || info.type == H5L_TYPE_HARD)
                {
                    char* ipath =  (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();
                    hid_t obj_id = H5Oopen(drive->FileHandle, ipath, H5P_DEFAULT);
                    
                    // If more than one property is requested, we return a hashtable of hashtables
                    // keyed by property name.
                    // If only one property is requested, we return only one hashtable.

                    Hashtable^ htAllAttributes = gcnew Hashtable();

                    for each (String^ attributeName in providerSpecificPickList)
                    {
                        char* attr_name = (char*)(Marshal::StringToHGlobalAnsi(attributeName)).ToPointer();
                        htri_t flag = H5Aexists(obj_id, attr_name);
                        if (flag > 0)
                        {
                            hid_t attr_id = H5Aopen(obj_id, attr_name, H5P_DEFAULT);
                            if (attr_id >= 0)
                            {
                                if (providerSpecificPickList->Count == 1)
                                {
                                    htAllAttributes =
                                        ProviderUtils::H5Attribute(attr_id, attributeName);
                                }
                                else
                                {
                                    htAllAttributes[attributeName] =
                                        ProviderUtils::H5Attribute(attr_id, attributeName);
                                }

                                if (H5Aclose(attr_id) < 0) { // TODO
                                }
                            }
                            else { // TODO
                            }
                        }
                        else
                        {
                            WriteWarning(
                                String::Format("Property name '{0}' doesn't exist for item at path '{1}'",
                                attributeName, path));
                        }
                    }

                    WritePropertyObject(htAllAttributes, path);
                    if (H5Oclose(obj_id) < 0) { // TODO
                    }
                }
                else
                {
                    ErrorRecord^ error = gcnew ErrorRecord(
                        gcnew InvalidProgramException("The item type for this path has no attributes."),
                        "H5L_TYPE", ErrorCategory::InvalidData, nullptr);
                    WriteError(error);
                }
            }
            else { // TODO
            }
        }
        else
        {
            WriteWarning(String::Format("Item not found at Path = '{0}'", path));
        }
        
        if (H5Gclose(group_id) < 0) { // TODO
        }

        return;
    }

    Object^ Provider::GetPropertyDynamicParameters(String^ path,
        Collection<String^>^ providerSpecificPickList)
    {
        WriteVerbose(String::Format("HDF5Provider::GetPropertyDynamicParameters(Path = '{0}')", path));
        return nullptr;
    }

    void Provider::SetProperty(String^ path, PSObject^ propertyValue)
    {
        WriteVerbose(String::Format("HDF5Provider::SetProperty(Path = '{0}')", path));
        return;
    }

    Object^ Provider::SetPropertyDynamicParameters(String^ path, PSObject^ propertyValue)
    {
        WriteVerbose(String::Format("HDF5Provider::SetPropertyDynamicParameters(Path = '{0}')", path));

        ErrorRecord^ error = gcnew ErrorRecord(
            gcnew InvalidProgramException("HDF5Provider::SetPropertyDynamicParameters(...) not implemented!"),
            "HDF5Provider", ErrorCategory::InvalidData, nullptr);
        WriteError(error);
        
        return nullptr;
    }

}