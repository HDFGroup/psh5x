#pragma once

namespace PSH5X
{
    public ref class PSH5XException : public System::Exception
    {
    public:

        PSH5XException(System::String^ message)
            : System::Exception(message)
        {
        }
    };

}