
#include "DatasetInfo.h"
#include "DatatypeInfo.h"
#include "GroupInfo.h"
#include "LinkInfo.h"
#include "ObjectInfo.h"
#include "ObjectInfoLite.h"
#include "Provider.h"
#include "ProviderUtils.h"

extern "C" {
#include "H5Gpublic.h"
#include "H5Ppublic.h"
#include "H5Spublic.h"
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

    void Provider::NewItem(String^ path, String^ itemTypeName, Object^ newValue)
    {
        WriteVerbose(
            String::Format("HDF5Provider::NewItem(Path = '{0}', ItemTypeName = '{1}')",
            path, itemTypeName));
        
        DriveInfo^ drive = nullptr;
        String^ h5path = nullptr;
        if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path))
        {
            ErrorRecord^ error = gcnew ErrorRecord(
                gcnew ArgumentException("Ill-formed HDF5 path name and/or unable to obtain drive name!"),
                "InvalidData", ErrorCategory::InvalidData, nullptr);
            ThrowTerminatingError(error);
        }

        if (itemTypeName == nullptr || itemTypeName->Trim() == "") {
            ErrorRecord^ error = gcnew ErrorRecord(
                gcnew ArgumentException("Item type must not be empty"),
                "InvalidData", ErrorCategory::InvalidData, nullptr);
            ThrowTerminatingError(error);
        }

#pragma region structural check

        // check if we should automatically create intermediate groups
        String^ parentPath = ProviderUtils::ParentPath(h5path);
        
        if (Force)
        {
            bool allClear = ProviderUtils::CanForceCreateItemAt(drive->FileHandle, h5path);

            if (!allClear) {
                ErrorRecord^ error = gcnew ErrorRecord(
                    gcnew ArgumentException(String::Format(
                    "Unable to create all intermediate groups for path '{0}'.", h5path)),
                    "ItemExists", ErrorCategory::InvalidData, nullptr);
                ThrowTerminatingError(error);
            }
        }
        else
        {
            if (!ProviderUtils::CanCreateItemAt(drive->FileHandle, h5path))
            {
                ErrorRecord^ error = gcnew ErrorRecord(
                    gcnew ArgumentException(String::Format(
                    "Unable to create item '{0}'! The item may exist or its parent either " +
                    "does not exist or it exists but is not a group. " +
                    " Use -Force to create intermediate groups!", h5path)),
                    "ItemExists", ErrorCategory::InvalidData, nullptr);
                ThrowTerminatingError(error);
            }
        }

        // at this point, there are no structural problems
        // let's se what we've got...

        hid_t lcplist = H5Pcreate(H5P_LINK_CREATE);
        if (lcplist >= 0) {
            if (Force) {
                if (H5Pset_create_intermediate_group(lcplist, 1) < 0) {
                    WriteWarning("H5Pset_create_intermediate_group failed!");
                }
            }
        }
        else {
            WriteWarning("H5Pcreate(H5P_LINK_CREATE) failed!");
        }

#pragma endregion

        String^ linkName = ProviderUtils::ChildName(h5path);
        char* name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

        if (itemTypeName->ToUpper() == "GROUP")
        {

#pragma region HDF5 group

            if (this->ShouldProcess(h5path,
                String::Format("HDF5 group '{0}' does not exist, create it", linkName)))
            {
                hid_t group = H5Gcreate2(drive->FileHandle, name, lcplist, H5P_DEFAULT, H5P_DEFAULT);
                if (group >= 0)
                {
                    if (H5Fflush(group, H5F_SCOPE_LOCAL) < 0) {
                        WriteWarning("H5Fflush failed!");
                    }

                    WriteItemObject(gcnew GroupInfo(group), path, true);   

                    if (H5Gclose(group) < 0) {
                        WriteWarning("H5Gclose failed!");
                    }
                }
                else {
                    ErrorRecord^ error = gcnew ErrorRecord(
                        gcnew ArgumentException(String::Format("H5Gcreate2 failed for '{0}'", h5path)),
                        "Unknown", ErrorCategory::InvalidData, nullptr);
                    ThrowTerminatingError(error);
                }
            }

            if (lcplist >= 0) {
                if (H5Pclose(lcplist) < 0) {
                    WriteWarning("H5Pclose failed.");
                }
            }

#pragma endregion

        }
        else if (itemTypeName->ToUpper() == "DATASET")
        {

#pragma region HDF5 dataset

            hid_t h5type = -1, h5space = -1, h5set = -1;

            try
            {
                RuntimeDefinedParameterDictionary^ dynamicParameters =
                    (RuntimeDefinedParameterDictionary^) DynamicParameters;

                // mandatory parameters -ElementType and -Dimensions

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

                array<hsize_t>^ dims = (array<hsize_t>^) dynamicParameters["Dimensions"]->Value;

                if (dims->Length < 1 || dims->Length > 32) {
                    throw gcnew ArgumentException("Invalid rank: the dataset rank must be between 1 and 32!");
                }

                // optional parameters, determine layout first

                String^ layout = "Contiguous";

                bool hasMaxDims = (dynamicParameters["MaxDimensions"]->Value != nullptr);
                bool isChunked = (dynamicParameters["Chunked"]->Value != nullptr);
                bool isCompact = (dynamicParameters["Compact"]->IsSet);
                bool applyGzip = (dynamicParameters["Gzip"]->Value != nullptr);

                if (hasMaxDims || isChunked || applyGzip) {
                    layout = "Chunked";
                    if (!isChunked) {
                        throw gcnew ArgumentException("The -MaxDimensions and -Gzip options valid only in conjunction " +
                            "with the -Chunked option. Please specify!");
                    }
                    if (isCompact) {
                        throw gcnew ArgumentException("The -Compact switch is incompatible with the " +
                            "-MaxDimensions, -Chunked, and -Gzip options.");
                    }
                }
                else if (isCompact) {
                    layout = "Compact";
                }

                hsize_t* current_dims = new hsize_t [dims->Length];
                for (int i = 0; i < dims->Length; ++i) {
                    current_dims[i] = dims[i];
                }

                if (hasMaxDims)
                {
                    array<long long>^ maxDims = (array<long long>^) dynamicParameters["MaxDimensions"]->Value;
                    if (dims->Length != maxDims->Length) {
                        delete [] current_dims;
                        throw gcnew RankException("Rank mismatch between the dimensions and max. dimensions arrays!");
                    }
                    hsize_t* maximum_dims = new hsize_t [dims->Length];
                    for (int i = 0; i < dims->Length; ++i)
                    {
                        current_dims[i] = dims[i];
                        hsize_t m = safe_cast<hsize_t>(maxDims[i]);
                        if (maxDims[i] >= 0 && m < dims[i]) {
                            delete [] maximum_dims;
                            delete [] current_dims;
                            throw gcnew ArgumentException("Unless unlimited (H5S_UNLIMITED) dimensions are specified" +
                                " max. dimensions must be >= dimensions (elementwise)!");
                        }
                        maximum_dims[i] = (maxDims[i] >= 0) ? m : H5S_UNLIMITED;
                    }
                    h5space = H5Screate_simple(dims->Length, current_dims, maximum_dims);
                    delete [] maximum_dims;
                }
                else {
                    h5space = H5Screate_simple(dims->Length, current_dims, current_dims);
                }
                delete [] current_dims;

                if (h5space >= 0)
                {
                    hid_t dcplist = H5Pcreate(H5P_DATASET_CREATE);
                    if (dcplist >= 0)
                    {
                        if (layout == "Chunked") {
                            if (H5Pset_layout(dcplist, H5D_CHUNKED) < 0) {
                                throw gcnew InvalidOperationException("H5Pset_layout failed!");
                            }

                            array<hsize_t>^ cdims = (array<hsize_t>^) dynamicParameters["Chunked"]->Value;
                            if (cdims->Length != dims->Length) {
                                throw gcnew RankException("Rank mismatch between the dimensions and chunk dimensions arrays!");
                            }

                            hsize_t* dim = new hsize_t [cdims->Length];
                            for (int i = 0; i < cdims->Length; ++i) { dim[i] = cdims[i]; }
                            if (H5Pset_chunk(dcplist, cdims->Length, dim) < 0) {
                                delete [] dim;
                                throw gcnew InvalidOperationException("H5Pset_chunk failed!");
                            }
                            delete [] dim;
                        }
                        else if (layout == "Contiguous")
                        {
                            if (H5Pset_layout(dcplist, H5D_CONTIGUOUS) < 0) {
                               throw gcnew InvalidOperationException("H5Pset_layout failed!");
                            }
                        }
                        else if (layout == "Compact")
                        {
                            if (H5Pset_layout(dcplist, H5D_COMPACT) < 0) {
                                throw gcnew InvalidOperationException("H5Pset_layout failed!");
                            }
                        }
                        else {
                             throw gcnew InvalidOperationException("Unknown layout!");
                        }


                        if (applyGzip)
                        {
                            unsigned level = (unsigned) dynamicParameters["Gzip"]->Value;
                            if (level > 9) {
                                throw gcnew InvalidOperationException("Invalid compression level! Must be in [0,9].");
                            }

                            if (H5Pset_deflate(dcplist, level) < 0) {
                                throw gcnew InvalidOperationException("H5Pset_deflate failed!");
                            }
                        }

                        if (this->ShouldProcess(h5path,
                            String::Format("HDF5 dataset '{0}' does not exist, create it", linkName)))
                        {

                            h5set = H5Dcreate2(drive->FileHandle, name, h5type, h5space, lcplist, dcplist, H5P_DEFAULT);
                            if (h5set >= 0)
                            {
                                if (H5Fflush(h5set, H5F_SCOPE_LOCAL) < 0) {
                                    WriteWarning("H5Fflush failed!");
                                }

                                WriteItemObject(gcnew DatasetInfo(h5set), path, false);
                            }
                            else {
                                throw gcnew InvalidOperationException("H5Dcreate2 failed!");
                            }
                        }
                    }
                    else {
                        throw gcnew InvalidOperationException("H5Pcreate(H5P_DATASET_CREATE) failed!");
                    }
                }
                else {
                    throw gcnew InvalidOperationException("H5Screate_simple failed!");
                }

            }
            catch (Exception^ ex)
            {
                ErrorRecord^ error = gcnew ErrorRecord(ex, "InvalidData", ErrorCategory::InvalidData, nullptr);
                ThrowTerminatingError(error);
            }
            finally
            {
                if (h5set >= 0) {
                    if (H5Dclose(h5set) < 0) {
                        WriteWarning("H5Dclose failed.");
                    }
                }
                if (h5space >= 0) {
                    if (H5Sclose(h5space) < 0) {
                        WriteWarning("H5Sclose failed.");
                    }
                }
                if (h5type >= 0)
                {
                    if (H5Tclose(h5type) < 0) {
                        WriteWarning("H5Tclose failed.");
                    }
                }
                if (lcplist >= 0) {
                    if (H5Pclose(lcplist) < 0) {
                        WriteWarning("H5Pclose failed.");
                    }
                }
            }

#pragma endregion

        }
        else if (itemTypeName->ToUpper() == "DATATYPE")
        {

#pragma region HDF5 datatype object

            hid_t h5type = -1;

            try
            {
                RuntimeDefinedParameterDictionary^ dynamicParameters =
                    (RuntimeDefinedParameterDictionary^) DynamicParameters;

                // mandatory parameter -TypeDefintion

                Object^ typeDef = dynamicParameters["Definition"]->Value;

                Hashtable^ ht = nullptr;
                String^ type = nullptr;

                if (ProviderUtils::TryGetValue(typeDef, ht)) {
                    h5type = ProviderUtils::ParseH5Type(ht);
                    if (h5type < 0) {
                        throw gcnew ArgumentException("Invalid HDF5 datatype specified!");
                    }
                }
                else if (ProviderUtils::TryGetValue(typeDef, type))
                {
                    h5type = ProviderUtils::H5Type(type);
                    if (h5type < 0) {
                        throw gcnew ArgumentException("Invalid HDF5 datatype specified!");
                    }
                }
                else {
                    throw gcnew ArgumentException("Unrecognized type: must be string or hashtable.");
                }


                if (this->ShouldProcess(h5path,
                    String::Format("HDF5 datatype '{0}' does not exist, create it", linkName)))
                {
                    if (H5Tcommit2(drive->FileHandle, name, h5type, lcplist, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                        throw gcnew InvalidOperationException("H5Tcommit2 failed!");
                    }

                    WriteItemObject(gcnew DatatypeInfo(h5type), path, false);

                    if (H5Fflush(drive->FileHandle, H5F_SCOPE_LOCAL) < 0) {
                        WriteWarning("H5Fflush failed!");
                    }
                }
            }
            catch (Exception^ ex)
            {
                ErrorRecord^ error = gcnew ErrorRecord(ex, "InvalidData", ErrorCategory::InvalidData, nullptr);
                ThrowTerminatingError(error);
            }
            finally
            {
                if (h5type >= 0) {
                    if (H5Tclose(h5type) < 0) {
                        WriteWarning("H5Tclose failed.");
                    }
                }
                if (lcplist >= 0) {
                    if (H5Pclose(lcplist) < 0) {
                        WriteWarning("H5Pclose failed.");
                    }
                }
            }

#pragma endregion

        }
        else if (itemTypeName->ToUpper() == "HARDLINK")
        {

#pragma region HDF5 hard link

            try
            {
                // mandatory parameters -Value

                if (newValue != nullptr)
                {
                    String^ dest = nullptr;
                    if (ProviderUtils::TryGetValue(newValue, dest))
                    {
                        if (ProviderUtils::IsH5Object(drive->FileHandle, dest))
                        {
                            char* hard = (char*)(Marshal::StringToHGlobalAnsi(dest)).ToPointer();

                            if (this->ShouldProcess(h5path,
                                String::Format("HDF5 hard link '{0}' does not exist, create it", linkName)))
                            {
                                if (H5Lcreate_hard(drive->FileHandle, hard, drive->FileHandle, name, lcplist, H5P_DEFAULT) < 0) {
                                    throw gcnew InvalidOperationException("H5Lcreate_hard failed!");
                                }

                                if (H5Fflush(drive->FileHandle, H5F_SCOPE_LOCAL) < 0) {
                                    WriteWarning("H5Fflush failed!");
                                }

                                hid_t obj = H5Oopen(drive->FileHandle, hard, H5P_DEFAULT);
                                if (obj >= 0)
                                {
                                    H5I_type_t t = H5Iget_type(obj);
                                    String^ objType = nullptr;
                                    if (t == H5I_GROUP) {
                                        objType = "Group";
                                    }
                                    else if (t == H5I_DATASET) {
                                        objType = "Dataset";
                                    }
                                    else if (t == H5I_DATATYPE) {
                                        objType = "Datatype";
                                    }

                                    if (objType != nullptr) {
                                        if (objType != "Group") {
                                            WriteItemObject(gcnew ObjectInfoLite(obj), path, false);
                                        }
                                        else {
                                            WriteItemObject(gcnew ObjectInfoLite(obj), path, true);
                                        }
                                    }

                                    if (H5Oclose(obj) < 0) { //TODO
                                    }
                                }
                                else {
                                    throw gcnew InvalidOperationException("H5Oopen failed!");
                                }
                            }
                        }
                        else {
                            throw gcnew ArgumentException(String::Format("Destination item '{0}' does not exist.", dest));
                        }

                    }
                    else {
                        throw gcnew ArgumentException("Cannot convert value argument (object) to link destination (string).");
                    }
                }
                else {
                    throw gcnew ArgumentException("No link destination found. Use -Value to specify!");
                }
            }
            catch (Exception^ ex) {
                ErrorRecord^ error = gcnew ErrorRecord(ex, "InvalidData", ErrorCategory::InvalidData, nullptr);
                ThrowTerminatingError(error);
            }
            finally
            {
                if (lcplist >= 0) {
                    if (H5Pclose(lcplist) < 0) {
                        WriteWarning("H5Pclose failed.");
                    }
                }
            }

#pragma endregion

        }
        else if (itemTypeName->ToUpper() == "SOFTLINK")
        {

#pragma region HDF5 soft link

            try
            {
                // mandatory parameters -Value

                if (newValue != nullptr)
                {
                    String^ dest = nullptr;
                    if (ProviderUtils::TryGetValue(newValue, dest))
                    {
                        char* soft = (char*)(Marshal::StringToHGlobalAnsi(dest)).ToPointer();

                        if (this->ShouldProcess(h5path,
                            String::Format("HDF5 soft link '{0}' does not exist, create it", linkName)))
                        {
                            if (H5Lcreate_soft(soft, drive->FileHandle, name, lcplist, H5P_DEFAULT) < 0) {
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
            }
            catch (Exception^ ex) {
                ErrorRecord^ error = gcnew ErrorRecord(ex, "InvalidData", ErrorCategory::InvalidData, nullptr);
                ThrowTerminatingError(error);
            }
            finally
            {
                if (lcplist >= 0) {
                    if (H5Pclose(lcplist) < 0) {
                        WriteWarning("H5Pclose failed.");
                    }
                }
            }

#pragma endregion

        }
        else if (itemTypeName->ToUpper() == "EXTLINK")
        {

#pragma region HDF5 external link

            try
            {
                // mandatory parameters -Value

                if (newValue != nullptr)
                {
                    array<String^>^ dest = nullptr;
                    if (ProviderUtils::TryGetValue(newValue, dest))
                    {
                        if (dest->Length != 2) {
                            throw gcnew ArgumentException("The length of the destination array must be " +
                                "two @(file name,path name)!");
                        }

                        char* file = (char*)(Marshal::StringToHGlobalAnsi(dest[0])).ToPointer();
                        char* link = (char*)(Marshal::StringToHGlobalAnsi(dest[1])).ToPointer();

                        if (this->ShouldProcess(h5path,
                            String::Format("HDF5 external link '{0}' does not exist, create it", linkName)))
                        {
                            if (H5Lcreate_external(file, link, drive->FileHandle, name, lcplist, H5P_DEFAULT) < 0) {
                                throw gcnew InvalidOperationException("H5Lcreate_external failed!");
                            }

                            if (H5Fflush(drive->FileHandle, H5F_SCOPE_LOCAL) < 0) {
                                WriteWarning("H5Fflush failed!");
                            }

                            WriteItemObject(gcnew LinkInfo(drive->FileHandle, h5path, "ExtLink"), path, false);
                        }
                    }
                    else {
                        throw gcnew ArgumentException("Cannot convert value argument (object) to link destination (string,string).");
                    }
                }
                else {
                    throw gcnew ArgumentException("No link destination found. Use -Value to specify!");
                }
            }
            catch (Exception^ ex) {
                ErrorRecord^ error = gcnew ErrorRecord(ex, "InvalidData", ErrorCategory::InvalidData, nullptr);
                ThrowTerminatingError(error);
            }
            finally
            {
                if (lcplist >= 0) {
                    if (H5Pclose(lcplist) < 0) {
                        WriteWarning("H5Pclose failed.");
                    }
                }
            }

#pragma endregion

        }
        else if (itemTypeName->ToUpper() == "DIMENSIONSCALE")
        {
            ErrorRecord^ error = gcnew ErrorRecord(
                gcnew NotImplementedException(),
                "InvalidData", ErrorCategory::InvalidData, nullptr);
            ThrowTerminatingError(error);
        }
        else if (itemTypeName->ToUpper() == "IMAGE")
        {
            ErrorRecord^ error = gcnew ErrorRecord(
                gcnew NotImplementedException(),
                "InvalidData", ErrorCategory::InvalidData, nullptr);
            ThrowTerminatingError(error);
        }
        else if (itemTypeName->ToUpper() == "PALETTE")
        {
            ErrorRecord^ error = gcnew ErrorRecord(
                gcnew NotImplementedException(),
                "InvalidData", ErrorCategory::InvalidData, nullptr);
            ThrowTerminatingError(error);
        }
        else if (itemTypeName->ToUpper() == "PACKETTABLE")
        {
            ErrorRecord^ error = gcnew ErrorRecord(
                gcnew NotImplementedException(),
                "InvalidData", ErrorCategory::InvalidData, nullptr);
            ThrowTerminatingError(error);
        }
        else if (itemTypeName->ToUpper() == "TABLE")
        {
            ErrorRecord^ error = gcnew ErrorRecord(
                gcnew NotImplementedException(),
                "InvalidData", ErrorCategory::InvalidData, nullptr);
            ThrowTerminatingError(error);
        }
        else
        {
            WriteWarning("Unknown item type!");
        }

        return;
    }
}