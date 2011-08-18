
#include "StringDatasetReader.h"
#include "HDF5Exception.h"
#include "Providerutils.h"
#include "PSH5XException.h"

extern "C" {
#include "H5Dpublic.h"
#include "H5Spublic.h"
#include "H5Tpublic.h"
}

using namespace System;
using namespace System::Collections;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Provider;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
    StringDatasetReader::StringDatasetReader(hid_t h5file, String^ h5path)
        : m_array(nullptr), m_position(0)
    {
        char* name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

        char** rdata = NULL;
        char** vrdata = NULL;

        hid_t dset = -1, ftype = -1, mtype = -1, fspace = -1, mspace = -1;

        try
        {
            dset = H5Dopen2(h5file, name, H5P_DEFAULT);
            if (dset < 0) {
                throw gcnew HDF5Exception("H5Dopen2 failed!");
            }

            fspace = H5Dget_space(dset);
            if (fspace < 0) {
                throw gcnew HDF5Exception("H5Dget_space failed!");
            }

            hssize_t npoints = H5Sget_simple_extent_npoints(fspace);
            if (npoints > 0)
            {
                hsize_t dims[1];
                dims[0] = static_cast<hsize_t>(npoints);
                mspace = H5Screate_simple(1, dims, NULL);

                m_array = gcnew array<String^>(dims[0]);

                ftype = H5Dget_type(dset);
                if (ftype < 0) {
                    throw gcnew HDF5Exception("H5Dget_type failed!");
                }

                mtype = H5Tcopy(H5T_C_S1);

                htri_t is_vlen = H5Tis_variable_str(ftype);
                if (is_vlen > 0)
                {
                    if (H5Tset_size(mtype, H5T_VARIABLE) < 0) {
                        throw gcnew HDF5Exception("H5Tset_size failed!");
                    }

                    vrdata = new char* [dims[0]];

                    if (H5Dread (dset, mtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, vrdata) < 0) {
                        throw gcnew HDF5Exception("H5Dread failed!!!");
                    }

                    for (size_t i = 0; i < m_array->Length; ++i)
                    {
                        m_array[i] = Marshal::PtrToStringAnsi(IntPtr(vrdata[i]));
                    }

                    if (H5Dvlen_reclaim(mtype, mspace, H5P_DEFAULT, vrdata) < 0) {
                        throw gcnew HDF5Exception("H5Dvlen_reclaim failed!");
                    }
                }
                else if (is_vlen == 0)
                {
                    size_t size = H5Tget_size(ftype);

                    if (H5Tset_size(mtype, size+1) < 0) {
                        throw gcnew HDF5Exception("H5Tset_size failed!");
                    }

                    rdata = new char* [dims[0]];
                    rdata[0] = new char [dims[0]*(size+1)];
                    for (size_t i = 1; i < dims[0]; ++i)
                    {
                        rdata[i] = rdata[0] + i * (size+1);
                    }

                    if (H5Dread (dset, mtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, rdata[0]) < 0) {
                        throw gcnew HDF5Exception("H5Dread failed!!!");
                    }

                    for (size_t i = 0; i < m_array->Length; ++i)
                    {
                        m_array[i] = Marshal::PtrToStringAnsi(IntPtr(rdata[i]));
                    }
                }
                else {
                    throw gcnew HDF5Exception("H5Tis_variable_str failed!");
                }
            }
            else
            {
                m_array = gcnew array<String^>(0);
            }
        }
        finally
        {
            if (rdata != NULL) {
                delete [] rdata[0];
                delete [] rdata;
            }

            if (vrdata != NULL) {
                delete [] vrdata;
            }

            if (mspace >= 0) {
                H5Sclose(mspace);
            }
            if (fspace >= 0) {
                H5Sclose(fspace);
            }
            if (ftype >= 0) {
                H5Tclose(ftype);
            }
            if (mtype >= 0) {
                H5Tclose(mtype);
            }
            if (dset >= 0) {
                H5Dclose(dset);
            }

            if (name != NULL) {
                Marshal::FreeHGlobal(IntPtr(name));
            }
        }
    }

    IList^ StringDatasetReader::Read(long long readCount)
    {
        array<String^>^ result = nullptr;

        long long remaining = m_array->LongLength - m_position;
        if (remaining > 0)
        {
            long long length = 0;
            if (readCount > remaining) { length = remaining; }
            else { length = readCount; }

            result = gcnew array<String^>(length);
    
            long long pos = m_position;
            for (long long i = 0; i < length; ++i)
            {
                result[i] = m_array[pos++];
            }

            m_position += length;
        }

        return result;
    }
}