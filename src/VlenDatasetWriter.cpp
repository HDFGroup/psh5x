
#include "HDF5Exception.h"
#include "VlenDatasetWriter.h"
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
    VlenDatasetWriter::VlenDatasetWriter(hid_t h5file, System::String^ h5path)
        : m_h5file(h5file), m_h5path(h5path), m_array(nullptr)
    {
    }

    IList^ VlenDatasetWriter::Write(IList^ content)
    {
        char* name = (char*)(Marshal::StringToHGlobalAnsi(m_h5path)).ToPointer();

        hid_t dset = -1, fspace = -1, ftype = -1, ntype = -1, mtype = -1, base_type = -1;

        hvl_t* wdata = NULL;

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

            if (npoints > 0)
            {
                ftype = H5Dget_type(dset);
                if (ftype < 0) {
                    throw gcnew HDF5Exception("H5Dget_type failed!");
                }

                base_type = H5Tget_super(ftype);
                if (base_type < 0) {
                    throw gcnew HDF5Exception("H5Tget_super failed!");
                }

                Type^ t = ProviderUtils::H5Type2DotNet(base_type);
                if (t != nullptr)
                {
                    size_t size = H5Tget_size(base_type);
                    wdata = new hvl_t [npoints];

                    if (H5Tget_class(base_type) == H5T_BITFIELD) {
                        ntype = H5Tget_native_type(base_type, H5T_DIR_DESCEND);
                    }
                    else {
                        ntype = H5Tget_native_type(base_type, H5T_DIR_ASCEND);
                    }
                    if (ntype < 0) {
                        throw gcnew HDF5Exception("H5Tget_native_type failed!");
                    }

                    mtype = H5Tvlen_create(ntype);
                    if (mtype < 0) {
                        throw gcnew HDF5Exception("H5Tvlen_create failed!");
                    }

                    IEnumerator^ ienum = content->GetEnumerator();
                    ienum->MoveNext();

                    for (int i = 0; i < npoints; ++i)
                    {
                        pin_ptr<int> ptr;

                        if (t == Int32::typeid)
                        {
                            Object^ obj = nullptr;
                            if (ienum->Current->GetType() == PSObject::typeid) {
                                obj = ((PSObject^)ienum->Current)->BaseObject;
                            }
                            else {
                                obj = ienum->Current;
                            }

                            array<int>^ a = reinterpret_cast<array<int>^>(obj);
                            int len = a->Length;

                            if (len > 0) {
                                ptr = &a[0];
                            }

                            wdata[i].len = len;
                            wdata[i].p = ptr;
                        }

                        ienum->MoveNext();
                    }

                    if (H5Dwrite(dset, mtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, wdata)) {
                        throw gcnew HDF5Exception("H5Dwrite failed!");
                    }
                }
            }
        }
        finally
        {
            if (wdata != NULL) {
                delete [] wdata;
            }

            if (mtype >= 0) {
                H5Tclose(mtype);
            }
            if (ntype >= 0) {
                H5Tclose(ntype);
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
