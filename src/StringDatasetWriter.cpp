
#include "HDF5Exception.h"
#include "StringDatasetWriter.h"
#include "PSH5XException.h"
#include "ProviderUtils.h"

extern "C" {
#include "H5Dpublic.h"
#include "H5Spublic.h"
#include "H5Tpublic.h"
}

#include <cstring>

using namespace System;
using namespace System::Collections;
using namespace System::Collections::ObjectModel;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Provider;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
    StringDatasetWriter::StringDatasetWriter(hid_t h5file, System::String^ h5path)
        : m_h5file(h5file), m_h5path(h5path), m_array(nullptr)
    {
    }

    IList^ StringDatasetWriter::Write(IList^ content)
    {
        char* name = (char*)(Marshal::StringToHGlobalAnsi(m_h5path)).ToPointer();

        hid_t dset = -1, fspace = -1, ftype = -1, mtype = -1;

        htri_t is_vlen = -1;

        char** wdata = NULL;
        char** vwdata = NULL;

        int i = 0;

        hssize_t npoints = -1;

        try
        {
            dset = H5Dopen2(m_h5file, name, H5P_DEFAULT);
            if (dset < 0) {
                throw gcnew HDF5Exception("H5Dopen2 failed!");
            }

            fspace = H5Dget_space(dset);
            if (fspace < 0) {
                throw gcnew HDF5Exception("H5Dget_space failed!");
            }

            npoints = H5Sget_simple_extent_npoints(fspace);

            if (content->Count != safe_cast<int>(npoints)) {
                throw gcnew PSH5XException("Size mismatch!");
            }

            ftype = H5Dget_type(dset);
            if (ftype < 0) {
                throw gcnew HDF5Exception("H5Dget_type failed!");
            }

            is_vlen = H5Tis_variable_str(ftype);

            array<String^>^ astring = nullptr;

            if (is_vlen > 0)
            {
                vwdata = new char* [npoints];

                astring = gcnew array<String^>(safe_cast<int>(npoints));

                if (ProviderUtils::TryGetValue(content, astring))
                {
                    for (i = 0; i < npoints; ++i)
                    {
                        vwdata[i] = (char*) Marshal::StringToHGlobalAnsi(astring[i]).ToPointer();
                    }

                    mtype = H5Tcopy(H5T_C_S1);
                    if (H5Tset_size(mtype, H5T_VARIABLE) < 0) {
                        throw gcnew HDF5Exception("H5Tset_size failed!");
                    }

                    if (H5Dwrite(dset, mtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, vwdata) < 0) {
                        throw gcnew HDF5Exception("H5Dwrite failed!");
                    }

                    if (H5Fflush(dset, H5F_SCOPE_LOCAL) < 0) {
                        throw gcnew HDF5Exception("H5Fflush failed!");
                    }
                }
                else {
                    throw gcnew PSH5XException("Value type mismatch!");
                }
            }
            else if (is_vlen == 0)
            {
                size_t size = H5Tget_size(ftype);

                wdata = new char* [npoints];
                wdata[0] = new char [npoints*(size+1)];
                for (i = 1; i < npoints; ++i) {
                    wdata[i] = wdata[0] + i*(size+1);
                }

                astring = gcnew array<String^>(safe_cast<int>(npoints));

                if (ProviderUtils::TryGetValue(content, astring))
                {
                    for (i = 0; i < npoints; ++i) {
                        if (astring[i]->Length > size) {
                            throw gcnew PSH5XException("String too long!");
                        }
                    }

                    for (i = 0; i < npoints; ++i)
                    {
                        char* buf = (char*) Marshal::StringToHGlobalAnsi(astring[i]).ToPointer();
                        memcpy((void*) wdata[i], (void*) buf, size);
                        Marshal::FreeHGlobal(IntPtr(buf));
                    }

                    mtype = H5Tcopy(H5T_C_S1);
                    if (H5Tset_size(mtype, size+1) < 0) {
                        throw gcnew HDF5Exception("H5Tset_size failed!!!");
                    }

                    if (H5Dwrite(dset, mtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, wdata[0]) < 0) {
                        throw gcnew HDF5Exception("H5Dwrite failed!");
                    }

                    if (H5Fflush(dset, H5F_SCOPE_LOCAL) < 0) {
                        throw gcnew HDF5Exception("H5Fflush failed!");
                    }
                }
                else {
                    throw gcnew PSH5XException("Value size or type mismatch!");
                }
            }
            else {
                throw gcnew PSH5XException("Unknown STRING type found!!!");
            }
        }
        finally
        {
            if (vwdata != NULL) {
                for (i = 0; i < npoints; ++i) {   
                    Marshal::FreeHGlobal(IntPtr(vwdata[i]));
                }
                delete [] vwdata;
            }

            if (wdata != NULL) {
                delete [] wdata[0];
                delete [] wdata;
            }

            if (mtype >= 0) {
                H5Tclose(mtype);
            }
            if (ftype >= 0) {
                H5Tclose(ftype);
            }
            if (fspace >= 0) {
                H5Sclose(fspace);
            }
            if (dset >= 0) {
                H5Dclose(dset);
            }

            Marshal::FreeHGlobal(IntPtr(name));
        }

        return nullptr;
    }


}
