#pragma once

namespace PSH5X
{
    [System::Management::Automation::Cmdlet("Show", "H5Types",
        SupportsShouldProcess=false,
        DefaultParameterSetName="Native")]
    public ref class ShowH5TypesCommand : System::Management::Automation::PSCmdlet
    {
    public:

        ShowH5TypesCommand() {}

        [System::Management::Automation::Parameter(ParameterSetName="Bitfield")]
        System::Management::Automation::SwitchParameter Bitfield;

        [System::Management::Automation::Parameter(ParameterSetName="Float")]
        System::Management::Automation::SwitchParameter Float;

        [System::Management::Automation::Parameter(ParameterSetName="Integer")]
        System::Management::Automation::SwitchParameter Integer;

        [System::Management::Automation::Parameter(ParameterSetName="Native")]
        System::Management::Automation::SwitchParameter Native;

        [System::Management::Automation::Parameter(ParameterSetName="Reference")]
        System::Management::Automation::SwitchParameter Reference;

        [System::Management::Automation::Parameter(ParameterSetName="Standard")]
        System::Management::Automation::SwitchParameter Standard;

    protected:

        virtual void ProcessRecord() override;
        
    private:

    };

}