
#include "DatasetInfo.h"
#include "DatasetInfoLite.h"
#include "DatatypeInfo.h"
#include "GroupInfo.h"
#include "GroupInfoLite.h"
#include "LinkInfo.h"
#include "ObjectInfo.h"
#include "Provider.h"
#include "ProviderUtils.h"

extern "C" {
#include "H5Apublic.h"
#include "H5Gpublic.h"
#include "H5Lpublic.h"
#include "H5Opublic.h"
#include "H5Spublic.h"
}

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Collections::ObjectModel;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Provider;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
    void Provider::ClearItem(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::ClearItem(Path = '{0}')", path));
        WriteWarning("The HDF5Provider::ClearItem() method has not (yet) been implemented.");
        return;
    }

    Object^ Provider::ClearItemDynamicParameters(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::ClearItemDynamicParameters(Path = '{0}')", path));
        return nullptr;
    }

    bool Provider::IsValidPath(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::IsValidPath(Path = '{0}')", path));
        String^ npath = ProviderUtils::NormalizePath(path);
        String^ h5path = ProviderUtils::PathNoDrive(npath);
        return ProviderUtils::IsWellFormedH5Path(h5path);
    }

    void Provider::GetItem(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::GetItem(Path = '{0}')", path));
        
        DriveInfo^ drive = nullptr;
        String^ h5path = nullptr;
        if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path))
        {
            ErrorRecord^ error = gcnew ErrorRecord(
                gcnew ArgumentException("Ill-formed HDF5 path name and/or unable to obtain drive name!"),
                "InvalidData", ErrorCategory::InvalidData, nullptr);
            ThrowTerminatingError(error);
        }

        bool detailed = false;
        RuntimeDefinedParameterDictionary^ dynamicParameters =
            (RuntimeDefinedParameterDictionary^) DynamicParameters;
        if (dynamicParameters != nullptr && dynamicParameters->ContainsKey("Detailed"))
        {
            detailed = dynamicParameters["Detailed"]->IsSet;
        }

#pragma region root group

        if (ProviderUtils::IsH5RootPathName(h5path)) // root group
        {
            String^ rootName = "/";
            char* root_name =  (char*)(Marshal::StringToHGlobalAnsi(rootName)).ToPointer();
            
            hid_t group = H5Oopen(drive->FileHandle, root_name, H5P_DEFAULT);
            if (group >= 0)
            {
                if (!detailed)
                {
                    WriteItemObject(gcnew GroupInfoLite(group), path, true);
                }
                else
                {
                    WriteItemObject(gcnew GroupInfo(group), path, true);
                }

                if (H5Oclose(group) < 0) { // TODO
                }
            }
            else { // TODO
            }

            return;
        }

#pragma endregion

        String^ groupPath = ProviderUtils::ParentPath(h5path);
        char* group_path = (char*)(Marshal::StringToHGlobalAnsi(groupPath)).ToPointer();
        hid_t group_id = H5Gopen2(drive->FileHandle, group_path, H5P_DEFAULT);
        if (group_id >= 0)
        {
            String^ linkName = ProviderUtils::ChildName(h5path);
            char* link_name = (char*)(Marshal::StringToHGlobalAnsi(linkName)).ToPointer();
            if (H5Lexists(group_id, link_name, H5P_DEFAULT) > 0)
            {
                H5L_info_t linfo;
                if (H5Lget_info(group_id, link_name, &linfo, H5P_DEFAULT) >= 0)
                {
                    hid_t obj_id;
                    char* path_name;

                    switch (linfo.type)
                    {
                    case H5L_TYPE_HARD:

                        path_name =  (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();
                        obj_id = H5Oopen(drive->FileHandle, path_name, H5P_DEFAULT);
                        if (obj_id >= 0)
                        {
                            H5O_info_t oinfo;
                            if (H5Oget_info(obj_id, &oinfo) >= 0)
                            {
                                switch (oinfo.type)
                                {
                                case H5O_TYPE_GROUP:
                                    if (!detailed) {
                                        WriteItemObject(gcnew GroupInfoLite(obj_id), path, true);
                                    }
                                    else {
                                        WriteItemObject(gcnew GroupInfo(obj_id), path, true);
                                    }
                                    break;
                                case H5O_TYPE_DATASET:
                                    if (!detailed) {
                                        WriteItemObject(gcnew DatasetInfoLite(obj_id), path, false);
                                    }
                                    else {
                                        WriteItemObject(gcnew DatasetInfo(obj_id), path, false);
                                    }
                                    break;
                                case H5O_TYPE_NAMED_DATATYPE:
                                    WriteItemObject(gcnew DatatypeInfo(obj_id), path, false);
                                    break;
                                default:
                                    WriteItemObject(gcnew ObjectInfo(obj_id), path, false);
                                    break;
                                }
                            }

                            if (H5Oclose(obj_id) < 0) { // TODO
                            }
                        }
                        else { //TODO
                        }

                        break;

                    case H5L_TYPE_SOFT:
                        WriteItemObject(gcnew LinkInfo(group_id, linkName, "SoftLink"),
                            path, false);
                        break;
                    case H5L_TYPE_EXTERNAL:
                        WriteItemObject(gcnew LinkInfo(group_id, linkName, "ExtLink"),
                            path, false);
                        break;
                    default:
                        ErrorRecord^ error = gcnew ErrorRecord(
                            gcnew InvalidProgramException("Unable to determine the item type for this path"),
                            "H5L_TYPE", ErrorCategory::InvalidData, nullptr);
                        WriteError(error);
                        break;
                    }
                }
            }
            else
            {
                WriteWarning(String::Format("Item not found at Path = '{0}'", path));
            }

            if (H5Gclose(group_id) < 0) { // TODO
            }
        }
        
        return;
    }

    Object^ Provider::GetItemDynamicParameters(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::GetItemDynamicParameters(Path = '{0}')", path));
        
        RuntimeDefinedParameterDictionary^ dynamicParameters =
            gcnew RuntimeDefinedParameterDictionary();

        Collection<Attribute^>^ atts = gcnew Collection<Attribute^>();
        ParameterAttribute^ paramAttr = gcnew ParameterAttribute();
        paramAttr->Mandatory = false;
        paramAttr->ValueFromPipeline = false;
        atts->Add(paramAttr);
        dynamicParameters->Add("Detailed",
            gcnew RuntimeDefinedParameter("Detailed", SwitchParameter::typeid, atts));
        
        return dynamicParameters;
    }

    void Provider::InvokeDefaultAction(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::InvokeDefaultAction(Path = '{0}')", path));
        WriteWarning("The HDF5Provider::InvokeDefaultAction() method has not (yet) been implemented.");
        return;
    }

    Object^ Provider::InvokeDefaultActionDynamicParameters(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::InvokeDefaultActionDynamicParameters(Path = '{0}')", path));
        WriteWarning("The HDF5Provider::InvokeDefaultActionDynamicParameters() method has not (yet) been implemented.");
        return nullptr;
    }

    bool Provider::ItemExists(System::String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::ItemExists(Path = '{0}')",path));

        DriveInfo^ drive = nullptr;
        String^ h5path = nullptr;
        if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path))
        {
            ErrorRecord^ error = gcnew ErrorRecord(
                gcnew ArgumentException("Ill-formed HDF5 path name and/or unable to obtain drive name!"),
                "InvalidData", ErrorCategory::InvalidData, nullptr);
            ThrowTerminatingError(error);
        }

        return ProviderUtils::IsValidH5Path(drive->FileHandle, h5path);
        
    }

    Object^ Provider::ItemExistsDynamicParameters(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::ItemExistsDynamicParameters(Path = '{0}')", path));
        
        // add -itemType optional parameter so that existence and item type can be tested
        // e.g., Test-Path HDF5::h5:/foo/bar -itemType Group

        RuntimeDefinedParameterDictionary^ dynamicParameters =
            gcnew RuntimeDefinedParameterDictionary();

        Collection<Attribute^>^ atts = gcnew Collection<Attribute^>();
        ParameterAttribute^ paramAttr = gcnew ParameterAttribute();
        paramAttr->Mandatory = false;
        atts->Add(paramAttr);
        dynamicParameters->Add("itemType",
            gcnew RuntimeDefinedParameter("itemType", String::typeid, atts));
        
        return dynamicParameters;
    }

    // Dataset: if chunked and extendible call H5Dset_extent

    void Provider::SetItem(String^ path, Object^ value)
    {
        WriteVerbose(
            String::Format("HDF5Provider::SetItem(Path = '{0}', Value = '{1}')",
            path, value->ToString()));

        DriveInfo^ drive = nullptr;
        String^ h5path = nullptr;
        if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path))
        {
            ErrorRecord^ error = gcnew ErrorRecord(
                gcnew ArgumentException("Ill-formed HDF5 path name and/or unable to obtain drive name!"),
                "InvalidData", ErrorCategory::InvalidData, nullptr);
            ThrowTerminatingError(error);
        }

        RuntimeDefinedParameterDictionary^ dynamicParameters =
                    (RuntimeDefinedParameterDictionary^) DynamicParameters;

        if (ProviderUtils::IsH5Dataset(drive->FileHandle, h5path))
        {
#pragma region HDF5 dataset

            bool hasValue = (value != nullptr);
            bool hasDims = (dynamicParameters["Dimensions"]->Value != nullptr);
            bool hasHyper = (dynamicParameters["Hyperslab"]->Value != nullptr);
            bool hasPtSet = (dynamicParameters["PointSet"]->Value != nullptr);
            bool isForce = Force.IsPresent;
            bool isChunked = ProviderUtils::IsH5ChunkedDataset(drive->FileHandle, h5path);
                
            hid_t h5set = -1, h5space = -1;

            try
            {
                array<hsize_t>^ currDims = nullptr;
                if (ProviderUtils::TryGetValue(value, currDims))
                {
                    char* name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();
                    h5set = H5Dopen2(drive->FileHandle, name, H5P_DEFAULT);
                    if (h5set < 0) {
                        throw gcnew ArgumentException("H5Dopen failed."); }

                    if (isChunked)
                    {
                        h5space = H5Dget_space(h5set);
                        if (h5space >= 0)
                        {
                            int rank = H5Sget_simple_extent_ndims(h5space);
                            if (rank > 0)
                            {
                                if (rank != currDims->Length) {
                                    throw gcnew ArgumentException("Rank mismatch. The rank of the -Dimensions argument must equal the HDF5 dataset rank!");
                                }

                                hsize_t* maxdims = new hsize_t [rank];

                                if (H5Sget_simple_extent_dims(h5space, NULL, maxdims) == rank)
                                {
                                    for (int i = 0; i < rank; ++i)
                                    {
                                        if (currDims[i] == H5S_UNLIMITED ||
                                            (currDims[i] > maxdims[i] && !(maxdims[i] == H5S_UNLIMITED)))
                                        {
                                            delete [] maxdims;
                                            throw gcnew ArgumentException(
                                                "The dimensions specified must not exceed the HDF5 dataset's maximum dimensions!");
                                        }
                                    }

                                    hsize_t* size = new hsize_t [rank];
                                    for (int i = 0; i < rank; ++i) { size[i] = currDims[i]; }

                                    if (this->ShouldProcess(h5path,
                                        String::Format("Resizing HDF5 dataset '{0}'", path)))
                                    {
                                        if (H5Dset_extent(h5set, size) < 0) {
                                            delete [] size;
                                            delete [] maxdims;
                                            throw gcnew ArgumentException("H5Sget_simple_extent_dims failed.");
                                        }
                                    }

                                    delete [] size;
                                }
                                else {
                                    delete [] maxdims;
                                    throw gcnew ArgumentException("H5Sget_simple_extent_dims failed.");
                                }

                                delete [] maxdims;

                            }
                            else {
                                throw gcnew ArgumentException("H5Sget_simple_extent_ndims failed.");
                            }
                        }
                        else {
                            throw gcnew ArgumentException("H5Dget_space failed.");
                        }
                    }
                    else {
                        throw gcnew ArgumentException(String::Format(
                            "The dataset at '{0}' is not chunked and cannot be resized.", h5path));
                    }
                }
                else {
                    throw gcnew
                        ArgumentException("Cannot convert -Value argument to hsize_t array.");
                }
            }
            catch (Exception^ ex)
            {
                ErrorRecord^ error = gcnew ErrorRecord(ex, "InvalidData", ErrorCategory::InvalidData, nullptr);
                ThrowTerminatingError(error);
            }
            finally
            {
                if (h5space >= 0) {
                    H5Sclose(h5space);
                }

                if (h5set >= 0) {
                    H5Dclose(h5set);
                }
            }

#pragma endregion
        }
        else if (ProviderUtils::IsH5SymLink(drive->FileHandle, h5path))
        {
            // The HDF5 C API does not have a call for setting the value
            // of s symbolic link. We gotta delete it and create a new one...

            if (ProviderUtils::IsH5SoftLink(drive->FileHandle, h5path))
            {
#pragma region HDF5 soft link

                if (value != nullptr)
                {
                    String^ dest = nullptr;
                    if (ProviderUtils::TryGetValue(value, dest))
                    {
                        // delete the old one first

                        char* name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

                        if (this->ShouldProcess(h5path,
                            String::Format("HDF5 soft link '{0}' exists, delete it", h5path)))
                        {
                            if (H5Ldelete(drive->FileHandle, name, H5P_DEFAULT) < 0) {
                                throw gcnew ArgumentException("H5Ldelete failed!");
                            }
                        }

                        char* soft = (char*)(Marshal::StringToHGlobalAnsi(dest)).ToPointer();

                        if (this->ShouldProcess(h5path,
                            String::Format("HDF5 soft link '{0}' does not exist, create it", dest)))
                        {
                            if (H5Lcreate_soft(soft, drive->FileHandle, name, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                                throw gcnew InvalidOperationException("H5Lcreate_soft failed!");
                            }

                            if (H5Fflush(drive->FileHandle, H5F_SCOPE_LOCAL) < 0) {
                                WriteWarning("H5Fflush failed!");
                            }

                            WriteItemObject(gcnew LinkInfo(drive->FileHandle, h5path, "SoftLink"), path, false);
                        }
                    }
                    else {
                        throw gcnew ArgumentException("Cannot convert value argument (object) to link destination (string).");
                    }
                }
                else {
                    throw gcnew ArgumentException("No link destination found. Use -Value to specify!");
                }

#pragma endregion
            }
            else if (ProviderUtils::IsH5ExternalLink(drive->FileHandle, h5path))
            {
#pragma region HDF5 external link

                if (value != nullptr)
                {
                    array<String^>^ dest = nullptr;
                    if (ProviderUtils::TryGetValue(value, dest))
                    {
                        if (dest->Length != 2) {
                            throw gcnew ArgumentException("The length of the destination array must be " +
                                "two @(file name,path name)!");
                        }

                        // delete the old one first

                        char* name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

                        if (this->ShouldProcess(h5path,
                            String::Format("HDF5 external link '{0}' exists, delete it", h5path)))
                        {
                            if (H5Ldelete(drive->FileHandle, name, H5P_DEFAULT) < 0) {
                                throw gcnew ArgumentException("H5Ldelete failed!");
                            }
                        }

                        char* file = (char*)(Marshal::StringToHGlobalAnsi(dest[0])).ToPointer();
                        char* link = (char*)(Marshal::StringToHGlobalAnsi(dest[1])).ToPointer();

                        if (this->ShouldProcess(h5path,
                            String::Format("HDF5 external link '{0}' does not exist, create it", h5path)))
                        {
                            if (H5Lcreate_external(file, link, drive->FileHandle, name, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                                throw gcnew InvalidOperationException("H5Lcreate_external failed!");
                            }

                            if (H5Fflush(drive->FileHandle, H5F_SCOPE_LOCAL) < 0) {
                                WriteWarning("H5Fflush failed!");
                            }

                            WriteItemObject(gcnew LinkInfo(drive->FileHandle, h5path, "ExtLink"), path, false);
                        }
                    }
                    else {
                        throw gcnew ArgumentException("Cannot convert value argument (object) to link destination string[2].");
                    }
                }
                else {
                    throw gcnew ArgumentException("No link destination found. Use -Value to specify!");
                }

#pragma endregion
            }
            else
            {
                ErrorRecord^ error = gcnew ErrorRecord(
                    gcnew ArgumentException("What kind of symbolic link is this?"),
                    "InvalidData", ErrorCategory::InvalidData, nullptr);
                ThrowTerminatingError(error);
            }
        }

        return;
    }

    Object^ Provider::SetItemDynamicParameters(String^ path, Object^ value)
    {
        WriteVerbose(
            String::Format("HDF5Provider::SetItemDynamicParameters(Path = '{0}', Value = '{1}')",
            path, value->ToString()));

        DriveInfo^ drive = nullptr;
        String^ h5path = nullptr;
        if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path))
        {
            ErrorRecord^ error = gcnew ErrorRecord(
                gcnew ArgumentException("Ill-formed HDF5 path name and/or unable to obtain drive name!"),
                "InvalidData", ErrorCategory::InvalidData, nullptr);
            ThrowTerminatingError(error);
        }

        RuntimeDefinedParameterDictionary^ dict = gcnew RuntimeDefinedParameterDictionary();


        if (ProviderUtils::IsH5Dataset(drive->FileHandle, h5path))
        {
            ParameterAttribute^ attr2 = gcnew ParameterAttribute();
            attr2->Mandatory = false;
            attr2->ValueFromPipeline = false;

            RuntimeDefinedParameter^ paramDimensions = gcnew RuntimeDefinedParameter();
            paramDimensions->Name = "Dimensions";
            paramDimensions->ParameterType = array<hsize_t>::typeid;
            paramDimensions->Attributes->Add(attr2);

            dict->Add("Dimensions", paramDimensions);

            ParameterAttribute^ attr3 = gcnew ParameterAttribute();
            attr3->Mandatory = false;
            attr3->ValueFromPipeline = false;

            RuntimeDefinedParameter^ paramHyperslab = gcnew RuntimeDefinedParameter();
            paramHyperslab->Name = "Hyperslab";
            paramHyperslab->ParameterType = array<hsize_t>::typeid;
            paramHyperslab->Attributes->Add(attr3);

            dict->Add("Hyperslab", paramHyperslab);

            ParameterAttribute^ attr4 = gcnew ParameterAttribute();
            attr3->Mandatory = false;
            attr3->ValueFromPipeline = false;

            RuntimeDefinedParameter^ paramPointSet = gcnew RuntimeDefinedParameter();
            paramPointSet->Name = "PointSet";
            paramPointSet->ParameterType = array<hsize_t,2>::typeid;
            paramPointSet->Attributes->Add(attr4);

            dict->Add("PointSet", paramPointSet);
        }

        return dict;
    }
}