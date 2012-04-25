
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
    void Provider::GetChildItems(System::String^ path, bool recurse)
    {
        WriteVerbose(String::Format("HDF5Provider::GetChildItems(Path = '{0}', recurse = '{1}')",
            path, recurse));

        hid_t gid = -1, oid = -1;

        char *gpath = NULL, *link_name = NULL;

#pragma region item type filter

        Hashtable dropItem = gcnew Hashtable();
        dropItem["D"] = false;
        dropItem["EL"] = false;
        dropItem["G"] = false;
        dropItem["SL"] = false;
        dropItem["T"] = false;

        if (Filter != nullptr)
        {
            String^ s = String::Copy(Filter)->Trim()->ToUpper();

            if (!s->StartsWith("-") && s->Contains("-")) {
                throw gcnew PSH5XException("Invalid filter expression!");
            }

            if (s->StartsWith("-"))
            {                
                array<wchar_t>^ a = s->Substring(1)->ToCharArray();
                for (int i = 0; i < a->Length; ++i)
                {
                    if (a[i] == 'D') {
                        dropItem["D"] = true;
                    }
                    else if (a[i] == 'E') {
                        dropItem["EL"] = true;
                    }
                    else if (a[i] == 'G') {
                        dropItem["G"] = true;
                    }
                    else if (a[i] == 'S') {
                        dropItem["SL"] = true;
                    }
                    else if (a[i] == 'T') {
                        dropItem["T"] = true;
                    }
                }
            }
            else
            {
                dropItem["D"] = true;
                dropItem["EL"] = true;
                dropItem["G"] = true;
                dropItem["SL"] = true;
                dropItem["T"] = true;

                array<wchar_t>^ a = s->ToCharArray();
                for (int i = 0; i < a->Length; ++i)
                {
                    if (a[i] == 'D') {
                        dropItem["D"] = false;
                    }
                    else if (a[i] == 'E') {
                        dropItem["EL"] = false;
                    }
                    else if (a[i] == 'G') {
                        dropItem["G"] = false;
                    }
                    else if (a[i] == 'S') {
                        dropItem["SL"] = false;
                    }
                    else if (a[i] == 'T') {
                        dropItem["T"] = false;
                    }
                }
            }
        }

#pragma endregion

        try
        {
            DriveInfo^ drive = nullptr;
            String^ h5path = nullptr;
            if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path))
            {
                throw gcnew PSH5XException("Ill-formed HDF5 path name and/or unable to obtain drive name!");
            }

            bool detailed = false;
            RuntimeDefinedParameterDictionary^ dynamicParameters =
                (RuntimeDefinedParameterDictionary^) DynamicParameters;
            if (dynamicParameters != nullptr && dynamicParameters->ContainsKey("Detailed")) {
                detailed = dynamicParameters["Detailed"]->IsSet;
            }
            
            if (ProviderUtils::IsH5Group(drive->FileHandle, h5path))
            {
                gpath = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

                gid = H5Gopen2(drive->FileHandle, gpath, H5P_DEFAULT);
                if (gid < 0) {
                    throw gcnew HDF5Exception("H5Gopen2 failed!!!");
                }

                array<String^>^ linkNames = ProviderUtils::GetGroupLinkNames(gid, recurse);

                for each (String^ linkName in linkNames)
                {
                    String^ childPath = path;
                    if (path != (drive->Name + ":\\"))
                        childPath += "\\";
                    childPath += linkName->Replace('/','\\');

                    link_name = (char*)(Marshal::StringToHGlobalAnsi(linkName)).ToPointer();
                    H5L_info_t linfo;
                    if (H5Lget_info(gid, link_name, &linfo, H5P_DEFAULT) >= 0)
                    {
                        switch (linfo.type)
                        {
                        case H5L_TYPE_HARD:
							{
#pragma region HDF5 hard link
								oid = H5Oopen(gid, link_name, H5P_DEFAULT);
								if (oid < 0) {
									throw gcnew HDF5Exception("H5Oopen failed!");
								}

								H5O_info_t oinfo;
								if (H5Oget_info(oid, &oinfo) >= 0)
								{
									switch (oinfo.type)
									{
									case H5O_TYPE_GROUP:
										{
											if ((bool) dropItem["G"]) {
												break;
											}

											if (!detailed) {
												WriteItemObject(gcnew GroupInfoLite(oid), childPath,
													true);
											}
											else {
												WriteItemObject(gcnew GroupInfo(oid), childPath, true);
											}
										}
										break;

									case H5O_TYPE_DATASET:
										{
											if ((bool) dropItem["D"]) {
												break;
											}

											if (!detailed) {
												WriteItemObject(gcnew DatasetInfoLite(oid), childPath,
													false);
											}
											else {
												WriteItemObject(gcnew DatasetInfo(oid), childPath,
													false);
											}
										}
										break;

									case H5O_TYPE_NAMED_DATATYPE:
										{
											if ((bool) dropItem["T"]) {
												break;
											}

											WriteItemObject(gcnew DatatypeInfo(oid), childPath, false);
										}
										break;

									default:

										WriteItemObject(gcnew ObjectInfo(oid), childPath, false);
										break;
									}
								}
								else {
									throw gcnew HDF5Exception("H5Oget_info failed!");
								}

								if (H5Oclose(oid) < 0) {
									throw gcnew HDF5Exception("H5Oclose failed!");
								}
								oid = -1;
#pragma endregion
							}
                            break;

                        case H5L_TYPE_SOFT:
							{
								if ((bool) dropItem["SL"]) {
									break;
								}

								WriteItemObject(gcnew LinkInfo(gid, linkName),
									childPath, false);
							}
                            break;

                        case H5L_TYPE_EXTERNAL:
							{
								if ((bool) dropItem["EL"]) {
									break;
								}

								WriteItemObject(gcnew LinkInfo(gid, linkName),
									childPath, false);
							}
                            break;

                        default:

                            throw gcnew PSH5XException("Unable to determine the item type for this path!");
                            break;
                        }
                    }
                    else {
                        throw gcnew HDF5Exception("H5Lget_info failed!!!");
                    }

                    Marshal::FreeHGlobal(IntPtr(link_name));
                    link_name = NULL;
                }
            }
        }
        finally
        {
            if (oid >= 0) {
                H5Oclose(oid);
            }
            if (gid >= 0) {
                H5Gclose(gid);
            }
            if (link_name != NULL) {
                Marshal::FreeHGlobal(IntPtr(link_name));
            }
            if (gpath != NULL) {
                Marshal::FreeHGlobal(IntPtr(gpath));
            }
        }

        return;

    }

    Object^ Provider::GetChildItemsDynamicParameters(System::String^ path, bool recurse)
    {
        WriteVerbose(String::Format("HDF5Provider::GetChildItemsDynamicParameters(Path = '{0}', recurse = '{1}')",
            path, recurse));

        RuntimeDefinedParameterDictionary^ dynamicParameters =
            gcnew RuntimeDefinedParameterDictionary();

        Collection<Attribute^>^ atts1 = gcnew Collection<Attribute^>();
        ParameterAttribute^ paramAttr1 = gcnew ParameterAttribute();
        paramAttr1->Mandatory = false;
        paramAttr1->ValueFromPipeline = false;
        atts1->Add(paramAttr1);
        dynamicParameters->Add("Detailed",
            gcnew RuntimeDefinedParameter("Detailed", SwitchParameter::typeid, atts1));
        
        return dynamicParameters;
    }

    void Provider::GetChildNames(System::String^ path, ReturnContainers returnContainers)
    {
        WriteVerbose(String::Format("HDF5Provider::GetChildNames(Path = '{0}', ReturnContainers = '{1}')",
            path, returnContainers));

        hid_t gid = -1;

        char *gpath = NULL, *link_name = NULL;

        try
        {
            DriveInfo^ drive = nullptr;
            String^ h5path = nullptr;
            if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path))
            {
                throw gcnew PSH5XException("Ill-formed HDF5 path name and/or unable to obtain drive name!");
            }

            if (ProviderUtils::IsH5Group(drive->FileHandle, h5path))
            {
				gpath = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

                gid = H5Gopen2(drive->FileHandle, gpath, H5P_DEFAULT);
                if (gid < 0) {
                    throw gcnew HDF5Exception("H5Gopen2 failed!");
                }

                array<String^>^ linkNames = ProviderUtils::GetGroupLinkNames(gid, false);

                for each (String^ linkName in linkNames)
                {
                    String^ childPath = nullptr;
					if (!path->EndsWith("\\")) {
						childPath = path + "\\" + linkName->Replace('/','\\');
					}
					else {
						childPath = path + linkName->Replace('/','\\');
					}

                    link_name = (char*)(Marshal::StringToHGlobalAnsi(linkName)).ToPointer();

                    H5L_info_t info;
                    if (H5Lget_info(gid, link_name, &info, H5P_DEFAULT) >= 0)
                    {
                        switch (info.type)
                        {
                        case H5L_TYPE_HARD:

                            switch (info.type)
                            {
                            case H5O_TYPE_GROUP:
                                WriteItemObject(linkName, childPath, true);
                                break;
                            default:
                                WriteItemObject(linkName, childPath, false);
                                break;
                            }
                            break;

                        default:
                            WriteItemObject(linkName, childPath, false);
                            break;
                        }
                    }
                    else {
                        throw gcnew HDF5Exception("H5Lget_info failed!");
                    }

					Marshal::FreeHGlobal(IntPtr(link_name));
					link_name = NULL;
                }
            }
        }
        finally
        {
            if (gid >= 0) {
                H5Gclose(gid);
            }
            if (link_name != NULL) {
                Marshal::FreeHGlobal(IntPtr(link_name));
            }
            if (gpath != NULL) {
                Marshal::FreeHGlobal(IntPtr(gpath));
            }
        }

        return;
    }

    /*
    Object^ Provider::GetChildNamesDynamicParameters(System::String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::GetChildNamesDynamicParameters(Path = '{0}')",
            path));
        return nullptr;
    }
    */

    bool Provider::HasChildItems(System::String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::HasChildItems(Path = '{0}')", path));

        bool result = false;

        hid_t gid = -1;

        char* gpath = NULL;

        try
        {
            DriveInfo^ drive = nullptr;
            String^ h5path = nullptr;
            if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path))
            {
                throw gcnew PSH5XException("Ill-formed HDF5 path name and/or unable to obtain drive name!");
            }

            if (ProviderUtils::IsH5Group(drive->FileHandle, h5path))
            {
                gpath = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

                gid = H5Gopen2(drive->FileHandle, gpath, H5P_DEFAULT);
                if (gid < 0) {
                    throw gcnew HDF5Exception("H5Gopen2 failed!");
                }

                H5G_info_t info;
                if (H5Gget_info(gid, &info) >= 0) {
                    result = (info.nlinks > 0);
                }
                else {
                    throw gcnew HDF5Exception("H5Gget_info failed!");
                }
            }
        }
        finally
        {
            if (gid >= 0) {
                H5Gclose(gid);
            }
            if (gpath != NULL) {
                Marshal::FreeHGlobal(IntPtr(gpath));
            }
        }

        return result;
    }
}
