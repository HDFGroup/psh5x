
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

        Exception^ ex = nullptr;

        hid_t gid = -1, oid = -1, aid = -1;

        DriveInfo^ drive = nullptr;
        String^ h5path = nullptr;
        if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path))
        {
            ErrorRecord^ error = gcnew ErrorRecord(
                gcnew ArgumentException("Ill-formed HDF5 path name and/or unable to obtain drive name!"),
                "InvalidData", ErrorCategory::InvalidData, nullptr);
            ThrowTerminatingError(error);
        }

        String^ groupPath = ProviderUtils::ParentPath(h5path);
        
        char* group_path = (char*)(Marshal::StringToHGlobalAnsi(groupPath)).ToPointer();
        
        gid = H5Gopen2(drive->FileHandle, group_path, H5P_DEFAULT);
        if (gid < 0) {
            ex = gcnew Exception("H5Gopen2 failed!!!");
            goto error;
        }

        String^ linkName = ProviderUtils::ChildName(h5path);
        char* link_name = (char*)(Marshal::StringToHGlobalAnsi(linkName)).ToPointer();

        if (ProviderUtils::IsH5RootPathName(groupPath) || H5Lexists(gid, link_name, H5P_DEFAULT) > 0)
        {
            H5L_info_t info;
            info.type = H5L_TYPE_ERROR;

            if (ProviderUtils::IsH5RootPathName(groupPath) ||
                H5Lget_info(gid, link_name, &info, H5P_DEFAULT) >= 0)
            {
                if (ProviderUtils::IsH5RootPathName(groupPath) || info.type == H5L_TYPE_HARD)
                {
                    char* ipath =  (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

                    oid = H5Oopen(drive->FileHandle, ipath, H5P_DEFAULT);
                    if (oid < 0) {
                        ex = gcnew Exception("H5Oopen failed!!!");
                        goto error;
                    }
                    
                    // If more than one property is requested, we return a hashtable of hashtables
                    // keyed by property name.
                    // If only one property is requested, we return only one hashtable.

                    Hashtable^ htAllAttributes = gcnew Hashtable();

                    for each (String^ attributeName in providerSpecificPickList)
                    {
                        char* attr_name = (char*)(Marshal::StringToHGlobalAnsi(attributeName)).ToPointer();
                        htri_t flag = H5Aexists(oid, attr_name);

                        if (flag > 0)
                        {
                            aid = H5Aopen(oid, attr_name, H5P_DEFAULT);
                            if (aid < 0) {
                                ex = gcnew Exception("H5Aopen failed!!!");
                                goto error;
                            }

                            if (providerSpecificPickList->Count == 1)
                            {
                                htAllAttributes =
                                    ProviderUtils::H5Attribute(aid, attributeName);
                            }
                            else
                            {
                                htAllAttributes[attributeName] =
                                    ProviderUtils::H5Attribute(aid, attributeName);
                            }

                            if (H5Aclose(aid) < 0) {
                                ex = gcnew Exception("H5Aclose failed!!!");
                                goto error;
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

error:

        if (aid >= 0) {
            H5Aclose(aid);
        }

        if (oid >= 0) {
            H5Oclose(oid);
        }

        if (gid >= 0) {
            H5Gclose(gid);
        }

        if (ex != nullptr) {
            throw ex;
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