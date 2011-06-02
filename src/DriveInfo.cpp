
#include "DriveInfo.h"

extern "C" {
#include "H5ACpublic.h"
#include "H5Ppublic.h"
}

using namespace System;
using namespace System::Collections;
using namespace System::IO;
using namespace System::Management::Automation;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{

    DriveInfo::DriveInfo(String^ path, bool readonly, PSDriveInfo^ drive, bool force)
        : PSDriveInfo(drive)
    {
        m_path = path;
        m_readonly = readonly;

        unsigned flags = H5F_ACC_RDONLY;
        if (!m_readonly)
        {
            flags = H5F_ACC_RDWR;
        }

        if (File::Exists(m_path))
        {
           char* name = (char*)(Marshal::StringToHGlobalAnsi(m_path)).ToPointer();
           m_handle = H5Fopen(name, flags, H5P_DEFAULT);
           if (m_handle < 0)
           {
               String^ msg = String::Format(
                   "H5Fopen failed with status {0} for name {1}", m_handle, path);
               throw gcnew ArgumentException(msg);
           }
        }
        else if (force)
        {
            FileInfo^ info = gcnew FileInfo(path);
            char* filename = (char*)(Marshal::StringToHGlobalAnsi(info->FullName)).ToPointer();
            hid_t file = H5Fcreate(filename, H5F_ACC_EXCL, H5P_DEFAULT, H5P_DEFAULT);
            if (file >= 0) {
                if (H5Fclose(file) < 0) { // TODO
                }
            }
            else {
                String^ msg = String::Format(
                    "H5Fcreate failed with status {0} for name {1}", file, info->FullName);
                throw gcnew ArgumentException(msg);
            }
            
            m_path = info->FullName;
            m_readonly = false;
            m_handle = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
            if (m_handle < 0)
            {
                String^ msg = String::Format(
                    "H5Fopen failed with status {0} for name {1}", m_handle, info->FullName);
                throw gcnew ArgumentException(msg);
            }
        }
        else
            throw gcnew Exception("File not found");
    }

    Hashtable^ DriveInfo::CreationProperties::get()
    {
        Hashtable^ ht = gcnew Hashtable();

        hid_t plist = H5Fget_create_plist(m_handle);

        if (plist >= 0)
        {
            unsigned super, freelist, stab, shhdr;
            if (H5Pget_version(plist, &super, &freelist, &stab, &shhdr) >= 0)
            {
                Hashtable^ ht1 = gcnew Hashtable();
                ht1["SuperBlockVersion"] = super;
                ht1["FreeListVersion"] = freelist;
                ht1["SymbolTableVersion"] = stab;
                ht1["SharedObjectHeaderVersion"] = shhdr;
                ht["Version"] = ht1;
            }
            else
            {
                // TODO
            }

            hsize_t size;
            if (H5Pget_userblock(plist, &size) >= 0)
            {
                ht["UserBlockBytes"] = size;
            }
            else
            {
                // TODO
            }

            unsigned ik, lk;
            if (H5Pget_sym_k(plist, &ik, &lk))
            {
                Hashtable^ ht1 = gcnew Hashtable();
                ht1["BTreeHalfRank"] = ik;
                ht1["LeafNodeHalfSize"] = ik;
                ht["SymbolTable"] = ht1;
            }
            else
            {
                // TODO
            }

            if (H5Pget_istore_k(plist, &ik) >= 0)
            {
                ht["ChunkBTreeHalfRank"] = ik;
            }
            else
            {
                // TODO
            }

            unsigned nindexes;
            if (H5Pget_shared_mesg_nindexes(plist, &nindexes ) >= 0)
            {
                ht["SohmIndexCount"] = nindexes;

                if (nindexes > 0)
                {
                    // TODO
                }
            }
            else
            {
                // TODO
            }

            unsigned max_list, min_btree;
            if (H5Pget_shared_mesg_phase_change(plist, &max_list, &min_btree ) >= 0)
            {
                Hashtable^ ht1 = gcnew Hashtable();
                ht1["List2BTree"] = max_list;
                ht1["BTree2List"] = min_btree;
                ht["SohmPhaseChange"] = ht1;
            }
            else
            {
                // TODO
            }





            if (H5Pclose(plist) < 0)
            {
                // TODO
            }
        }
        else
        {
            // TODO
        }

        return ht;
    }

    
    hid_t DriveInfo::FileHandle::get()
    {
        return m_handle;
    }
    
    void DriveInfo::FileHandle::set(hid_t value)
    {
        m_handle = value;
    }

    hsize_t DriveInfo::FileSizeBytes::get()
    {
        hsize_t size;

        if (H5Fget_filesize(m_handle, &size) >= 0)
        {
            return size;
        }
        else
        {
            return 0;
        }
    }

    hssize_t DriveInfo::FreeSpaceBytes::get()
    {
        hssize_t size = H5Fget_freespace(m_handle);

        if (size >= 0)
        {
            return size;
        }
        else
        {
            return -1;
        }
    }

    Hashtable^ DriveInfo::H5FInfo::get()
    {
        Hashtable^ ht = gcnew Hashtable();

        H5F_info_t info;

        if (H5Fget_info(m_handle, &info ) >= 0)
        {
            ht["SuperBlockExtensionBytes"] = info.super_ext_size;
            ht["Sohm"] = gcnew Hashtable();
            ((Hashtable^)ht["Sohm"])->Add("MessageBytes", info.sohm.hdr_size);

            Hashtable^ ht1 = gcnew Hashtable();
            ht1["IndexBytes"] = info.sohm.msgs_info.index_size;
            ht1["HeapBytes"] = info.sohm.msgs_info.heap_size;

            ((Hashtable^)ht["Sohm"])->Add("IHSize", ht1);
        }
        else
        {
            // TODO
        }

        return ht;
    }

    Hashtable^ DriveInfo::MdcConfiguration::get()
    {
        Hashtable^ ht = gcnew Hashtable();

        H5AC_cache_config_t config;
        config.version = H5AC__CURR_CACHE_CONFIG_VERSION; 

        if (H5Fget_mdc_config(m_handle, &config) >= 0)
        {
            ht["MdcEvictionsEnabled"] = (config.evictions_enabled > 0);
            ht["MdcInitialBytes"] = config.initial_size;
            ht["MdcMinCleanFraction"] = config.min_clean_fraction;
            ht["MdcMaxBytes"] = config.max_size;
            ht["MdcMinBytes"] = config.min_size;
            ht["MdcEpochLength"] = config.epoch_length;

            switch (config.incr_mode)
            {
            case H5C_incr__off:
                ht["MdcCacheIncreaseMode"] = "Off";
                break;
            case H5C_incr__threshold:
                ht["MdcCacheIncreaseMode"] = "Threshold";
                break;
            default:
                break;
            }

            ht["MdcLowerHitRateThreshold"] = config.lower_hr_threshold;
            ht["MdcIncrement"] = config.increment;
            ht["MdcApplyMaxIncrement"] = (config.apply_max_increment > 0);
            ht["MdcMaxIncrementBytes"] = config.max_increment;

            switch (config.flash_incr_mode)
            {
            case H5C_flash_incr__off:
                ht["MdcFlashIncreaseMode"] = "Off";
                break;
            case H5C_flash_incr__add_space:
                ht["MdcFlashIncreaseMode"] = "AddSpace";
                break;
            default:
                break;
            }

            ht["MdcFlashThreshold"] = config.flash_threshold;
            ht["MdcFlashMultiple"] = config.flash_multiple;

            switch (config.decr_mode)
            {
            case H5C_decr__off:
                ht["MdcCacheDecreaseMode"] = "Off";
                break;
            case H5C_decr__threshold:
                ht["MdcCacheDecreaseMode"] = "Threshold";
                break;
            case H5C_decr__age_out:
                ht["MdcCacheDecreaseMode"] = "AgeOut";
                break;
            case H5C_decr__age_out_with_threshold:
                ht["MdcCacheDecreaseMode"] = "AgeOutWithThreshold";
                break;
            default:
                break;
            }

            ht["MdcUpperHitRateThreshold"] = config.upper_hr_threshold;
            ht["MdcDecrement"] = config.decrement;
            ht["MdcApplyMaxDecrement"] = (config.apply_max_decrement > 0);
            ht["MdcMaxDecrementBytes"] = config.max_decrement;
            ht["MdcEpochsBeforeEviction"] = config.epochs_before_eviction;
            ht["MdcApplyEmptyReserve"] = (config.apply_empty_reserve > 0);
            ht["MdcEmptyReserve"] = config.empty_reserve;
        }

        return ht;
    }

    double DriveInfo::MdcHitRate::get()
    {
        double hit_rate;

        if (H5Fget_mdc_hit_rate(m_handle, &hit_rate) >= 0)
        {
            return hit_rate;
        }
        else
        {
            return -1.0;
        }
    }

    Hashtable^ DriveInfo::MdcSize::get()
    {
        Hashtable^ ht = gcnew Hashtable();

        size_t max_size, min_clean_size, cur_size;
        int cur_num_entries;

        if (H5Fget_mdc_size(m_handle, &max_size, &min_clean_size, &cur_size, &cur_num_entries) >= 0)
        {
            ht["MdcMaxBytes"] = max_size;
            ht["MdcMinCleanBytes"] = min_clean_size;
            ht["MdcBytes"] = cur_size;
            ht["MdcNumberOfEntries"] = cur_num_entries;
        }

        return ht;
    }

    ssize_t DriveInfo::OpenFileCount::get()
    {
        return H5Fget_obj_count(m_handle, H5F_OBJ_FILE);
    }

    ssize_t DriveInfo::OpenDatasetCount::get()
    {
        return H5Fget_obj_count(m_handle, H5F_OBJ_DATASET);
    }

    ssize_t DriveInfo::OpenGroupCount::get()
    {
        return H5Fget_obj_count(m_handle, H5F_OBJ_GROUP);
    }

    ssize_t DriveInfo::OpenDatatypeObjectCount::get()
    {
        return H5Fget_obj_count(m_handle, H5F_OBJ_DATATYPE);
    }

    ssize_t DriveInfo::OpenAttributeCount::get()
    {
        return H5Fget_obj_count(m_handle, H5F_OBJ_ATTR);
    }

    String^ DriveInfo::Path::get()
    {
        return m_path;
    }

    bool DriveInfo::ReadOnly::get()
    {
        return m_readonly;
    }

}
