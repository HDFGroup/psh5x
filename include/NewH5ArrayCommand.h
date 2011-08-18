#pragma once

namespace PSH5X
{
    [System::Management::Automation::Cmdlet("New", "H5Array",
        SupportsShouldProcess=false,
        DefaultParameterSetName="Native")]
    public ref class NewH5ArrayCommand : System::Management::Automation::PSCmdlet
    {
    public:

        NewH5ArrayCommand()
            : m_length(-1), m_obj(nullptr),
            m_is_ht(false), m_ht(nullptr), m_is_str(false), m_str(nullptr) {}

        [System::Management::Automation::Parameter(Mandatory=true, Position=1)]
        property long long Length
        {
            long long get() { return m_length ; }
            void set(long long value) { m_length = value; }
        }

        [System::Management::Automation::Parameter(Mandatory=true, Position=2)]
        property System::Object^ Type
        {
            System::Object^ get() { return m_obj; }
            void set(Object^ value) { m_obj = value; }
        }

    protected:

        virtual void BeginProcessing() override;

        virtual void ProcessRecord() override;
        
    private:

        long long m_length;

        Object^ m_obj;

        bool m_is_ht;

        System::Collections::Hashtable^ m_ht;

        bool m_is_str;

        System::String^ m_str;

    };

}