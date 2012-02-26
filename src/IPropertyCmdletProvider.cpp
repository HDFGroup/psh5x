
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
    void Provider::ClearProperty(String^ path, Collection<String^>^ propertyToClear)
    {
        WriteVerbose(String::Format("HDF5Provider::ClearProperty(Path = '{0}')", path));
        
        // Provider::ClearProperty is being called by Set-ItemProperty, so we can't throw an exception
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

        hid_t gid = -1, oid = -1, aid = -1;

        char *group_path = NULL, *link_name = NULL, *ipath = NULL, *attr_name = NULL;

        try
        {
            DriveInfo^ drive = nullptr;
            String^ h5path = nullptr;
            if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path))
            {
                throw gcnew PSH5XException("Ill-formed HDF5 path name and/or unable to obtain drive name!");
            }

            String^ groupPath = ProviderUtils::ParentPath(h5path);

            group_path = (char*)(Marshal::StringToHGlobalAnsi(groupPath)).ToPointer();

            gid = H5Gopen2(drive->FileHandle, group_path, H5P_DEFAULT);
            if (gid < 0) {
                throw gcnew HDF5Exception("H5Gopen2 failed!");
            }

            String^ linkName = ProviderUtils::ChildName(h5path);
            link_name = (char*)(Marshal::StringToHGlobalAnsi(linkName)).ToPointer();

            if (ProviderUtils::IsH5RootPathName(groupPath) || H5Lexists(gid, link_name, H5P_DEFAULT) > 0)
            {
                H5L_info_t info;
                info.type = H5L_TYPE_ERROR;

                if (ProviderUtils::IsH5RootPathName(groupPath) ||
                    H5Lget_info(gid, link_name, &info, H5P_DEFAULT) >= 0)
                {
                    if (ProviderUtils::IsH5RootPathName(groupPath) || info.type == H5L_TYPE_HARD)
                    {
                        ipath =  (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

                        oid = H5Oopen(drive->FileHandle, ipath, H5P_DEFAULT);
                        if (oid < 0) {
                            throw gcnew HDF5Exception("H5Oopen failed!");
                        }

                        for each (String^ attributeName in providerSpecificPickList)
                        {
                            attr_name = (char*)(Marshal::StringToHGlobalAnsi(attributeName)).ToPointer();

                            htri_t flag = H5Aexists(oid, attr_name);

                            if (flag > 0)
                            {
                                aid = H5Aopen(oid, attr_name, H5P_DEFAULT);
                                if (aid < 0) {
                                    throw gcnew HDF5Exception("H5Aopen failed!");
                                }

                                WritePropertyObject(ProviderUtils::H5Attribute(aid, attributeName), path);

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
                    else
                    {
                        throw gcnew PSH5XException("The item type for this path has no attributes.");
                    }
                }
                else { // TODO
                }
            }
            else
            {
                WriteWarning(String::Format("Item not found at Path = '{0}'", path));
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

            if (gid >= 0) {
                H5Gclose(gid);
            }

            if (attr_name != NULL) {
                Marshal::FreeHGlobal(IntPtr(attr_name));
            }

            if (ipath != NULL) {
                Marshal::FreeHGlobal(IntPtr(ipath));
            }

            if (link_name != NULL) {
                Marshal::FreeHGlobal(IntPtr(link_name));
            }

            if (group_path != NULL) {
                Marshal::FreeHGlobal(IntPtr(group_path));
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

    void Provider::SetProperty(String^ path, PSObject^ propertyValue)
    {
        WriteVerbose(String::Format("HDF5Provider::SetProperty(Path = '{0}')", path));

        hid_t oid = -1, attr = -1, fspace = -1;

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
			if (drive->ReadOnly)
			{
				throw gcnew PSH5XException("The drive is read-only and cannot be modified!");
			}
			if (!ProviderUtils::IsResolvableH5Path(drive->FileHandle, h5path))
			{
				throw gcnew PSH5XException(String::Format("HDF5 object '{0}' not found", path));
			}

#pragma endregion

			path_str =  (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

			oid = H5Oopen(drive->FileHandle, path_str, H5P_DEFAULT);
			if (oid < 0) {
				throw gcnew HDF5Exception("H5Oopen failed!!!");
			}

			for each (PSPropertyInfo^ info in propertyValue->Properties)
			{
				String^ attributeName = info->Name;
				attr_name = (char*)(Marshal::StringToHGlobalAnsi(attributeName)).ToPointer();

				if (H5Aexists(oid, attr_name) > 0)
				{
					attr = H5Aopen(oid, attr_name, H5P_DEFAULT);
					if (attr < 0) {
						throw gcnew HDF5Exception("H5Aopen failed!!!");
					}

					fspace = H5Aget_space(attr);
					if (fspace < 0) {
						throw gcnew HDF5Exception("H5Aget_space failed!");
					}
				
					H5S_class_t stype = H5Sget_simple_extent_type(fspace);
					
					Object^ obj = nullptr;
					if (info->Value->GetType() == System::Management::Automation::PSObject::typeid)	{
						obj = safe_cast<System::Management::Automation::PSObject^>(info->Value)->BaseObject;
					}
					else {
						obj = info->Value;
					}
					
					if (obj != nullptr)
					{
						if (this->ShouldProcess(h5path,
							String::Format("Setting HDF5 attribute '{0}'", attributeName)))
						{
							if (stype == H5S_SIMPLE) {
								ProviderUtils::SetH5AttributeValue(attr, obj);
							}
							else if (stype == H5S_SCALAR) {
								ProviderUtils::SetScalarH5AttributeValue(attr, obj);
							}

							if (H5Fflush(oid, H5F_SCOPE_LOCAL) < 0) {
								throw gcnew HDF5Exception("H5Fflush failed!");
							}
						}
					}
					else {
						throw gcnew PSH5XException("Empty attribute value!");
					}
					
					if (H5Sclose(fspace) < 0) {
						throw gcnew HDF5Exception("H5Sclose failed!");
					}
					else {
						fspace = -1;
					}

					if (H5Aclose(attr) < 0) {
						throw gcnew HDF5Exception("H5Aclose failed!");
					}
					else {
						attr = -1;
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
			if (fspace >= 0) {
                H5Sclose(fspace);
            }

            if (attr >= 0) {
                H5Aclose(attr);
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

    Object^ Provider::SetPropertyDynamicParameters(String^ path, PSObject^ propertyValue)
    {
        WriteVerbose(String::Format("HDF5Provider::SetPropertyDynamicParameters(Path = '{0}')", path));
        return nullptr;
    }

}