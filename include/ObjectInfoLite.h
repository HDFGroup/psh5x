#pragma once

#include "ItemInfo.h"
#include "ProviderUtils.h"

extern "C" {
#include "H5Opublic.h"
}

namespace PSH5X
{

    public ref class ObjectInfoLite : ItemInfo
    {
    public:

        property hsize_t AttributeCount
        {
            hsize_t get() { return m_num_attrs; }
        }

        property array<System::String^>^ AttributeNames
        {
            array<System::String^>^ get() { return m_attribute_names; }
        }

        property System::Collections::Hashtable^ Timestamps
        {
            System::Collections::Hashtable^ get()
            {
                System::Collections::Hashtable^ ht = gcnew System::Collections::Hashtable();
                ht["AccessTime"] = ProviderUtils::UnixTime2DateTime(m_atime);
                ht["BirthTime"] = ProviderUtils::UnixTime2DateTime(m_btime);
                ht["ChangeTime"] = ProviderUtils::UnixTime2DateTime(m_ctime);
                ht["ModificationTime"] = ProviderUtils::UnixTime2DateTime(m_mtime);
                return ht;
            }
        }

        ObjectInfoLite(hid_t obj_id);

        ~ObjectInfoLite()
        {
            for (int i = 0; i < m_attribute_names->Length; ++i)
            {
                delete m_attribute_names[i];
            }

            delete m_attribute_names;
        }
   
    protected:

        time_t                  m_atime;
        time_t                  m_mtime;
        time_t                  m_ctime;
        time_t                  m_btime;

        hsize_t                 m_num_attrs;

        array<System::String^>^ m_attribute_names;
    };

}