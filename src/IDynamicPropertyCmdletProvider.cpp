
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
    void Provider::CopyProperty(String^ sourcePath, String^ sourceProperty,
        String^ destinationPath, String^ destinationProperty)
    {
        WriteVerbose(
            String::Format("HDF5Provider::CopyProperty(SourcePath = '{0}',SourceProperty='{1}',DestinationPath = '{2}',DestinationProperty='{3}')",
            sourcePath, sourceProperty, destinationPath, destinationProperty));
        
		char *src_path_str = NULL, *src_attr_name_str = NULL, *dst_path_str = NULL, *dst_attr_name_str = NULL;

		hid_t src_attr = -1, type = -1, space = -1, dst_attr = -1;

		char* buffer = NULL;

		try
		{
#pragma region sanity check

			DriveInfo^ sourceDrive = nullptr;
			String^ sourceH5path = nullptr;
			if (!ProviderUtils::TryGetDriveEtH5Path(sourcePath, ProviderInfo, sourceDrive, sourceH5path))
			{
				throw gcnew PSH5XException("Ill-formed HDF5 path name and/or unable to obtain drive name!");
			}
			DriveInfo^ destinationDrive = nullptr;
			String^ destinationH5path = nullptr;
			if (!ProviderUtils::TryGetDriveEtH5Path(destinationPath, ProviderInfo, destinationDrive, destinationH5path))
			{
				throw gcnew PSH5XException("Ill-formed HDF5 path name and/or unable to obtain drive name!");
			}

			if (destinationDrive->ReadOnly)
			{
				throw gcnew PSH5XException("The destination drive is read-only and cannot be modified!");
			}

			if (!ProviderUtils::IsResolvableH5Path(sourceDrive->FileHandle, sourceH5path))
			{
				throw gcnew PSH5XException(String::Format("The source object '{0}' does not exist", sourcePath));
			}

			if (!ProviderUtils::IsResolvableH5Path(destinationDrive->FileHandle, destinationH5path))
			{
				throw gcnew PSH5XException(String::Format("The destination object '{0}' does not exist", destinationPath));
			}

			src_path_str = (char*) Marshal::StringToHGlobalAnsi(sourceH5path).ToPointer();
			dst_path_str = (char*) Marshal::StringToHGlobalAnsi(destinationH5path).ToPointer();
			src_attr_name_str = (char*) Marshal::StringToHGlobalAnsi(sourceProperty).ToPointer();
			dst_attr_name_str = (char*) Marshal::StringToHGlobalAnsi(destinationProperty).ToPointer();

			if (H5Aexists_by_name(sourceDrive->FileHandle, src_path_str, src_attr_name_str, H5P_DEFAULT) <= 0) {
				throw gcnew PSH5XException(
					String::Format("Attribute '{0}' on object '{1}' not found", sourceProperty, sourcePath));
			}
			if (H5Aexists_by_name(destinationDrive->FileHandle, dst_path_str, dst_attr_name_str, H5P_DEFAULT) > 0) {
				throw gcnew PSH5XException(
					String::Format("Attribute '{0}' on object '{1}' exists", destinationProperty, destinationPath));
			}

#pragma endregion

			if (this->ShouldProcess(sourceH5path,
                    String::Format("Copying HDF5 attribute '{0}' from '{1}' to '{2}'",
                    sourceProperty, sourcePath, destinationPath)))
			{
				src_attr = H5Aopen_by_name(sourceDrive->FileHandle, src_path_str, src_attr_name_str, H5P_DEFAULT, H5P_DEFAULT);
				if (src_attr < 0) {
					throw gcnew HDF5Exception("H5Aopen_by_name failed!!!");
				}
				type = H5Aget_type(src_attr);
				if (type < 0) {
					throw gcnew HDF5Exception("H5Aget_type failed!!!");
				}
				space = H5Aget_space(src_attr);
				if (space < 0) {
					throw gcnew HDF5Exception("H5Aget_space failed!!!");
				}
				dst_attr = H5Acreate_by_name(destinationDrive->FileHandle, dst_path_str, dst_attr_name_str, type, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
				if (dst_attr < 0) {
					throw gcnew HDF5Exception("H5Acreate_by_name failed!!!");
				}

				// TODO: check if we are dealing with a reference type and copy between different files -> TROUBLE

				H5S_class_t cls = H5Sget_simple_extent_type(space);
				if (cls != H5S_NULL)
				{
					if (cls == H5S_SIMPLE) {
						buffer = (char*)malloc(H5Tget_size(type));
					}
					else {
						buffer = (char*)malloc(H5Tget_size(type) * H5Sget_simple_extent_npoints(space));
					}

					if(H5Aread(src_attr, type, buffer) < 0) {
						throw gcnew HDF5Exception("H5Aread failed!!!");
					}

					if(H5Awrite(dst_attr, type, buffer) < 0) {
						throw gcnew HDF5Exception("H5Awrite failed!!!");
					}
				}
			}
		}
		finally
		{
			if (src_path_str != NULL) {
				Marshal::FreeHGlobal(IntPtr(src_path_str));
			}
			if (dst_path_str != NULL) {
				Marshal::FreeHGlobal(IntPtr(dst_path_str));
			}
			if (src_attr_name_str != NULL) {
				Marshal::FreeHGlobal(IntPtr(src_attr_name_str));
			}
			if (dst_attr_name_str != NULL) {
				Marshal::FreeHGlobal(IntPtr(dst_attr_name_str));
			}
			if (src_attr != -1) {
				H5Aclose(src_attr);
			}
			if (dst_attr != -1) {
				H5Aclose(dst_attr);
			}
			if (type != -1) {
				H5Tclose(type);
			}
			if (space != -1) {
				H5Sclose(space);
			}
			if (buffer != NULL) {
				free(buffer);
			}
		}

        return;
    }

    Object^ Provider::CopyPropertyDynamicParameters(String^ sourcePath, String^ sourceProperty,
        String^ destinationPath, String^ destinationProperty)
    {
        WriteVerbose(
            String::Format("HDF5Provider::CopyPropertyDynamicParameters(SourcePath = '{0}',SourceProperty='{1}',DestinationPath = '{2}',DestinationProperty='{3}')",
            sourcePath, sourceProperty, destinationPath, destinationProperty));
        return nullptr;
    }

    void Provider::MoveProperty(String^ sourcePath, String^ sourceProperty,
        String^ destinationPath, String^ destinationProperty)
    {
        WriteVerbose(
            String::Format("HDF5Provider::MoveProperty(SourcePath = '{0}',SourceProperty='{1}',DestinationPath = '{2}',DestinationProperty='{3}')",
            sourcePath, sourceProperty, destinationPath, destinationProperty));

		char *src_path_str = NULL, *src_attr_name_str = NULL, *dst_path_str = NULL, *dst_attr_name_str = NULL;

		hid_t src_attr = -1, type = -1, space = -1, dst_attr = -1;

		char* buffer = NULL;

		try
		{
#pragma region sanity check

			DriveInfo^ sourceDrive = nullptr;
			String^ sourceH5path = nullptr;
			if (!ProviderUtils::TryGetDriveEtH5Path(sourcePath, ProviderInfo, sourceDrive, sourceH5path))
			{
				throw gcnew PSH5XException("Ill-formed HDF5 path name and/or unable to obtain drive name!");
			}
			DriveInfo^ destinationDrive = nullptr;
			String^ destinationH5path = nullptr;
			if (!ProviderUtils::TryGetDriveEtH5Path(destinationPath, ProviderInfo, destinationDrive, destinationH5path))
			{
				throw gcnew PSH5XException("Ill-formed HDF5 path name and/or unable to obtain drive name!");
			}
			if (sourceDrive->ReadOnly)
			{
				throw gcnew PSH5XException("The source drive is read-only and cannot be modified!");
			}
			if (destinationDrive->ReadOnly)
			{
				throw gcnew PSH5XException("The destination drive is read-only and cannot be modified!");
			}

			if (!ProviderUtils::IsResolvableH5Path(sourceDrive->FileHandle, sourceH5path))
			{
				throw gcnew PSH5XException(String::Format("The source object '{0}' does not exist", sourcePath));
			}

			if (!ProviderUtils::IsResolvableH5Path(destinationDrive->FileHandle, destinationH5path))
			{
				throw gcnew PSH5XException(String::Format("The destination object '{0}' does not exist", destinationPath));
			}

			src_path_str = (char*) Marshal::StringToHGlobalAnsi(sourceH5path).ToPointer();
			dst_path_str = (char*) Marshal::StringToHGlobalAnsi(destinationH5path).ToPointer();
			src_attr_name_str = (char*) Marshal::StringToHGlobalAnsi(sourceProperty).ToPointer();
			dst_attr_name_str = (char*) Marshal::StringToHGlobalAnsi(destinationProperty).ToPointer();

			if (H5Aexists_by_name(sourceDrive->FileHandle, src_path_str, src_attr_name_str, H5P_DEFAULT) <= 0) {
				throw gcnew PSH5XException(
					String::Format("Attribute '{0}' on object '{1}' not found", sourceProperty, sourcePath));
			}
			if (H5Aexists_by_name(destinationDrive->FileHandle, dst_path_str, dst_attr_name_str, H5P_DEFAULT) > 0) {
				throw gcnew PSH5XException(
					String::Format("Attribute '{0}' on object '{1}' exists", destinationProperty, destinationPath));
			}

#pragma endregion

			if (this->ShouldProcess(sourceH5path,
                    String::Format("Moving HDF5 attribute '{0}' from '{1}' to '{2}'",
                    sourceProperty, sourcePath, destinationPath)))
			{
				src_attr = H5Aopen_by_name(sourceDrive->FileHandle, src_path_str, src_attr_name_str, H5P_DEFAULT, H5P_DEFAULT);
				if (src_attr < 0) {
					throw gcnew HDF5Exception("H5Aopen_by_name failed!!!");
				}
				type = H5Aget_type(src_attr);
				if (type < 0) {
					throw gcnew HDF5Exception("H5Aget_type failed!!!");
				}
				space = H5Aget_space(src_attr);
				if (space < 0) {
					throw gcnew HDF5Exception("H5Aget_space failed!!!");
				}
				dst_attr = H5Acreate_by_name(destinationDrive->FileHandle, dst_path_str, dst_attr_name_str, type, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
				if (dst_attr < 0) {
					throw gcnew HDF5Exception("H5Acreate_by_name failed!!!");
				}

				// TODO: check if we are dealing with a reference type and copy between different files -> TROUBLE

				H5S_class_t cls = H5Sget_simple_extent_type(space);
				if (cls != H5S_NULL)
				{
					if (cls == H5S_SIMPLE) {
						buffer = (char*)malloc(H5Tget_size(type));
					}
					else {
						buffer = (char*)malloc(H5Tget_size(type) * H5Sget_simple_extent_npoints(space));
					}

					if(H5Aread(src_attr, type, buffer) < 0) {
						throw gcnew HDF5Exception("H5Aread failed!!!");
					}

					if(H5Awrite(dst_attr, type, buffer) < 0) {
						throw gcnew HDF5Exception("H5Awrite failed!!!");
					}
				}

				// close the source attribute before deletion
				H5Aclose(src_attr);
				src_attr = -1;

				if (H5Adelete_by_name(sourceDrive->FileHandle, src_path_str, src_attr_name_str, H5P_DEFAULT) < 0) {
					throw gcnew HDF5Exception("H5Adelete_by_name failed!!!");
				}
			}
		}
		finally
		{
			if (src_path_str != NULL) {
				Marshal::FreeHGlobal(IntPtr(src_path_str));
			}
			if (dst_path_str != NULL) {
				Marshal::FreeHGlobal(IntPtr(dst_path_str));
			}
			if (src_attr_name_str != NULL) {
				Marshal::FreeHGlobal(IntPtr(src_attr_name_str));
			}
			if (dst_attr_name_str != NULL) {
				Marshal::FreeHGlobal(IntPtr(dst_attr_name_str));
			}
			if (src_attr != -1) {
				H5Aclose(src_attr);
			}
			if (dst_attr != -1) {
				H5Aclose(dst_attr);
			}
			if (type != -1) {
				H5Tclose(type);
			}
			if (space != -1) {
				H5Sclose(space);
			}
			if (buffer != NULL) {
				free(buffer);
			}
		}

        return;
    }

    Object^ Provider::MovePropertyDynamicParameters(String^ sourcePath, String^ sourceProperty,
        String^ destinationPath, String^ destinationProperty)
    {
        WriteVerbose(
            String::Format("HDF5Provider::MovePropertyDynamicParameters(SourcePath = '{0}',SourceProperty='{1}',DestinationPath = '{2}',DestinationProperty='{3}')",
            sourcePath, sourceProperty, destinationPath, destinationProperty));
        return nullptr;
    }

    void Provider::NewProperty(String^ path, String^ propertyName, String^ propertyTypeName,
        Object^ value)
    {
        WriteVerbose(
            String::Format("HDF5Provider::NewProperty(Path = '{0}',PropertyName='{1}',propertyTypeName = '{2}')",
            path, propertyName, propertyTypeName));

        hid_t ftype = -1, fspace = -1, oid = -1, aid = -1;

        char *path_str = NULL, *attr_name = NULL, *obj_path = NULL;

        hsize_t* current_size = NULL;

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
                throw gcnew PSH5XException(String::Format("HDF5 object '{0}' does not exist", path));
            }

#pragma endregion

			attr_name = (char*)(Marshal::StringToHGlobalAnsi(propertyName)).ToPointer();
            obj_path = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

            RuntimeDefinedParameterDictionary^ dynamicParameters =
                (RuntimeDefinedParameterDictionary^) DynamicParameters;

#pragma region mandatory parameters

            String^ elemType = (String^) dynamicParameters["ElementType"]->Value;

			if (!elemType->Trim()->StartsWith("{")) // not JSON, alias or predefined type
				{
					if (elemType->StartsWith("/")) {
                        if (ProviderUtils::IsH5DatatypeObject(drive->FileHandle, elemType))
                        {
                            path_str = (char*)(Marshal::StringToHGlobalAnsi(elemType)).ToPointer();
                            ftype = H5Topen2(drive->FileHandle, path_str, H5P_DEFAULT);
                            if (ftype < 0) {
                                throw gcnew HDF5Exception("H5Topen2 failed!!!");
                            }
                        }
                        else {
                            throw gcnew PSH5XException("The HDF5 path name specified does not refer to an datatype object.");
                        }
                    }
                    else
                    {
                        ftype = ProviderUtils::H5Type(elemType);
                        if (ftype < 0) {
                            throw gcnew PSH5XException("Invalid HDF5 datatype specified!");
                        }
                    }
				}
				else
				{
					JavaScriptSerializer^ serializer = gcnew JavaScriptSerializer();
					Dictionary<String^,Object^>^ type = nullptr;
					try
					{
						type = safe_cast<Dictionary<String^,Object^>^>(serializer->DeserializeObject(elemType));
						
					}
					catch (...)
					{
						throw gcnew PSH5XException("JSON format error.");
					}

					ftype = ProviderUtils::ParseH5Type(type);
                    if (ftype < 0) {
                        throw gcnew PSH5XException("Invalid HDF5 datatype specified!");
                    }
				}

#pragma endregion

#pragma region optional parameters

            String^ shape = "scalar";

            bool isNull = (dynamicParameters["Null"]->IsSet);
            bool isSimple = (dynamicParameters["Simple"]->Value != nullptr);

            if (isNull && isSimple) {
                throw gcnew PSH5XException("The -Null and -Simple options are incompatible. Choose one!");
            }
            if (isNull)   { shape = "Null";   }
            if (isSimple) { shape = "Simple"; }

            oid = H5Oopen(drive->FileHandle, obj_path, H5P_DEFAULT);
            if (oid < 0) {
                throw gcnew HDF5Exception("H5Oopen failed!");
            }

            if (H5Aexists(oid, attr_name) > 0) {
                throw gcnew PSH5XException(String::Format(
                    "Item '{0}' already has a property named '{1}'", path, propertyName));
            }

            if (shape->ToUpper() == "NULL")
            {
                fspace = H5Screate(H5S_NULL);
                if (fspace < 0) {
                    throw gcnew HDF5Exception("H5Screate failed!");
                }
            }
            else if (shape->ToUpper() == "SCALAR")
            {
                fspace = H5Screate(H5S_SCALAR);
                if (fspace < 0) {
                    throw gcnew HDF5Exception("H5Screate failed!");
                }
            }
            else if (shape->ToUpper() == "SIMPLE")
            {
                fspace = H5Screate(H5S_SIMPLE);
                if (fspace < 0) {
                    throw gcnew Exception("H5Screate failed!");
                }

                array<hsize_t>^ dims = (array<hsize_t>^) dynamicParameters["Simple"]->Value;

                int rank = dims->Length;
                current_size = new hsize_t [rank];
                for (int i = 0; i < rank; ++i) { current_size[i] = dims[i]; }

                if (H5Sset_extent_simple(fspace, rank, current_size, current_size) < 0) {
                    throw gcnew HDF5Exception("H5Sset_extent_simple failed!");
                }
            }
            else {
                throw gcnew PSH5XException("Invalid shape!");
            }

			bool setValue = false;
			Object^ attrValue = nullptr;
			if (value != nullptr)
			{
				attrValue = ProviderUtils::GetDotNetObject(value);
				setValue = true;
			}

#pragma endregion

            if (this->ShouldProcess(h5path,
                String::Format("HDF5 attribute '{0}' does not exist, create it",
                propertyName)))
            {
                aid = H5Acreate2(oid, attr_name, ftype, fspace, H5P_DEFAULT, H5P_DEFAULT);
                if (aid < 0) {
                    throw gcnew HDF5Exception("H5Acreate2 failed!");
                }

				if (setValue)
				{
					if (shape->ToUpper() == "SIMPLE") {
						ProviderUtils::SetH5AttributeValue(aid, attrValue);
					}
					else if (shape->ToUpper() == "SCALAR") {
						ProviderUtils::SetScalarH5AttributeValue(aid, attrValue);
					}
				}

				if (H5Fflush(aid, H5F_SCOPE_LOCAL) < 0) {
                    throw gcnew HDF5Exception("H5Fflush failed!");
                }

                Hashtable^ ht = ProviderUtils::H5Attribute(aid, propertyName);
                WritePropertyObject(ht, path);
            }
        }
        finally
        {
            if (aid >= 0) {
                H5Aclose(aid);
            }

            if (current_size != NULL) {
                delete [] current_size;
            }

            if (fspace >= 0) {
                H5Sclose(fspace);
            }

            if (oid >= 0) {
                H5Oclose(oid);
            }

            if (ftype >= 0) {
                H5Tclose(ftype);
            }

            if (path_str != NULL) {
                Marshal::FreeHGlobal(IntPtr(path_str));
            }

			if (attr_name != NULL) {
                Marshal::FreeHGlobal(IntPtr(attr_name));
            }

			if (obj_path != NULL) {
                Marshal::FreeHGlobal(IntPtr(obj_path));
            }
        }

        return;
    }

    Object^ Provider::NewPropertyDynamicParameters(String^ path, String^ propertyName,
        String^ propertyTypeName, Object^ value)
    {
        WriteVerbose(
            String::Format("HDF5Provider::NewPropertyDynamicParameters(Path = '{0}', PropertyName='{1}',propertyTypeName = '{2}')",
            path, propertyName, propertyTypeName));

        RuntimeDefinedParameterDictionary^ dict = gcnew RuntimeDefinedParameterDictionary();

        // ElementType

        ParameterAttribute^ attr1 = gcnew ParameterAttribute();
        attr1->Mandatory = true;
        attr1->ValueFromPipeline = false;

        RuntimeDefinedParameter^ paramElementType = gcnew RuntimeDefinedParameter();
        paramElementType->Name = "ElementType";
        // can be String or Hashtable
        paramElementType->ParameterType = String::typeid;
        paramElementType->Attributes->Add(attr1);

        dict->Add("ElementType", paramElementType);

        // Simple

        ParameterAttribute^ attr2 = gcnew ParameterAttribute();
        attr2->Mandatory = false;
        attr2->ValueFromPipeline = false;

        RuntimeDefinedParameter^ paramSimple = gcnew RuntimeDefinedParameter();
        paramSimple->Name = "Simple";
        paramSimple->ParameterType = array<hsize_t>::typeid;
        paramSimple->Attributes->Add(attr2);

        dict->Add("Simple", paramSimple);

        // Null

        ParameterAttribute^ attr3 = gcnew ParameterAttribute();
        attr3->Mandatory = false;
        attr3->ValueFromPipeline = false;

        RuntimeDefinedParameter^ paramNull = gcnew RuntimeDefinedParameter();
        paramNull->Name = "Null";
        paramNull->ParameterType = SwitchParameter::typeid;
        paramNull->Attributes->Add(attr3);

        dict->Add("Null", paramNull);

        return dict;
    }

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