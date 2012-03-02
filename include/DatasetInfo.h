#pragma once

#include "ObjectInfo.h"

namespace PSH5X
{

    public ref class DatasetInfo : ObjectInfo
    {
    public:

        DatasetInfo(hid_t dset);

        property long long Offset
        {
            long long get() { return safe_cast<long long>(m_offset); }
        }

        property System::String^ ElementTypeClass
        {
            System::String^ get() { return m_elem_type_class; }
        }

        property System::String^ ElementType
        {
            System::String^ get() { return m_elem_type; }
        }

        property long long StorageSizeBytes
        {
            long long get() { return safe_cast<long long>(m_storage_size); }
        }

        property System::String^ DataspaceAllocationStatus
        {
            System::String^ get() { return m_dataspace_alloc_status; }
        }

        property System::String^ ExtentType
        {
            System::String^ get () { return m_simple_extent_type; }
        }

        property int Rank
        {
            int get() { return m_rank; }
        }

        property long long ElementCount
        {
            long long get() { return safe_cast<long long>(m_npoints);  }
        }

        property System::String^ Layout
        {
            System::String^ get() { return m_layout; }
        }

        property System::String^ FillValueDefinition
        {
            System::String^ get() { return m_fill_value_def; }
        }

        property System::String^ FillTime
        {
            System::String^ get() { return m_fill_time; }
        }

        property System::String^ AllocationTime
        {
            System::String^ get() { return m_alloc_time; }
        }

        property int FilterCount
        {
            int get() { return m_nfilters; }
        }

        ~DatasetInfo()
        {
            delete m_elem_type_class;
            delete m_elem_type;
            delete m_dataspace_alloc_status;
            delete m_simple_extent_type;
            delete m_layout;
            delete m_fill_value_def;
            delete m_fill_time;
            delete m_alloc_time;
        }

    protected:

        int               m_rank;

    private:

        haddr_t           m_offset;

        System::String^   m_elem_type_class;

        System::String^   m_elem_type;

        hsize_t           m_storage_size;

        System::String^   m_dataspace_alloc_status;

        System::String^   m_simple_extent_type;

        hssize_t          m_npoints;

        System::String^   m_layout;

        System::String^   m_fill_value_def;

        System::String^   m_fill_time;

        System::String^   m_alloc_time;

        int               m_nfilters;

    };

}
