
#include "DatasetReader.h"
#include "DatasetWriter.h"
#include "DriveInfo.h"
#include "Provider.h"
#include "ProviderUtils.h"

extern "C" {
#include "H5public.h"
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
        //WriteWarning("The HDF5Provider::ClearContent() method has not (yet) been implemented.");
        return;
    }

    Object^ Provider::ClearContentDynamicParameters(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::ClearContentDynamicParameters(Path = '{0}')", path));
        //WriteWarning("The HDF5Provider::ClearContentDynamicParameters() method has not (yet) been implemented.");
        return nullptr;
    }

    IContentReader^ Provider::GetContentReader(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::GetContentReader(Path = '{0}')", path));
        
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
            return gcnew DatasetReader(drive->FileHandle, h5path);
        }
        else {
            WriteWarning("Invalid path or no content available for this item.");
            return nullptr;
        }
    }

    Object^ Provider::GetContentReaderDynamicParameters(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::GetContentReaderDynamicParameters(Path = '{0}')", path));

        RuntimeDefinedParameterDictionary^ dynamicParameters =
            gcnew RuntimeDefinedParameterDictionary();

        Collection<Attribute^>^ atts1 = gcnew Collection<Attribute^>();
        ParameterAttribute^ paramAttr1 = gcnew ParameterAttribute();
        paramAttr1->Mandatory = false;
        paramAttr1->ValueFromPipeline = false;
        atts1->Add(paramAttr1);
        dynamicParameters->Add("Csv",
            gcnew RuntimeDefinedParameter("Csv", SwitchParameter::typeid, atts1));
        
        return dynamicParameters;
    }

    IContentWriter^ Provider::GetContentWriter(String ^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::GetContentWriter(Path = '{0}')", path));

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
            return gcnew DatasetWriter(drive->FileHandle, h5path);
        }
        else {
            WriteWarning("Invalid path or no content available for this item.");
            return nullptr;
        }
    }

    Object^ Provider::GetContentWriterDynamicParameters(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::GetContentWriterDynamicParameters(Path = '{0}')", path));
        //WriteWarning("The HDF5Provider::GetContentWriterDynamicParameters() method has not (yet) been implemented.");
        return nullptr;
    }
}