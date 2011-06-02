
#include "AttributeInfo.h"
#include "Provider.h"
#include "ProviderUtils.h"

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
        WriteWarning("The HDF5Provider::CopyProperty() method has not (yet) been implemented.");
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
        WriteWarning("The HDF5Provider::MoveProperty() method has not (yet) been implemented.");
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

        // Normalize path & get drive
        String^ npath = ProviderUtils::NormalizePath(path);
        String^ h5path = ProviderUtils::PathNoDrive(npath);

        if (!ProviderUtils::IsWellFormedH5Path(h5path))
        {
            WriteWarning(String::Format("'{0}' is not a well-formed HDF5 path name.", h5path));
            return;
        }
        DriveInfo^ drive = ProviderUtils::GetDriveFromPath(path, ProviderInfo);
        if (drive == nullptr)
        {
            WriteWarning(String::Format("Cannot obtain drive from path!.", h5path));
            return;
        }

        if (!ProviderUtils::IsValidAbsoluteH5Path(drive->FileHandle, h5path))
        {
            ErrorRecord^ error = gcnew ErrorRecord(
                gcnew ArgumentException(String::Format("Item '{0}' does not exist", path)),
                "ItemExists", ErrorCategory::InvalidData, nullptr);
            ThrowTerminatingError(error);
        }
        
        if (!ProviderUtils::IsH5Object(drive->FileHandle, h5path))
        {
            ErrorRecord^ error = gcnew ErrorRecord(
                gcnew ArgumentException(String::Format(
                "Item '{0}' is not an HDF5 dataset, group, or datatype object and cannot have attributes", path)),
                "ItemWrongType", ErrorCategory::InvalidData, nullptr);
            ThrowTerminatingError(error);
        }

        hid_t h5type = -1, h5space = -1, h5obj = -1, h5attr = -1;

        try
        {
            RuntimeDefinedParameterDictionary^ dynamicParameters =
                (RuntimeDefinedParameterDictionary^) DynamicParameters;

            // mandatory parameters -ElementType

            Object^ elemType = dynamicParameters["ElementType"]->Value;

            Hashtable^ ht = nullptr;
            String^ typeOrPath = nullptr;

            if (ProviderUtils::TryGetValue(elemType, ht)) {
                h5type = ProviderUtils::ParseH5Type(ht);
                if (h5type < 0) {
                    throw gcnew ArgumentException("Invalid HDF5 datatype specified!");
                }
            }
            else if (ProviderUtils::TryGetValue(elemType, typeOrPath))
            {
                if (typeOrPath->StartsWith("/")) {
                    if (ProviderUtils::IsH5DatatypeObject(drive->FileHandle, typeOrPath))
                    {
                        char* path = (char*)(Marshal::StringToHGlobalAnsi(typeOrPath)).ToPointer();
                        h5type = H5Topen2(drive->FileHandle, path, H5P_DEFAULT);
                    }
                    else {
                        throw gcnew
                            ArgumentException("The HDF5 path name specified does not refer to an datatype object.");
                    }
                }
                else
                {
                    h5type = ProviderUtils::H5Type(typeOrPath);
                    if (h5type < 0) {
                        throw gcnew ArgumentException("Invalid HDF5 datatype specified!");
                    }
                }
            }
            else {
                throw gcnew ArgumentException("Unrecognized type: must be string or hashtable.");
            }

            // optional parameters, determine shape first

            String^ shape = "scalar";

            bool isNull = (dynamicParameters["Null"]->IsSet);
            bool isSimple = (dynamicParameters["Simple"]->Value != nullptr);

            if (isNull && isSimple) {
                throw gcnew ArgumentException("The -Null and -Simple options are incompatible. Choose one!");
            }
            if (isNull)   { shape = "Null";   }
            if (isSimple) { shape = "Simple"; }

            char* attr_name = (char*)(Marshal::StringToHGlobalAnsi(propertyName)).ToPointer();
            char* obj_path = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();
            h5obj = H5Oopen(drive->FileHandle, obj_path, H5P_DEFAULT);

            if (h5obj >= 0)
            {
                if (H5Aexists(h5obj, attr_name) > 0) {
                    throw gcnew ArgumentException(String::Format(
                        "Item '{0}' already has a property named '{1}'", path, propertyName));
                }

                if (shape->ToUpper() == "NULL") {
                   h5space = H5Screate(H5S_NULL);
                } else if (shape->ToUpper() == "SCALAR") {
                   h5space = H5Screate(H5S_SCALAR);
                } else if (shape->ToUpper() == "SIMPLE") {

                    h5space = H5Screate(H5S_SIMPLE);
                    array<hsize_t>^ dims = (array<hsize_t>^) dynamicParameters["Simple"]->Value;
                    
                    int rank = dims->Length;
                    hsize_t* current_size = new hsize_t [rank];
                    for (int i = 0; i < rank; ++i) { current_size[i] = dims[i]; }

                    if (H5Sset_extent_simple(h5space, rank, current_size, current_size) < 0) {
                        delete [] current_size;
                        throw gcnew InvalidOperationException("H5Sset_extent_simple failed!");
                    }
                    delete [] current_size;
                }
                else {
                    throw gcnew ArgumentException("Invalid shape!");
                }

                if (h5space >= 0)
                {
                    if (this->ShouldProcess(h5path,
                        String::Format("HDF5 attribute '{0}' does not exist, create it",
                        propertyName)))
                    {
                        h5attr = H5Acreate2(h5obj, attr_name, h5type, h5space,
                            H5P_DEFAULT, H5P_DEFAULT);

                        if (h5attr >= 0)
                        {
                            if (H5Fflush(h5attr, H5F_SCOPE_LOCAL) < 0) {
                                WriteWarning("H5Fflush failed!");
                            }

                            Hashtable^ ht = ProviderUtils::H5Attribute(h5attr, propertyName);
                            WritePropertyObject(ht, path);
                        }
                        else {
                            throw gcnew InvalidOperationException("H5Acreate2 failed!");
                        }
                    }
                }
                else {
                    throw gcnew InvalidOperationException("H5Screate failed!");
                }
            }
            else {
                throw gcnew InvalidOperationException("H5Oopen failed!");
            }
        }
        catch (Exception^ ex)
        {
            ErrorRecord^ error = gcnew ErrorRecord(ex, "InvalidData", ErrorCategory::InvalidData, nullptr);
            ThrowTerminatingError(error);
        }
        finally
        {
            if (h5attr >= 0) {
                if (H5Aclose(h5attr)) {
                    WriteWarning("H5Aclose failed.");
                }
            }

            if (h5space >= 0) {
                if (H5Sclose(h5space)) {
                    WriteWarning("H5Sclose failed.");
                }
            }

            if (h5obj >= 0) {
                if (H5Oclose(h5obj) < 0) {
                     WriteWarning("H5Oclose failed.");
                }
            }

            if (h5type >= 0) {
                if (H5Tclose(h5type)) {
                    WriteWarning("H5Tclose failed.");
                }
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
        String^ npath = ProviderUtils::NormalizePath(path);
        String^ h5path = ProviderUtils::PathNoDrive(npath);
        if (!ProviderUtils::IsWellFormedH5Path(h5path)) { return; }
        DriveInfo^ drive = ProviderUtils::GetDriveFromPath(path, ProviderInfo);
        if (drive == nullptr) { return; }

        if (!ProviderUtils::IsValidAbsoluteH5Path(drive->FileHandle, h5path))
        {
            ErrorRecord^ error = gcnew ErrorRecord(
                gcnew ArgumentException(String::Format("Item '{0}' does not exist", path)),
                "ItemExists", ErrorCategory::InvalidData, nullptr);
            ThrowTerminatingError(error);
        }

        if (!ProviderUtils::IsH5Object(drive->FileHandle, h5path))
        {
            ErrorRecord^ error = gcnew ErrorRecord(
                gcnew ArgumentException(String::Format(
                "Item '{0}' is not an HDF5 dataset, group, or datatype object and cannot have attributes", path)),
                "ItemWrongType", ErrorCategory::InvalidData, nullptr);
            ThrowTerminatingError(error);
        }

        char* attr_name = (char*)(Marshal::StringToHGlobalAnsi(propertyName)).ToPointer();
        char* obj_path = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();
        hid_t obj_id = H5Oopen(drive->FileHandle, obj_path, H5P_DEFAULT);
        
        if (obj_id >= 0)
        {
            if (H5Aexists(obj_id, attr_name) <= 0)
            {
                H5Oclose(obj_id);
                ErrorRecord^ error = gcnew ErrorRecord(
                    gcnew ArgumentException(String::Format(
                    "Item '{0}' doesn't have an HDF5 attribute named '{1}'", path, propertyName)),
                    "NoSuchProperty", ErrorCategory::InvalidData, nullptr);
                ThrowTerminatingError(error);
            }
            else
            {
                if (this->ShouldProcess(h5path,
                    String::Format("Removing HDF5 attribute '{0}' from item '{1}'", propertyName, path)))
                {
                    if (H5Adelete(obj_id, attr_name) >= 0)
                    {
                        if (H5Fflush(obj_id, H5F_SCOPE_LOCAL) < 0) { // TODO
                        }
                    }
                    else { // TODO
                    }
                }
            }

            if (H5Oclose(obj_id) < 0) { // TODO
            }
        }
        else { // TODO
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
        String^ npath = ProviderUtils::NormalizePath(path);
        String^ h5path = ProviderUtils::PathNoDrive(npath);
        if (!ProviderUtils::IsWellFormedH5Path(h5path)) { return; }
        DriveInfo^ drive = ProviderUtils::GetDriveFromPath(path, ProviderInfo);
        if (drive == nullptr) { return; }

        if (!ProviderUtils::IsValidAbsoluteH5Path(drive->FileHandle, h5path))
        {
            ErrorRecord^ error = gcnew ErrorRecord(
                gcnew ArgumentException(String::Format("Item '{0}' does not exist", path)),
                "ItemExists", ErrorCategory::InvalidData, nullptr);
            ThrowTerminatingError(error);
        }

        if (!ProviderUtils::IsH5Object(drive->FileHandle, h5path))
        {
            ErrorRecord^ error = gcnew ErrorRecord(
                gcnew ArgumentException(String::Format(
                "Item '{0}' is not an HDF5 dataset, group, or datatype object and cannot have attributes", path)),
                "ItemWrongType", ErrorCategory::InvalidData, nullptr);
            ThrowTerminatingError(error);
        }

        char* old_attr_name = (char*)(Marshal::StringToHGlobalAnsi(sourceProperty)).ToPointer();
        char* new_attr_name = (char*)(Marshal::StringToHGlobalAnsi(destinationProperty)).ToPointer();

        char* obj_path = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();
        hid_t obj_id = H5Oopen(drive->FileHandle, obj_path, H5P_DEFAULT);
        
        if (obj_id >= 0)
        {
            if (H5Aexists(obj_id, old_attr_name) <= 0)
            {
                H5Oclose(obj_id);
                ErrorRecord^ error = gcnew ErrorRecord(
                    gcnew ArgumentException(String::Format(
                    "Item '{0}' doesn't have an HDF5 attribute named '{1}'", path, sourceProperty)),
                    "NoSuchProperty", ErrorCategory::InvalidData, nullptr);
                ThrowTerminatingError(error);
            }
            else
            {
                if (this->ShouldProcess(h5path,
                    String::Format("Renaming HDF5 attribute '{0}' to '{1}' for item '{2}'",
                    sourceProperty, destinationProperty, path)))
                {
                    if (H5Arename(obj_id, old_attr_name, new_attr_name) >= 0)
                    {
                        if (H5Fflush(obj_id, H5F_SCOPE_LOCAL) < 0) { // TODO
                        }

                        hid_t attr_id = H5Aopen(obj_id, new_attr_name, H5P_DEFAULT);
                        if (attr_id >= 0)
                        {
                            Hashtable^ ht = ProviderUtils::H5Attribute(attr_id, destinationProperty);

                            WritePropertyObject(ht, path);

                            if (H5Aclose(attr_id) < 0) { // TODO
                            }
                        }
                        else { // TODO
                        }
                    }
                    else { // TODO
                    }
                }
            }

            if (H5Oclose(obj_id) < 0) { // TODO
            }
        }
        else { // TODO
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