#pragma once

#include "ObjectInfo.h"
#include "ProviderUtils.h"

extern "C" {
#include "H5Tpublic.h"
}

namespace PSH5X
{

    public ref class DatatypeInfo : ObjectInfo
    {
    public:

        DatatypeInfo(hid_t type) : ObjectInfo(type)
        {
            m_definition = ProviderUtils::ParseH5Type(type);
        }

        property System::Collections::Hashtable^ Definition
        {
            System::Collections::Hashtable^ get() { return m_definition; }
        }

        ~DatatypeInfo()
        {
            delete m_definition;
        }

    private:

        System::Collections::Hashtable^ m_definition;

    };

}
