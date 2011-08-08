
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

        Exception^ ex = nullptr;

        hid_t dset = -1, fspace = -1;

        hsize_t* maxDims = NULL;
        hsize_t* size = NULL;

        DriveInfo^ drive = nullptr;
        String^ h5path = nullptr;
        if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path))
        {
            ErrorRecord^ error = gcnew ErrorRecord(
                gcnew ArgumentException("Ill-formed HDF5 path name and/or unable to obtain drive name!"),
                "InvalidData", ErrorCategory::InvalidData, nullptr);
            ThrowTerminatingError(error);
        }

        if (ProviderUtils::IsH5Dataset(drive->FileHandle, h5path))
        {
#pragma region HDF5 dataset

            array<hsize_t>^ newDims = nullptr;
            if (!ProviderUtils::TryGetValue(value, newDims)) {
                ErrorRecord^ error = gcnew ErrorRecord(
                    gcnew ArgumentException(
                    "Cannot convert the -Dimensions argument to hsize_t[]!"),
                    "InvalidData", ErrorCategory::InvalidData, nullptr);
                ThrowTerminatingError(error);
            }

            if (ProviderUtils::IsH5ChunkedDataset(drive->FileHandle, h5path))
            {
                char* name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();
                dset = H5Dopen2(drive->FileHandle, name, H5P_DEFAULT);
                if (dset < 0) {
                    ex = gcnew ArgumentException("H5Dopen failed.");
                    goto error;
                }

                fspace = H5Dget_space(dset);

                if (fspace < 0) {
                    ex = gcnew ArgumentException("H5Dget_space failed.");
                    goto error;
                }

                int rank = H5Sget_simple_extent_ndims(fspace);
                if (rank <= 0) {
                    ex = gcnew ArgumentException("Rank must be positive!");
                    goto error;
                }

                if (rank != newDims->Length) {
                    ex = gcnew ArgumentException(
                        "Rank mismatch. The rank of the -Dimensions argument must equal the HDF5 dataset rank!");
                    goto error;
                }

                maxDims = new hsize_t [rank];

                if (H5Sget_simple_extent_dims(fspace, NULL, maxDims) != rank)
                {
                    ex = gcnew ArgumentException("H5Sget_simple_extent_dims failed.");
                    goto error;
                }

                for (int i = 0; i < rank; ++i)
                {
                    if (newDims[i] == H5S_UNLIMITED ||
                        (newDims[i] > maxDims[i] && !(maxDims[i] == H5S_UNLIMITED)))
                    {
                        ex = gcnew ArgumentException(
                            "The dimensions specified must not exceed the HDF5 dataset's maximum dimensions!");
                        goto error;
                    }
                }

                size = new hsize_t [rank];
                for (int i = 0; i < rank; ++i) { size[i] = newDims[i]; }

                if (this->ShouldProcess(h5path,
                    String::Format("Resizing HDF5 dataset '{0}'", path)))
                {
                    if (H5Dset_extent(dset, size) < 0) {
                        ex = gcnew ArgumentException("H5Sget_simple_extent_dims failed.");
                        goto error;
                    }
                }
            }
            else {
                ex = gcnew ArgumentException(String::Format(
                    "The dataset at '{0}' is not chunked and cannot be resized.", h5path));
                goto error;
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


                String^ dest = nullptr;
                if (ProviderUtils::TryGetValue(value, dest))
                {
                    // delete the old one first

                    char* name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

                    if (this->ShouldProcess(h5path,
                        String::Format("HDF5 soft link '{0}' exists, delete it", h5path)))
                    {
                        if (H5Ldelete(drive->FileHandle, name, H5P_DEFAULT) < 0) {
                            ex = gcnew ArgumentException("H5Ldelete failed!");
                            goto error;
                        }
                    }

                    char* soft = (char*)(Marshal::StringToHGlobalAnsi(dest)).ToPointer();

                    if (this->ShouldProcess(h5path,
                        String::Format("HDF5 soft link '{0}' does not exist, create it", dest)))
                    {
                        if (H5Lcreate_soft(soft, drive->FileHandle, name, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                            ex = gcnew InvalidOperationException("H5Lcreate_soft failed!");
                            goto error;
                        }

                        if (H5Fflush(drive->FileHandle, H5F_SCOPE_LOCAL) < 0) {
                            WriteWarning("H5Fflush failed!");
                        }

                        WriteItemObject(gcnew LinkInfo(drive->FileHandle, h5path, "SoftLink"), path, false);
                    }
                }
                else {
                    ex = gcnew ArgumentException("Cannot convert value argument (object) to link destination (string).");
                    goto error;
                }


#pragma endregion
            }
            else if (ProviderUtils::IsH5ExternalLink(drive->FileHandle, h5path))
            {
#pragma region HDF5 external link

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
                            ex = gcnew ArgumentException("H5Ldelete failed!");
                            goto error;
                        }
                    }

                    char* file = (char*)(Marshal::StringToHGlobalAnsi(dest[0])).ToPointer();
                    char* link = (char*)(Marshal::StringToHGlobalAnsi(dest[1])).ToPointer();

                    if (this->ShouldProcess(h5path,
                        String::Format("HDF5 external link '{0}' does not exist, create it", h5path)))
                    {
                        if (H5Lcreate_external(file, link, drive->FileHandle, name, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                            ex = gcnew InvalidOperationException("H5Lcreate_external failed!");
                            goto error;
                        }

                        if (H5Fflush(drive->FileHandle, H5F_SCOPE_LOCAL) < 0) {
                            WriteWarning("H5Fflush failed!");
                        }

                        WriteItemObject(gcnew LinkInfo(drive->FileHandle, h5path, "ExtLink"), path, false);
                    }
                }
                else {
                    ex = gcnew ArgumentException("Cannot convert value argument (object) to link destination string[2].");
                    goto error;
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

error:

        if (size) {
            delete [] size;
        }

        if (maxDims) {
            delete [] maxDims;
        }

        if (fspace >= 0) {
            H5Sclose(fspace);
        }

        if (dset >= 0) {
            H5Dclose(dset);
        }

        if (ex != nullptr) {
            throw ex;
        }

        return;
    }

    Object^ Provider::SetItemDynamicParameters(String^ path, Object^ value)
    {
        if (value == nullptr) {
            ErrorRecord^ error = gcnew ErrorRecord(
                gcnew ArgumentException("No value found!"),
                "InvalidData", ErrorCategory::InvalidData, nullptr);
            ThrowTerminatingError(error);
        }

        WriteVerbose(
            String::Format("HDF5Provider::SetItemDynamicParameters(Path = '{0}', Value = '{1}')",
            path, value->ToString()));

        return nullptr;
    }

}