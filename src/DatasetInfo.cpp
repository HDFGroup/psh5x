
#include "DatasetInfo.h"

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
    DatasetInfo::DatasetInfo(hid_t dset) : ObjectInfo(dset)
    {
        m_offset = H5Dget_offset(dset);
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

#pragma region H5Dget_space_status, H5Dget_space

        // dataspace

        H5D_space_status_t status;
        herr_t ierr = H5Dget_space_status(dset, &status);
        if (ierr >= 0)
        {
            switch (status)
            {
            case H5D_SPACE_STATUS_NOT_ALLOCATED:
                m_dataspace_alloc_status = "NotAllocated";
                break;
            case H5D_SPACE_STATUS_ALLOCATED:
                m_dataspace_alloc_status = "Allocated";
                break;
            case H5D_SPACE_STATUS_PART_ALLOCATED:
                m_dataspace_alloc_status = "PartiallyAllocated";
                break;
            default:
                m_dataspace_alloc_status = "UNKNOWN";
                break;
            }
        }
        else
        {
            // TODO
        }


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
        else
        {
            // TODO
        }

        m_rank = H5Sget_simple_extent_ndims(dspace);
        m_npoints = H5Sget_simple_extent_npoints(dspace);

        hsize_t* dims = new hsize_t [m_rank];
        hsize_t* maxdims = new hsize_t [m_rank];

        int rank = H5Sget_simple_extent_dims(dspace, dims, maxdims);
        if (rank < 0)
        {
            // TODO
        }
        else
        {
            m_dims    = gcnew array<long long>(m_rank);
            m_maxdims = gcnew array<long long>(m_rank);

            for (int i = 0; i < m_rank; ++i)
            {
                m_dims[i]     = dims[i];
                m_maxdims[i] = maxdims[i];
            }
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
                    m_chunk = gcnew array<long long>(m_rank);
                    for (int i = 0; i < m_rank; ++i)
                        m_chunk[i] = dims[i];
                }
                delete [] dims;
            }

            m_fill_value_def = "UNKNOWN";
            H5D_fill_value_t status;
            herr_t ierr = H5Pfill_value_defined(plist, &status);
            if (ierr >= 0)
            {
                switch (status)
                {
                case H5D_FILL_VALUE_UNDEFINED:
                    m_fill_value_def = "Undefined";
                    break;
                case H5D_FILL_VALUE_DEFAULT:
                    m_fill_value_def = "Default";
                    break;
                case H5D_FILL_VALUE_USER_DEFINED:
                    m_fill_value_def = "UserDefined";
                    break;
                default:
                    break;
                }
            }

            m_fill_time = "UNKNOWN";
            H5D_fill_time_t fill_time;
            ierr = H5Pget_fill_time(plist, &fill_time);
            if (ierr >= 0)
            {
                switch (fill_time)
                {
                case H5D_FILL_TIME_IFSET:
                    m_fill_time = "IfSet";
                    break;
                case H5D_FILL_TIME_ALLOC:
                    m_fill_time = "Allocation";
                    break;
                case H5D_FILL_TIME_NEVER:
                    m_fill_time = "Never";
                    break;
                default:
                    break;
                }
            }

            m_alloc_time = "UNKNOWN";
            H5D_alloc_time_t alloc_time;
            ierr = H5Pget_alloc_time(plist, &alloc_time);
            if (ierr >= 0)
            {
                switch (alloc_time)
                {
                case H5D_ALLOC_TIME_DEFAULT:
                    m_alloc_time = "Default";
                    break;
                case H5D_ALLOC_TIME_EARLY:
                    m_alloc_time = "Early";
                    break;
                case H5D_ALLOC_TIME_INCR:
                    m_alloc_time = "Incremental";
                    break;
                case H5D_ALLOC_TIME_LATE:
                    m_alloc_time = "Late";
                    break;
                default:
                    break;
                }
            }

            m_nfilters = H5Pget_nfilters(plist);
            m_external_count = H5Pget_external_count(plist);
        }
        else
        {
            // TODO
        }

        H5Pclose(plist);

#pragma endregion

    }
}
