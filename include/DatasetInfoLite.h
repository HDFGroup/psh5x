#pragma once

#include "ObjectInfoLite.h"

namespace PSH5X
{

    public ref class DatasetInfoLite : ObjectInfoLite
    {
    public:

        property array<hsize_t>^ Chunk
        {
            array<hsize_t>^ get() { return m_chunk; }
        }

        property array<hsize_t>^ Dimensions
        {
            array<hsize_t>^ get() { return m_dims; }
        }
        
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

        property array<long long>^ MaxDimensions
        {
            array<long long>^ get() { return m_maxdims; }
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
             delete m_dims;
             delete m_maxdims;
             delete m_layout;
             delete m_chunk;
         }

    private:

        System::String^   m_elem_type_class;

        System::Object^   m_elem_type;

        hsize_t           m_storage_size;

        System::String^   m_simple_extent_type;

        int               m_rank;

        hssize_t          m_npoints;

        array<hsize_t>^   m_dims;

        array<long long>^ m_maxdims;

        System::String^   m_layout;

        array<hsize_t>^   m_chunk;
    };

}
