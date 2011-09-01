#pragma once

#include "ObjectInfoLite.h"

namespace PSH5X
{

    public ref class DatasetInfoLite : ObjectInfoLite
    {
    public:

        property long long ElementCount
        {
            long long get() { return safe_cast<long long>(m_npoints);  }
        }

        property System::String^ ElementTypeClass
        {
            System::String^ get() { return m_elem_type_class; }
        }

        property System::Object^ ElementType
        {
            System::Object^ get() { return m_elem_type; }
        }

        property System::String^ ExtentType
        {
            System::String^ get () { return m_simple_extent_type; }
        }
       
        property System::String^ Layout
        {
            System::String^ get() { return m_layout; }
        }

        property int Rank
        {
            int get() { return m_rank; }
        }

        property hsize_t StorageSizeBytes
        {
            hsize_t get() { return m_storage_size; }
        }

        DatasetInfoLite(hid_t dset);

        ~DatasetInfoLite()
        {
            delete m_elem_type_class;
            delete m_elem_type;
            delete m_simple_extent_type;
            delete m_layout;
        }

    protected:

        int               m_rank;

    private:

        System::String^   m_elem_type_class;

        System::Object^   m_elem_type;

        hsize_t           m_storage_size;

        System::String^   m_simple_extent_type;

        hssize_t          m_npoints;

        System::String^   m_layout;
    };

}
