#pragma once

#include "ItemInfo.h"

extern "C" {
#include "H5Lpublic.h"
}

namespace PSH5X
{

    public ref class LinkInfo : ItemInfo
    {
    public:

        property System::String^ FileName
        {
            System::String^ get() { return m_file_name; }
        }

        property System::String^ H5PathName
        {
            System::String^ get() { return m_h5_path_name; }
        }

        property bool CreationOrderIsValid
        {
            bool get() { return (m_corder_valid > 0); }
        }

        property long long CreationOrder
        {
            long long get() { return m_corder; }
        }

        property System::String^ LinkNameCharacterSet
        {
            System::String^ get() { return m_cset; }
        }

        LinkInfo(hid_t locid, System::String^ linkName, System::String^ itemType);

    private:

        hbool_t         m_corder_valid;

        int64_t         m_corder;

        System::String^ m_cset;

        System::String^ m_file_name;

        System::String^ m_h5_path_name;

    };

}