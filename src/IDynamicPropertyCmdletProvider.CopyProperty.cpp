
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

				WritePropertyObject(ProviderUtils::H5Attribute(dst_attr, destinationProperty, destinationDrive->FileHandle), destinationPath);
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
}