
#include "LinkInfo.h"

extern "C" {
#include "H5Lpublic.h"
#include "H5Ppublic.h"
}

using namespace System;
using namespace System::Collections;
using namespace System::IO;
using namespace System::Management::Automation;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{

    LinkInfo::LinkInfo(hid_t locid, System::String^ linkName)
        : ItemInfo("Unknown")
    {
        H5L_info_t info;
        char* name = (char*)(Marshal::StringToHGlobalAnsi(linkName)).ToPointer();

        if (H5Lget_info(locid, name, &info, H5P_DEFAULT) >= 0)
        {
			switch (info.type)
			{
			case H5L_TYPE_HARD:
				m_item_type = "HardLink";
				break;
			case H5L_TYPE_SOFT:
				m_item_type = "SoftLink";
				break;
			case H5L_TYPE_EXTERNAL:
				m_item_type = "ExternalLink";
				break;
			case H5L_TYPE_ERROR:
				m_item_type = "Unknown";
				break;
			}

            m_corder_valid = info.corder_valid;
            m_corder = info.corder;

            switch (info.cset)
            {
            case H5T_CSET_ASCII:
                m_cset = gcnew System::String("ASCII");
                break;
            case H5T_CSET_UTF8:
                m_cset = gcnew System::String("UTF-8");
                break;
            default:
                m_cset = gcnew System::String("UNKNOWN");
                break;
            }

            size_t val_size = info.u.val_size;

			array<char>^ buf = gcnew array<char>(safe_cast<int>(val_size));
            pin_ptr<char> buf_ptr = &buf[0];
							
            if (H5Lget_val(locid, name, buf_ptr, val_size, H5P_DEFAULT) < 0) {
                throw gcnew InvalidOperationException("H5Lget_val failed!");
            }

            if (m_item_type == "SoftLink")
            {
                m_h5_path_name = gcnew String(buf_ptr);
                m_file_name = ".";
            }
            else if (m_item_type == "ExtLink")
            {
                unsigned flags;
                const char* filename;
                const char* obj_path;

                if (H5Lunpack_elink_val(buf_ptr, val_size, &flags, &filename, &obj_path) >= 0)
                {
                    m_h5_path_name = gcnew String(obj_path);
                    m_file_name = gcnew String(filename);
                }
                else {
                    throw gcnew InvalidOperationException("H5Lunpack_elink_val failed!");
                }
            }
        }
        else {
            throw gcnew InvalidOperationException("H5Lget_info failed!");
        }

        Marshal::FreeHGlobal(IntPtr(name));
    }

}