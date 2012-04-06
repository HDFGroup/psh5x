#pragma once

#include "ObjectInfo.h"
#include "ProviderUtils.h"

extern "C" {
#include "H5Ipublic.h"
}

namespace PSH5X
{

    public ref class DatatypeInfo : ObjectInfo
    {
    public:

        DatatypeInfo(hid_t type) : ObjectInfo(type)
        {
			System::Web::Script::Serialization::JavaScriptSerializer^ ser =
				gcnew System::Web::Script::Serialization::JavaScriptSerializer();
            m_definition = ser->Serialize(ProviderUtils::ParseH5Type(type));
        }

        property System::String^ Definition
        {
            System::String^ get() { return m_definition; }
        }

    private:

        System::String^ m_definition;

    };

}
