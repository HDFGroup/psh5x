
#include "HDF5Exception.h"
#include "FormatH5FileCommand.h"

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
    FormatH5FileCommand::FormatH5FileCommand()
        : System::Management::Automation::PSCmdlet(), 
        m_path(nullptr), m_fileInfo(nullptr) {}

    String^ FormatH5FileCommand::Path::get()
    {
        return m_path;
    }

    void FormatH5FileCommand::Path::set(String^ value)
    {
        m_path = value;
    }

    System::IO::FileInfo^ FormatH5FileCommand::FileInfo::get()
    {
        return m_fileInfo;
    }

    void FormatH5FileCommand::FileInfo::set(System::IO::FileInfo^ value)
    {
        m_fileInfo = value;
    }

    void FormatH5FileCommand::ProcessRecord()
    {
        if (m_fileInfo != nullptr)
        {
            if (!m_fileInfo->Exists)
			{
                CreateH5File(m_fileInfo);
			}
            UpdateLastWriteTime(m_fileInfo);
			WriteObject(m_fileInfo);
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
            String^ fullPath = nullptr;
            bool containsWildcards = false;
            try
            {
                fullPath = GetUnresolvedProviderPathFromPSPath(m_path);
            }
            catch(ItemNotFoundException^ ex)
            {
                containsWildcards = true;
                ErrorRecord^ error = gcnew ErrorRecord(ex,
                    "FileCreationFailed",
                    ErrorCategory::WriteError,
                    m_path);
                WriteError(error);
                return;
            }

            System::IO::FileInfo^ info = gcnew System::IO::FileInfo(fullPath);
            if (!info->Exists)
			{
                CreateH5File(info);
			}
            UpdateLastWriteTime(info);
			WriteObject(info);
        }
        else
		{
            for (int i = 0; i < resolvedPaths->Count; ++i)
            {
                System::IO::FileInfo^ info =
                    gcnew System::IO::FileInfo(resolvedPaths[i]);

                if (!info->Exists)
				{
                    CreateH5File(info);
				}
                UpdateLastWriteTime(info);
				WriteObject(info);
            }
		}
    }

    void FormatH5FileCommand::CreateH5File(System::IO::FileInfo^ info)
    {
        if (!info->Exists)
		{
            if (this->ShouldProcess(info->FullName,
                "file does not exist, create it"))
            {
                try
                {
                    char* name = (char*)(Marshal::StringToHGlobalAnsi(info->FullName)).ToPointer();
                    hid_t status = H5Fcreate(name,
                        H5F_ACC_EXCL, H5P_DEFAULT, H5P_DEFAULT);
                    if (status < 0)
                    {
                        String^ msg = String::Format(
                            "H5Fcreate failed with status {0} for file '{1}'",
                            status, info->FullName);
                        throw gcnew HDF5Exception(msg); 
                    }
                    else
                    {
                        herr_t ierr = H5Fclose(status);
                        if (ierr < 0)
                        {
                            String^ msg = String::Format(
                                "H5Fclose failed with status {0} for name {1}",
                                ierr, info->FullName);
                            throw gcnew HDF5Exception(msg);
                        }
                    }
                }
                catch (Exception^ ex)
                {
                    ErrorRecord^ error = gcnew ErrorRecord(ex,
                        "FileCreationFailed",
                        ErrorCategory::WriteError,
                        info->FullName);
                    WriteError(error);
                    return;
                }
            }
		}
    }

    void FormatH5FileCommand::UpdateLastWriteTime(System::IO::FileInfo^ info)
    {
        try
        {
            if (info != nullptr)
			{
                if (this->ShouldProcess(info->FullName,
                    "set last write time to be " + DateTime::Now))
                {
                    info->LastWriteTime = DateTime::Now;
                }
			}
        }
        catch (UnauthorizedAccessException^ ex)
        {
            ErrorRecord^ error = gcnew ErrorRecord(ex,
                "UnauthorizedFileAccess",
                ErrorCategory::PermissionDenied,
                info->FullName);
            WriteError(error);
        }
    }
}