
#include "Provider.h"
#include "ProviderUtils.h"

extern "C" {
#include "H5Opublic.h"
}

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Provider;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
    String^ Provider::GetChildName(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::GetChildName(Path = '{0}')", path));
        return __super::GetChildName(path);
    }

    String^ Provider::GetParentPath(System::String^ path, System::String^ root)
    {
        WriteVerbose(String::Format("HDF5Provider::GetParentPath(Path = '{0}', Root = '{1}')",
            path, root));
        
        return __super::GetParentPath(path,root);
    }

    bool Provider::IsItemContainer(System::String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::IsItemContainer(Path = '{0}')", path));
        
        DriveInfo^ drive = nullptr;
        String^ h5path = nullptr;
        if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path))
        {
            ErrorRecord^ error = gcnew ErrorRecord(
                gcnew ArgumentException("Ill-formed HDF5 path name and/or unable to obtain drive name!"),
                "InvalidData", ErrorCategory::InvalidData, nullptr);
            ThrowTerminatingError(error);
        }

        if (ProviderUtils::IsH5RootPathName(h5path)) { return true; }

        return ProviderUtils::IsH5Group(drive->FileHandle, h5path);
    }

    String^ Provider::MakePath(System::String^ parent, System::String^ child)
    {
        WriteVerbose(String::Format("HDF5Provider::MakePath(Parent = '{0}', Child = '{1}')",
            parent, child));
        return __super::MakePath(parent, child);
    }
}