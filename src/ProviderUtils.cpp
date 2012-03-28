
#include "HDF5Exception.h"
#include "ProviderUtils.h"
#include "PSH5XException.h"

extern "C" {
#include "H5Apublic.h"
#include "H5Gpublic.h"
#include "H5Ipublic.h"
#include "H5Lpublic.h"
}

#include <string>
#include <vector>

using namespace std;

using namespace System;
using namespace System::Collections;
using namespace System::Collections::Generic;
using namespace System::Management::Automation;
using namespace System::Runtime::InteropServices;
using namespace System::Text;
using namespace System::Web::Script::Serialization;

namespace PSH5X
{
    DriveInfo^ ProviderUtils::GetDriveFromPath(String^ path, ProviderInfo^ provider)
    {
        DriveInfo^ result = nullptr;

        // three formats possible?
        // provider::drive:\path
        // drive:\path
        // //drive/path
        array<String^>^ paths = path->Split(ProviderUtils::DriveSeparator, 2);

        if (paths->Length > 0)
        {
            String^ drivepath = paths[0];
            for each (System::Management::Automation::PSDriveInfo^ drive in provider->Drives)
            {
                if (drive->Name == drivepath) {
                    result = (DriveInfo^) drive;
                }
            }
        }

        return result;
    }

    // TODO: This is not quite right. A well-formed HDF5 path may contain back slashes,
    // which currently get wiped out (replaced by forward slashes)
    // remove duplicate slashes as part of the normalization process

    String^ ProviderUtils::NormalizePath(String^ path)
    {
        if (path->Contains("/") && path->Contains("\\")) {
            throw gcnew ArgumentException("A path name must not simultaneously contain forward- and back-slashes.");
        }

        String^ result = path;

        if (!String::IsNullOrEmpty(path))
            result = path->Replace("\\", ProviderUtils::HDF5_PATH_SEP);

        // Eliminate consecutive and trailing slashes

        array<String^>^ segments = result->Split((gcnew array<wchar_t>{'/'}), StringSplitOptions::RemoveEmptyEntries);
        result = nullptr;
        for (int i = 0; i < segments->Length; ++i)
        {
            result += ProviderUtils::HDF5_PATH_SEP + segments[i];
        }

        // this is for Container & Navigation providers. They parse out the drive and
        // the leading path separator "\" or "/" so we need to add it if it isn't already
        // present at the beginning
        if (!result->StartsWith(ProviderUtils::HDF5_PATH_SEP))
            result = ProviderUtils::HDF5_PATH_SEP + result;

        return result;
    }

    String^ ProviderUtils::PathNoDrive(String^ path)
    {
        array<String^>^ paths = path->Split(ProviderUtils::DriveSeparator, 2);
        String^ pathNoDrive = paths[0];
        if (paths->Length == 2)
            pathNoDrive = paths[1];

        if (pathNoDrive->Trim() == "")
            pathNoDrive = ProviderUtils::HDF5_PATH_SEP;

        return pathNoDrive;
    }

    bool ProviderUtils::TryGetDriveEtH5Path(String^ path, ProviderInfo^ provider,
        DriveInfo^% drive, String^% h5path)
    {
        String^ npath = ProviderUtils::NormalizePath(path);
        h5path = ProviderUtils::PathNoDrive(npath);

        if (!ProviderUtils::IsWellFormedH5Path(h5path)) { return false; }

        drive = ProviderUtils::GetDriveFromPath(path, provider);
        if (drive == nullptr) { return false; }

        return true;
    }

    bool ProviderUtils::IsH5RootPathName(String^ h5path)
    {
        return (h5path->Trim() == "/");
    }

    bool ProviderUtils::IsWellFormedH5Path(System::String^ h5path)
    {
        String^ tpath = h5path->Trim();

        // the root path is well-formed
        if (tpath == "/") {
			return true;
		}

		// the current location path is well-formed
        if (tpath == ".") {
			return true;
		}

		// the '..' is non-sense in HDF5 path names
        if (tpath->StartsWith("..")) {
			return false;
		}

        // (unless the root path) a well-formed path must not end with a slash
        if (tpath->EndsWith("/")) {
			return false;
		}

        // a relative path may begin with a dot
        if (tpath->StartsWith(".")) {
			tpath = tpath->Substring(1);
		}

        array<String^>^ linkNames = tpath->Split((gcnew array<wchar_t>{'/'}),
            StringSplitOptions::RemoveEmptyEntries);

        bool invalidLinkNameFound = false;

        for each (String^ name in linkNames)
        {
            // a link name must not consist of dots only
            if (name->Replace('.', '\0')->Trim()->Length == 0)
            {
                invalidLinkNameFound = true;
                break;
            }
        }

        return !invalidLinkNameFound;
    }

    bool ProviderUtils::IsValidH5Path(hid_t loc, String^ h5path)
    {
		bool result = false;

		char *path_str = NULL, *name_str = NULL;

		hid_t group = -1, obj = -1;

		try
		{
			bool isValidLocation = false;
			switch (H5Iget_type(loc))
			{
			case H5I_FILE:
				isValidLocation = true;
				break;
			case H5I_GROUP:
				isValidLocation = true;
				break;
			default:
				break;
			}
			if (!isValidLocation) {
				throw gcnew PSH5XException("Unsuitable or invalid location handle!.");
			}

			array<String^>^ linkNames = GetLinkNames(h5path);

			if (linkNames->Length == 0 || (linkNames->Length == 1 && linkNames[0] == ".")) {
				return true;
			}

			String^ currentPath = ".";

			for (int i = 0; i < linkNames->Length; ++i)
			{
				path_str = (char*)(Marshal::StringToHGlobalAnsi(currentPath)).ToPointer();
			
				group = H5Gopen2(loc, path_str, H5P_DEFAULT);
				if (group < 0) {
					throw gcnew HDF5Exception("H5Gopen2 failed!");
				}
					
				name_str = (char*)(Marshal::StringToHGlobalAnsi(linkNames[i])).ToPointer();

				if (H5Lexists(group, name_str, H5P_DEFAULT) > 0)
				{
					if (i < linkNames->Length-1)
					{
						if (H5Oexists_by_name(group, name_str, H5P_DEFAULT) > 0)
						{
							obj = H5Oopen(group, name_str, H5P_DEFAULT);
							if (obj < 0) {
								throw gcnew HDF5Exception("H5Oopen failed!!!");
							}
							if (H5Iget_type(obj) != H5I_GROUP) {
								break;
							}
							H5Oclose(obj);
							obj = -1;
						}
						else {
							break;
						}
					}
					else {
						result = true;
					}
				}
				else {
					break;
				}

				Marshal::FreeHGlobal(IntPtr(name_str));
				name_str = NULL;

				H5Gclose(group);
				group = -1;

				Marshal::FreeHGlobal(IntPtr(path_str));
				path_str = NULL;

				currentPath += "/" + linkNames[i];
			}
		}
		finally
		{
			if (name_str != NULL) {
				Marshal::FreeHGlobal(IntPtr(name_str));
			}
			if (path_str != NULL) {
				Marshal::FreeHGlobal(IntPtr(path_str));
			}
			if (obj >= 0) {
				H5Oclose(obj);
			}
			if (group >= 0) {
				H5Gclose(group);
			}
		}

		return result;
    }

	bool ProviderUtils::IsResolvableH5Path(hid_t loc, String^ h5path)
	{
		bool result = false;

		hid_t obj = -1;

		char *obj_path_str = NULL, *name_str = NULL;

		try
		{
			if (IsValidH5Path(loc, h5path))
			{
				array<String^>^ linkNames = GetLinkNames(h5path);

				if (linkNames->Length > 1)
				{
					String^ name = linkNames[linkNames->Length-1];
					StringBuilder^ sb = gcnew StringBuilder();
					for (int i = 0; i < linkNames->Length-1; ++i)
					{
						if (i > 0) {
							sb->Append("/");
						}
						sb->Append(linkNames[i]);
					}
					String^ objPath = sb->ToString();

					obj_path_str = (char*) Marshal::StringToHGlobalAnsi(objPath).ToPointer();
					obj = H5Oopen(loc, obj_path_str, H5P_DEFAULT);
					H5I_type_t type = H5Iget_type(obj);
					if (type == H5I_FILE || type == H5I_GROUP)
					{
						name_str = (char*) Marshal::StringToHGlobalAnsi(name).ToPointer();
						if (H5Oexists_by_name(obj, name_str, H5P_DEFAULT) > 0) {
							result = true;
						}
					}
				}
				else
				{
					H5I_type_t type = H5Iget_type(loc);
					if (type == H5I_FILE || type == H5I_GROUP)
					{
						if (linkNames->Length == 1)
						{
							name_str = (char*) Marshal::StringToHGlobalAnsi(linkNames[0]).ToPointer();
							if (H5Lexists(loc, name_str, H5P_DEFAULT) > 0) {
								if (H5Oexists_by_name(loc, name_str, H5P_DEFAULT) > 0) {
									result = true;
								}
							}
						}
						else {
							result = (type == H5I_FILE);
						}
					}
				}
			}
		}
		finally
		{
			if (obj != -1) {
				H5Oclose(obj);
			}
			if (obj_path_str != NULL) {
				Marshal::FreeHGlobal(IntPtr(obj_path_str));
			}
			if (name_str != NULL) {
				Marshal::FreeHGlobal(IntPtr(name_str));
			}
		}

		return result;
	}

    bool ProviderUtils::CanCreateItemAt(hid_t loc, String^ h5path)
	{
		char *name_str = NULL, *new_name_str = NULL, *path_str = NULL;

		if (IsValidH5Path(loc, h5path)) { return false; }

		array<String^>^ linkNames = GetLinkNames(h5path);

		bool result = false;

		hid_t group = -1, obj = -1;

		String^ currentPath = ".";

		try
		{
			for (int i = 0; i < linkNames->Length; ++i)
			{
				path_str = (char*)(Marshal::StringToHGlobalAnsi(currentPath)).ToPointer();
				
				group = H5Gopen2(loc, path_str, H5P_DEFAULT);
				if (group >= 0)
				{
					name_str = (char*)(Marshal::StringToHGlobalAnsi(linkNames[i])).ToPointer();
					
					if (H5Lexists(group, name_str, H5P_DEFAULT) > 0)
					{
						if (H5Oexists_by_name(group, name_str, H5P_DEFAULT) > 0) {
							if (i == linkNames->Length-2) // we have to deal with the corner case linkNames->Length = 1
							{
								obj = H5Oopen(group, name_str, H5P_DEFAULT);
								if (obj < 0) {
									throw gcnew HDF5Exception("H5Oopen failed!!!");
								}

								new_name_str = (char*)(Marshal::StringToHGlobalAnsi(linkNames[i+1])).ToPointer(); 
								if (H5Iget_type(obj) == H5I_GROUP && (H5Lexists(obj, name_str, H5P_DEFAULT) == 0)) {
									result = true;
								}
								Marshal::FreeHGlobal(IntPtr(new_name_str));

								H5Oclose(obj);
								obj = -1;
							}
						}
						else {
							break;
						}
					}
					else {
						if (linkNames->Length == 1 && H5Lexists(group, name_str, H5P_DEFAULT) == 0) { // corner case
							result = true;
						}
						else {
							break;
						}
					}

					Marshal::FreeHGlobal(IntPtr(name_str));
					name_str = NULL;
				}
				H5Gclose(group);
				group = -1;

				currentPath += "/" + linkNames[i];

				Marshal::FreeHGlobal(IntPtr(path_str));
				path_str = NULL;
			}
		}
		finally
		{
			if (group != -1) {
				H5Gclose(group);
			}

			if (obj != -1) {
				H5Oclose(obj);
			}

			if (path_str != NULL) {
				Marshal::FreeHGlobal(IntPtr(path_str));
			}

			if (name_str != NULL) {
				Marshal::FreeHGlobal(IntPtr(name_str));
			}
		}
		return result;
	}

    bool ProviderUtils::CanForceCreateItemAt(hid_t loc, String^ h5path)
    {
		char *name_str = NULL, *new_name_str = NULL, *path_str = NULL;

		if (IsValidH5Path(loc, h5path)) { return false; }

		array<String^>^ linkNames = GetLinkNames(h5path);

		bool result = false;

		hid_t group = -1, obj = -1;

		String^ currentPath = ".";

		try
		{
			for (int i = 0; i < linkNames->Length; ++i)
			{
				path_str = (char*)(Marshal::StringToHGlobalAnsi(currentPath)).ToPointer();
				
				group = H5Gopen2(loc, path_str, H5P_DEFAULT);
				if (group >= 0)
				{
					name_str = (char*)(Marshal::StringToHGlobalAnsi(linkNames[i])).ToPointer();
					
					htri_t flag = H5Lexists(group, name_str, H5P_DEFAULT);
					if (flag < 0) {
						throw gcnew HDF5Exception("H5Lexists failed!!!");
					}

					if (flag > 0)
					{
						if (H5Oexists_by_name(group, name_str, H5P_DEFAULT) > 0) {
							if (i == linkNames->Length-2) // we have to deal with the corner case linkNames->Length = 1
							{
								obj = H5Oopen(group, name_str, H5P_DEFAULT);
								if (obj < 0) {
									throw gcnew HDF5Exception("H5Oopen failed!!!");
								}

								new_name_str = (char*)(Marshal::StringToHGlobalAnsi(linkNames[i+1])).ToPointer(); 
								if (H5Iget_type(obj) == H5I_GROUP && (H5Lexists(obj, name_str, H5P_DEFAULT) == 0)) {
									result = true;
								}
								Marshal::FreeHGlobal(IntPtr(new_name_str));

								H5Oclose(obj);
								obj = -1;
							}
						}
						else {
							break;
						}
					}
					else {
						if (flag == 0)
						{
							result = true;
							break;
						}
					}

					Marshal::FreeHGlobal(IntPtr(name_str));
					name_str = NULL;
				}
				H5Gclose(group);
				group = -1;

				currentPath += "/" + linkNames[i];

				Marshal::FreeHGlobal(IntPtr(path_str));
				path_str = NULL;
			}
		}
		finally
		{
			if (group != -1) {
				H5Gclose(group);
			}

			if (obj != -1) {
				H5Oclose(obj);
			}

			if (path_str != NULL) {
				Marshal::FreeHGlobal(IntPtr(path_str));
			}

			if (name_str != NULL) {
				Marshal::FreeHGlobal(IntPtr(name_str));
			}
		}

        return result;
    }

    String^ ProviderUtils::ParentPath(String^ h5path)
    {
        if (ProviderUtils::IsWellFormedH5Path(h5path))
        {
            String^ result = "/";
            int pos = h5path->LastIndexOf('/');
            if (pos > 0)
                result = h5path->Substring(0, pos);
            return result;
        }
        else
        {
            throw gcnew Exception(String::Format("Invalid absolute HDF5 path name '{0}'",
                h5path));
        }
    }

    String^ ProviderUtils::ChildName(String^ h5path)
    {
        if (ProviderUtils::IsWellFormedH5Path(h5path))
        {
            if (ProviderUtils::IsH5RootPathName(h5path))
                return nullptr;
            else
                return h5path->Substring(h5path->LastIndexOf('/')+1);
        }
        else
        {
            throw gcnew Exception(String::Format("Invalid absolute HDF5 path name '{0}'",
                h5path));
        }
    }

    DateTime^ ProviderUtils::UnixTime2DateTime(time_t secondsSinceJan1)
    {
        int unixStartMonth = 1;
        int unixStartDay = 1;
        int unixStartYear = 1970;
        int unixHour = 0;
        int unixMinute = 0;
        int unixSecond = 0;

        DateTime^ unixStartDate = 
            gcnew DateTime(unixStartYear, unixStartMonth, unixStartDay,
            unixHour, unixMinute, unixSecond);

        return unixStartDate->AddSeconds(
            static_cast<double>(secondsSinceJan1));
    }

    herr_t H5AIterateCallback(
        hid_t             location_id,
        const char*       attr_name,
        const H5A_info_t* ainfo,
        void*             op_data)
    {
        vector<string>* v = static_cast<vector<string>*>(op_data);
        size_t i = 0;
        while ((*v)[i] != "") {
            ++i;
        }

        (*v)[i] = string(attr_name);

        return 0;
    }

    array<String^>^ ProviderUtils::GetObjectAttributeNames(hid_t obj_id)
    {
        H5O_info_t info;
        if (H5Oget_info(obj_id, &info) < 0) {
            throw gcnew Exception("H5Oget_info failed!");
        }

        vector<string> v(info.num_attrs);
        hsize_t n = 0;

        herr_t status = H5Aiterate2(obj_id, H5_INDEX_NAME,
            H5_ITER_NATIVE, &n, &H5AIterateCallback, (void*) &v);

        array<String^>^ result = gcnew array<String^>(safe_cast<int>(info.num_attrs));
        for (int i = 0; i < result->Length; ++i)
            result[i] = gcnew String(v[i].c_str());

        return result;
    }

    herr_t H5LVisitCallback(
        hid_t             group,
        const char*       name,
        const H5L_info_t* info,
        void*             op_data)
    {
        vector<string>* v = static_cast<vector<string>*>(op_data);
        v->push_back(string(name));
        return 0;
    }

        herr_t H5LIterateCallback(
        hid_t             group,
        const char*       name,
        const H5L_info_t* info,
        void*             op_data)
    {
        vector<string>* v = static_cast<vector<string>*>(op_data);
        v->push_back(string(name));
        return 0;
    }

    array<String^>^ ProviderUtils::GetGroupLinkNames(hid_t group_id, bool recurse)
    {
        array<String^>^ result = nullptr;
        hsize_t idx = 0;
        vector<string> v;
        herr_t status = -1;
        
        if (recurse)
        {
            // TODO: add max. recusion depth protection
            status = H5Lvisit(group_id, H5_INDEX_NAME, H5_ITER_NATIVE, &H5LVisitCallback,
                (void*) &v);
        }
        else {
            status = H5Literate(group_id, H5_INDEX_NAME, H5_ITER_NATIVE, &idx,
                &H5LIterateCallback, (void*) &v);
        }

        result = gcnew array<String^>(safe_cast<int>(v.size()));
        for (int i = 0; i < result->Length; ++i)
            result[i] = gcnew String(v[i].c_str());

        return result;
    }

	Object^ ProviderUtils::GetDotNetObject(Object^ obj)
	{
		Object^ result = nullptr;

		if (obj->GetType() == System::Management::Automation::PSObject::typeid)	{
			result = safe_cast<System::Management::Automation::PSObject^>(obj)->BaseObject;
		}
		else {
			result = obj;
		}

		return result;
	}

#pragma region TryGetValue

	bool ProviderUtils::TryGetValue(Object^ obj, char& i)
    {
        bool result = false;

        try
        {
            i = Convert::ToSByte(obj);
            result = true;
        }
        catch (...) {}

        return result;
    }

	bool ProviderUtils::TryGetValue(Object^ obj, unsigned char& i)
    {
        bool result = false;

        try
        {
            i = Convert::ToByte(obj);
            result = true;
        }
        catch (...) {}

        return result;
    }

	bool ProviderUtils::TryGetValue(Object^ obj, short& i)
    {
        bool result = false;

        try
        {
            i = Convert::ToInt16(obj);
            result = true;
        }
        catch (...) {}

        return result;
    }

	bool ProviderUtils::TryGetValue(Object^ obj, unsigned short& i)
    {
        bool result = false;

        try
        {
            i = Convert::ToUInt16(obj);
            result = true;
        }
        catch (...) {}

        return result;
    }

    bool ProviderUtils::TryGetValue(Object^ obj, int& i)
    {
        bool result = false;

        try
		{
			i = Convert::ToInt32(obj);
			result = true;
		}
        catch (...) {}

        return result;
    }

	bool ProviderUtils::TryGetValue(Object^ obj, unsigned int& i)
    {
        bool result = false;

        try
        {
            i = Convert::ToUInt32(obj);
            result = true;
        }
        catch (...) {}

        return result;
    }

	bool ProviderUtils::TryGetValue(Object^ obj, long long& i)
    {
        bool result = false;

        try
        {
            i = Convert::ToInt64(obj);
            result = true;
        }
        catch (...) {}

        return result;
    }

	bool ProviderUtils::TryGetValue(Object^ obj, unsigned long long& i)
    {
        bool result = false;

        try
        {
            i = Convert::ToUInt64(obj);
            result = true;
        }
        catch (...) {}

        return result;
    }

	bool ProviderUtils::TryGetValue(Object^ obj, float& i)
    {
        bool result = false;

        try
        {
            i = Convert::ToSingle(obj);
            result = true;
        }
        catch (...) {}

        return result;
    }

	bool ProviderUtils::TryGetValue(Object^ obj, double& i)
    {
        bool result = false;

        try
        {
            i = Convert::ToDouble(obj);
            result = true;
        }
        catch (...) {}

        return result;
    }

    bool ProviderUtils::TryGetValue(Object^ obj, String^% str)
    {
        bool result = false;

        try
        {
			if (obj != nullptr) {
				str = safe_cast<String^>(obj);
			}
			else {
				str = "";
			}
			result = true;
        }
        catch (...) {}

        return result;
    }

    bool ProviderUtils::TryGetValue(Object^ obj, Hashtable^% ht)
    {
        bool result = false;

        try
        {
            ht = safe_cast<Hashtable^>(obj);
            result = true;
        }
        catch (...) {}

        return result;
    }

    bool ProviderUtils::TryGetValue(Object^ obj, array<hsize_t>^% arr)
    {
        bool result = false;

        try
        {
			Array^ tmp = safe_cast<Array^>(ProviderUtils::GetDotNetObject(obj));
            arr = gcnew array<hsize_t>(tmp->Length);
            for (int i = 0; i < tmp->Length; ++i) {
                arr[i] = Convert::ToUInt64(tmp->GetValue(i));
            }

            result = true;
        }
        catch (...) {}

        return result;
    }

    bool ProviderUtils::TryGetValue(Object^ obj, array<String^>^% arr)
    {
        bool result = false;

        try
        {
            array<Object^>^ tmp = safe_cast<array<Object^>^>(obj);
            arr = gcnew array<String^>(tmp->Length);
            for (int i = 0; i < tmp->Length; ++i)
            {
				if (tmp[i] != nullptr) {
					arr[i] = tmp[i]->ToString();
				}
				else {
					arr[i] = "";
				}
            }

            result = true;
        }
        catch (...) {}

        return result;
    }

	bool ProviderUtils::TryGetValue(Object^ obj, array<Object^>^% arr)
    {
        bool result = false;

        try
        {
            arr = safe_cast<array<Object^>^>(obj);
            result = true;
        }
        catch (...) {}

        return result;
    }

    bool ProviderUtils::TryGetValue(Object^ obj, Dictionary<String^, Object^>^% dict)
    {
        bool result = false;

        try
        {
            dict = safe_cast<Dictionary<String^, Object^>^>(obj);
            result = true;
        }
        catch (...) {}

        return result;
    }


#pragma endregion


    bool ProviderUtils::ResolveItemType(String^ str, String^% itemType)
    {
        if (m_item_types->ContainsKey(str->Trim()->ToUpper())) {
            itemType = gcnew String((String^) m_item_types[str->Trim()->ToUpper()]);
            return true;
        }
        else {
            return false;
        }
    }

	array<String^>^ ProviderUtils::GetLinkNames(String^ h5path)
	{
		return h5path->Split((gcnew array<wchar_t>{'/'}), StringSplitOptions::RemoveEmptyEntries);
	}

    static ProviderUtils::ProviderUtils()
    {
        m_hdf5_path_sep = "/";

        m_hdf5_drive_sep = ":";

        m_hdf5_provider_sep = "::";

        m_drive_sep = gcnew array<wchar_t> {':'};

        m_type_classes = gcnew array<String^>{
            "ARRAY", "BITFIELD", "COMPOUND", "ENUM", "FLOAT",
            "INTEGER", "OPAQUE", "REFERENCE", "STRING", "VLEN" };


#pragma region HDF5 predefined types

        /* http://www.hdfgroup.org/HDF5/doc/RM/PredefDTypes.html */


		JavaScriptSerializer^ serializer = gcnew JavaScriptSerializer();

        m_predefined_types = gcnew Hashtable();

        // IEEE floating point datatypes

        hid_t type = H5Tcopy(H5T_IEEE_F32BE);
        m_predefined_types["H5T_IEEE_F32BE"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_IEEE_F32LE);
        m_predefined_types["H5T_IEEE_F32LE"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_IEEE_F64BE);
        m_predefined_types["H5T_IEEE_F64BE"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_IEEE_F64LE);
        m_predefined_types["H5T_IEEE_F64LE"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);

        // Standard datatypes

        type = H5Tcopy(H5T_STD_I8BE);
        m_predefined_types["H5T_STD_I8BE"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_STD_I8LE);
        m_predefined_types["H5T_STD_I8LE"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_STD_I16BE);
        m_predefined_types["H5T_STD_I16BE"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_STD_I16LE);
        m_predefined_types["H5T_STD_I16LE"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_STD_I32BE);
        m_predefined_types["H5T_STD_I32BE"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_STD_I32LE);
        m_predefined_types["H5T_STD_I32LE"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_STD_I64BE);
        m_predefined_types["H5T_STD_I64BE"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_STD_I32LE);
        m_predefined_types["H5T_STD_I64LE"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);

        type = H5Tcopy(H5T_STD_U8BE);
        m_predefined_types["H5T_STD_U8BE"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_STD_U8LE);
        m_predefined_types["H5T_STD_U8LE"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_STD_U16BE);
        m_predefined_types["H5T_STD_U16BE"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_STD_U16LE);
        m_predefined_types["H5T_STD_U16LE"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_STD_U32BE);
        m_predefined_types["H5T_STD_U32BE"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_STD_U32LE);
        m_predefined_types["H5T_STD_U32LE"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_STD_U64BE);
        m_predefined_types["H5T_STD_U64BE"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_STD_U32LE);
        m_predefined_types["H5T_STD_U64LE"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);

        type = H5Tcopy(H5T_STD_B8BE);
        m_predefined_types["H5T_STD_B8BE"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_STD_B8LE);
        m_predefined_types["H5T_STD_B8LE"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_STD_B16BE);
        m_predefined_types["H5T_STD_B16BE"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_STD_B16LE);
        m_predefined_types["H5T_STD_B16LE"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_STD_B32BE);
        m_predefined_types["H5T_STD_B32BE"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_STD_B32LE);
        m_predefined_types["H5T_STD_B32LE"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_STD_B64BE);
        m_predefined_types["H5T_STD_B64BE"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_STD_B64LE);
        m_predefined_types["H5T_STD_B64LE"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);

        type = H5Tcopy(H5T_STD_REF_OBJ);
        m_predefined_types["H5T_STD_REF_OBJ"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_STD_REF_DSETREG);
        m_predefined_types["H5T_STD_REF_DSETREG"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);

        // C-specific datatype

        type = H5Tcopy(H5T_C_S1);
        m_predefined_types["H5T_C_S1"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);

		// FORTRAN-specific datatype

        type = H5Tcopy(H5T_FORTRAN_S1);
        m_predefined_types["H5T_FORTRAN_S1"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);

        // Predefined native datatypes

        type = H5Tcopy(H5T_NATIVE_CHAR);
        m_predefined_types["H5T_NATIVE_CHAR"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_NATIVE_SHORT);
        m_predefined_types["H5T_NATIVE_SHORT"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_NATIVE_INT);
        m_predefined_types["H5T_NATIVE_INT"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_NATIVE_LONG);
        m_predefined_types["H5T_NATIVE_LONG"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_NATIVE_LLONG);
        m_predefined_types["H5T_NATIVE_LLONG"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_NATIVE_UCHAR);
        m_predefined_types["H5T_NATIVE_UCHAR"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_NATIVE_USHORT);
        m_predefined_types["H5T_NATIVE_USHORT"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_NATIVE_UINT);
        m_predefined_types["H5T_NATIVE_UINT"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_NATIVE_ULONG);
        m_predefined_types["H5T_NATIVE_ULONG"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_NATIVE_ULLONG);
        m_predefined_types["H5T_NATIVE_ULLONG"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_NATIVE_FLOAT);
        m_predefined_types["H5T_NATIVE_FLOAT"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_NATIVE_DOUBLE);
        m_predefined_types["H5T_NATIVE_DOUBLE"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_NATIVE_LDOUBLE);
        m_predefined_types["H5T_NATIVE_LDOUBLE"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_NATIVE_B8);
        m_predefined_types["H5T_NATIVE_B8"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_NATIVE_B16);
        m_predefined_types["H5T_NATIVE_B16"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_NATIVE_B32);
        m_predefined_types["H5T_NATIVE_B32"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);
        type = H5Tcopy(H5T_NATIVE_B64);
        m_predefined_types["H5T_NATIVE_B64"] = serializer->Serialize(ParseH5Type(type));
        H5Tclose(type);

        m_bitfield_types = gcnew Hashtable();

        array<String^>^ a = gcnew array<String^>{
            "H5T_STD_B8BE", "H5T_STD_B8LE", "H5T_STD_B16BE",
            "H5T_STD_B16LE", "H5T_STD_B32BE", "H5T_STD_B32LE",
            "H5T_STD_B64BE", "H5T_STD_B64LE", "H5T_NATIVE_B8",
            "H5T_NATIVE_B16", "H5T_NATIVE_B32", "H5T_NATIVE_B64"
        };

        for each (String^ s in a) {
            m_bitfield_types[s] = m_predefined_types[s];
        }

        
        m_float_types = gcnew Hashtable();

        a = gcnew array<String^>{
            "H5T_IEEE_F32BE", "H5T_IEEE_F32LE", "H5T_IEEE_F64BE",
            "H5T_IEEE_F64LE", "H5T_NATIVE_FLOAT", "H5T_NATIVE_DOUBLE",
            "H5T_NATIVE_LDOUBLE"
        };

        for each (String^ s in a) {
            m_float_types[s] = m_predefined_types[s];
        }

        m_integer_types = gcnew Hashtable();

        a = gcnew array<String^>{
            "H5T_STD_I8BE", "H5T_STD_I8LE", "H5T_STD_I16BE",
            "H5T_STD_I16LE", "H5T_STD_I32BE", "H5T_STD_I32LE",
            "H5T_STD_I64BE", "H5T_STD_I64LE", "H5T_STD_U8BE",
            "H5T_STD_U8LE", "H5T_STD_U16BE", "H5T_STD_U16LE",
            "H5T_STD_U32BE", "H5T_STD_U32LE", "H5T_STD_U64BE",
            "H5T_STD_U64LE", "H5T_NATIVE_CHAR",
            "H5T_NATIVE_UCHAR", "H5T_NATIVE_SHORT", "H5T_NATIVE_USHORT",
            "H5T_NATIVE_INT", "H5T_NATIVE_UINT", "H5T_NATIVE_LONG",
            "H5T_NATIVE_ULONG", "H5T_NATIVE_LLONG", "H5T_NATIVE_ULLONG"
        };

        for each (String^ s in a) {
            m_integer_types[s] = m_predefined_types[s];
        }

        m_native_types = gcnew Hashtable();

        a = gcnew array<String^>{
            "H5T_NATIVE_CHAR", "H5T_NATIVE_SHORT", "H5T_NATIVE_INT",
            "H5T_NATIVE_LONG", "H5T_NATIVE_LLONG", "H5T_NATIVE_UCHAR",
            "H5T_NATIVE_USHORT", "H5T_NATIVE_UINT", "H5T_NATIVE_ULONG",
            "H5T_NATIVE_ULLONG", "H5T_NATIVE_FLOAT", "H5T_NATIVE_DOUBLE",
            "H5T_NATIVE_LDOUBLE", "H5T_NATIVE_B8", "H5T_NATIVE_B16",
            "H5T_NATIVE_B32", "H5T_NATIVE_B64" };

        for each (String^ s in a) {
            m_native_types[s] = m_predefined_types[s];
        }

        m_reference_types = gcnew Hashtable();

        a = gcnew array<String^>{
            "H5T_STD_REF_OBJ", "H5T_STD_REF_DSETREG"
        };

        for each (String^ s in a) {
            m_reference_types[s] = m_predefined_types[s];
        }

        m_standard_types = gcnew Hashtable();

        a = gcnew array<String^>{
            "H5T_STD_I8BE", "H5T_STD_I8LE", "H5T_STD_I16BE",
            "H5T_STD_I16LE", "H5T_STD_I32BE", "H5T_STD_I32LE",
            "H5T_STD_I64BE", "H5T_STD_I64LE", "H5T_STD_U8BE",
            "H5T_STD_U8LE", "H5T_STD_U16BE", "H5T_STD_U16LE",
            "H5T_STD_U32BE", "H5T_STD_U32LE", "H5T_STD_U64BE",
            "H5T_STD_U64LE", "H5T_STD_B8BE", "H5T_STD_B8LE",
            "H5T_STD_B16BE", "H5T_STD_B16LE", "H5T_STD_B32BE",
            "H5T_STD_B32LE", "H5T_STD_B64BE", "H5T_STD_B64LE",
            "H5T_STD_REF_OBJ", "H5T_STD_REF_DSETREG"
        };

        for each (String^ s in a) {
            m_standard_types[s] = m_predefined_types[s];
        }

#pragma endregion

#pragma region HDF5 known types

        m_known_types = gcnew Hashtable();

		m_known_types["CHAR"] = H5T_NATIVE_CHAR;
        m_known_types["H5T_NATIVE_CHAR"] = H5T_NATIVE_CHAR;

#pragma region integer

		m_known_types["I8BE"] = H5T_STD_I8BE;
        m_known_types["I8LE"] = H5T_STD_I8LE;
		m_known_types["H5T_NATIVE_SCHAR"] = H5T_NATIVE_SCHAR;
		m_known_types["H5T_STD_I8BE"] = H5T_STD_I8BE;
        m_known_types["H5T_STD_I8LE"] = H5T_STD_I8LE;
		m_known_types["SCHAR"] = H5T_NATIVE_SCHAR;
        
		m_known_types["H5T_STD_U8BE"] = H5T_STD_U8BE;
        m_known_types["H5T_STD_U8LE"] = H5T_STD_U8LE;
        m_known_types["H5T_NATIVE_UCHAR"] = H5T_NATIVE_UCHAR;
		m_known_types["U8"] = H5T_NATIVE_UCHAR;		
		m_known_types["U8BE"] = H5T_STD_U8BE;
        m_known_types["U8LE"] = H5T_STD_U8LE;
        m_known_types["UCHAR"] = H5T_NATIVE_UCHAR;
       
		m_known_types["I16"] = H5T_NATIVE_SHORT;
		m_known_types["I16BE"] = H5T_STD_I16BE;
		m_known_types["I16LE"] = H5T_STD_I16LE;
        m_known_types["H5T_NATIVE_SHORT"] = H5T_NATIVE_SHORT;
		m_known_types["H5T_STD_I16BE"] = H5T_STD_I16BE;
		m_known_types["H5T_STD_I16LE"] = H5T_STD_I16LE;
        m_known_types["SHORT"] = H5T_NATIVE_SHORT;

        m_known_types["H5T_STD_U16BE"] = H5T_STD_U16BE;
		m_known_types["H5T_STD_U16LE"] = H5T_STD_U16LE;
        m_known_types["H5T_NATIVE_USHORT"] = H5T_NATIVE_USHORT;
		m_known_types["U16"] = H5T_NATIVE_USHORT;        
		m_known_types["U16BE"] = H5T_STD_U16BE;
		m_known_types["U16LE"] = H5T_STD_U16LE;
		m_known_types["USHORT"] = H5T_NATIVE_USHORT;

		m_known_types["I32"] = H5T_NATIVE_INT;
		m_known_types["I32BE"] = H5T_STD_I32BE;
        m_known_types["I32LE"] = H5T_STD_I32LE;
        m_known_types["INT"] = H5T_NATIVE_INT;
        m_known_types["H5T_NATIVE_INT"] = H5T_NATIVE_INT;
		m_known_types["H5T_STD_I32BE"] = H5T_STD_I32BE;
		m_known_types["H5T_STD_I32LE"] = H5T_STD_I32LE;
        
		m_known_types["H5T_NATIVE_LONG"] = H5T_NATIVE_LONG;
        m_known_types["LONG"] = H5T_NATIVE_LONG;
        
		m_known_types["H5T_STD_U32BE"] = H5T_STD_U32BE;        
        m_known_types["H5T_STD_U32LE"] = H5T_STD_U32LE;
		m_known_types["H5T_NATIVE_UINT"] = H5T_NATIVE_UINT;
		m_known_types["U32"] = H5T_NATIVE_UINT;
		m_known_types["U32BE"] = H5T_STD_U32BE;
        m_known_types["U32LE"] = H5T_STD_U32LE;
		m_known_types["UINT"] = H5T_NATIVE_UINT;
        
		m_known_types["H5T_NATIVE_ULONG"] = H5T_NATIVE_ULONG;
		m_known_types["ULONG"] = H5T_NATIVE_ULONG;

		m_known_types["H5T_STD_I64BE"] = H5T_STD_I64BE;
        m_known_types["H5T_STD_I64LE"] = H5T_STD_I64LE;
		m_known_types["I64BE"] = H5T_STD_I64BE;
		m_known_types["I64LE"] = H5T_STD_I64LE;
        
		m_known_types["I64"] = H5T_NATIVE_LLONG;
		m_known_types["H5T_NATIVE_LLONG"] = H5T_NATIVE_LLONG;
		m_known_types["LLONG"] = H5T_NATIVE_LLONG;
        
		m_known_types["H5T_STD_U64BE"] = H5T_STD_U64BE;
        m_known_types["H5T_STD_U64LE"] = H5T_STD_U64LE;
        m_known_types["U64BE"] = H5T_STD_U64BE;
        m_known_types["U64LE"] = H5T_STD_U64LE;
		
		m_known_types["H5T_NATIVE_ULLONG"] = H5T_NATIVE_ULLONG;
        m_known_types["U64"] = H5T_NATIVE_ULLONG;
		m_known_types["ULLONG"] = H5T_NATIVE_ULLONG;
        
#pragma endregion

#pragma region bitfield

        m_known_types["B8"] = H5T_NATIVE_B8;
        m_known_types["B8BE"] = H5T_STD_B8BE;
		m_known_types["B8LE"] = H5T_STD_B8LE;
		m_known_types["H5T_NATIVE_B8"] = H5T_NATIVE_B8;
		m_known_types["H5T_STD_B8BE"] = H5T_STD_B8BE;
        m_known_types["H5T_STD_B8LE"] = H5T_STD_B8LE;

        m_known_types["B16"] = H5T_NATIVE_B16;
	    m_known_types["B16BE"] = H5T_STD_B16BE;
		m_known_types["B16LE"] = H5T_STD_B16LE;
        m_known_types["H5T_NATIVE_B16"] = H5T_NATIVE_B16;
		m_known_types["H5T_STD_B16BE"] = H5T_STD_B16BE;
        m_known_types["H5T_STD_B16LE"] = H5T_STD_B16LE;
		
		m_known_types["B32"] = H5T_NATIVE_B32;
		m_known_types["B32BE"] = H5T_STD_B32BE;
		m_known_types["B32LE"] = H5T_STD_B32LE;
		m_known_types["H5T_NATIVE_B32"] = H5T_NATIVE_B32;
		m_known_types["H5T_STD_B32BE"] = H5T_STD_B32BE;
		m_known_types["H5T_STD_B32LE"] = H5T_STD_B32LE;

		m_known_types["B64"] = H5T_NATIVE_B64;
		m_known_types["B64BE"] = H5T_STD_B64BE;
		m_known_types["B64LE"] = H5T_STD_B64LE;
		m_known_types["H5T_NATIVE_B64"] = H5T_NATIVE_B64;
		m_known_types["H5T_STD_B64BE"] = H5T_STD_B64BE;
		m_known_types["H5T_STD_B64LE"] = H5T_STD_B64LE;

#pragma endregion
    
#pragma region float

		m_known_types["F32"] = H5T_NATIVE_FLOAT;
        m_known_types["F32BE"] = H5T_IEEE_F32BE;
        m_known_types["F32LE"] = H5T_IEEE_F32LE;
		m_known_types["FLOAT"] = H5T_NATIVE_FLOAT;
        m_known_types["H5T_IEEE_F32BE"] = H5T_IEEE_F32BE;
        m_known_types["H5T_IEEE_F32LE"] = H5T_IEEE_F32LE;
	    m_known_types["H5T_NATIVE_FLOAT"] = H5T_NATIVE_FLOAT;
    	m_known_types["SINGLE"] = H5T_NATIVE_FLOAT;
        
		m_known_types["DOUBLE"] = H5T_NATIVE_DOUBLE;
		m_known_types["F64"] = H5T_NATIVE_DOUBLE;
        m_known_types["F64BE"] = H5T_IEEE_F64BE;
        m_known_types["F64LE"] = H5T_IEEE_F64LE;
        m_known_types["H5T_IEEE_F64BE"] = H5T_IEEE_F64BE;
		m_known_types["H5T_IEEE_F64LE"] = H5T_IEEE_F64LE;
		m_known_types["H5T_NATIVE_DOUBLE"] = H5T_NATIVE_DOUBLE;
        
	    m_known_types["H5T_NATIVE_LDOUBLE"] = H5T_NATIVE_LDOUBLE;
		m_known_types["LDOUBLE"] = H5T_NATIVE_LDOUBLE;
        
#pragma endregion

		m_known_types["H5T_STD_REF_OBJ"] = H5T_STD_REF_OBJ;
        m_known_types["OBJREF"] = H5T_STD_REF_OBJ;
        m_known_types["REF_OBJ"] = H5T_STD_REF_OBJ;
        
		m_known_types["H5T_STD_REF_DSETREG"] = H5T_STD_REF_DSETREG;
        m_known_types["REF_DSETREG"] = H5T_STD_REF_DSETREG;
		m_known_types["REGREF"] = H5T_STD_REF_DSETREG;

#pragma endregion

#pragma region HDF5 item types

        m_item_types = gcnew Hashtable();

        m_item_types["DATASET"] = "DATASET";
        m_item_types["DSET"] = "DATASET";
        m_item_types["SET"] = "DATASET";
        m_item_types["D"] = "DATASET";

        m_item_types["DATATYPE"] = "DATATYPE";
        m_item_types["DTYPE"] = "DATATYPE";
        m_item_types["TYPE"] = "DATATYPE";
        m_item_types["T"] = "DATATYPE";

        m_item_types["EXTLINK"] = "EXTLINK";
        m_item_types["EXTERNALLINK"] = "EXTLINK";
        m_item_types["EXTERNAL"] = "EXTLINK";
        m_item_types["ELINK"] = "EXTLINK";
        m_item_types["EXT"] = "EXTLINK";


        m_item_types["GROUP"] = "GROUP";
        m_item_types["GRP"] = "GROUP";
        m_item_types["GR"] = "GROUP";
        m_item_types["G"] = "GROUP";

        m_item_types["HARDLINK"] = "HARDLINK";
        m_item_types["HLINK"] = "HARDLINK";
        m_item_types["HARD"] = "HARDLINK";

        m_item_types["IMAGE"] = "IMAGE";
        m_item_types["IMG"] = "IMAGE";
        m_item_types["IM"] = "IMAGE";

        m_item_types["PACKETTABLE"] = "PACKETTABLE";
        m_item_types["PTABLE"] = "PACKETTABLE";
        m_item_types["PACKET"] = "PACKETTABLE";
        m_item_types["PACK"] = "PACKETTABLE";
        m_item_types["PT"] = "PACKETTABLE";

        m_item_types["SOFTLINK"] = "SOFTLINK";
        m_item_types["SLINK"] = "SOFTLINK";
        m_item_types["SOFT"] = "SOFTLINK";

#pragma endregion
    }
}
