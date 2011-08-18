
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

using namespace System;
using namespace System::Collections;
using namespace System::Collections::ObjectModel;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Provider;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
    void Provider::CopyProperty(String^ sourcePath, String^ sourceProperty,
        String^ destinationPath, String^ destinationProperty)
    {
        WriteVerbose(
            String::Format("HDF5Provider::CopyProperty(SourcePath = '{0}',SourceProperty='{1}',DestinationPath = '{2}',DestinationProperty='{3}')",
            sourcePath, sourceProperty, destinationPath, destinationProperty));
        
        throw gcnew PSH5XException("The HDF5Provider::CopyProperty() method has not (yet) been implemented.");

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

        throw gcnew PSH5XException("The HDF5Provider::MoveProperty() method has not (yet) been implemented.");
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

        char *path_str = NULL;

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

            if (!ProviderUtils::IsValidAbsoluteH5Path(drive->FileHandle, h5path))
            {
                throw gcnew PSH5XException(String::Format("Item '{0}' does not exist", path));
            }

            if (!ProviderUtils::IsH5Object(drive->FileHandle, h5path))
            {
                throw gcnew PSH5XException(String::Format(
                    "Item '{0}' is not an HDF5 dataset, group, or datatype object and cannot have attributes", path));
            }

#pragma endregion

            RuntimeDefinedParameterDictionary^ dynamicParameters =
                (RuntimeDefinedParameterDictionary^) DynamicParameters;

            // mandatory parameters -ElementType

            Object^ elemType = dynamicParameters["ElementType"]->Value;

            Hashtable^ ht = nullptr;
            String^ typeOrPath = nullptr;

            if (ProviderUtils::TryGetValue(elemType, ht)) {
                ftype = ProviderUtils::ParseH5Type(ht);
                if (ftype < 0) {
                    throw gcnew PSH5XException("Invalid HDF5 datatype specified!");
                }
            }
            else if (ProviderUtils::TryGetValue(elemType, typeOrPath))
            {
                if (typeOrPath->StartsWith("/")) {
                    if (ProviderUtils::IsH5DatatypeObject(drive->FileHandle, typeOrPath))
                    {
                        path_str = (char*)(Marshal::StringToHGlobalAnsi(typeOrPath)).ToPointer();
                        ftype = H5Topen2(drive->FileHandle, path_str, H5P_DEFAULT);
                        if (ftype < 0) {
                            throw gcnew HDF5Exception("H5Topen2 failed!");
                        }
                    }
                    else {
                        throw gcnew
                            PSH5XException("The HDF5 path name specified does not refer to an datatype object.");
                    }
                }
                else
                {
                    ftype = ProviderUtils::H5Type(typeOrPath);
                    if (ftype < 0) {
                        throw gcnew PSH5XException("Invalid HDF5 datatype specified!");
                    }
                }
            }
            else {
                throw gcnew PSH5XException("Unrecognized type: must be string or hashtable.");
            }

            // optional parameters, determine shape first

            String^ shape = "scalar";

            bool isNull = (dynamicParameters["Null"]->IsSet);
            bool isSimple = (dynamicParameters["Simple"]->Value != nullptr);

            if (isNull && isSimple) {
                throw gcnew PSH5XException("The -Null and -Simple options are incompatible. Choose one!");
            }
            if (isNull)   { shape = "Null";   }
            if (isSimple) { shape = "Simple"; }

            char* attr_name = (char*)(Marshal::StringToHGlobalAnsi(propertyName)).ToPointer();
            char* obj_path = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

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

            // TODO: set the value

            if (this->ShouldProcess(h5path,
                String::Format("HDF5 attribute '{0}' does not exist, create it",
                propertyName)))
            {
                aid = H5Acreate2(oid, attr_name, ftype, fspace, H5P_DEFAULT, H5P_DEFAULT);
                if (aid < 0) {
                    throw gcnew HDF5Exception("H5Acreate2 failed!");
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
        paramElementType->ParameterType = Object::typeid;
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

            if (!ProviderUtils::IsValidAbsoluteH5Path(drive->FileHandle, h5path))
            {
                throw gcnew PSH5XException(String::Format("Item '{0}' does not exist", path));
            }

            if (!ProviderUtils::IsH5Object(drive->FileHandle, h5path))
            {
                throw gcnew PSH5XException(String::Format(
                    "Item '{0}' is not an HDF5 dataset, group, or datatype object and cannot have attributes", path));
            }

#pragma endregion

            attr_name = (char*)(Marshal::StringToHGlobalAnsi(propertyName)).ToPointer();
            obj_path = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

            oid = H5Oopen(drive->FileHandle, obj_path, H5P_DEFAULT);
            if (oid < 0) {
                throw gcnew HDF5Exception("H5Oopen failed!");
            }

            if (H5Aexists(oid, attr_name) <= 0)
            {
                throw gcnew PSH5XException(String::Format(
                    "Item '{0}' doesn't have an HDF5 attribute named '{1}'", path, propertyName));
            }

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
            if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path))
            {
                throw gcnew PSH5XException("Ill-formed HDF5 path name and/or unable to obtain drive name!");
            }

            if (drive->ReadOnly)
            {
                throw gcnew PSH5XException("The drive is read-only and cannot be modified!");
            }

            if (!ProviderUtils::IsValidAbsoluteH5Path(drive->FileHandle, h5path))
            {
                throw gcnew PSH5XException(String::Format("Item '{0}' does not exist", path));
            }

            if (!ProviderUtils::IsH5Object(drive->FileHandle, h5path))
            {
                throw gcnew PSH5XException(String::Format(
                    "Item '{0}' is not an HDF5 dataset, group, or datatype object and cannot have attributes", path));
            }

#pragma endregion

            old_attr_name = (char*)(Marshal::StringToHGlobalAnsi(sourceProperty)).ToPointer();
            new_attr_name = (char*)(Marshal::StringToHGlobalAnsi(destinationProperty)).ToPointer();

            obj_path = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

            oid = H5Oopen(drive->FileHandle, obj_path, H5P_DEFAULT);
            if (oid < 0) {
                throw gcnew HDF5Exception("H5Oopen failed!");
            }

            if (H5Aexists(oid, old_attr_name) <= 0)
            {
                throw gcnew PSH5XException(String::Format(
                    "Item '{0}' doesn't have an HDF5 attribute named '{1}'", path, sourceProperty));
            }
            else
            {
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