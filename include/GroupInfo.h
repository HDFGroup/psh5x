#pragma once

#include "ObjectInfo.h"

extern "C" {
#include "H5Gpublic.h"
#include "H5Ppublic.h"
}

namespace PSH5X
{

    public ref class GroupInfo : ObjectInfo
    {
    public:

        property System::String^ StorageType
        {
            System::String^ get() { return m_storage_type; }
        }

        property long long LinkCount
        {
            long long get() { return safe_cast<long long>(m_nlinks); }
        }

        property long long MaxCreationOrder
        {
            long long get() { return safe_cast<long long>(m_max_corder); }
        }

        property bool Mounted
        {
            bool get() { return (m_mounted > 0); }
        }

        GroupInfo(hid_t group) : ObjectInfo(group)
        {
            H5G_info_t grp_info;
            if (H5Gget_info(group, &grp_info) >= 0)
            {
                switch (grp_info.storage_type)
                {
                case H5G_STORAGE_TYPE_SYMBOL_TABLE:
                    m_storage_type = "SymbolTable";
                    break;
                case H5G_STORAGE_TYPE_COMPACT:
                    m_storage_type = "Compact";
                    break;
                case H5G_STORAGE_TYPE_DENSE:
                    m_storage_type = "Dense";
                    break;
                default:
                    m_storage_type = "UNKNOWN";
                    break;
                }

                m_nlinks = grp_info.nlinks;
                m_max_corder = grp_info.max_corder;
                m_mounted = grp_info.mounted;
            }
            else
            {
                // TODO
            }

            hid_t cplist = H5Gget_create_plist(group);
            if (cplist >= 0)
            {
                unsigned est_num_entries, est_name_len;
                if (H5Pget_est_link_info(cplist, &est_num_entries, &est_name_len ) >= 0)
                {
                    System::Collections::Hashtable^ ht = gcnew System::Collections::Hashtable();
                    ht["LinkCount"] = est_num_entries;
                    ht["AverageNameLength"] = est_name_len;
                    __super::m_cplist["Estimates"] = ht;
                }
                else { // TODO
                }

                unsigned crt_order_flags;
                if (H5Pget_link_creation_order(cplist, &crt_order_flags) >= 0)
                {
                    if (crt_order_flags == 0)
                    {
                        __super::m_cplist["LinkCreationOrder"] = "NotSet";
                    }
                    else if (crt_order_flags & H5P_CRT_ORDER_TRACKED)
                    {
                        if (crt_order_flags & H5P_CRT_ORDER_INDEXED)
                        {
                            __super::m_cplist["LinkCreationOrder"] = "Indexed";
                        }
                        else
                        {
                            __super::m_cplist["LinkCreationOrder"] = "Tracked";
                        }
                    }
                }
                else { // TODO
                }
                
                unsigned max_compact, min_dense;
                if (H5Pget_link_phase_change(cplist, &max_compact, &min_dense) >= 0)
                {
                    System::Collections::Hashtable^ ht = gcnew System::Collections::Hashtable();
                    ht["Compact2Dense"] = max_compact;
                    ht["Dense2Compact"] = min_dense;
                    __super::m_cplist["LinkStoragePhaseChangeThresholds"] = ht; 
                }
                else { // TODO
                }

                if (H5Pclose(cplist) < 0) { // TODO
                }
            }
            else { // TODO
            }
        }

    private:

        System::String^ m_storage_type;

        hsize_t 	    m_nlinks;
        
        int64_t         m_max_corder;
        
        hbool_t         m_mounted;
    };

}