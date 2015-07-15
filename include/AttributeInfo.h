#pragma once

extern "C" {
#include "H5Apublic.h"
#include "H5Opublic.h"
}

namespace PSH5X
{

    public ref class AttributeInfo
    {
    public:

        property System::String^ AttributeName
        {
            System::String^ get() { return m_attribute_name; }
        }

        property bool CreationOrderIsValid
        {
            bool get() { return (m_corder_valid > 0); }
        }

        property long long CreationOrder
        {
            long long get() { return safe_cast<long long>(m_corder); }
        }

        property System::String^ AttributeNameCharacterSet
        {
            System::String^ get() { return m_cset; }
        }

        property hsize_t DataSizeBytes
        {
            hsize_t get() { return m_data_size; }
        }

        AttributeInfo(System::String^ name, H5A_info_t* info)
        {
            m_attribute_name = name;
            m_corder_valid = info->corder_valid;
            m_corder = info->corder;
            switch (info->cset)
            {
            case H5T_CSET_ASCII:
                m_cset = gcnew System::String("ASCII");
                break;
            case H5T_CSET_UTF8:
                m_cset = gcnew System::String("UTF-8");
                break;
            default:
                m_cset = gcnew System::String("UNKNOWN");
                break;
            }
            m_data_size = info->data_size;
        }

        ~AttributeInfo()
        {
            delete m_attribute_name;
            delete m_cset;
        }

    private:
        
        System::String^ m_attribute_name;

        hbool_t         m_corder_valid;
        
        int64_t         m_corder;

        System::String^ m_cset;

        hsize_t         m_data_size;

    };

}