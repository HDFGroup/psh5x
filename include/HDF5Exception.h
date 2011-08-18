#pragma once

namespace PSH5X
{
    public ref class HDF5Exception : public System::Exception
    {
    public:

        HDF5Exception(System::String^ message)
            : System::Exception(message)
        {
        }
    };

}