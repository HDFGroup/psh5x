
#include "HDF5Exception.h"
#include "Provider.h"
#include "ProviderUtils.h"
#include "PSH5XException.h"

extern "C" {
#include "H5Apublic.h"
#include "H5Gpublic.h"
#include "H5Spublic.h"
}

#include <cstdlib>

using namespace System;
using namespace System::Collections;
using namespace System::Collections::ObjectModel;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Provider;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
    void Provider::ClearProperty(String^ path, Collection<String^>^ propertyToClear)
    {
        WriteVerbose(String::Format("HDF5Provider::ClearProperty(Path = '{0}')", path));
        
        // Provider::ClearProperty is being called by Set-ItemProperty, so we can't throw an exception
        WriteWarning("The HDF5Provider::ClearProperty() method has not (yet) been implemented.");
        
        return;
    }

    Object^ Provider::ClearPropertyDynamicParameters(String^ path,
        Collection<String^>^ propertyToClear)
    {
        WriteVerbose(String::Format("HDF5Provider::ClearPropertyDynamicParameters(Path = '{0}')", path));
        return nullptr;
    }
}