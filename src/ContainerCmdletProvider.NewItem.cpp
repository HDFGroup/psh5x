
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
#include "H5IMpublic.h"
#include "H5Ppublic.h"
#include "H5PTpublic.h"
#include "H5Spublic.h"
}

using namespace System;
using namespace System::Collections;
using namespace System::Collections::Generic;
using namespace System::Collections::ObjectModel;
using namespace System::Drawing;
using namespace System::Drawing::Imaging;
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

        Exception^ ex = nullptr;

        hid_t lcplist = -1, gid = -1, dtype = -1, fspace = -1, dset = -1, dcplist = -1, oid = -1;

        char *name = NULL, *topath = NULL, *hard = NULL, *soft = NULL, *file = NULL, *link = NULL, *mode = NULL, *pal_name = NULL;

        hsize_t* current_dims = NULL;
        hsize_t* maximum_dims = NULL;
        hsize_t* dim = NULL;

        unsigned char *rgbValues = NULL, *rgbPal = NULL;
        
        DriveInfo^ drive = nullptr;
        String^ h5path = nullptr;
        if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path))
        {
            ErrorRecord^ error = gcnew ErrorRecord(
                gcnew ArgumentException("Ill-formed HDF5 path name and/or unable to obtain drive name!"),
                "InvalidData", ErrorCategory::InvalidData, nullptr);
            ThrowTerminatingError(error);
        }

        if (drive->ReadOnly)
        {
            ErrorRecord^ error = gcnew ErrorRecord(
                gcnew ArgumentException("The drive is read-only and cannot be modified!"),
                "InvalidData", ErrorCategory::InvalidData, nullptr);
            ThrowTerminatingError(error);
        }

        if (itemTypeName == nullptr || itemTypeName->Trim() == "") {
            ErrorRecord^ error = gcnew ErrorRecord(
                gcnew ArgumentException("Item type must not be empty"),
                "InvalidData", ErrorCategory::InvalidData, nullptr);
            ThrowTerminatingError(error);
        }

        if (!ProviderUtils::ResolveItemType(itemTypeName, itemTypeName)) {
            ErrorRecord^ error = gcnew ErrorRecord(
                gcnew ArgumentException("Invalid item type"),
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

#pragma endregion

        // at this point, there are no structural problems
        // let's se what we've got...

        lcplist = H5Pcreate(H5P_LINK_CREATE);
        if (lcplist < 0) {
            ex = gcnew Exception("H5Pcreate failed!!!");
            goto error;
        }
        if (Force) {
            if (H5Pset_create_intermediate_group(lcplist, 1) < 0) {
                ex = gcnew Exception("H5Pset_create_intermediate_group failed!!!");
                goto error;
            }
        }

        String^ linkName = ProviderUtils::ChildName(h5path);
        name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

        if (itemTypeName->ToUpper() == "GROUP")
        {

#pragma region HDF5 group

            if (this->ShouldProcess(h5path,
                String::Format("HDF5 group '{0}' does not exist, create it", linkName)))
            {
                gid = H5Gcreate2(drive->FileHandle, name, lcplist, H5P_DEFAULT, H5P_DEFAULT);
                if (gid < 0) {
                    ex = gcnew Exception("H5Gcreate2 failed!!!");
                    goto error;
                }

                if (H5Fflush(gid, H5F_SCOPE_LOCAL) < 0) {
                    ex = gcnew Exception("H5Fflush failed!!!");
                    goto error;
                }

                WriteItemObject(gcnew GroupInfo(gid), path, true);   
            }

#pragma endregion

        }
        else if (itemTypeName->ToUpper() == "DATASET")
        {

#pragma region HDF5 dataset

            RuntimeDefinedParameterDictionary^ dynamicParameters =
                (RuntimeDefinedParameterDictionary^) DynamicParameters;

            // mandatory parameters -ElementType and -Dimensions

            Object^ elemType = dynamicParameters["ElementType"]->Value;

            Hashtable^ ht = nullptr;
            String^ typeOrPath = nullptr;

            if (ProviderUtils::TryGetValue(elemType, ht)) {
                dtype = ProviderUtils::ParseH5Type(ht);
                if (dtype < 0) {
                    ex = gcnew ArgumentException("Invalid HDF5 datatype specified!");
                    goto error;
                }
            }
            else if (ProviderUtils::TryGetValue(elemType, typeOrPath))
            {
                if (typeOrPath->StartsWith("/")) {
                    if (ProviderUtils::IsH5DatatypeObject(drive->FileHandle, typeOrPath))
                    {
                        topath = (char*)(Marshal::StringToHGlobalAnsi(typeOrPath)).ToPointer();
                        dtype = H5Topen2(drive->FileHandle, topath, H5P_DEFAULT);
                        if (dtype < 0) {
                            ex = gcnew Exception("H5Topen2 failed!!!");
                            goto error;
                        }
                    }
                    else {
                        ex = gcnew ArgumentException("The HDF5 path name specified does not refer to an datatype object.");
                        goto error;
                    }
                }
                else
                {
                    dtype = ProviderUtils::H5Type(typeOrPath);
                    if (dtype < 0) {
                        ex = gcnew ArgumentException("Invalid HDF5 datatype specified!");
                        goto error;
                    }
                }
            }
            else {
                ex = gcnew ArgumentException("Unrecognized type: must be string or hashtable.");
                goto error;
            }

            array<hsize_t>^ dims = (array<hsize_t>^) dynamicParameters["Dimensions"]->Value;

            if (dims->Length < 1 || dims->Length > 32) {
                ex = gcnew ArgumentException("Invalid rank: the dataset rank must be between 1 and 32!");
                goto error;
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
                    ex = gcnew ArgumentException("The -MaxDimensions and -Gzip options valid only in conjunction " +
                        "with the -Chunked option. Please specify!");
                    goto error;
                }
                if (isCompact) {
                    ex = gcnew ArgumentException("The -Compact switch is incompatible with the " +
                        "-MaxDimensions, -Chunked, and -Gzip options.");
                    goto error;
                }
            }
            else if (isCompact) {
                layout = "Compact";
            }

            current_dims = new hsize_t [dims->Length];
            for (int i = 0; i < dims->Length; ++i) {
                current_dims[i] = dims[i];
            }

            if (hasMaxDims)
            {
                array<long long>^ maxDims = (array<long long>^) dynamicParameters["MaxDimensions"]->Value;
                if (dims->Length != maxDims->Length) {
                    ex = gcnew RankException("Rank mismatch between the dimensions and max. dimensions arrays!");
                    goto error;
                }
                maximum_dims = new hsize_t [dims->Length];
                for (int i = 0; i < dims->Length; ++i)
                {
                    current_dims[i] = dims[i];
                    hsize_t m = safe_cast<hsize_t>(maxDims[i]);
                    if (maxDims[i] >= 0 && m < dims[i]) {
                        ex = gcnew ArgumentException("Unless unlimited (H5S_UNLIMITED) dimensions are specified" +
                            " max. dimensions must be >= dimensions (elementwise)!");
                        goto error;
                    }
                    maximum_dims[i] = (maxDims[i] >= 0) ? m : H5S_UNLIMITED;
                }
                fspace = H5Screate_simple(dims->Length, current_dims, maximum_dims);
                if (fspace < 0) {
                    ex = gcnew Exception("H5Screate_simple failed!!!");
                    goto error;
                }
            }
            else {
                fspace = H5Screate_simple(dims->Length, current_dims, current_dims);
                if (fspace < 0) {
                    ex = gcnew Exception("H5Screate_simple failed!!!");
                    goto error;
                }
            }

            if (fspace >= 0)
            {
                dcplist = H5Pcreate(H5P_DATASET_CREATE);
                if (dcplist < 0) {
                    ex = gcnew Exception("H5Pcreate failed!!!");
                    goto error;
                }

                if (layout == "Chunked") {
                    if (H5Pset_layout(dcplist, H5D_CHUNKED) < 0) {
                        ex = gcnew InvalidOperationException("H5Pset_layout failed!");
                        goto error;
                    }

                    array<hsize_t>^ cdims = (array<hsize_t>^) dynamicParameters["Chunked"]->Value;
                    if (cdims->Length != dims->Length) {
                        ex = gcnew RankException("Rank mismatch between the dimensions and chunk dimensions arrays!");
                        goto error;
                    }

                    dim = new hsize_t [cdims->Length];
                    for (int i = 0; i < cdims->Length; ++i) { dim[i] = cdims[i]; }
                    
                    if (H5Pset_chunk(dcplist, cdims->Length, dim) < 0) {
                        ex = gcnew InvalidOperationException("H5Pset_chunk failed!");
                        goto error;
                    }
                }
                else if (layout == "Contiguous")
                {
                    if (H5Pset_layout(dcplist, H5D_CONTIGUOUS) < 0) {
                        ex = gcnew InvalidOperationException("H5Pset_layout failed!");
                        goto error;
                    }
                }
                else if (layout == "Compact")
                {
                    if (H5Pset_layout(dcplist, H5D_COMPACT) < 0) {
                        ex = gcnew InvalidOperationException("H5Pset_layout failed!");
                        goto error;
                    }
                }
                else {
                    ex = gcnew InvalidOperationException("Unknown layout!");
                    goto error;
                }


                if (applyGzip)
                {
                    unsigned level = (unsigned) dynamicParameters["Gzip"]->Value;
                    if (level > 9) {
                        ex = gcnew InvalidOperationException("Invalid compression level! Must be in [0,9].");
                        goto error;
                    }

                    if (H5Pset_deflate(dcplist, level) < 0) {
                        ex = gcnew InvalidOperationException("H5Pset_deflate failed!");
                        goto error;
                    }
                }

                if (this->ShouldProcess(h5path,
                    String::Format("HDF5 dataset '{0}' does not exist, create it", linkName)))
                {

                    dset = H5Dcreate2(drive->FileHandle, name, dtype, fspace, lcplist, dcplist, H5P_DEFAULT);
                    if (dset < 0) {
                        ex = gcnew InvalidOperationException("H5Dcreate2 failed!");
                        goto error;
                    }

                    if (H5Fflush(dset, H5F_SCOPE_LOCAL) < 0) {
                        ex = gcnew InvalidOperationException("H5Fflush failed!");
                        goto error;
                    }

                    WriteItemObject(gcnew DatasetInfo(dset), path, false);
                }
            }

#pragma endregion

        }
        else if (itemTypeName->ToUpper() == "DATATYPE")
        {

#pragma region HDF5 datatype object

            RuntimeDefinedParameterDictionary^ dynamicParameters =
                (RuntimeDefinedParameterDictionary^) DynamicParameters;

            // mandatory parameter -Defintion

            Object^ typeDef = dynamicParameters["Definition"]->Value;

            Hashtable^ ht = nullptr;
            String^ type = nullptr;

            if (ProviderUtils::TryGetValue(typeDef, ht)) {
                dtype = ProviderUtils::ParseH5Type(ht);
                if (dtype < 0) {
                    ex = gcnew ArgumentException("Invalid HDF5 datatype specified!");
                    goto error;
                }
            }
            else if (ProviderUtils::TryGetValue(typeDef, type))
            {
                dtype = ProviderUtils::H5Type(type);
                if (dtype < 0) {
                    ex = gcnew ArgumentException("Invalid HDF5 datatype specified!");
                    goto error;
                }
            }
            else {
                ex = gcnew ArgumentException("Unrecognized type: must be string or hashtable.");
                goto error;
            }

            if (this->ShouldProcess(h5path,
                String::Format("HDF5 datatype '{0}' does not exist, create it", linkName)))
            {
                if (H5Tcommit2(drive->FileHandle, name, dtype, lcplist, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                    ex = gcnew InvalidOperationException("H5Tcommit2 failed!");
                    goto error;
                }

                if (H5Fflush(drive->FileHandle, H5F_SCOPE_LOCAL) < 0) {
                    ex = gcnew InvalidOperationException("H5Fflush failed!");
                    goto error;
                }

                WriteItemObject(gcnew DatatypeInfo(dtype), path, false);
            }

#pragma endregion

        }
        else if (itemTypeName->ToUpper() == "HARDLINK")
        {

#pragma region HDF5 hard link

            // mandatory parameters -Value

            if (newValue != nullptr)
            {
                String^ dest = nullptr;
                if (ProviderUtils::TryGetValue(newValue, dest))
                {
                    if (ProviderUtils::IsH5Object(drive->FileHandle, dest))
                    {
                        hard = (char*)(Marshal::StringToHGlobalAnsi(dest)).ToPointer();

                        if (this->ShouldProcess(h5path,
                            String::Format("HDF5 hard link '{0}' does not exist, create it", linkName)))
                        {
                            if (H5Lcreate_hard(drive->FileHandle, hard, drive->FileHandle, name, lcplist, H5P_DEFAULT) < 0) {
                                ex = gcnew InvalidOperationException("H5Lcreate_hard failed!");
                                goto error;
                            }

                            if (H5Fflush(drive->FileHandle, H5F_SCOPE_LOCAL) < 0) {
                                ex = gcnew Exception("H5Fflush failed!");
                                goto error;
                            }

                            oid = H5Oopen(drive->FileHandle, hard, H5P_DEFAULT);
                            if (oid < 0) {
                                ex = gcnew Exception("H5Oopen failed!");
                                goto error;
                            }

                            H5I_type_t t = H5Iget_type(oid);
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
                                    WriteItemObject(gcnew ObjectInfoLite(oid), path, false);
                                }
                                else {
                                    WriteItemObject(gcnew ObjectInfoLite(oid), path, true);
                                }
                            }
                        }
                    }
                    else {
                        ex = gcnew ArgumentException(String::Format("Destination item '{0}' does not exist.", dest));
                        goto error;
                    }

                }
                else {
                    ex = gcnew ArgumentException("Cannot convert value argument (object) to link destination (string).");
                    goto error;
                }
            }
            else {
                ex = gcnew ArgumentException("No link destination found. Use -Value to specify!");
                goto error;
            }

#pragma endregion

        }
        else if (itemTypeName->ToUpper() == "SOFTLINK")
        {

#pragma region HDF5 soft link

            // mandatory parameters -Value

            if (newValue != nullptr)
            {
                String^ dest = nullptr;
                if (ProviderUtils::TryGetValue(newValue, dest))
                {
                    soft = (char*)(Marshal::StringToHGlobalAnsi(dest)).ToPointer();

                    if (this->ShouldProcess(h5path,
                        String::Format("HDF5 soft link '{0}' does not exist, create it", linkName)))
                    {
                        if (H5Lcreate_soft(soft, drive->FileHandle, name, lcplist, H5P_DEFAULT) < 0) {
                            ex = gcnew InvalidOperationException("H5Lcreate_soft failed!");
                            goto error;
                        }

                        if (H5Fflush(drive->FileHandle, H5F_SCOPE_LOCAL) < 0) {
                            ex = gcnew Exception("H5Fflush failed!!!");
                            goto error;
                        }

                        WriteItemObject(gcnew LinkInfo(drive->FileHandle, h5path, "SoftLink"), path, false);
                    }
                }
                else {
                    ex = gcnew ArgumentException("Cannot convert value argument (object) to link destination (string).");
                    goto error;
                }
            }
            else {
                ex = gcnew ArgumentException("No link destination found. Use -Value to specify!");
                goto error;
            }

#pragma endregion

        }
        else if (itemTypeName->ToUpper() == "EXTLINK")
        {

#pragma region HDF5 external link

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

                    file = (char*)(Marshal::StringToHGlobalAnsi(dest[0])).ToPointer();
                    link = (char*)(Marshal::StringToHGlobalAnsi(dest[1])).ToPointer();

                    if (this->ShouldProcess(h5path,
                        String::Format("HDF5 external link '{0}' does not exist, create it", linkName)))
                    {
                        if (H5Lcreate_external(file, link, drive->FileHandle, name, lcplist, H5P_DEFAULT) < 0) {
                            ex = gcnew InvalidOperationException("H5Lcreate_external failed!");
                            goto error;
                        }

                        if (H5Fflush(drive->FileHandle, H5F_SCOPE_LOCAL) < 0) {
                            ex = gcnew Exception("H5Fflush failed!!!");
                            goto error;
                        }

                        WriteItemObject(gcnew LinkInfo(drive->FileHandle, h5path, "ExtLink"), path, false);
                    }
                }
                else {
                    ex = gcnew ArgumentException("Cannot convert value argument (object) to link destination (string,string).");
                    goto error;
                }
            }
            else {
                ex = gcnew ArgumentException("No link destination found. Use -Value to specify!");
                goto error;
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

#pragma region HDF5 image

            unsigned bits = 0;

            array<hsize_t,1>^ wxh = {0, 0};

            String^ interlace = "INTERLACE_PIXEL";

            hsize_t width, height;

            bool link_pal = false;

            RuntimeDefinedParameterDictionary^ dynamicParameters =
                (RuntimeDefinedParameterDictionary^) DynamicParameters;

            if (newValue != nullptr)
            {
                String^ file = nullptr;

                if (ProviderUtils::TryGetValue(newValue, file))
                {
                    Bitmap^ bmp = gcnew Bitmap(file);
                    wxh[0] = safe_cast<hsize_t>(bmp->Width);
                    wxh[1] = safe_cast<hsize_t>(bmp->Height);

                    PixelFormat pfmt = bmp->PixelFormat;
                    if (pfmt == PixelFormat::Format8bppIndexed) {
                        bits = 8;
                    }
                    else if (pfmt == PixelFormat::Format24bppRgb) {
                        bits = 24;
                    }
                    else {
                        ex = gcnew ArgumentException("Only 8- or 24-bit images are supported!");
                        goto error;
                    }

                    Rectangle rect = Rectangle(0, 0, bmp->Width, bmp->Height);
                    BitmapData^ bmpData = bmp->LockBits(rect, ImageLockMode::ReadOnly, bmp->PixelFormat);
                    IntPtr ptr = bmpData->Scan0;
                    int bytes = Math::Abs(bmpData->Stride) * bmp->Height;
                    array<Byte>^ a = gcnew array<Byte>(bytes);
                    Marshal::Copy(ptr, a, 0, bytes);
                    bmp->UnlockBits(bmpData);
                    bytes = bmp->Width * bmp->Height;
                    if (bits == 24) {
                        bytes *= 3;
                    }
                    rgbValues = new unsigned char [bytes];

                    // scan lines start at 4-byte aligned addresses
                    // and we can't just copy the byte array
                    // have to do it row by row

                    bytes = (bits == 24) ? 3*bmp->Width : bmp->Width;
                    int src_offset = 0, dst_offset = 0;
                    for (int row = 0; row < bmp->Height; ++row)
                    {
                        unsigned char* p = (rgbValues+dst_offset);
                        Marshal::Copy(a, src_offset, IntPtr((void*) p), bytes);

                        // RGB -> BGR
                        unsigned char swap;
                        if (bits == 24) {
                            for (int i = 0; i < bytes; i += 3) {
                                swap = p[i+2];
                                p[i+2] = p[i];
                                p[i] = swap;
                            }
                        }

                        src_offset += bmpData->Stride;
                        dst_offset += bytes;
                    }

                    // get palette data

                    if (bits == 8)
                    {
                        array<Color>^ pal_entries = bmp->Palette->Entries;

                        if (pal_entries->Length != 256) {
                            ex = gcnew ArgumentException("Palette length <> 256!");
                            goto error;
                        }

                        rgbPal = new unsigned char [3*pal_entries->Length];
                        for (int i = 0; i < pal_entries->Length; ++i)
                        {
                            rgbPal[3*i]   = pal_entries[i].R;
                            rgbPal[3*i+1] = pal_entries[i].G;
                            rgbPal[3*i+2] = pal_entries[i].B;
                        }

                        link_pal = true;
                    }
                }
                else {
                    ex = gcnew
                        ArgumentException("Cannot convert -Value argument to file name!");
                    goto error;
                }
            }
            else
            {
                if (dynamicParameters["Bits"]->Value != nullptr)
                {
                    bits = (unsigned) dynamicParameters["Bits"]->Value;
                    if (bits != 8 && bits != 24) {
                        ex = gcnew ArgumentException("Only 8- or 24-bit images are supported!");
                        goto error;
                    }
                }
                else {
                    ex = gcnew
                        ArgumentException("Specify bits per pixel with -Bits [8,24] !");
                    goto error;
                }

                if (dynamicParameters["WxH"]->Value != nullptr)
                {
                    wxh = (array<hsize_t,1>^) dynamicParameters["WxH"]->Value;
                    if (wxh->Length < 2) {
                        ex = gcnew ArgumentException("The image dimensions must be specified as an array of length 2!");
                        goto error;
                    }
                }
                else {
                    ex = gcnew
                        ArgumentException("Specify image dimensions with -WxH Width,Height!");
                    goto error;
                }

                array<unsigned char,1>^ rand_bytes = nullptr;

                if (bits == 24) {
                    rgbValues = new unsigned char [3*wxh[0]*wxh[1]];
                    rand_bytes = gcnew array<unsigned char>(3*wxh[0]*wxh[1]);
                }
                else {
                    rgbValues = new unsigned char [wxh[0]*wxh[1]];
                    rand_bytes = gcnew array<unsigned char>(wxh[0]*wxh[1]);
                }

                Random^ random = gcnew Random();
                random->NextBytes(rand_bytes);
                Marshal::Copy(rand_bytes, 0, IntPtr((void*) rgbValues), rand_bytes->Length);
            }

            width = wxh[0]; height = wxh[1];

            if (bits == 24)
            {
                if (dynamicParameters["InterlaceMode"]->Value != nullptr)
                {
                    String^ s = ((String^) dynamicParameters["InterlaceMode"]->Value)->Trim()->ToUpper();

                    if (s == "PLANE") {
                        interlace = "INTERLACE_PLANE";
                    }
                }

                if (this->ShouldProcess(h5path,
                    String::Format("HDF5 image '{0}' does not exist, create it", linkName)))
                {
                    mode = (char*)(Marshal::StringToHGlobalAnsi(interlace)).ToPointer();

                    if (H5IMmake_image_24bit(drive->FileHandle, name, width, height, mode, rgbValues) < 0) {
                        ex = gcnew ArgumentException("H5IMmake_image_24bit failed!");
                        goto error;
                    }
                }
            }
            else // 8-bit indexed GIF, TIFF
            {
                if (this->ShouldProcess(h5path,
                    String::Format("HDF5 image '{0}' does not exist, create it", linkName)))
                {
                    if (H5IMmake_image_8bit(drive->FileHandle, name, width, height, rgbValues) < 0) {
                        ex = gcnew ArgumentException("H5IMmake_image_8bit failed!");
                        goto error;
                    }

                    if (link_pal)
                    {
                        Guid guid = Guid::NewGuid();

                        pal_name = (char*)(Marshal::StringToHGlobalAnsi("PALETTE-" + guid.ToString())).ToPointer();
                        hsize_t pal_dims[2] = {256, 3};

                        if (H5IMmake_palette(drive->FileHandle, pal_name, pal_dims, rgbPal) < 0) {
                            ex = gcnew ArgumentException("H5IMmake_palette failed!");
                            goto error;
                        }
                        if (H5IMlink_palette(drive->FileHandle, name, pal_name) < 0) {
                            ex = gcnew ArgumentException("H5IMlink_palette failed!");
                            goto error;
                        }
                    }
                }
            }

            if (H5Fflush(drive->FileHandle, H5F_SCOPE_LOCAL) < 0) {
                ex = gcnew Exception("H5Fflush failed!!!");
                goto error;
            }

            dset = H5Dopen2(drive->FileHandle, name, H5P_DEFAULT);
            if (dset < 0) {
                ex = gcnew Exception("H5Dopen2 failed!!!");
                goto error;
            }

            WriteItemObject(gcnew DatasetInfo(dset), path, false);

#pragma endregion

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

#pragma region HDF5 packet table

            RuntimeDefinedParameterDictionary^ dynamicParameters =
                (RuntimeDefinedParameterDictionary^) DynamicParameters;

            // mandatory parameters -PacketType and -ChunkByteSize

            Object^ packetType = dynamicParameters["PacketType"]->Value;

            Hashtable^ ht = nullptr;
            String^ typeOrPath = nullptr;

            if (ProviderUtils::TryGetValue(packetType, ht)) {
                dtype = ProviderUtils::ParseH5Type(ht);
                if (dtype < 0) {
                    ex = gcnew ArgumentException("Invalid HDF5 datatype specified!");
                    goto error;
                }
            }
            else if (ProviderUtils::TryGetValue(packetType, typeOrPath))
            {
                if (typeOrPath->StartsWith("/")) {
                    if (ProviderUtils::IsH5DatatypeObject(drive->FileHandle, typeOrPath))
                    {
                        topath = (char*)(Marshal::StringToHGlobalAnsi(typeOrPath)).ToPointer();
                        dtype = H5Topen2(drive->FileHandle, topath, H5P_DEFAULT);
                        if (dtype < 0) {
                            ex = gcnew Exception("H5Topen2 failed!!!");
                            goto error;
                        }
                    }
                    else {
                        ex = gcnew
                            ArgumentException("The HDF5 path name specified does not refer to an datatype object.");
                        goto error;
                    }
                }
                else
                {
                    dtype = ProviderUtils::H5Type(typeOrPath);
                    if (dtype < 0) {
                        ex = gcnew ArgumentException("Invalid HDF5 datatype specified!");
                        goto error;
                    }
                }
            }
            else {
                ex = gcnew ArgumentException("Unrecognized type: must be string or hashtable.");
                goto error;
            }

            hsize_t chunk_size = (hsize_t) dynamicParameters["ChunkByteSize"]->Value;
            if (chunk_size == 0)
            {
                ex = gcnew ArgumentException("Chunk size (bytes) must be positive.");
                goto error;
            }

            int compression = 5;
            if (dynamicParameters["Gzip"]->Value != nullptr)
            {
                compression = (int) dynamicParameters["Gzip"]->Value;
                if (compression < 0) {
                    compression = -1;
                }
                else {
                    if (compression > 9) {
                        ex = gcnew ArgumentException("Compression must be in [-1,9].");
                        goto error;
                    }
                }
            }

            if (this->ShouldProcess(h5path,
                String::Format("HDF5 packet table '{0}' does not exist, create it", linkName)))
            {
                dset = H5PTcreate_fl(drive->FileHandle, name, dtype, chunk_size, compression);
                if (dset < 0) {
                    ex = gcnew Exception("H5PTcreate_fl failed!!!");
                    goto error;
                }

                if (H5PTclose(dset) < 0) {
                    ex = gcnew Exception("H5PTclose failed!!!");
                    goto error;
                }
                // We can't call H5Fflush on a packet table...
                if (H5Fflush(drive->FileHandle, H5F_SCOPE_LOCAL) < 0) {
                    ex = gcnew Exception("H5Fflush failed!!!");
                    goto error;
                }

                dset = H5Dopen2(drive->FileHandle, name, H5P_DEFAULT);
                if (dset < 0) {
                    ex = gcnew Exception("H5Dopen2 failed!!!");
                    goto error;
                }

                WriteItemObject(gcnew DatasetInfo(dset), path, false);
            }

#pragma endregion

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

error:

        if (rgbValues != NULL) {
            delete [] rgbValues;
        }

        if (rgbPal != NULL) {
            delete [] rgbPal;
        }

        if (oid >= 0) {
            H5Oclose(oid);
        }

        if (dim != NULL) {
            delete [] dim;
        }

        if (dcplist >= 0) {
            H5Pclose(dcplist);
        }
        
        if (maximum_dims != NULL) {
            delete [] maximum_dims;
        }

        if (current_dims != NULL) {
            delete [] current_dims;
        }

        if (dtype >= 0) {
            H5Tclose(dtype);
        }
        
        if (fspace >= 0) {
            H5Sclose(fspace);
        }
        
        if (dset >= 0) {
            H5Dclose(dset);
        }

        if (gid >= 0) {
            H5Gclose(gid);
        }

        if (lcplist >= 0) {
            H5Pclose(lcplist);
        }

        if (name != NULL) {
            Marshal::FreeHGlobal(IntPtr(name));
        }

        if (topath != NULL) {
            Marshal::FreeHGlobal(IntPtr(topath));
        }

        if (hard != NULL) {
            Marshal::FreeHGlobal(IntPtr(hard));
        }

        if (soft != NULL) {
            Marshal::FreeHGlobal(IntPtr(soft));
        }

        if (file != NULL) {
            Marshal::FreeHGlobal(IntPtr(file));
        }

        if (link != NULL) {
            Marshal::FreeHGlobal(IntPtr(link));
        }

        if (mode != NULL) {
            Marshal::FreeHGlobal(IntPtr(mode));
        }

        if (pal_name != NULL) {
            Marshal::FreeHGlobal(IntPtr(pal_name));
        }

        if (ex != nullptr) {
            throw ex;
        }

        return;
    }
}