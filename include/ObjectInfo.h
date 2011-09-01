#pragma once


#include "ObjectInfoLite.h"

extern "C" {
#include "H5Opublic.h"
}

namespace PSH5X
{

    public ref class ObjectInfo : ObjectInfoLite
    {
    public:

        property haddr_t Address
        {
            haddr_t get() { return m_addr; }
        }

        property System::Collections::Hashtable^ CreationPropertyList
        {
            System::Collections::Hashtable^ get() { return m_cplist; } 
        }

        property unsigned long FileNo
        {
            unsigned long get() { return m_fileno; }
        }

        property System::Collections::Hashtable^ Header
        {
            System::Collections::Hashtable^ get() { return m_header; } 
        }
        
        property System::Collections::Hashtable^ AdditionalMetadata
        {
            System::Collections::Hashtable^ get() { return m_meta_size; }
        }

        property hid_t ObjectID
        {
            hid_t get() { return m_obj_id; }
        }

        property unsigned ReferenceCount
        {
            unsigned get() { return m_rc; }
        }

        ObjectInfo(hid_t obj_id);

        ~ObjectInfo()
        {
            delete m_cplist;
            delete m_header;
            delete m_meta_size;
        }


    protected:

        System::Collections::Hashtable^ m_cplist;
            
    private:

        hid_t                           m_obj_id;

        unsigned long                   m_fileno;

        haddr_t                         m_addr;

        unsigned                        m_rc;

        System::Collections::Hashtable^ m_header;

        System::Collections::Hashtable^ m_meta_size;
    };

}