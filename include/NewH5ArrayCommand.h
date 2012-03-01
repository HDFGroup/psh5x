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
            : m_length(-1), m_type(nullptr) {}

		[System::Management::Automation::Parameter(Mandatory=true, Position=1)]
        property System::String^ Type
        {
            System::String^ get() { return m_type; }
            void set(System::String^ value) { m_type = value; }
        }

        [System::Management::Automation::Parameter(Mandatory=true, Position=2)]
        property long long Length
        {
            long long get() { return m_length ; }
            void set(long long value) { m_length = value; }
        }

    protected:

        virtual void BeginProcessing() override;

        virtual void ProcessRecord() override;
        
    private:

        long long m_length;

        System::String^ m_type;

    };

}