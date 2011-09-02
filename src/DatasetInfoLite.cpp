
#include "DatasetInfoLite.h"
#include "HDF5Exception.h"
#include "ProviderUtils.h"

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
    DatasetInfoLite::DatasetInfoLite(hid_t dset) : ObjectInfoLite(dset)
    {
        hid_t dtype = -1, dspace = -1, plist = -1;

        m_storage_size = H5Dget_storage_size(dset);

        try
        {
#pragma region H5Dget_type

            dtype = H5Dget_type(dset);
            if (dtype < 0) {
                throw gcnew HDF5Exception("H5Dget_type failed!");
            }

            switch (H5Tget_class(dtype))
            {
            case H5T_INTEGER:
                m_elem_type_class = "Integer";
                break;
            case H5T_FLOAT:
                m_elem_type_class = "Float";
                break;
            case H5T_STRING:
                m_elem_type_class = "String";
                break;
            case H5T_BITFIELD:
                m_elem_type_class = "Bitfield";
                break;
            case H5T_OPAQUE:
                m_elem_type_class = "Opaque";
                break;
            case H5T_COMPOUND:
                m_elem_type_class = "Compound";
                break;
            case H5T_REFERENCE:
                m_elem_type_class = "Reference";
                break;
            case H5T_ENUM:
                m_elem_type_class = "Enum";
                break;
            case H5T_VLEN:
                m_elem_type_class = "Vlen";
                break;
            case H5T_ARRAY:
                m_elem_type_class = "Array";
                break;
            default:
                m_elem_type_class = "UNKNOWN";
                break;
            }

            m_elem_type = ProviderUtils::ParseH5Type(dtype);

#pragma endregion

#pragma region H5Dget_space

            dspace = H5Dget_space(dset);
            if (dspace < 0) {
                throw gcnew HDF5Exception("H5Dget_space failed!");
            }

            switch (H5Sget_simple_extent_type(dspace))
            {
            case H5S_SCALAR:
                m_simple_extent_type = gcnew String("Scalar");
                m_rank = 0;
                break;
            case H5S_SIMPLE:
                m_simple_extent_type = gcnew String("Simple");
                m_rank = H5Sget_simple_extent_ndims(dspace);
                break;
            case H5S_NULL:
                m_simple_extent_type = gcnew String("Null");
                m_rank = -1;
                break;
            default:
                m_simple_extent_type = gcnew String("UNKNOWN");
                break;
            }

            m_npoints = H5Sget_simple_extent_npoints(dspace);
            
#pragma endregion

#pragma region H5Dget_create_plist

            plist = H5Dget_create_plist(dset);
            if (plist < 0) {
                throw gcnew HDF5Exception("H5Dget_create_plist failed!");
            }

            switch(H5Pget_layout(plist))
            {
            case H5D_COMPACT:
                m_layout = "Compact";
                break;
            case H5D_CONTIGUOUS:
                m_layout = "Contiguous";
                break;
            case H5D_CHUNKED:
                m_layout = "Chunked";
                break;
            default:
                m_layout = "UNKNOWN";
                break;
            }

#pragma endregion
        }
        finally
        {
            if (dtype >= 0) {
                H5Tclose(dtype);
            }

            if (dspace >= 0) {
                H5Sclose(dspace);
            }

            if (plist >= 0) {
                H5Pclose(plist);
            }
        }
    }
}
