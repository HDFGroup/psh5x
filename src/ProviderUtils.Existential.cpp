
#include "HDF5Exception.h"
#include "ProviderUtils.h"

extern "C" {
#include "H5Dpublic.h"
#include "H5Gpublic.h"
#include "H5Lpublic.h"
#include "H5Ppublic.h"
}

#include <string>
#include <vector>

using namespace std;

using namespace System;
using namespace System::Management::Automation;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
    bool ProviderUtils::IsH5Object(hid_t file, String^ h5path)
    {
		return IsResolvableH5Path(file, h5path);
    }

    bool ProviderUtils::IsH5Group(hid_t file, String^ h5path)
    {
        bool result = false;

		char* name = NULL;

		hid_t obj_id = -1;

		try
		{
			if (ProviderUtils::IsH5Object(file, h5path))
			{
				name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();
				if ((obj_id = H5Oopen(file, name, H5P_DEFAULT)) < 0) {
					throw gcnew HDF5Exception("H5Oopen failed!");
				}

				H5O_info_t info;
				if (H5Oget_info(obj_id, &info) >= 0) {
					result = (info.type == H5O_TYPE_GROUP);
				}
			}
		}
		finally
		{
			if (obj_id >= 0) {
				H5Oclose(obj_id);
			}
			if (name != NULL) {
				Marshal::FreeHGlobal(IntPtr(name));
			}
		}

        return result;
    }

    bool ProviderUtils::IsH5Dataset(hid_t file, String^ h5path)
    {
        bool result = false;

		char* name = NULL;

		hid_t obj_id = -1;

		try
		{
			if (ProviderUtils::IsH5Object(file, h5path))
			{
				if (ProviderUtils::IsH5Object(file, h5path))
				{
					name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();
					if ((obj_id = H5Oopen(file, name, H5P_DEFAULT)) < 0) {
						throw gcnew HDF5Exception("H5Oopen failed!");
					}

					H5O_info_t info;
					if (H5Oget_info(obj_id, &info) >= 0) {
						result = (info.type == H5O_TYPE_DATASET);
					}
				}
			}
		}
		finally
		{
			if (obj_id >= 0) {
				H5Oclose(obj_id);
			}
			if (name != NULL) {
				Marshal::FreeHGlobal(IntPtr(name));
			}
		}

        return result;
    }

    bool ProviderUtils::IsH5ChunkedDataset(hid_t file, String^ h5path)
    {
        bool result = false;
		
		char* name = NULL;

		hid_t dset = -1, plist = -1;

		try
		{
			if (ProviderUtils::IsH5Dataset(file, h5path))
			{
				name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();
				if ((dset = H5Dopen2(file, name, H5P_DEFAULT)) < 0) {
					throw gcnew HDF5Exception("H5Dopen2 failed!");
				}
				if ((plist = H5Dget_create_plist(dset)) < 0) {
					throw gcnew HDF5Exception("H5Dget_create_plist failed!");
				}

				result = (H5Pget_layout(plist) == H5D_CHUNKED);
			}
		}
		finally
		{
			if (plist >= 0) {
				H5Pclose(plist);
			}
			if (dset >= 0) {
				H5Dclose(dset);
			}
			if (name != NULL) {
				Marshal::FreeHGlobal(IntPtr(name));
			}
		}

        return result;
    }

    bool ProviderUtils::IsH5DatatypeObject(hid_t file, String^ h5path)
    {
        bool result = false;

		char* name = NULL;

		hid_t obj_id = -1;

		try
		{
			if (ProviderUtils::IsH5Object(file, h5path))
			{
				if (ProviderUtils::IsH5Object(file, h5path))
				{
					name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();
					if ((obj_id = H5Oopen(file, name, H5P_DEFAULT)) < 0) {
						throw gcnew HDF5Exception("H5Oopen failed!");
					}

					H5O_info_t info;
					if (H5Oget_info(obj_id, &info) >= 0) {
						result = (info.type == H5O_TYPE_NAMED_DATATYPE);
					}
				}
			}
		}
		finally
		{
			if (obj_id >= 0) {
				H5Oclose(obj_id);
			}
			if (name != NULL) {
				Marshal::FreeHGlobal(IntPtr(name));
			}
		}

        return result;
    }

    bool ProviderUtils::IsH5SymLink(hid_t file, String^ h5path)
    {
        if (ProviderUtils::IsH5RootPathName(h5path)) { return false; }

        bool result = false;
		char* name = NULL;

		try
		{
			if (ProviderUtils::IsValidH5Path(file, h5path))
			{
				name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();
				H5L_info_t info;
				if (H5Lget_info(file, name, &info, H5P_DEFAULT) >= 0)
				{
					result = (info.type == H5L_TYPE_SOFT || info.type == H5L_TYPE_EXTERNAL);
				}
			}
		}
		finally
		{
			if (name != NULL) {
				Marshal::FreeHGlobal(IntPtr(name));
			}
		}

        return result;
    }

    bool ProviderUtils::IsH5SoftLink(hid_t file, String^ h5path)
    {
        if (ProviderUtils::IsH5RootPathName(h5path)) { return false; }

        bool result = false;
		char* name = NULL;

		try
		{
			if (ProviderUtils::IsValidH5Path(file, h5path))
			{
				name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();
				H5L_info_t info;
				if (H5Lget_info(file, name, &info, H5P_DEFAULT) >= 0)
				{
					result = (info.type == H5L_TYPE_SOFT);
				}
			}
		}
		finally
		{
			if (name != NULL) {
				Marshal::FreeHGlobal(IntPtr(name));
			}
		}

        return result;
    }

    bool ProviderUtils::IsH5ExternalLink(hid_t file, String^ h5path)
    {
        if (ProviderUtils::IsH5RootPathName(h5path)) { return false; }

        bool result = false;
		char* name = NULL;

		try
		{
			if (ProviderUtils::IsValidH5Path(file, h5path))
			{
				name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();
				H5L_info_t info;
				if (H5Lget_info(file, name, &info, H5P_DEFAULT) >= 0)
				{
					result = (info.type == H5L_TYPE_EXTERNAL);
				}
			}
		}
		finally
		{
			if (name != NULL) {
				Marshal::FreeHGlobal(IntPtr(name));
			}
		}

        return result;
    }
}