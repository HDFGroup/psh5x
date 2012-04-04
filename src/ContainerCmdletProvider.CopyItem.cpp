
#include "HDF5Exception.h"
#include "LinkInfo.h"
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
    void Provider::CopyItem(String^ path, String^ copyPath, bool recurse)
    {
        WriteVerbose(String::Format(
            "HDF5Provider::CopyItem(Path = '{0}', CopyPath = '{1}', Recurse = {2})",
            path, copyPath, recurse));
        
		hid_t ocpypl= H5Pcreate(H5P_OBJECT_COPY), lcpl = H5Pcreate(H5P_LINK_CREATE), oid = -1;

		char *path_str = NULL, *copy_path_str = NULL;
		
		try
		{
			RuntimeDefinedParameterDictionary^ dynamicParameters =
                (RuntimeDefinedParameterDictionary^) DynamicParameters;

			bool ignoreAttributes = false;
            if (dynamicParameters != nullptr && dynamicParameters->ContainsKey("IgnoreAttributes")) {
                ignoreAttributes = dynamicParameters["IgnoreAttributes"]->IsSet;
            }
			bool copyLink = false;
            if (dynamicParameters != nullptr && dynamicParameters->ContainsKey("copyLink")) {
                copyLink = dynamicParameters["copyLink"]->IsSet;
            }

			if (copyLink && (ignoreAttributes || recurse)) {
				WriteWarning("The -IgnoreAttributes and -Recurse switches have no effect when -CopyLink is specified!");
			}

#pragma region sanity check

			DriveInfo^ drive = nullptr;
            String^ h5path = nullptr;
            if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path)) {
                throw gcnew PSH5XException("Ill-formed HDF5 path name and/or unable to obtain drive name!");
            }

			DriveInfo^ copyDrive = nullptr;
            String^ copyH5Path = nullptr;
            if (!ProviderUtils::TryGetDriveEtH5Path(copyPath, ProviderInfo, copyDrive, copyH5Path)) {
                throw gcnew PSH5XException("Ill-formed HDF5 path name and/or unable to obtain drive name!");
            }

			if (!copyLink) {
				// Does the source object exist?
				if (!ProviderUtils::IsResolvableH5Path(drive->FileHandle, h5path)) { 
					throw gcnew PSH5XException(String::Format("The source object '{0}' does not exist", path));
				}
			}
			else {
				if (!ProviderUtils::IsValidH5Path(drive->FileHandle, h5path)) { 
					throw gcnew PSH5XException(String::Format("The source link '{0}' does not exist", path));
				}
			}

			// Is the destination drive writable?
			if (copyDrive->ReadOnly) {
                throw gcnew PSH5XException("The destination drive is read-only and cannot be modified!");
            }
			// Can we create the destination object(s)?
			if (ProviderUtils::CanCreateItemAt(copyDrive->FileHandle, copyH5Path) ||
				(Force && ProviderUtils::CanForceCreateItemAt(copyDrive->FileHandle, copyH5Path)))
			{
				// we are good
			}
			else if (ProviderUtils::IsResolvableH5Path(copyDrive->FileHandle, copyH5Path) &&
				ProviderUtils::IsH5Group(copyDrive->FileHandle, copyH5Path))
			{
					String^ checkH5path = copyH5Path + "/" + ProviderUtils::ChildName(h5path);
					if (ProviderUtils::CanCreateItemAt(copyDrive->FileHandle, checkH5path)) {
						copyH5Path = checkH5path;
					}
					else {
						throw gcnew PSH5XException(String::Format("Unable to create the destination object '{0}'!", copyPath));
					}
			}
			else {
				throw gcnew PSH5XException(String::Format("Unable to create the destination object '{0}'!", copyPath));
			}

#pragma endregion

			if (Force) {
				if (H5Pset_create_intermediate_group(lcpl, 1) < 0) {
					throw gcnew HDF5Exception("H5Pset_create_intermediate_group failed!");
				}
			}
			if (!recurse) {
				if(H5Pset_copy_object(ocpypl, H5O_COPY_SHALLOW_HIERARCHY_FLAG) < 0) {
					throw gcnew HDF5Exception("H5Pset_copy_object failed!");
				}
			}
			if (ignoreAttributes) {
				if(H5Pset_copy_object(ocpypl, H5O_COPY_WITHOUT_ATTR_FLAG) < 0) {
					throw gcnew HDF5Exception("H5Pset_copy_object failed!");
				}
			}

			path_str = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();
			copy_path_str = (char*)(Marshal::StringToHGlobalAnsi(copyH5Path)).ToPointer();

			if (this->ShouldProcess(h5path, String::Format("Copying HDF5 item '{0}'", path)))
			{
				if (!copyLink)
				{
					if (H5Ocopy(drive->FileHandle, path_str, copyDrive->FileHandle, copy_path_str, ocpypl, lcpl) >= 0)
					{
						if (H5Fflush(copyDrive->FileHandle, H5F_SCOPE_LOCAL) >= 0)
						{
							oid = H5Oopen(copyDrive->FileHandle, copy_path_str, H5P_DEFAULT);
							if (oid >= 0) {
								WriteItemObject(gcnew ObjectInfo(oid), copyPath,
									ProviderUtils::IsH5Group(copyDrive->FileHandle, copyH5Path));
							}
							else {
								throw gcnew HDF5Exception("H5Oopen failed!");
							}
						}
						else {
							throw gcnew HDF5Exception("H5Fflush failed!");
						}
					}
					else {
						throw gcnew HDF5Exception("H5Ocopy failed!");
					}
				}
				else 
				{
					if (H5Lcopy(drive->FileHandle, path_str, copyDrive->FileHandle, copy_path_str, lcpl,
						H5P_DEFAULT) >= 0)
					{
						if (H5Fflush(copyDrive->FileHandle, H5F_SCOPE_LOCAL) >= 0) {
							WriteItemObject(gcnew LinkInfo(copyDrive->FileHandle, copyH5Path), copyPath, false);
						}
						else {
							throw gcnew HDF5Exception("H5Fflush failed!");
						}
					}
					else {
						throw gcnew HDF5Exception("H5Lcopy failed!");
					}
				}
			}
		}
		finally
		{
			if (ocpypl >= 0) {
                H5Pclose(ocpypl);
            }

			if (lcpl >= 0) {
                H5Pclose(lcpl);
            }

			if (path_str != NULL) {
                Marshal::FreeHGlobal(IntPtr(path_str));
            }

			if (copy_path_str != NULL) {
                Marshal::FreeHGlobal(IntPtr(copy_path_str));
            }

			if (oid >= 0) {
                H5Oclose(oid);
            }
		}

        return;
    }

    Object^ Provider::CopyItemDynamicParameters(String^ path, String^ copyPath, bool recurse)
    {
        WriteVerbose(String::Format(
            "HDF5Provider::CopyItemDynamicParameters(Path = '{0}', CopyPath = '{1}', Recurse = {2})",
            path, copyPath, recurse));

        RuntimeDefinedParameterDictionary^ dynamicParameters =
            gcnew RuntimeDefinedParameterDictionary();

        Collection<Attribute^>^ atts1 = gcnew Collection<Attribute^>();
        ParameterAttribute^ paramAttr1 = gcnew ParameterAttribute();
        paramAttr1->Mandatory = false;
        paramAttr1->ValueFromPipeline = false;
        atts1->Add(paramAttr1);
        dynamicParameters->Add("IgnoreAttributes",
            gcnew RuntimeDefinedParameter("IgnoreAttributes", SwitchParameter::typeid, atts1));
        
		Collection<Attribute^>^ atts2 = gcnew Collection<Attribute^>();
        ParameterAttribute^ paramAttr2 = gcnew ParameterAttribute();
        paramAttr2->Mandatory = false;
        paramAttr2->ValueFromPipeline = false;
        atts2->Add(paramAttr2);
        dynamicParameters->Add("CopyLink",
            gcnew RuntimeDefinedParameter("CopyLink", SwitchParameter::typeid, atts2));
        
        return dynamicParameters;
    }
}
