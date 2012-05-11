
#include "HDF5Exception.h"
#include "Provider.h"
#include "ProviderUtils.h"
#include "PSH5XException.h"

extern "C" {
#include "H5Gpublic.h"
#include "H5Ppublic.h"
}

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Provider;
using namespace System::Runtime::InteropServices;

/*
   
   There appears to be bug in the default implementation of the
   Get* methods. The problem can be reproduced with the Filesystem
   provider. Tab completion stops working and traversal into
   directories with brackets in their name fails.
   HACK: For now we strip backticks from any path and do not
   allow them in HDF5 path names. That way we can at least
   find our way with -LiteralPath. Tab completion is still broken though.

   Dan Harman says:

   This is a known issue that we're tracking for the next release of PowerShell (not PowerShell v3).

 */

namespace PSH5X
{
    String^ Provider::GetChildName(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::GetChildName(Path = '{0}')", path));
		// HACK!!!
		String^ result = __super::GetChildName(path)->Replace("`","");
		return result;
    }

    String^ Provider::GetParentPath(System::String^ path, System::String^ root)
    {
        WriteVerbose(String::Format("HDF5Provider::GetParentPath(Path = '{0}', Root = '{1}')",
            path, root));
		// HACK!!!
		String^ result = __super::GetParentPath(path,root)->Replace("`",""); 
        return result;
    }

    bool Provider::IsItemContainer(System::String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::IsItemContainer(Path = '{0}')", path));
        
        DriveInfo^ drive = nullptr;
        String^ h5path = nullptr;
        if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path)) {
            throw gcnew PSH5XException("Ill-formed HDF5 path name and/or unable to obtain drive name!");
        }

        return ProviderUtils::IsH5Group(drive->FileHandle, h5path);
    }

    String^ Provider::MakePath(System::String^ parent, System::String^ child)
    {
        WriteVerbose(String::Format("HDF5Provider::MakePath(Parent = '{0}', Child = '{1}')",
            parent, child));
		// HACK!!!
		String^ result = __super::MakePath(parent, child)->Replace("`",""); 
		return result;
    }
}