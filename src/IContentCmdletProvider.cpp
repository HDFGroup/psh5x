
#include "DatasetReader.h"
#include "DatasetReaderT.h"
#include "DatasetWriter.h"
#include "DatasetWriterT.h"
#include "DriveInfo.h"
#include "Provider.h"
#include "ProviderUtils.h"

extern "C" {
#include "H5public.h"
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
        Exception^ ex = nullptr;

        hid_t dset = -1, ftype = -1, ntype = -1;

        DriveInfo^ drive = nullptr;
        String^ h5path = nullptr;
        if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path))
        {
            ErrorRecord^ error = gcnew ErrorRecord(
                gcnew ArgumentException("Ill-formed HDF5 path name and/or unable to obtain drive name!"),
                "InvalidData", ErrorCategory::InvalidData, nullptr);
            ThrowTerminatingError(error);
        }

        if (!ProviderUtils::IsH5Dataset(drive->FileHandle, h5path))
        {
            ex = gcnew ArgumentException("This is not an HDF5 dataset!");
            goto error;
        }

        char* name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

        dset = H5Dopen2(drive->FileHandle, name, H5P_DEFAULT);
        if (dset < 0) {
            ex = gcnew ArgumentException("H5Dopen2 failed!");
            goto error;
        }

        ftype = H5Dget_type(dset);
        if (ftype < 0) {
            ex = gcnew ArgumentException("H5Dget_type failed!");
            goto error;
        }

        ntype = H5Tget_native_type(ftype, H5T_DIR_ASCEND);
        if (ntype < 0) {
            ex = gcnew ArgumentException("H5Tget_native_type failed!");
            goto error;
        }

        bool isSimple = ProviderUtils::IsH5SimpleType(ntype);

        if (isSimple)
        {
            Type^ t = ProviderUtils::H5NativeType2DotNet(ntype);
            H5T_class_t cls = H5Tget_class(ntype);

            if (cls == H5T_INTEGER)
            {
                if (t == SByte::typeid) {
                    result = gcnew DatasetReaderT<SByte>(drive->FileHandle, h5path);
                }
                else if (t == Int16::typeid) {
                    result = gcnew DatasetReaderT<Int16>(drive->FileHandle, h5path);
                }
                else if (t == Int32::typeid) {
                    result = gcnew DatasetReaderT<Int32>(drive->FileHandle, h5path);
                }
                else if (t == Int64::typeid) {
                    result = gcnew DatasetReaderT<Int64>(drive->FileHandle, h5path);
                }
                else if (t == Byte::typeid) {
                    result = gcnew DatasetReaderT<Byte>(drive->FileHandle, h5path);
                }
                else if (t == UInt16::typeid) {
                    result = gcnew DatasetReaderT<UInt16>(drive->FileHandle, h5path);
                }
                else if (t ==  UInt32::typeid) {
                    result = gcnew DatasetReaderT<UInt32>(drive->FileHandle, h5path);
                }
                else if (t ==  UInt64::typeid) {
                    result = gcnew DatasetReaderT<UInt64>(drive->FileHandle, h5path);
                }
                else {
                    ex = gcnew ArgumentException("Unsupported type!");
                    goto error;
                }
            }
            else if (cls == H5T_FLOAT)
            {
                if (t == Single::typeid) {
                    result = gcnew DatasetReaderT<Single>(drive->FileHandle, h5path);
                }
                else if (t == Double::typeid) {
                    result = gcnew DatasetReaderT<Double>(drive->FileHandle, h5path);
                }
                else {
                    ex = gcnew ArgumentException("Unsupported type!");
                    goto error;
                }
            }
            else if (cls == H5T_STRING)
            {
                result = gcnew DatasetReaderT<String^>(drive->FileHandle, h5path);
            }
            else if (cls == H5T_BITFIELD)
            {
                if (t == Byte::typeid) {
                    result = gcnew DatasetReaderT<Byte>(drive->FileHandle, h5path);
                }
                else if (t == UInt16::typeid) {
                    result = gcnew DatasetReaderT<UInt16>(drive->FileHandle, h5path);
                }
                else if (t ==  UInt32::typeid) {
                    result = gcnew DatasetReaderT<UInt32>(drive->FileHandle, h5path);
                }
                else if (t ==  UInt64::typeid) {
                    result = gcnew DatasetReaderT<UInt64>(drive->FileHandle, h5path);
                }
                else {
                    ex = gcnew ArgumentException("Unsupported type!");
                    goto error;
                }
            }
            else if (cls == H5T_OPAQUE)
            {
                result = gcnew DatasetReaderT<Byte>(drive->FileHandle, h5path);
            }
            else {
                ex = gcnew ArgumentException("Unsupported type class!");
                goto error;
            }
        }
        else
        {
            result = gcnew DatasetReader(drive->FileHandle, h5path);
        }

error:

        if (ntype >= 0) {
            H5Tclose(ntype); 
        }

        if (ftype >= 0) {
            H5Tclose(ftype); 
        }
        if (dset >= 0) {
            H5Dclose(dset);
        }

        if (ex != nullptr) {
            throw ex;
        }

        return result;
    }

    Object^ Provider::GetContentReaderDynamicParameters(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::GetContentReaderDynamicParameters(Path = '{0}')", path));
        return nullptr;
    }

    IContentWriter^ Provider::GetContentWriter(String ^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::GetContentWriter(Path = '{0}')", path));

        IContentWriter^ result = nullptr;
        Exception^ ex = nullptr;

        hid_t dset = -1, ftype = -1, ntype = -1;

        DriveInfo^ drive = nullptr;
        String^ h5path = nullptr;
        if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path))
        {
            ErrorRecord^ error = gcnew ErrorRecord(
                gcnew ArgumentException("Ill-formed HDF5 path name and/or unable to obtain drive name!"),
                "InvalidData", ErrorCategory::InvalidData, nullptr);
            ThrowTerminatingError(error);
        }

        if (!ProviderUtils::IsH5Dataset(drive->FileHandle, h5path))
        {
            ex = gcnew ArgumentException("This is not an HDF5 dataset!");
            goto error;
        }

        char* name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

        dset = H5Dopen2(drive->FileHandle, name, H5P_DEFAULT);
        if (dset < 0) {
            ex = gcnew ArgumentException("H5Dopen2 failed!");
            goto error;
        }

        ftype = H5Dget_type(dset);
        if (ftype < 0) {
            ex = gcnew ArgumentException("H5Dget_type failed!");
            goto error;
        }

        ntype = H5Tget_native_type(ftype, H5T_DIR_ASCEND);
        if (ntype < 0) {
            ex = gcnew ArgumentException("H5Tget_native_type failed!");
            goto error;
        }

        if (ProviderUtils::IsH5SimpleType(ftype))
        {
            Type^ t = ProviderUtils::H5NativeType2DotNet(ntype);
            H5T_class_t cls = H5Tget_class(ntype);

            if (cls == H5T_INTEGER)
            {
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
                    ex = gcnew ArgumentException("Unsupported type!");
                    goto error;
                }
            }
            else if (cls == H5T_FLOAT)
            {
                if (t == Single::typeid) {
                    result = gcnew DatasetWriterT<Single>(drive->FileHandle, h5path);
                }
                else if (t == Double::typeid) {
                    result = gcnew DatasetWriterT<Double>(drive->FileHandle, h5path);
                }
                else {
                    ex = gcnew ArgumentException("Unsupported type!");
                    goto error;
                }
            }
            else if (cls == H5T_STRING)
            {
                result = gcnew DatasetWriterT<String^>(drive->FileHandle, h5path);
            }
            else if (cls == H5T_BITFIELD)
            {
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
                    ex = gcnew ArgumentException("Unsupported type!");
                    goto error;
                }
            }
            else if (cls == H5T_OPAQUE)
            {
                result = gcnew DatasetWriterT<Byte>(drive->FileHandle, h5path);
            }
            else {
                ex = gcnew ArgumentException("Unsupported type class!");
                goto error;
            }
        }
        else
        {
            result = gcnew DatasetWriter(drive->FileHandle, h5path);
        }            

error:

        if (ntype >= 0) {
            H5Tclose(ntype); 
        }
        if (ftype >= 0) {
            H5Tclose(ftype); 
        }
        if (dset >= 0) {
            H5Dclose(dset);
        }

        if (ex != nullptr) {
            throw ex;
        }

        return result;
    }

    Object^ Provider::GetContentWriterDynamicParameters(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::GetContentWriterDynamicParameters(Path = '{0}')", path));
        return nullptr;
    }
}