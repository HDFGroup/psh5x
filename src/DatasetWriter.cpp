#include "DatasetWriter.h"
#include "Providerutils.h"

extern "C" {
#include "H5Dpublic.h"
#include "H5Spublic.h"
#include "H5Tpublic.h"
}

#include <cstdlib>
#include <cstring>

using namespace System;
using namespace System::Collections;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Provider;
using namespace System::Reflection;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
    DatasetWriter::DatasetWriter(hid_t h5file, String^ h5path)
        : m_array(nullptr), m_position(0)
    {
    }

    IList^ DatasetWriter::Write(IList^ content)
    {
        Console::WriteLine("{0} {1}", content->GetType(), content->Count);

        return nullptr;
    }

}