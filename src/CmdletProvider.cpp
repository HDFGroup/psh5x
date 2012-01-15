
#include "HDF5Exception.h"
#include "Provider.h"

extern "C" {
#include "H5public.h"
}

using namespace System;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Provider;
using namespace System::Reflection;

namespace PSH5X
{
    System::Management::Automation::ProviderInfo^ Provider::Start(
        System::Management::Automation::ProviderInfo^ providerInfo)
    {
        WriteVerbose("HDF5Provider::Start()");

        String^ version = Assembly::GetExecutingAssembly()->GetName()->Version->ToString();
        
        Assembly^ assy = Assembly::GetExecutingAssembly();
        bool isdef = assy->IsDefined(AssemblyDescriptionAttribute::typeid, false);
        if (isdef)
        {
            AssemblyDescriptionAttribute^ adAttr =
                (AssemblyDescriptionAttribute^)Attribute::GetCustomAttribute(
                assy, AssemblyDescriptionAttribute::typeid);
            if (adAttr != nullptr)
			{
				Console::WriteLine("\n" + adAttr->Description + " " + version);
			}
        }

        isdef = assy->IsDefined(AssemblyCopyrightAttribute::typeid, false);
        if (isdef)
        {
            AssemblyCopyrightAttribute^ adAttr =
                (AssemblyCopyrightAttribute^)Attribute::GetCustomAttribute(
                assy, AssemblyCopyrightAttribute::typeid);
            if (adAttr != nullptr)
			{
				Console::WriteLine(adAttr->Copyright);
			}
        }
        
        herr_t status = H5open();
        if (status < 0)
        {
            String^ msg = String::Format("H5open failed with status {0}", status);
            ErrorRecord^ error = gcnew ErrorRecord(gcnew HDF5Exception(msg), "OpenError",
                ErrorCategory::OpenError, status);
            ThrowTerminatingError(error);
        }

        unsigned majnum, minnum, relnum;
        status = H5get_libversion(&majnum, &minnum, &relnum);
        if (status < 0)
        {
            String^ msg = String::Format("H5get_libversion failed with status {0}", status);
            ErrorRecord^ error = gcnew ErrorRecord(gcnew HDF5Exception(msg), "ReadError",
                ErrorCategory::ReadError, status);
            ThrowTerminatingError(error);
        }
        Console::WriteLine("Welcome to HDF5 {0}.{1}.{2} !\n", majnum, minnum, relnum);

        return providerInfo;
    }

    void Provider::Stop()
    {
        WriteVerbose("HDF5Provider::Stop()");

        Console::WriteLine("\nThank you for using HDF5.\n");

        herr_t status = H5close();
        if (status < 0)
        {
            String^ msg = String::Format("H5close failed with status {0}", status);
            ErrorRecord^ error = gcnew ErrorRecord(gcnew HDF5Exception(msg), "CloseError",
                ErrorCategory::CloseError, status);
            ThrowTerminatingError(error);
        }
    }

}