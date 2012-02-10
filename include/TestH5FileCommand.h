#pragma once

namespace PSH5X
{
    [System::Management::Automation::Cmdlet("Test", "H5File",
        DefaultParameterSetName = "PathSet",
        SupportsShouldProcess=true,
        ConfirmImpact=System::Management::Automation::ConfirmImpact::Medium)]
    public ref class TestH5FileCommand : System::Management::Automation::PSCmdlet
    {
    public:

        TestH5FileCommand();

        [System::Management::Automation::Parameter(
            ParameterSetName="PathSet", Mandatory=true, Position=1,
            ValueFromPipeline=true, ValueFromPipelineByPropertyName=true)]
        [System::Management::Automation::Alias("FullName")]
        [System::Management::Automation::ValidateNotNullOrEmpty]
        property System::String^ Path
        {
            System::String^ get();
            void set(System::String^ value);
        }

        [System::Management::Automation::Parameter(
            ParameterSetName="FileInfoSet", Mandatory=true, Position=1,
            ValueFromPipeline=true)]
        property System::IO::FileInfo^ FileInfo
        {
            System::IO::FileInfo^ get();
            void set(System::IO::FileInfo^ value);
        }

    protected:

        virtual void ProcessRecord() override;

    private:

        System::String^ m_path;

        System::IO::FileInfo^ m_fileInfo;

    };

}