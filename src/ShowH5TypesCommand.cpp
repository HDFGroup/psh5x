

#include "ProviderUtils.h"
#include "ShowH5TypesCommand.h"

using namespace System;
using namespace System::Collections::ObjectModel;
using namespace System::IO;
using namespace System::Management::Automation;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
    void ShowH5TypesCommand::ProcessRecord()
    {
        if (Bitfield.IsPresent)
        {
            WriteObject(ProviderUtils::BitfieldH5Types);
            return;
        }
        if (Float.IsPresent)
        {
            WriteObject(ProviderUtils::FloatH5Types);
            return;
        }
        if (Integer.IsPresent)
        {
            WriteObject(ProviderUtils::IntegerH5Types);
            return;
        }
        else if (Native.IsPresent)
        {
            WriteObject(ProviderUtils::NativeH5Types);
            return;
        }
        if (Reference.IsPresent)
        {
            WriteObject(ProviderUtils::ReferenceH5Types);
            return;
        }
        if (Standard.IsPresent)
        {
            WriteObject(ProviderUtils::StandardH5Types);
            return;
        }
        else
        {
            WriteObject(ProviderUtils::PredefinedH5Types);
            return;
        }
    }
}