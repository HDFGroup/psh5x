
#include "DatasetInfoLite.h"

extern "C" {
#include "H5Dpublic.h"
#include "H5Ppublic.h"
#include "H5Spublic.h"
#include "H5Tpublic.h"
}

using namespace System;
using namespace System::IO;
using namespace System::Management::Automation;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
    DatasetInfoLite::DatasetInfoLite(hid_t dset) : ObjectInfoLite(dset)
    {
        m_storage_size = H5Dget_storage_size(dset);

#pragma region H5Dget_type

        hid_t dtype = H5Dget_type(dset);
        if (dtype >= 0)
        {
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
        }
        else { // TODO
        }

        H5Tclose(dtype);

#pragma endregion

#pragma region H5Dget_space

        hid_t dspace = H5Dget_space(dset);
        if (dspace >= 0)
        {
            switch (H5Sget_simple_extent_type(dspace))
            {
            case H5S_SCALAR:
                m_simple_extent_type = gcnew String("Scalar");
                break;
            case H5S_SIMPLE:
                m_simple_extent_type = gcnew String("Simple");
                break;
            case H5S_NULL:
                m_simple_extent_type = gcnew String("Null");
                break;
            default:
                m_simple_extent_type = gcnew String("UNKNOWN");
                break;
            }
        }
        else { // TODO
        }

        m_rank = H5Sget_simple_extent_ndims(dspace);
        m_npoints = H5Sget_simple_extent_npoints(dspace);

        hsize_t* dims = new hsize_t [m_rank];
        hsize_t* maxdims = new hsize_t [m_rank];

        int rank = H5Sget_simple_extent_dims(dspace, dims, maxdims);
        if (rank > 0)
        {
            m_dims    = gcnew array<hsize_t>(m_rank);
            m_maxdims = gcnew array<long long>(m_rank);

            for (int i = 0; i < m_rank; ++i)
            {
                m_dims[i]    = dims[i];
                m_maxdims[i] = safe_cast<long long>(maxdims[i]);
            }
        }
        else { // TODO
        }
        
        delete [] maxdims;
        delete [] dims;

        H5Sclose(dspace);

#pragma endregion

#pragma region H5Dget_create_plist

        hid_t plist = H5Dget_create_plist(dset);

        if (plist >= 0)
        {
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

            m_chunk = nullptr;
            if (m_layout == "Chunked")
            {
                hsize_t* dims = new hsize_t [m_rank];
                int rank = H5Pget_chunk(plist, m_rank, dims);
                if (rank == m_rank)
                {
                    m_chunk = gcnew array<hsize_t>(m_rank);
                    for (int i = 0; i < m_rank; ++i)
                        m_chunk[i] = dims[i];
                }
                delete [] dims;
            }
        }
        else { // TODO
        }

        if (H5Pclose(plist) < 0) { // TODO
        }

#pragma endregion

    }
}
