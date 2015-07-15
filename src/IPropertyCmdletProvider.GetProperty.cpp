
#include "HDF5Exception.h"
#include "Provider.h"
#include "ProviderUtils.h"
#include "PSH5XException.h"

extern "C" {
#include "H5Apublic.h"
#include "H5Gpublic.h"
#include "H5Spublic.h"
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
    void Provider::GetProperty(String^ path, Collection<String^>^ providerSpecificPickList)
    {
        WriteVerbose(String::Format("HDF5Provider::GetProperty(Path = '{0}')", path));

		hid_t oid = -1, aid = -1;

        char *path_str = NULL, *attr_name = NULL;

        try
        {
#pragma region sanity check

			DriveInfo^ drive = nullptr;
			String^ h5path = nullptr;
			if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path))
			{
				throw gcnew PSH5XException("Ill-formed HDF5 path name and/or unable to obtain drive name!");
			}
			if (!ProviderUtils::IsResolvableH5Path(drive->FileHandle, h5path))
			{
				throw gcnew PSH5XException(String::Format("HDF5 object '{0}' not found", path));
			}

#pragma endregion

			path_str =  (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

			oid = H5Oopen(drive->FileHandle, path_str, H5P_DEFAULT);
			if (oid < 0) {
				throw gcnew HDF5Exception("H5Oopen failed!");
			}

			for each (String^ attributeName in providerSpecificPickList)
			{
				attr_name = (char*)(Marshal::StringToHGlobalAnsi(attributeName)).ToPointer();

				if (H5Aexists(oid, attr_name) > 0)
				{
					aid = H5Aopen(oid, attr_name, H5P_DEFAULT);
					if (aid < 0) {
						throw gcnew HDF5Exception("H5Aopen failed!");
					}

					WritePropertyObject(ProviderUtils::H5Attribute(aid, attributeName, drive->FileHandle), path);

					if (H5Aclose(aid) < 0) {
						throw gcnew HDF5Exception("H5Aclose failed!");
					}
					else {
						aid = -1;
					}
				}
				else
				{
					WriteWarning(
						String::Format("Property name '{0}' doesn't exist for item at path '{1}'",
						attributeName, path));
				}

				if (attr_name != NULL) {
					Marshal::FreeHGlobal(IntPtr(attr_name));
					attr_name = NULL;
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
            if (attr_name != NULL) {
                Marshal::FreeHGlobal(IntPtr(attr_name));
            }
            if (path_str != NULL) {
                Marshal::FreeHGlobal(IntPtr(path_str));
            }
        }

        return;
    }

    Object^ Provider::GetPropertyDynamicParameters(String^ path,
        Collection<String^>^ providerSpecificPickList)
    {
        WriteVerbose(String::Format("HDF5Provider::GetPropertyDynamicParameters(Path = '{0}')", path));
        return nullptr;
    }
}