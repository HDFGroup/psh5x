
#include "HDF5Exception.h"
#include "LinkInfo.h"
#include "Provider.h"
#include "ProviderUtils.h"
#include "PSH5XException.h"
#include "SimpleChunkedDatasetInfo.h"

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

        hid_t dset = -1, fspace = -1;

        char *name = NULL, *soft = NULL, *file = NULL, *link = NULL;

        try
        {
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

            if (ProviderUtils::IsH5Dataset(drive->FileHandle, h5path))
            {
#pragma region HDF5 dataset

                array<hsize_t>^ newDims = nullptr;
                if (!ProviderUtils::TryGetValue(value, newDims)) {
                    throw gcnew PSH5XException(
                        "Cannot convert the Value argument to hsize_t[]!");
                }

                if (ProviderUtils::IsH5ChunkedDataset(drive->FileHandle, h5path))
                {
                    name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();
                    dset = H5Dopen2(drive->FileHandle, name, H5P_DEFAULT);
                    if (dset < 0) {
                        throw gcnew HDF5Exception("H5Dopen failed.");
                    }

                    fspace = H5Dget_space(dset);
                    if (fspace < 0) {
                        throw gcnew HDF5Exception("H5Dget_space failed.");
                    }

					if (H5Sget_simple_extent_type(fspace) != H5S_SIMPLE) {
						throw gcnew PSH5XException("This is not a simple HDF5 dataset!");
					}

                    int rank = H5Sget_simple_extent_ndims(fspace);
                    if (rank <= 0) {
                        throw gcnew HDF5Exception("H5Sget_simple_extent_ndims failed!");
                    }

                    if (rank != newDims->Length) {
                        throw gcnew PSH5XException(
                            "Rank mismatch. The rank of the dimensions array must match the HDF5 dataset rank!");
                    }

					array<hsize_t>^ maxDims = gcnew array<hsize_t>(rank);
					pin_ptr<hsize_t> maxDims_ptr = &maxDims[0];

                    if (H5Sget_simple_extent_dims(fspace, NULL, maxDims_ptr) != rank) {
                        throw gcnew HDF5Exception("H5Sget_simple_extent_dims failed.");
                    }

                    for (int i = 0; i < rank; ++i)
                    {
                        if (newDims[i] == H5S_UNLIMITED ||
                            (newDims[i] > maxDims[i] && !(maxDims[i] == H5S_UNLIMITED)))
                        {
                            throw gcnew PSH5XException(
                                "The dimensions specified must not exceed the HDF5 dataset's maximum dimensions!");
                        }
                    }

                    array<hsize_t>^ size = gcnew array<hsize_t>(rank);
					pin_ptr<hsize_t> size_ptr = &size[0];

                    for (int i = 0; i < rank; ++i) {
						size[i] = newDims[i];
					}

                    if (this->ShouldProcess(h5path,
                        String::Format("Resizing HDF5 dataset '{0}'", path)))
                    {
                        if (H5Dset_extent(dset, size_ptr) < 0) {
                            throw gcnew HDF5Exception("H5Sget_simple_extent_dims failed.");
                        }

						if (H5Fflush(dset, H5F_SCOPE_LOCAL) < 0) {
							throw gcnew HDF5Exception("H5Fflush failed!");
						}

						// extendible datasets must be simple and chunked (at the moment)
						WriteItemObject(gcnew SimpleChunkedDatasetInfo(dset), path, false);
                    }
                }
                else {
                    throw gcnew PSH5XException(String::Format(
                        "The dataset at '{0}' is not chunked and cannot be resized.", h5path));
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

                        name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

                        if (this->ShouldProcess(h5path,
                            String::Format("HDF5 soft link '{0}' exists, delete it", h5path)))
                        {
                            if (H5Ldelete(drive->FileHandle, name, H5P_DEFAULT) < 0) {
                                throw gcnew HDF5Exception("H5Ldelete failed!");
                            }
                        }

                        soft = (char*)(Marshal::StringToHGlobalAnsi(dest)).ToPointer();

                        if (this->ShouldProcess(h5path,
                            String::Format("HDF5 soft link '{0}' does not exist, create it", dest)))
                        {
                            if (H5Lcreate_soft(soft, drive->FileHandle, name, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                                throw gcnew HDF5Exception("H5Lcreate_soft failed!");
                            }

                            if (H5Fflush(drive->FileHandle, H5F_SCOPE_LOCAL) < 0) {
                                throw gcnew HDF5Exception("H5Fflush failed!");
                            }

                            WriteItemObject(gcnew LinkInfo(drive->FileHandle, h5path), path, false);
                        }
                    }
                    else {
                        throw gcnew PSH5XException("Cannot convert value argument (object) to link destination (string).");
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
                            throw gcnew PSH5XException("The length of the destination array must be " +
                                "two @(file name,path name)!");
                        }

                        // delete the old one first

                        name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

                        if (this->ShouldProcess(h5path,
                            String::Format("HDF5 external link '{0}' exists, delete it", h5path)))
                        {
                            if (H5Ldelete(drive->FileHandle, name, H5P_DEFAULT) < 0) {
                                throw gcnew PSH5XException("H5Ldelete failed!");
                            }
                        }

                        file = (char*)(Marshal::StringToHGlobalAnsi(dest[0])).ToPointer();
                        link = (char*)(Marshal::StringToHGlobalAnsi(dest[1])).ToPointer();

                        if (this->ShouldProcess(h5path,
                            String::Format("HDF5 external link '{0}' does not exist, create it", h5path)))
                        {
                            if (H5Lcreate_external(file, link, drive->FileHandle, name, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                                throw gcnew HDF5Exception("H5Lcreate_external failed!");
                            }

                            if (H5Fflush(drive->FileHandle, H5F_SCOPE_LOCAL) < 0) {
                                throw gcnew HDF5Exception("H5Fflush failed!");
                            }

                            WriteItemObject(gcnew LinkInfo(drive->FileHandle, h5path), path, false);
                        }
                    }
                    else {
                        throw gcnew PSH5XException("Cannot convert value argument (object) to link destination string[2].");
                    }

#pragma endregion
                }
                else {
                    throw gcnew PSH5XException("What kind of symbolic link is this?");
                }
            }
        }
        finally
        {
            if (fspace >= 0) {
                H5Sclose(fspace);
            }
            if (dset >= 0) {
                H5Dclose(dset);
            }
            if (link != NULL) {
                Marshal::FreeHGlobal(IntPtr(link));
            }
            if (file != NULL) {
                Marshal::FreeHGlobal(IntPtr(file));
            }
            if (soft != NULL) {
                Marshal::FreeHGlobal(IntPtr(soft));
            }
            if (name != NULL) {
                Marshal::FreeHGlobal(IntPtr(name));
            }
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