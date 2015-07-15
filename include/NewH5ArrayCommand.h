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
            : m_dims(nullptr), m_type(nullptr) {}

		[System::Management::Automation::Parameter(Mandatory=true, Position=1)]
		[System::Management::Automation::ValidateNotNullAttribute()]
        property System::String^ Type
        {
            System::String^ get() { return m_type; }
            void set(System::String^ value) { m_type = value; }
        }

        [System::Management::Automation::Parameter(Mandatory=true, Position=2)]
		[System::Management::Automation::ValidateNotNullAttribute()]
        property array<long long>^ Dimensions
        {
            array<long long>^ get() { return m_dims; }
            void set(array<long long>^ value) { m_dims = value; }
        }

    protected:

        virtual void BeginProcessing() override;

        virtual void ProcessRecord() override;
        
    private:

        array<long long>^ m_dims;

        System::String^ m_type;

    };

}