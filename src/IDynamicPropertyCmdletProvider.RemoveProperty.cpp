
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
	void Provider::RemoveProperty(String^ path, String^ propertyName)
	{
		WriteVerbose(
			String::Format("HDF5Provider::RemoveProperty(Path = '{0}',PropertyName='{1}')",
			path, propertyName));

		hid_t oid = -1;

		char *obj_path = NULL, *attr_name = NULL;

		try
		{

#pragma region sanity check

			DriveInfo^ drive = nullptr;
			String^ h5path = nullptr;
			if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path))
			{
				throw gcnew PSH5XException("Ill-formed HDF5 path name and/or unable to obtain drive name!");
			}
			if (drive->ReadOnly)
			{
				throw gcnew PSH5XException("The drive is read-only and cannot be modified!");
			}
			if (!ProviderUtils::IsResolvableH5Path(drive->FileHandle, h5path))
			{
				throw gcnew PSH5XException(String::Format("HDF5 object'{0}' not found", path));
			}

			attr_name = (char*)(Marshal::StringToHGlobalAnsi(propertyName)).ToPointer();
			obj_path = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

			oid = H5Oopen(drive->FileHandle, obj_path, H5P_DEFAULT);
			if (oid < 0) {
				throw gcnew HDF5Exception("H5Oopen failed!");
			}

			if (H5Aexists(oid, attr_name) <= 0) {
				throw gcnew PSH5XException(String::Format(
					"HDF5 object '{0}' doesn't have an HDF5 attribute named '{1}'", path, propertyName));
			}

#pragma endregion

			if (this->ShouldProcess(h5path,
				String::Format("Removing HDF5 attribute '{0}' from item '{1}'", propertyName, path)))
			{
				if (H5Adelete(oid, attr_name) >= 0)
				{
					if (H5Fflush(oid, H5F_SCOPE_LOCAL) < 0) {
						throw gcnew HDF5Exception("H5Fflush failed!");
					}
				}
				else {
					throw gcnew HDF5Exception("H5Adelete failed!");
				}
			}
		}
		finally
		{
			if (oid >= 0) {
				H5Oclose(oid);
			}

			if (obj_path != NULL) {
				Marshal::FreeHGlobal(IntPtr(obj_path));
			}

			if (attr_name != NULL) {
				Marshal::FreeHGlobal(IntPtr(attr_name));
			}
		}

		return;
	}

	Object^ Provider::RemovePropertyDynamicParameters(String^ path, String^ propertyName)
	{
		WriteVerbose(
			String::Format("HDF5Provider::RemovePropertyDynamicParameters(Path = '{0}',PropertyName='{1}')",
			path, propertyName));
		return nullptr;
	}

}