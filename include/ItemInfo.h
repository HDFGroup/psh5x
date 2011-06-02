#pragma once

namespace PSH5X
{

    public ref class ItemInfo
    {
    public:

        property System::String^ ItemType { System::String^ get() { return m_item_type; } }

        ItemInfo() : m_item_type(nullptr) {}

        ItemInfo(System::String^ itemType)
        {
            m_item_type = itemType;
        }

    protected:

        System::String^ m_item_type;

    };

}