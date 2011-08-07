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
        : m_h5file(h5file), m_h5path(h5path), m_array(nullptr)
    {
    }

    IList^ DatasetWriter::Write(IList^ content)
    {
        m_array = safe_cast<Array^>(content);

        if (ProviderUtils::IsH5Dataset(m_h5file, m_h5path))
        {
            char* name = (char*)(Marshal::StringToHGlobalAnsi(m_h5path)).ToPointer();
            hid_t dset = H5Dopen2(m_h5file, name, H5P_DEFAULT);
            
            hid_t dspace = H5Dget_space(dset);
            hssize_t npoints = H5Sget_simple_extent_npoints(dspace);

            if (content->Count != safe_cast<int>(npoints))
            {
                Console::WriteLine("Size mismatch!!!");
                return nullptr;
            }

            hid_t dtype = H5Dget_type(dset);
            hid_t ntype = H5Tget_native_type(dtype, H5T_DIR_ASCEND);

            Type^ t = ProviderUtils::H5NativeType2DotNet(ntype);

            if (t != nullptr)
            {
                if (t == int::typeid)
                {
                    array<int>^ a = gcnew array<int>(content->Count);
                    Array::Copy(m_array, a, a->Length);
                    pin_ptr<int> ptr = &a[0];

                    herr_t ierr = H5Dwrite(dset, ntype, H5S_ALL, H5S_ALL, H5P_DEFAULT, ptr);
                    
                    ierr = H5Fflush(dset, H5F_SCOPE_LOCAL);
                }
            }

            H5Tclose(ntype);
            H5Tclose(dtype);
            H5Sclose(dspace);
            H5Dclose(dset);
        }

        return nullptr;
    }

}