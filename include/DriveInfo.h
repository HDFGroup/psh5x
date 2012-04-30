#pragma once

extern "C" {
#include "H5Fpublic.h"
}

namespace PSH5X
{
    public ref class DriveInfo : System::Management::Automation::PSDriveInfo
    {
    public:

        DriveInfo(System::String^ path, bool readonly,
            System::Management::Automation::PSDriveInfo^ drive, bool force, bool core);

        property System::Collections::Hashtable^ CreationProperties
        {
            System::Collections::Hashtable^ get();
        }

		property System::String^ Driver
        {
            System::String^ get();
        }

        property hid_t FileHandle
        {
            hid_t get();

        internal:
            void set(hid_t value);
        }

        property hsize_t FileSizeBytes
        {
            hsize_t get();
        }

        property hssize_t FreeSpaceBytes
        {
            hssize_t get();
        }

        property System::Collections::Hashtable^ H5FInfo
        {
            System::Collections::Hashtable^ get();
        }

        property System::Collections::Hashtable^ MdcConfiguration
        {
            System::Collections::Hashtable^ get();
        }

        property double MdcHitRate
        {
            double get();
        }

        property System::Collections::Hashtable^ MdcSize
        {
            System::Collections::Hashtable^ get();
        }

        property ssize_t OpenFileCount
        {
            ssize_t get();
        }

        property ssize_t OpenDatasetCount
        {
            ssize_t get();
        }

        property ssize_t OpenGroupCount
        {
            ssize_t get();
        }

        property ssize_t OpenDatatypeObjectCount
        {
            ssize_t get();
        }

        property ssize_t OpenAttributeCount
        {
            ssize_t get();
        }

        property System::String^ Path
        {
            System::String^ get();
        }

        property bool ReadOnly
        {
            bool get();
        }        

    private:

        System::String^ m_path;
        
        bool            m_readonly;

        hid_t           m_handle;
    };
}