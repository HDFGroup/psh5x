
#include "ArrayDatasetReader.h"
//#include "ArrayDatasetWriter.h"
#include "CompoundDatasetReader.h"
//#include "CompoundDatasetWriter.h"
#include "DatasetReaderT.h"
#include "DatasetWriter.h"
#include "DatasetWriterT.h"
#include "DriveInfo.h"
#include "HDF5Exception.h"
#include "Provider.h"
#include "ProviderUtils.h"
#include "PSH5XException.h"
#include "StringDatasetReader.h"
#include "StringDatasetWriter.h"
#include "VlenDatasetReader.h"
#include "VlenDatasetWriter.h"

extern "C" {
#include "H5public.h"
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
    void Provider::ClearContent(String ^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::ClearContent(Path = '{0}')", path));
        return;
    }

    Object^ Provider::ClearContentDynamicParameters(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::ClearContentDynamicParameters(Path = '{0}')", path));
        return nullptr;
    }

    IContentReader^ Provider::GetContentReader(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::GetContentReader(Path = '{0}')", path));

        IContentReader^ result = nullptr;

        hid_t dset = -1, ftype = -1, fspace = -1;

        char *name = NULL;

        try
        {
            DriveInfo^ drive = nullptr;
            String^ h5path = nullptr;
            if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path)) {
                throw gcnew PSH5XException("Ill-formed HDF5 path name and/or unable to obtain drive name!");
            }
            if (!ProviderUtils::IsH5Dataset(drive->FileHandle, h5path)) {
                throw gcnew PSH5XException("This is not an HDF5 dataset!");
            }

            name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

            dset = H5Dopen2(drive->FileHandle, name, H5P_DEFAULT);
            if (dset < 0) {
                throw gcnew HDF5Exception("H5Dopen2 failed!");
            }
            ftype = H5Dget_type(dset);
            if (ftype < 0) {
                throw gcnew HDF5Exception("H5Dget_type failed!");
            }
			fspace = H5Dget_space(dset);
            if (fspace < 0) {
                throw gcnew HDF5Exception("H5Dget_space failed!");
            }

			if (H5Sget_simple_extent_type(fspace) != H5S_NULL)
			{
				Type^ t = nullptr;

				H5T_class_t cls = H5Tget_class(ftype);

				switch (cls)
				{
				case H5T_INTEGER:

#pragma region HDF5 integer

					t = ProviderUtils::H5Type2DotNet(ftype);
					if (t == SByte::typeid) {
						result = gcnew DatasetReaderT<SByte>(dset, ftype, fspace);
					}
					else if (t == Int16::typeid) {
						result = gcnew DatasetReaderT<Int16>(dset, ftype, fspace);
					}
					else if (t == Int32::typeid) {
						result = gcnew DatasetReaderT<Int32>(dset, ftype, fspace);
					}
					else if (t == Int64::typeid) {
						result = gcnew DatasetReaderT<Int64>(dset, ftype, fspace);
					}
					else if (t == Byte::typeid) {
						result = gcnew DatasetReaderT<Byte>(dset, ftype, fspace);
					}
					else if (t == UInt16::typeid) {
						result = gcnew DatasetReaderT<UInt16>(dset, ftype, fspace);
					}
					else if (t ==  UInt32::typeid) {
						result = gcnew DatasetReaderT<UInt32>(dset, ftype, fspace);
					}
					else if (t ==  UInt64::typeid) {
						result = gcnew DatasetReaderT<UInt64>(dset, ftype, fspace);
					}
					else {
						throw gcnew PSH5XException("Unsupported integer type!");
					}

#pragma endregion

					break;

				case H5T_FLOAT:

#pragma region HDF5 float

					t = ProviderUtils::H5Type2DotNet(ftype);
					if (t == Single::typeid) {
						result = gcnew DatasetReaderT<Single>(dset, ftype, fspace);
					}
					else if (t == Double::typeid) {
						result = gcnew DatasetReaderT<Double>(dset, ftype, fspace);
					}
					else {
						throw gcnew PSH5XException("Unsupported float type!");
					}

#pragma endregion

					break;

				case H5T_STRING:

					result = gcnew StringDatasetReader(dset, ftype, fspace);
					break;

				case H5T_BITFIELD:

#pragma region HDF5 bitfield

					t = ProviderUtils::H5Type2DotNet(ftype);
					if (t == Byte::typeid) {
						result = gcnew DatasetReaderT<Byte>(dset, ftype, fspace);
					}
					else if (t == UInt16::typeid) {
						result = gcnew DatasetReaderT<UInt16>(dset, ftype, fspace);
					}
					else if (t ==  UInt32::typeid) {
						result = gcnew DatasetReaderT<UInt32>(dset, ftype, fspace);
					}
					else if (t ==  UInt64::typeid) {
						result = gcnew DatasetReaderT<UInt64>(dset, ftype, fspace);
					}
					else {
						throw gcnew PSH5XException("Unsupported bitfield type!");
					}

#pragma endregion

					break;

				case H5T_COMPOUND:

					result = gcnew CompoundDatasetReader(dset, ftype, fspace);
					break;

				case H5T_ENUM:

#pragma region HDF5 enumeration

					t = ProviderUtils::H5Type2DotNet(ftype);
					if (t == SByte::typeid) {
						result = gcnew DatasetReaderT<SByte>(dset, ftype, fspace);
					}
					else if (t == Int16::typeid) {
						result = gcnew DatasetReaderT<Int16>(dset, ftype, fspace);
					}
					else if (t == Int32::typeid) {
						result = gcnew DatasetReaderT<Int32>(dset, ftype, fspace);
					}
					else if (t == Int64::typeid) {
						result = gcnew DatasetReaderT<Int64>(dset, ftype, fspace);
					}
					else if (t == Byte::typeid) {
						result = gcnew DatasetReaderT<Byte>(dset, ftype, fspace);
					}
					else if (t == UInt16::typeid) {
						result = gcnew DatasetReaderT<UInt16>(dset, ftype, fspace);
					}
					else if (t ==  UInt32::typeid) {
						result = gcnew DatasetReaderT<UInt32>(dset, ftype, fspace);
					}
					else if (t ==  UInt64::typeid) {
						result = gcnew DatasetReaderT<UInt64>(dset, ftype, fspace);
					}
					else {
						throw gcnew PSH5XException("Unsupported enum type!");
					}

#pragma endregion

					break;

				case H5T_VLEN:

					result = gcnew VlenDatasetReader(dset, ftype, fspace);
					break;

				case H5T_ARRAY:

					result = gcnew ArrayDatasetReader(dset, ftype, fspace);
					break;

				default:

					throw gcnew PSH5XException("Unsupported type class!");
					break;
				}
			}
		}
        finally
        {
			if (fspace >= 0) {
                H5Sclose(fspace); 
            }
            if (ftype >= 0) {
                H5Tclose(ftype); 
            }
            if (dset >= 0) {
                H5Dclose(dset);
            }

            if (name != NULL) {
                Marshal::FreeHGlobal(IntPtr(name));
            }
        }

        return result;
    }

    Object^ Provider::GetContentReaderDynamicParameters(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::GetContentReaderDynamicParameters(Path = '{0}')", path));
        return nullptr;
    }

    IContentWriter^ Provider::GetContentWriter(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::GetContentWriter(Path = '{0}')", path));

        IContentWriter^ result = nullptr;

        hid_t dset = -1, ftype = -1, fspace = -1;

        char *name = NULL;

        try
        {
            DriveInfo^ drive = nullptr;
            String^ h5path = nullptr;
            if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path)) {
                throw gcnew PSH5XException("Ill-formed HDF5 path name and/or unable to obtain drive name!");
            }
            if (drive->ReadOnly) {
                throw gcnew PSH5XException("The drive is read-only and cannot be modified!");
            }
            if (!ProviderUtils::IsH5Dataset(drive->FileHandle, h5path)) {
                throw gcnew PSH5XException("This is not an HDF5 dataset!");
            }

            name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

            dset = H5Dopen2(drive->FileHandle, name, H5P_DEFAULT);
            if (dset < 0) {
                throw gcnew HDF5Exception("H5Dopen2 failed!");
            }
            ftype = H5Dget_type(dset);
            if (ftype < 0) {
                throw gcnew HDF5Exception("H5Dget_type failed!");
            }
			fspace = H5Dget_space(dset);
            if (fspace < 0) {
                throw gcnew HDF5Exception("H5Dget_space failed!");
            }

			if (H5Sget_simple_extent_type(fspace) != H5S_NULL)
			{
				Type^ t = nullptr;

				H5T_class_t cls = H5Tget_class(ftype);

				switch (cls)
				{
				case H5T_INTEGER:

#pragma region HDF5 integer

					t = ProviderUtils::H5Type2DotNet(ftype);
					if (t == SByte::typeid) {
						result = gcnew DatasetWriterT<SByte>(drive->FileHandle, h5path);
					}
					else if (t == Int16::typeid) {
						result = gcnew DatasetWriterT<Int16>(drive->FileHandle, h5path);
					}
					else if (t ==  Int32::typeid) {
						result = gcnew DatasetWriterT<Int32>(drive->FileHandle, h5path);
					}
					else if (t ==  Int64::typeid) {
						result = gcnew DatasetWriterT<Int64>(drive->FileHandle, h5path);
					}
					else if (t == Byte::typeid) {
						result = gcnew DatasetWriterT<Byte>(drive->FileHandle, h5path);
					}
					else if (t == UInt16::typeid) {
						result = gcnew DatasetWriterT<UInt16>(drive->FileHandle, h5path);
					}
					else if (t ==  UInt32::typeid) {
						result = gcnew DatasetWriterT<UInt32>(drive->FileHandle, h5path);
					}
					else if (t ==  UInt64::typeid) {
						result = gcnew DatasetWriterT<UInt64>(drive->FileHandle, h5path);
					}
					else {
						throw gcnew PSH5XException("Unsupported integer type!");
					}

#pragma endregion

					break;

				case H5T_FLOAT:

#pragma region HDF5 float

					t = ProviderUtils::H5Type2DotNet(ftype);
					if (t == Single::typeid) {
						result = gcnew DatasetWriterT<Single>(drive->FileHandle, h5path);
					}
					else if (t == Double::typeid) {
						result = gcnew DatasetWriterT<Double>(drive->FileHandle, h5path);
					}
					else {
						throw gcnew PSH5XException("Unsupported float type!");
					}

#pragma endregion

					break;

				case H5T_STRING:

					result = gcnew StringDatasetWriter(drive->FileHandle, h5path);
					break;


				case H5T_BITFIELD:

#pragma region HDF5 bitfield

					t = ProviderUtils::H5Type2DotNet(ftype);
					if (t == Byte::typeid) {
						result = gcnew DatasetWriterT<Byte>(drive->FileHandle, h5path);
					}
					else if (t == UInt16::typeid) {
						result = gcnew DatasetWriterT<UInt16>(drive->FileHandle, h5path);
					}
					else if (t ==  UInt32::typeid) {
						result = gcnew DatasetWriterT<UInt32>(drive->FileHandle, h5path);
					}
					else if (t ==  UInt64::typeid) {
						result = gcnew DatasetWriterT<UInt64>(drive->FileHandle, h5path);
					}
					else {
						throw gcnew PSH5XException("Unsupported bitfield type!");
					}

#pragma endregion

					break;

					/*
					case H5T_COMPOUND:

					result = gcnew CompoundDatasetWriter(drive->FileHandle, h5path);
					break;
					*/

				case H5T_VLEN:

					result = gcnew VlenDatasetWriter(drive->FileHandle, h5path);
					break;

					/*
					case H5T_ARRAY:

					result = gcnew ArrayDatasetWriter(drive->FileHandle, h5path);
					break;
					*/

				default:

					throw gcnew PSH5XException("Unsupported type class!");
					break;
				}
			}
        }
        finally
        {
			if (fspace >= 0) {
                H5Sclose(fspace); 
            }
            if (ftype >= 0) {
                H5Tclose(ftype); 
            }
            if (dset >= 0) {
                H5Dclose(dset);
            }

            if (name != NULL) {
                Marshal::FreeHGlobal(IntPtr(name));
            }
        }

        return result;
    }

    Object^ Provider::GetContentWriterDynamicParameters(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::GetContentWriterDynamicParameters(Path = '{0}')", path));
        return nullptr;
    }
}