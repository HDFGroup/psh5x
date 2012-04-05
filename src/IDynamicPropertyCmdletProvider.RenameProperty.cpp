
#include "AttributeInfo.h"
#include "HDF5Exception.h"
#include "Provider.h"
#include "ProviderUtils.h"
#include "PSH5XException.h"

extern "C" {
#include "H5Gpublic.h"
#include "H5Spublic.h"
#include "H5Tpublic.h"
}

#include <cstdlib>

using namespace System;
using namespace System::Collections;
using namespace System::Collections::Generic;
using namespace System::Collections::ObjectModel;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Provider;
using namespace System::Runtime::InteropServices;
using namespace System::Web::Script::Serialization;

namespace PSH5X
{
    void Provider::RenameProperty(String^ path, String^ sourceProperty,
        String^ destinationProperty)
    {
        WriteVerbose(
            String::Format("HDF5Provider::RenameProperty(Path = '{0}',SourceProperty='{1}',DestinationProperty = '{2}')",
            path, sourceProperty, destinationProperty));

        hid_t oid = -1, aid = -1;

        char *old_attr_name = NULL, *new_attr_name = NULL, *obj_path = NULL;

        try
        {
#pragma region sanity check

            DriveInfo^ drive = nullptr;
            String^ h5path = nullptr;
            if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path)) {
                throw gcnew PSH5XException("Ill-formed HDF5 path name and/or unable to obtain drive name!");
            }

            if (drive->ReadOnly) {
                throw gcnew PSH5XException("The drive is read-only and cannot be modified!");
            }

            if (!ProviderUtils::IsResolvableH5Path(drive->FileHandle, h5path))
            {
                throw gcnew PSH5XException(String::Format("HDF5 object '{0}' not found", path));
            }

            old_attr_name = (char*)(Marshal::StringToHGlobalAnsi(sourceProperty)).ToPointer();
            new_attr_name = (char*)(Marshal::StringToHGlobalAnsi(destinationProperty)).ToPointer();

            obj_path = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

            oid = H5Oopen(drive->FileHandle, obj_path, H5P_DEFAULT);
            if (oid < 0) {
                throw gcnew HDF5Exception("H5Oopen failed!");
            }

            if (H5Aexists(oid, old_attr_name) <= 0) {
                throw gcnew PSH5XException(String::Format(
                    "HDF5 object '{0}' doesn't have an HDF5 attribute named '{1}'", path, sourceProperty));
            }

#pragma endregion

			if (this->ShouldProcess(h5path,
				String::Format("Renaming HDF5 attribute '{0}' to '{1}' for item '{2}'",
				sourceProperty, destinationProperty, path)))
			{
				if (H5Arename(oid, old_attr_name, new_attr_name) >= 0)
				{
					if (H5Fflush(oid, H5F_SCOPE_LOCAL) < 0) {
						throw gcnew HDF5Exception("H5Fflush failed!");
					}

					aid = H5Aopen(oid, new_attr_name, H5P_DEFAULT);
					if (aid < 0) {
						throw gcnew HDF5Exception("H5Aopen failed!");
					}

					Hashtable^ ht = ProviderUtils::H5Attribute(aid, destinationProperty);
					WritePropertyObject(ht, path);
				}
				else {
					throw gcnew HDF5Exception("H5Arename failed!");
				}
			}

        }
        finally
        {
            if (aid >= 0) {
                H5Aclose(aid);
            }

            if (oid >= 0) {
                H5Oclose(oid);
            }

            if (old_attr_name != NULL) {
                Marshal::FreeHGlobal(IntPtr(old_attr_name));
            }

            if (new_attr_name != NULL) {
                Marshal::FreeHGlobal(IntPtr(new_attr_name));
            }

            if (obj_path != NULL) {
                Marshal::FreeHGlobal(IntPtr(obj_path));
            }
        }

        return;
    }

    Object^ Provider::RenamePropertyDynamicParameters(String^ path, String^ sourceProperty,
        String^ destinationProperty)
    {
        WriteVerbose(
            String::Format("HDF5Provider::RenamePropertyDynamicParameters(Path = '{0}',SourceProperty='{1}',DestinationProperty = '{2}')",
            path, sourceProperty, destinationProperty));
        return nullptr;
    }   
}