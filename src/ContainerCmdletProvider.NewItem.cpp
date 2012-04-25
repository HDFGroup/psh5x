
#include "DatatypeInfo.h"
#include "GroupInfo.h"
#include "HDF5Exception.h"
#include "LinkInfo.h"
#include "ObjectInfo.h"
#include "ObjectInfoLite.h"
#include "Provider.h"
#include "ProviderUtils.h"
#include "PSH5XException.h"
#include "SimpleChunkedDatasetInfo.h"

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
using namespace System::Web::Script::Serialization;

namespace PSH5X
{

    void Provider::NewItem(String^ path, String^ itemTypeName, Object^ newValue)
    {
        WriteVerbose(
            String::Format("HDF5Provider::NewItem(Path = '{0}', ItemTypeName = '{1}')",
            path, itemTypeName));

        hid_t lcplist = -1, gcplist = -1, gid = -1, dtype = -1, fspace = -1, dset = -1, dcplist = -1, oid = -1;

        char *name = NULL, *topath = NULL, *hard = NULL, *soft = NULL, *file = NULL, *link = NULL, *mode = NULL, *pal_name = NULL;

        hsize_t* current_dims = NULL;
        hsize_t* maximum_dims = NULL;
        hsize_t* dim = NULL;

        unsigned char *rgbValues = NULL, *rgbPal = NULL;
        
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

            if (itemTypeName == nullptr || itemTypeName->Trim() == "") {
                throw gcnew PSH5XException("Item type must not be empty");
            }

            if (!ProviderUtils::ResolveItemType(itemTypeName, itemTypeName)) {
                throw gcnew PSH5XException("Invalid item type");
            }

#pragma region structural check

            // check if we should automatically create intermediate groups
            
            if (Force)
            {
                bool allClear = ProviderUtils::CanForceCreateItemAt(drive->FileHandle, h5path);

                if (!allClear) {
                    throw gcnew PSH5XException(String::Format(
                        "Unable to create all intermediate groups for path '{0}'.", h5path));
                }
            }
            else
            {
                if (!ProviderUtils::CanCreateItemAt(drive->FileHandle, h5path))
                {
                    throw gcnew PSH5XException(String::Format(
                        "Unable to create item '{0}'! The item may exist or its parent either " +
                        "does not exist or it exists but is not a group. " +
                        " Use -Force to create intermediate groups!", h5path));
                }
            }

#pragma endregion

            // at this point, there are no structural problems
            // let's se what we've got...

            lcplist = H5Pcreate(H5P_LINK_CREATE);
            if (lcplist < 0) {
                throw gcnew HDF5Exception("H5Pcreate failed!");
            }
            if (Force) {
                if (H5Pset_create_intermediate_group(lcplist, 1) < 0) {
                    throw gcnew Exception("H5Pset_create_intermediate_group failed!");
                }
            }

            String^ linkName = ProviderUtils::ChildName(h5path);
            name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

			if (itemTypeName->ToUpper() == "DIRECTORY" || itemTypeName->ToUpper() == "GROUP")
            {
#pragma region HDF5 group

				bool trackAttrOrder = false;
				bool indexAttrOrder = false;
				bool trackLinkOrder = false;
				bool indexLinkOrder = false;

				gcplist = H5Pcreate(H5P_GROUP_CREATE);
				unsigned int flags = 0;
				
				RuntimeDefinedParameterDictionary^ dynamicParameters =
                    (RuntimeDefinedParameterDictionary^) DynamicParameters;

				if (dynamicParameters != nullptr)
				{
					if (dynamicParameters->ContainsKey("TrackAttributeOrder")) {
						trackAttrOrder = dynamicParameters["TrackAttributeOrder"]->IsSet;
					}
					if (dynamicParameters->ContainsKey("IndexAttributeOrder")) {
						indexAttrOrder = dynamicParameters["IndexAttributeOrder"]->IsSet;
					}
					if (trackAttrOrder) { flags |= H5P_CRT_ORDER_TRACKED; }
					if (indexAttrOrder) { flags |= H5P_CRT_ORDER_TRACKED|H5P_CRT_ORDER_INDEXED; }
					if(H5Pset_attr_creation_order(gcplist, flags) < 0) {
						throw gcnew HDF5Exception("H5Pset_attr_creation_order failed!!!");
					}

					if (dynamicParameters->ContainsKey("TrackLinkOrder")) {
						trackLinkOrder = dynamicParameters["TrackLinkOrder"]->IsSet;
					}
					if (dynamicParameters->ContainsKey("IndexLinkOrder")) {
						indexLinkOrder = dynamicParameters["IndexLinkOrder"]->IsSet;
					}
					flags = 0;
					if (trackLinkOrder) { flags |= H5P_CRT_ORDER_TRACKED; }
					if (indexLinkOrder) { flags |= H5P_CRT_ORDER_TRACKED|H5P_CRT_ORDER_INDEXED; }
					if(H5Pset_link_creation_order(gcplist, flags) < 0) {
						throw gcnew HDF5Exception("H5Pset_link_creation_order failed!!!");
					}
				}

                if (this->ShouldProcess(h5path,
                    String::Format("HDF5 group '{0}' does not exist, create it", linkName)))
                {
                    gid = H5Gcreate2(drive->FileHandle, name, lcplist, gcplist, H5P_DEFAULT);
                    if (gid < 0) {
                        throw gcnew Exception("H5Gcreate2 failed!");
                    }

                    if (H5Fflush(gid, H5F_SCOPE_LOCAL) < 0) {
                        throw gcnew Exception("H5Fflush failed!");
                    }
                }

#pragma endregion
            }
            else if (itemTypeName->ToUpper() == "DATASET")
            {
#pragma region HDF5 dataset

                RuntimeDefinedParameterDictionary^ dynamicParameters =
                    (RuntimeDefinedParameterDictionary^) DynamicParameters;

                // mandatory parameters -ElementType and -Dimensions

                String^ elemType = (String^) dynamicParameters["ElementType"]->Value;

				if (!elemType->Trim()->StartsWith("{")) // not JSON, alias or predefined type
				{
					if (elemType->StartsWith("/")) {
                        if (ProviderUtils::IsH5DatatypeObject(drive->FileHandle, elemType))
                        {
                            topath = (char*)(Marshal::StringToHGlobalAnsi(elemType)).ToPointer();
                            dtype = H5Topen2(drive->FileHandle, topath, H5P_DEFAULT);
                            if (dtype < 0) {
                                throw gcnew HDF5Exception("H5Topen2 failed!!!");
                            }
                        }
                        else {
                            throw gcnew PSH5XException("The HDF5 path name specified does not refer to an datatype object.");
                        }
                    }
                    else
                    {
                        dtype = ProviderUtils::H5Type(elemType);
                        if (dtype < 0) {
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

					dtype = ProviderUtils::ParseH5Type(type);
                    if (dtype < 0) {
                        throw gcnew PSH5XException("Invalid HDF5 datatype specified!");
                    }
				}

                // optional parameters

                String^ layout = "Contiguous";

				bool isNull = (dynamicParameters["Null"]->IsSet);
				bool isScalar = (dynamicParameters["Scalar"]->IsSet);
				bool isSimple = (dynamicParameters["Dimensions"]->Value != nullptr);
                bool hasMaxDims = (dynamicParameters["MaxDimensions"]->Value != nullptr);
                bool isChunked = (dynamicParameters["Chunked"]->Value != nullptr);
                bool isCompact = (dynamicParameters["Compact"]->IsSet);
                bool applyGzip = (dynamicParameters["Gzip"]->Value != nullptr);
				
				if (!(isNull || isScalar || isSimple)) {
					throw gcnew PSH5XException("Exactly one of -Null, -Scalar, or -Dimensions must be specified!");
				}
				else
				{
					if (isNull)
					{
						if (isScalar || isSimple || hasMaxDims || isChunked || applyGzip) {
							throw gcnew PSH5XException("The following options are incompatible with the -Null option: -Scalar, -Dimensions, -MaxDimensions, -Chunked, -Gzip.");
						}
					}
					else if (isScalar)
					{
						if (isSimple || hasMaxDims || isChunked || applyGzip) {
							throw gcnew PSH5XException("The following options are incompatible with the -Null option: -Simple, -Dimensions, -MaxDimensions, -Chunked, -Gzip.");
						}
					}
				}

				bool hasFillValue = (newValue != nullptr);

				dcplist = H5Pcreate(H5P_DATASET_CREATE);
				if (dcplist < 0) {
					throw gcnew HDF5Exception("H5Pcreate failed!");
				}

				if (isCompact) {
					layout = "Compact";
				}

				if (isSimple)
				{
#pragma region simple

					array<hsize_t>^ dims = (array<hsize_t>^) dynamicParameters["Dimensions"]->Value;

					if (dims->Length < 1 || dims->Length > 32) {
						throw gcnew PSH5XException("Invalid rank: the dataset rank must be between 1 and 32!");
					}

					if (hasMaxDims || isChunked || applyGzip) {
						layout = "Chunked";
						if (!isChunked) {
							throw gcnew PSH5XException("The -MaxDimensions and -Gzip options valid only in conjunction " +
								"with the -Chunked option. Please specify!");
						}
						if (isCompact) {
							throw gcnew PSH5XException("The -Compact switch is incompatible with the " +
								"-MaxDimensions, -Chunked, and -Gzip options.");
						}
					}

					current_dims = new hsize_t [dims->Length];
					for (int i = 0; i < dims->Length; ++i) {
						current_dims[i] = dims[i];
					}

					if (hasMaxDims)
					{
						array<long long>^ maxDims = (array<long long>^) dynamicParameters["MaxDimensions"]->Value;
						if (dims->Length != maxDims->Length) {
							throw gcnew PSH5XException("Rank mismatch between the dimensions and max. dimensions arrays!");
						}
						maximum_dims = new hsize_t [dims->Length];
						for (int i = 0; i < dims->Length; ++i)
						{
							current_dims[i] = dims[i];
							hsize_t m = safe_cast<hsize_t>(maxDims[i]);
							if (maxDims[i] >= 0 && m < dims[i]) {
								throw gcnew PSH5XException("Unless unlimited (H5S_UNLIMITED) dimensions are specified" +
									" max. dimensions must be >= dimensions (elementwise)!");
							}
							maximum_dims[i] = (maxDims[i] >= 0) ? m : H5S_UNLIMITED;
						}
						fspace = H5Screate_simple(dims->Length, current_dims, maximum_dims);
						if (fspace < 0) {
							throw gcnew HDF5Exception("H5Screate_simple failed!");
						}
					}
					else {
						fspace = H5Screate_simple(dims->Length, current_dims, current_dims);
						if (fspace < 0) {
							throw gcnew HDF5Exception("H5Screate_simple failed!");
						}
					}

					if (layout == "Chunked") {
						if (H5Pset_layout(dcplist, H5D_CHUNKED) < 0) {
							throw gcnew HDF5Exception("H5Pset_layout failed!");
						}

						array<hsize_t>^ cdims = (array<hsize_t>^) dynamicParameters["Chunked"]->Value;
						if (cdims->Length != dims->Length) {
							throw gcnew PSH5XException("Rank mismatch between the dimensions and chunk dimensions arrays!");
						}

						dim = new hsize_t [cdims->Length];
						for (int i = 0; i < cdims->Length; ++i) { dim[i] = cdims[i]; }

						if (H5Pset_chunk(dcplist, cdims->Length, dim) < 0) {
							throw gcnew HDF5Exception("H5Pset_chunk failed!");
						}
					}
					else if (layout == "Contiguous")
					{
						if (H5Pset_layout(dcplist, H5D_CONTIGUOUS) < 0) {
							throw gcnew HDF5Exception("H5Pset_layout failed!");
						}
					}
					else if (layout == "Compact")
					{
						if (H5Pset_layout(dcplist, H5D_COMPACT) < 0) {
							throw gcnew HDF5Exception("H5Pset_layout failed!");
						}
					}
					else {
						throw gcnew PSH5XException("Unknown layout!");
					}

					if (applyGzip)
					{
						unsigned level = (unsigned) dynamicParameters["Gzip"]->Value;
						if (level > 9) {
							throw gcnew PSH5XException("Invalid compression level! Must be in [0,9].");
						}

						if (H5Pset_deflate(dcplist, level) < 0) {
							throw gcnew HDF5Exception("H5Pset_deflate failed!");
						}
					}
#pragma endregion
				}
				else if (isScalar)
				{
#pragma region scalar

					fspace = H5Screate(H5S_SCALAR);
					if (fspace < 0) {
						throw gcnew HDF5Exception("H5Screate failed!");
					}

					if (layout == "Contiguous") {
						if (H5Pset_layout(dcplist, H5D_CONTIGUOUS) < 0) {
							throw gcnew HDF5Exception("H5Pset_layout failed!");
						}
					}
					else if (layout == "Compact") {
						if (H5Pset_layout(dcplist, H5D_COMPACT) < 0) {
							throw gcnew HDF5Exception("H5Pset_layout failed!");
						}
					}
					else {
						throw gcnew PSH5XException("Unknown layout!");
					}

#pragma endregion
				}
				else // isNull
				{
#pragma region null
					fspace = H5Screate(H5S_NULL);
					if (fspace < 0) {
						throw gcnew HDF5Exception("H5Screate failed!");
					}

					if (layout == "Contiguous") {
						if (H5Pset_layout(dcplist, H5D_CONTIGUOUS) < 0) {
							throw gcnew HDF5Exception("H5Pset_layout failed!");
						}
					}
					else if (layout == "Compact") {
						if (H5Pset_layout(dcplist, H5D_COMPACT) < 0) {
							throw gcnew HDF5Exception("H5Pset_layout failed!");
						}
					}
					else {
						throw gcnew PSH5XException("Unknown layout!");
					}
#pragma endregion
				}

				if (hasFillValue && (isSimple || isScalar))
				{
					array<unsigned char>^ fill = nullptr;
					
					if (ProviderUtils::TryGetFillValue(dtype, newValue, fill))
					{
						pin_ptr<unsigned char> fill_ptr = &fill[0];
						if (H5Pset_fill_value(dcplist, dtype, fill_ptr) < 0) {
							throw gcnew HDF5Exception("H5Pset_fill_value failed!");
						}
					}
					else {
						WriteWarning("Could not parse the fill value!");
					}
				}

				if (this->ShouldProcess(h5path,
					String::Format("HDF5 dataset '{0}' does not exist, create it", linkName)))
				{
					dset = H5Dcreate2(drive->FileHandle, name, dtype, fspace, lcplist, dcplist, H5P_DEFAULT);
					if (dset < 0) {
						throw gcnew HDF5Exception("H5Dcreate2 failed!");
					}

					if (H5Fflush(dset, H5F_SCOPE_LOCAL) < 0) {
						throw gcnew HDF5Exception("H5Fflush failed!");
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

                String^ def = (String^) dynamicParameters["Definition"]->Value;

				if (!def->Trim()->StartsWith("{")) // not JSON, alias or predefined type
				{
					dtype = ProviderUtils::H5Type(def);
                    if (dtype < 0) {
                        throw gcnew PSH5XException("Invalid HDF5 datatype specified!");
                    }
				}
				else
				{
					JavaScriptSerializer^ serializer = gcnew JavaScriptSerializer();
					Dictionary<String^,Object^>^ type = nullptr;
					try
					{
						type = safe_cast<Dictionary<String^,Object^>^>(serializer->DeserializeObject(def));
						
					}
					catch (...)
					{
						throw gcnew PSH5XException("JSON format error.");
					}

					dtype = ProviderUtils::ParseH5Type(type);
                    if (dtype < 0) {
                        throw gcnew PSH5XException("Invalid HDF5 datatype specified!");
                    }
				}

				if (this->ShouldProcess(h5path,
                    String::Format("HDF5 datatype '{0}' does not exist, create it", linkName)))
                {
                    if (H5Tcommit2(drive->FileHandle, name, dtype, lcplist, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                        throw gcnew HDF5Exception("H5Tcommit2 failed!");
                    }

                    if (H5Fflush(drive->FileHandle, H5F_SCOPE_LOCAL) < 0) {
                        throw gcnew HDF5Exception("H5Fflush failed!");
                    }
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
                                    throw gcnew HDF5Exception("H5Lcreate_hard failed!");
                                }

                                if (H5Fflush(drive->FileHandle, H5F_SCOPE_LOCAL) < 0) {
                                    throw gcnew HDF5Exception("H5Fflush failed!");
                                }
                            }
                        }
                        else {
                            throw gcnew PSH5XException(String::Format("Destination item '{0}' does not exist.", dest));
                        }

                    }
                    else {
                        throw gcnew PSH5XException("Cannot convert value argument (object) to link destination (string).");
                    }
                }
                else {
                    throw gcnew PSH5XException("No link destination found. Use -Value to specify!");
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
                                throw gcnew HDF5Exception("H5Lcreate_soft failed!");
                            }

                            if (H5Fflush(drive->FileHandle, H5F_SCOPE_LOCAL) < 0) {
                                throw gcnew Exception("H5Fflush failed!");
                            }
                        }
                    }
                    else {
                        throw gcnew PSH5XException("Cannot convert value argument (object) to link destination (string).");
                    }
                }
                else {
                    throw gcnew PSH5XException("No link destination found. Use -Value to specify!");
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
                            throw gcnew PSH5XException("The length of the destination array must be " +
                                "two @(file name,path name)!");
                        }

                        file = (char*)(Marshal::StringToHGlobalAnsi(dest[0])).ToPointer();
                        link = (char*)(Marshal::StringToHGlobalAnsi(dest[1])).ToPointer();

                        if (this->ShouldProcess(h5path,
                            String::Format("HDF5 external link '{0}' does not exist, create it", linkName)))
                        {
                            if (H5Lcreate_external(file, link, drive->FileHandle, name, lcplist, H5P_DEFAULT) < 0) {
                                throw gcnew HDF5Exception("H5Lcreate_external failed!");
                            }

                            if (H5Fflush(drive->FileHandle, H5F_SCOPE_LOCAL) < 0) {
                                throw gcnew HDF5Exception("H5Fflush failed!");
                            }
                        }
                    }
                    else {
                        throw gcnew PSH5XException("Cannot convert value argument (object) to link destination (string,string).");
                    }
                }
                else {
                    throw gcnew PSH5XException("No link destination found. Use -Value to specify!");
                }

#pragma endregion
            }
            else if (itemTypeName->ToUpper() == "DIMENSIONSCALE")
            {
                throw gcnew PSH5XException("Not implemented!");
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
                            throw gcnew PSH5XException("Only 8- or 24-bit images are supported!");
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
                                throw gcnew PSH5XException("Palette length <> 256!");
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
                        throw gcnew PSH5XException("Cannot convert -Value argument to file name!");
                    }
                }
                else
                {
                    if (dynamicParameters["Bits"]->Value != nullptr)
                    {
                        bits = (unsigned) dynamicParameters["Bits"]->Value;
                        if (bits != 8 && bits != 24) {
                            throw gcnew PSH5XException("Only 8- or 24-bit images are supported!");
                        }
                    }
                    else {
                        throw gcnew
                            PSH5XException("Specify bits per pixel with -Bits [8,24] !");
                    }

                    if (dynamicParameters["WxH"]->Value != nullptr)
                    {
                        wxh = (array<hsize_t,1>^) dynamicParameters["WxH"]->Value;
                        if (wxh->Length < 2) {
                            throw gcnew PSH5XException("The image dimensions must be specified as an array of length 2!");
                        }
                    }
                    else {
                        throw gcnew
                            PSH5XException("Specify image dimensions with -WxH Width,Height!");
                    }

                    array<unsigned char,1>^ rand_bytes = nullptr;

                    if (bits == 24) {
                        rgbValues = new unsigned char [3*wxh[0]*wxh[1]];
                        rand_bytes = gcnew array<unsigned char>(safe_cast<int>(3*wxh[0]*wxh[1]));
                    }
                    else {
                        rgbValues = new unsigned char [wxh[0]*wxh[1]];
                        rand_bytes = gcnew array<unsigned char>(safe_cast<int>(wxh[0]*wxh[1]));
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
                            throw gcnew PSH5XException("H5IMmake_image_24bit failed!");
                        }
                    }
                }
                else // 8-bit indexed GIF, TIFF
                {
                    if (this->ShouldProcess(h5path,
                        String::Format("HDF5 image '{0}' does not exist, create it", linkName)))
                    {
                        if (H5IMmake_image_8bit(drive->FileHandle, name, width, height, rgbValues) < 0) {
                            throw gcnew PSH5XException("H5IMmake_image_8bit failed!");
                        }

                        if (link_pal)
                        {
                            Guid guid = Guid::NewGuid();

                            pal_name = (char*)(Marshal::StringToHGlobalAnsi("PALETTE-" + guid.ToString())).ToPointer();
                            hsize_t pal_dims[2] = {256, 3};

                            if (H5IMmake_palette(drive->FileHandle, pal_name, pal_dims, rgbPal) < 0) {
                                throw gcnew HDF5Exception("H5IMmake_palette failed!");
                            }
                            if (H5IMlink_palette(drive->FileHandle, name, pal_name) < 0) {
                                throw gcnew HDF5Exception("H5IMlink_palette failed!");
                            }
                        }
                    }
                }

                if (H5Fflush(drive->FileHandle, H5F_SCOPE_LOCAL) < 0) {
                    throw gcnew HDF5Exception("H5Fflush failed!");
                }
#pragma endregion
            }
            else if (itemTypeName->ToUpper() == "PALETTE")
            {
                throw gcnew PSH5XException("Not implemented!");
            }
            else if (itemTypeName->ToUpper() == "PACKETTABLE")
            {
#pragma region HDF5 packet table

                RuntimeDefinedParameterDictionary^ dynamicParameters =
                    (RuntimeDefinedParameterDictionary^) DynamicParameters;

                // mandatory parameters -PacketType and -ChunkByteSize

				String^ elemType = (String^) dynamicParameters["PacketType"]->Value;

				if (!elemType->Trim()->StartsWith("{")) // not JSON, alias or predefined type
				{
					if (elemType->StartsWith("/")) {
                        if (ProviderUtils::IsH5DatatypeObject(drive->FileHandle, elemType))
                        {
                            topath = (char*)(Marshal::StringToHGlobalAnsi(elemType)).ToPointer();
                            dtype = H5Topen2(drive->FileHandle, topath, H5P_DEFAULT);
                            if (dtype < 0) {
                                throw gcnew HDF5Exception("H5Topen2 failed!!!");
                            }
                        }
                        else {
                            throw gcnew PSH5XException("The HDF5 path name specified does not refer to an datatype object.");
                        }
                    }
                    else
                    {
                        dtype = ProviderUtils::H5Type(elemType);
                        if (dtype < 0) {
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

					dtype = ProviderUtils::ParseH5Type(type);
                    if (dtype < 0) {
                        throw gcnew PSH5XException("Invalid HDF5 datatype specified!");
                    }
				}

                hsize_t chunk_size = (hsize_t) dynamicParameters["ChunkSize"]->Value;
                if (chunk_size == 0)
                {
                    throw gcnew PSH5XException("Chunk size must be positive.");
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
                            throw gcnew PSH5XException("Compression must be in [-1,9].");
                        }
                    }
                }

                if (this->ShouldProcess(h5path,
                    String::Format("HDF5 packet table '{0}' does not exist, create it", linkName)))
                {
                    dset = H5PTcreate_fl(drive->FileHandle, name, dtype, chunk_size, compression);
                    if (dset < 0) {
                        throw gcnew HDF5Exception("H5PTcreate_fl failed!");
                    }

                    if (H5PTclose(dset) < 0) {
                        throw gcnew HDF5Exception("H5PTclose failed!");
                    }
					dset = -1;

                    // We can't call H5Fflush on a packet table...
                    if (H5Fflush(drive->FileHandle, H5F_SCOPE_LOCAL) < 0) {
                        throw gcnew HDF5Exception("H5Fflush failed!");
                    }
                }

#pragma endregion
            }
            else if (itemTypeName->ToUpper() == "TABLE")
            {
                throw gcnew PSH5XException("Not implemented!");
            }
            else
            {
                WriteWarning("Unknown item type!");
            }
        }
        finally
        {
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
			if (gcplist >= 0) {
                H5Pclose(gcplist);
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
        }

        return;
    }
}