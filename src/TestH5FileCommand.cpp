
#include "HDF5Exception.h"
#include "TestH5FileCommand.h"

extern "C" {
#include "H5Fpublic.h"
}

using namespace System;
using namespace System::Collections::ObjectModel;
using namespace System::IO;
using namespace System::Management::Automation;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
    TestH5FileCommand::TestH5FileCommand()
        : System::Management::Automation::PSCmdlet(), 
        m_path(nullptr), m_fileInfo(nullptr) {}

    String^ TestH5FileCommand::Path::get()
    {
        return m_path;
    }

    void TestH5FileCommand::Path::set(String^ value)
    {
        m_path = value;
    }

    System::IO::FileInfo^ TestH5FileCommand::FileInfo::get()
    {
        return m_fileInfo;
    }

    void TestH5FileCommand::FileInfo::set(System::IO::FileInfo^ value)
    {
        m_fileInfo = value;
    }

    void TestH5FileCommand::ProcessRecord()
    {
        if (m_fileInfo != nullptr)
        {
            if (m_fileInfo->Exists)
			{
				char* name = (char*)(Marshal::StringToHGlobalAnsi(m_fileInfo->FullName)).ToPointer();
				htri_t status = H5Fis_hdf5(name);
				WriteObject(status > 0);
			}
			else
			{
				WriteObject(false);
			}
            return;
        }

		ProviderInfo^ provider = nullptr;
		Collection<String^>^ resolvedPaths = nullptr;
		bool notFound = false;

		try
		{
			resolvedPaths = GetResolvedProviderPathFromPSPath(m_path, provider);
		}
		catch (ItemNotFoundException^)
		{
			// The specified path does not contain wildcards and cannot be found.
			notFound = true;
		}

		if (notFound)
		{
			WriteObject(false);
			return;
		}
		else
		{
			for (int i = 0; i < resolvedPaths->Count; ++i)
			{
				System::IO::FileInfo^ info =
					gcnew System::IO::FileInfo(resolvedPaths[i]);

				if (info->Exists)
				{
					char* name = (char*)(Marshal::StringToHGlobalAnsi(info->FullName)).ToPointer();
					htri_t status = H5Fis_hdf5(name);
					WriteObject(status > 0);
				}
				else
				{
					WriteObject(false);
				}
			}
		}
    }
}