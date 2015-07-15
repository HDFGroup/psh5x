
#include "HDF5Exception.h"
#include "PSH5XException.h"
#include "SimpleExternalDatasetInfo.h"

extern "C" {
#include "H5Dpublic.h"
#include "H5Ppublic.h"
#include "H5Spublic.h"
#include "H5Tpublic.h"
}

#include <vector>

using namespace std;

using namespace System;
using namespace System::IO;
using namespace System::Management::Automation;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
    SimpleExternalDatasetInfo::SimpleExternalDatasetInfo(hid_t dset)
        : SimpleDatasetInfo(dset), m_efl_info(nullptr)
    {
        hid_t plist = -1;

        try
        {
            plist = H5Dget_create_plist(dset);
            if (plist < 0) {
                throw gcnew HDF5Exception("H5Dget_create_plist failed!");
            }

            m_external_count = H5Pget_external_count(plist);
            if (m_external_count < 0) {
                throw gcnew HDF5Exception("H5Pget_external_count failed!");
            }

            m_efl_info = gcnew array<Object^>(m_external_count);

            for (int i = 0; i < m_external_count; ++i)
            {
                vector<char> name(256);
                off_t offset;
                hsize_t size;

                if (H5Pget_external(plist, i, 255, &name[0], &offset, &size ) < 0)
                {
                    throw gcnew HDF5Exception("H5Pget_external failed!");
                }
                else
                {
                    array<Object^>^ a = gcnew array<Object^>(3);
                    
                    a[0] = Marshal::PtrToStringAnsi(IntPtr(&name[0]));
                    a[1] = (size_t) offset;
                    a[2] = (size_t) size;

                    m_efl_info->SetValue(a, i);
                }
            }
        }
        finally
        {
            if (plist >= 0) {
                H5Pclose(plist);
            }
        }
    }
}
