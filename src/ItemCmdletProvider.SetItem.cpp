
#include "LinkInfo.h"
#include "Provider.h"
#include "ProviderUtils.h"

extern "C" {
#include "H5Dpublic.h"
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
            
            if (hasHyper && hasPtSet) {
                ErrorRecord^ error = gcnew ErrorRecord(
                    gcnew ArgumentException("The -Hyperslab and -PointSet opetions are incompatible!"),
                    "InvalidData", ErrorCategory::InvalidData, nullptr);
                ThrowTerminatingError(error);
            }

            array<hsize_t>^ newDims = nullptr;
            if (hasDims)
            {
                if (ProviderUtils::TryGetValue(value, newDims)) {
                }
                else {
                    ErrorRecord^ error = gcnew ErrorRecord(
                    gcnew ArgumentException(
                    "Cannot convert the -Dimensions argument to hsize_t[]!"),
                    "InvalidData", ErrorCategory::InvalidData, nullptr);
                    ThrowTerminatingError(error);
                }
            }

            // hid_t dset = H5Dopen2(drive->FileHandle, name, H5P_DEFAULT);




            /*
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
            */

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