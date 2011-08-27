
#pragma once

namespace PSH5X
{

    [System::Management::Automation::Provider::CmdletProvider("HDF5",
        System::Management::Automation::Provider::ProviderCapabilities::Filter|
        System::Management::Automation::Provider::ProviderCapabilities::ShouldProcess)]
    public ref class Provider
        : System::Management::Automation::Provider::NavigationCmdletProvider,
        System::Management::Automation::Provider::IDynamicPropertyCmdletProvider,
        System::Management::Automation::Provider::IContentCmdletProvider
    {
    public:

        Provider() : System::Management::Automation::Provider::NavigationCmdletProvider() {}

    protected:

        virtual System::Management::Automation::ProviderInfo^
            Start(System::Management::Automation::ProviderInfo^ providerInfo) override;

        virtual void Stop() override;

#pragma region DriveCmdletProvider

        virtual System::Collections::ObjectModel::Collection<System::Management::Automation::PSDriveInfo^>^
            InitializeDefaultDrives() override;

        virtual System::Management::Automation::PSDriveInfo^
            NewDrive(System::Management::Automation::PSDriveInfo^ drive) override;

        virtual System::Object^ NewDriveDynamicParameters() override;

        virtual System::Management::Automation::PSDriveInfo^
            RemoveDrive(System::Management::Automation::PSDriveInfo^ drive) override;

#pragma endregion

#pragma region ItemCmdletProvider

        virtual void ClearItem(System::String^ path) override;

        virtual System::Object^ ClearItemDynamicParameters(System::String^ path) override;

        virtual bool IsValidPath(System::String^ path) override;

        virtual void GetItem(System::String^ path) override;

        virtual System::Object^ GetItemDynamicParameters(System::String^ path) override;

        virtual void InvokeDefaultAction(System::String^ path) override;

        virtual System::Object^ InvokeDefaultActionDynamicParameters(System::String^ path) override;

        virtual bool ItemExists(System::String^ path) override;

        virtual System::Object^ ItemExistsDynamicParameters(System::String^ path) override;

        virtual void SetItem(System::String^ path, System::Object^ value) override;

        virtual System::Object^ SetItemDynamicParameters(System::String^ path, System::Object^ value) override;

#pragma endregion

#pragma region ContainerCmdletProvider

        virtual void CopyItem(System::String^ path, System::String^ copyPath, bool recurse) override;

        virtual System::Object^ CopyItemDynamicParameters(System::String^ path, System::String^ copyPath, bool recurse) override;

        virtual void GetChildItems(System::String^ path, bool recurse) override;

        virtual System::Object^ GetChildItemsDynamicParameters(System::String^ path, bool recurse) override;
       
        virtual void GetChildNames(System::String^ path,
            System::Management::Automation::ReturnContainers returnContainers) override;

        // virtual System::Object^ GetChildNamesDynamicParameters(System::String^ path) override;

        virtual bool HasChildItems(System::String^ path) override;

        virtual void NewItem(System::String^ path, System::String^ itemTypeName,
            System::Object^ newValue) override;

        virtual System::Object^ NewItemDynamicParameters(System::String^ path, System::String^ itemTypeName,
            System::Object^ newValue) override;

        virtual void RemoveItem(System::String^ path, bool recurse) override;

        virtual System::Object^ RemoveItemDynamicParameters(System::String^ path, bool recurse) override;

        virtual void RenameItem(System::String^ path, System::String^ newName) override;

        virtual System::Object^ RenameItemDynamicParameters(System::String^ path, System::String^ newName) override;

#pragma endregion

#pragma region NavigationCmdletProvider

        virtual System::String^ GetChildName(System::String^ path) override;

        virtual System::String^ GetParentPath(System::String^ path, System::String^ root) override;

        virtual bool IsItemContainer(System::String^ path) override;

        virtual System::String^ MakePath(System::String^ parent, System::String^ child) override;

        virtual void MoveItem(System::String^ path, System::String^ destination) override;

        virtual System::Object^ MoveItemDynamicParameters(System::String^ path, System::String^ destination) override;

#pragma endregion

    public:

#pragma region IDynamicPropertyCmdletProvider

#pragma region IPropertyCmdletProvider
        
        virtual void ClearProperty(System::String^ path,
            System::Collections::ObjectModel::Collection<System::String^>^ propertyToClear);

        virtual System::Object^ ClearPropertyDynamicParameters(System::String^ path,
            System::Collections::ObjectModel::Collection<System::String^>^ propertyToClear);

        virtual void GetProperty(System::String^ path,
            System::Collections::ObjectModel::Collection<System::String^>^
            providerSpecificPickList);

        virtual System::Object^ GetPropertyDynamicParameters(System::String^ path,
            System::Collections::ObjectModel::Collection<System::String^>^
            providerSpecificPickList);

        virtual void SetProperty(System::String^ path,
            System::Management::Automation::PSObject^ propertyValue);

        virtual System::Object^ SetPropertyDynamicParameters(System::String^ path,
            System::Management::Automation::PSObject^ propertyValue);

#pragma endregion

        virtual void CopyProperty(System::String^ sourcePath, System::String^ sourceProperty,
            System::String^ destinationPath, System::String^ destinationProperty);

        virtual System::Object^ CopyPropertyDynamicParameters(
            System::String^ sourcePath, System::String^ sourceProperty,
            System::String^ destinationPath, System::String^ destinationProperty);

        virtual void MoveProperty(System::String^ sourcePath, System::String^ sourceProperty,
            System::String^ destinationPath, System::String^ destinationProperty);

        virtual System::Object^ MovePropertyDynamicParameters(
            System::String^ sourcePath, System::String^ sourceProperty,
            System::String^ destinationPath, System::String^ destinationProperty);

        virtual void NewProperty(System::String^ path, System::String^ propertyName,
            System::String^ propertyTypeName, System::Object^ value);

        virtual System::Object^ NewPropertyDynamicParameters(System::String^ path,
            System::String^ propertyName, System::String^ propertyTypeName,
            System::Object^ value);

        virtual void RemoveProperty(System::String^ path, System::String^ propertyName);

        virtual System::Object^ RemovePropertyDynamicParameters(System::String^ path,
            System::String^ propertyName);

        virtual void RenameProperty(System::String^ path, System::String^ sourceProperty,
            System::String^ destinationProperty);

        virtual System::Object^ RenamePropertyDynamicParameters(System::String^ path,
            System::String^ sourceProperty, System::String^ destinationProperty);

#pragma endregion

#pragma region IContentCmdletProvider

        virtual void ClearContent(System::String ^ path);

        virtual System::Object^ ClearContentDynamicParameters(System::String ^ path);


        virtual System::Management::Automation::Provider::IContentReader^
            GetContentReader(System::String ^ path);

        virtual System::Object^ GetContentReaderDynamicParameters(System::String^ path);

        virtual System::Management::Automation::Provider::IContentWriter^
            GetContentWriter(System::String ^ path);

        virtual System::Object^ GetContentWriterDynamicParameters(System::String^ path);

#pragma endregion

    };

}
