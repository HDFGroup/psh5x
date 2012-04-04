
#include "DatasetInfo.h"
#include "HDF5Exception.h"
#include "ProviderUtils.h"

extern "C" {
#include "H5Dpublic.h"
#include "H5Ppublic.h"
#include "H5Spublic.h"
#include "H5Tpublic.h"
}

using namespace System;
using namespace System::Collections;
using namespace System::IO;
using namespace System::Management::Automation;
using namespace System::Runtime::InteropServices;
using namespace System::Web::Script::Serialization;

namespace PSH5X
{
    DatasetInfo::DatasetInfo(hid_t dset) : ObjectInfo(dset)
    {
        m_offset = H5Dget_offset(dset);
        m_storage_size = H5Dget_storage_size(dset);

        hid_t dtype = -1, dspace = -1, plist = -1;

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

		JavaScriptSerializer^ serializer = gcnew JavaScriptSerializer();
        m_elem_type = serializer->Serialize(ProviderUtils::ParseH5Type(dtype));

#pragma endregion

#pragma region H5Dget_space_status, H5Dget_space

        H5D_space_status_t status;
        //herr_t ierr = H5Dget_space_status(dset, &status);
        if (H5Dget_space_status(dset, &status) < 0) {
            throw gcnew HDF5Exception("H5Dget_space_status failed!");
        }

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

        if ((dspace = H5Dget_space(dset)) < 0) {
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

        if ((plist = H5Dget_create_plist(dset)) < 0) {
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

        m_fill_value_def = "UNKNOWN";
        H5D_fill_value_t fvstatus;
        if (H5Pfill_value_defined(plist, &fvstatus) < 0) {
            throw gcnew HDF5Exception("H5Pfill_value_defined failed!");
        }
        
        switch (fvstatus)
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

        m_fill_time = "UNKNOWN";
        H5D_fill_time_t fill_time;
        if (H5Pget_fill_time(plist, &fill_time) < 0) {
            throw gcnew HDF5Exception("H5Pget_fill_time failed!");
        }

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

        m_alloc_time = "UNKNOWN";
        H5D_alloc_time_t alloc_time;
        if (H5Pget_alloc_time(plist, &alloc_time) < 0) {
            throw gcnew HDF5Exception("H5Pget_alloc_time failed!");
        }

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

        m_nfilters = H5Pget_nfilters(plist);
		m_filters = gcnew ArrayList();

		if (m_nfilters > 0)
		{
			for (unsigned int idx = 0; idx < safe_cast<unsigned int>(m_nfilters); ++idx)
			{
				Hashtable^ filter = gcnew Hashtable();

				unsigned int flags, filter_config;
				size_t cd_nelmts = 0;
				char name[256];

				switch (H5Pget_filter2(plist, idx, &flags, &cd_nelmts, NULL, 255, name, &filter_config ))
				{
				case H5Z_FILTER_DEFLATE:
					{
						filter["Type"] = "Deflate";
						filter["Index"] = idx;
						filter["Name"] = Marshal::PtrToStringAnsi(IntPtr(name));

						if (H5Z_FILTER_CONFIG_ENCODE_ENABLED & filter_config) {
							filter["EncodingEnabled"] = "True";
						}
						else {
							filter["EncodingEnabled"] = "False";
						}
						if (H5Z_FILTER_CONFIG_DECODE_ENABLED & filter_config) {
							filter["DecodingEnabled"] = "True";
						}
						else {
							filter["DecodingEnabled"] = "False";
						}
					}
					break;

				case H5Z_FILTER_SHUFFLE:
					{
						filter["Type"] = "Shuffle";
						filter["Index"] = idx;
						filter["Name"] = Marshal::PtrToStringAnsi(IntPtr(name));

						if (H5Z_FILTER_CONFIG_ENCODE_ENABLED & filter_config) {
							filter["EncodingEnabled"] = "True";
						}
						else {
							filter["EncodingEnabled"] = "False";
						}
						if (H5Z_FILTER_CONFIG_DECODE_ENABLED & filter_config) {
							filter["DecodingEnabled"] = "True";
						}
						else {
							filter["DecodingEnabled"] = "False";
						}
					}
					break;

				case H5Z_FILTER_FLETCHER32:
					{
						filter["Type"] = "Fletcher32";
						filter["Index"] = idx;
						filter["Name"] = Marshal::PtrToStringAnsi(IntPtr(name));

						if (H5Z_FILTER_CONFIG_ENCODE_ENABLED & filter_config) {
							filter["EncodingEnabled"] = "True";
						}
						else {
							filter["EncodingEnabled"] = "False";
						}
						if (H5Z_FILTER_CONFIG_DECODE_ENABLED & filter_config) {
							filter["DecodingEnabled"] = "True";
						}
						else {
							filter["DecodingEnabled"] = "False";
						}
					}
					break;

				case H5Z_FILTER_SZIP:
					{
						filter["Type"] = "Szip";
						filter["Index"] = idx;
						filter["Name"] = Marshal::PtrToStringAnsi(IntPtr(name));

						if (H5Z_FILTER_CONFIG_ENCODE_ENABLED & filter_config) {
							filter["EncodingEnabled"] = "True";
						}
						else {
							filter["EncodingEnabled"] = "False";
						}
						if (H5Z_FILTER_CONFIG_DECODE_ENABLED & filter_config) {
							filter["DecodingEnabled"] = "True";
						}
						else {
							filter["DecodingEnabled"] = "False";
						}
					}
					break;

				case H5Z_FILTER_NBIT:

					{
						filter["Type"] = "NBit";
						filter["Index"] = idx;
						filter["Name"] = Marshal::PtrToStringAnsi(IntPtr(name));

						if (H5Z_FILTER_CONFIG_ENCODE_ENABLED & filter_config) {
							filter["EncodingEnabled"] = "True";
						}
						else {
							filter["EncodingEnabled"] = "False";
						}
						if (H5Z_FILTER_CONFIG_DECODE_ENABLED & filter_config) {
							filter["DecodingEnabled"] = "True";
						}
						else {
							filter["DecodingEnabled"] = "False";
						}
					}
					break;

				case H5Z_FILTER_SCALEOFFSET:
					{
						filter["Type"] = "ScaleOffset";
						filter["Index"] = idx;
						filter["Name"] = Marshal::PtrToStringAnsi(IntPtr(name));

						if (H5Z_FILTER_CONFIG_ENCODE_ENABLED & filter_config) {
							filter["EncodingEnabled"] = "True";
						}
						else {
							filter["EncodingEnabled"] = "False";
						}
						if (H5Z_FILTER_CONFIG_DECODE_ENABLED & filter_config) {
							filter["DecodingEnabled"] = "True";
						}
						else {
							filter["DecodingEnabled"] = "False";
						}
					}
					break;

				default:
					break;
				}

				m_filters->Add(filter);
			}
		}

#pragma endregion
        }
        finally
        {
			if (plist >= 0) {
                H5Pclose(plist);
            }
			if (dspace >= 0) {
                H5Sclose(dspace);
            }
            if (dtype >= 0) {
                H5Tclose(dtype);
            }
        }
    }
}
