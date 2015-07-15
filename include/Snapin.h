
#pragma once

namespace PSH5X
{
    [System::ComponentModel::RunInstaller(true)]
    public ref class Snapin : System::Management::Automation::PSSnapIn
	{
	public:

        Snapin() : System::Management::Automation::PSSnapIn() {}
		
        virtual property System::String^ Name
        {
            System::String^ get() override
            {
                return gcnew System::String("HDF5");
            }
        }

        virtual property System::String^ Vendor
        {
            System::String^ get() override
            {
                return gcnew System::String("The HDF Group");
            }
        }

        virtual property System::String^ VendorResource
        {
            System::String^ get() override
            {
                return gcnew System::String("HDF5,The HDF Group");
            }
        }

        virtual property System::String^ Description
        {
            System::String^ get() override
            {
                return gcnew System::String("This is a PowerShell snap-in that includes several HDF5 cmdlets and an HDF5 provider.");
            }
        }

        virtual property System::String^ DescriptionResource
        {
            System::String^ get() override
            {
                return gcnew System::String("HDF5,This is a PowerShell snap-in that includes several HDF5 cmdlets and an HDF5 provider.");
            }
        }
	};
}
