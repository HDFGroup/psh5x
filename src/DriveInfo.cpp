
#include "DriveInfo.h"
#include "HDF5Exception.h"
#include "PSH5XException.h"

extern "C" {
#include "H5ACpublic.h"
#include "H5FDcore.h"
#include "H5FDdirect.h"
#include "H5FDfamily.h"
#include "H5FDlog.h"
#include "H5FDmulti.h"
#include "H5FDsec2.h"
#include "H5FDstdio.h"
#include "H5FDwindows.h"
#include "H5Ppublic.h"
}

using namespace System;
using namespace System::Collections;
using namespace System::IO;
using namespace System::Management::Automation;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{

    DriveInfo::DriveInfo(String^ path, bool readonly, PSDriveInfo^ drive, bool force, bool core)
        : PSDriveInfo(drive)
    {
        m_path = path;
        m_readonly = readonly;

        char *name = NULL, *filename = NULL;

        unsigned flags = H5F_ACC_RDONLY;

		hid_t fapl_id = H5P_DEFAULT;

		try
		{
			if (!m_readonly) {
				flags = H5F_ACC_RDWR;
			}

			if (core)
			{
				if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) {
					throw gcnew HDF5Exception("H5Pcreate failed!");
				}
				// 64 MB increments
				if (H5Pset_fapl_core(fapl_id, 6*1024*1024, 1) < 0) {
					throw gcnew HDF5Exception("H5Pset_fapl_core failed!");
				}
			}

			if (File::Exists(m_path))
			{
				name = (char*)(Marshal::StringToHGlobalAnsi(m_path)).ToPointer();
				if (H5Fis_hdf5(name) <= 0) {
					String^ msg = String::Format(
						"File '{0}' is not an HDF5 file", path);
					throw gcnew PSH5XException(msg);
				}

				m_handle = H5Fopen(name, flags, fapl_id);
				if (m_handle < 0) {
					String^ msg = String::Format(
						"H5Fopen failed with status {0} for name {1}", m_handle, path);
					throw gcnew HDF5Exception(msg);
				}
			}
			else if (force)
			{
				FileInfo^ info = gcnew FileInfo(path);
				filename = (char*)(Marshal::StringToHGlobalAnsi(info->FullName)).ToPointer();

				hid_t file = H5Fcreate(filename, H5F_ACC_EXCL, H5P_DEFAULT, H5P_DEFAULT);
				if (file >= 0) {
					if (H5Fclose(file) < 0) {
						throw gcnew HDF5Exception("H5Fclose failed!!!");
					}
				}
				else {
					String^ msg = String::Format(
						"H5Fcreate failed with status {0} for name {1}", file, info->FullName);
					throw gcnew HDF5Exception(msg);
				}

				m_path = info->FullName;
				m_readonly = false;

				m_handle = H5Fopen(filename, H5F_ACC_RDWR, fapl_id);
				if (m_handle < 0) {
					String^ msg = String::Format(
						"H5Fopen failed with status {0} for name {1}", m_handle, info->FullName);
					throw gcnew HDF5Exception(msg);
				}
			}
			else {
				throw gcnew PSH5XException(String::Format("File '{0}' not found", path));
			}
		}
		finally
		{
			if (name != NULL) {
				Marshal::FreeHGlobal(IntPtr(name));
			}

			if (filename != NULL) {
				Marshal::FreeHGlobal(IntPtr(filename));
			}

			if (fapl_id != H5P_DEFAULT) {
				H5Pclose(fapl_id);
			}
		}
    }

    Hashtable^ DriveInfo::CreationProperties::get()
    {
        Hashtable^ ht = gcnew Hashtable();

        hid_t plist = -1;
     
		try
		{
			plist = H5Fget_create_plist(m_handle);
			if (plist < 0) {
				throw gcnew HDF5Exception("H5Fget_create_plist failed!!!");
			}

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
			else {
				throw gcnew HDF5Exception("H5Pget_version failed!!!");
			}

			hsize_t size;
			if (H5Pget_userblock(plist, &size) >= 0)
			{
				ht["UserBlockBytes"] = size;
			}
			else {
				throw gcnew HDF5Exception("H5Pget_userblock failed!!!");
			}

			unsigned ik, lk;
			if (H5Pget_sym_k(plist, &ik, &lk))
			{
				Hashtable^ ht1 = gcnew Hashtable();
				ht1["BTreeHalfRank"] = ik;
				ht1["LeafNodeHalfSize"] = ik;
				ht["SymbolTable"] = ht1;
			}
			else {
				throw gcnew HDF5Exception("H5Pget_sym_k failed!!!");
			}

			if (H5Pget_istore_k(plist, &ik) >= 0)
			{
				ht["ChunkBTreeHalfRank"] = ik;
			}
			else {
				throw gcnew HDF5Exception("H5Pget_istore_k failed!!!");
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
			else {
				throw gcnew HDF5Exception("H5Pget_shared_mesg_nindexes failed!!!");
			}

			unsigned max_list, min_btree;
			if (H5Pget_shared_mesg_phase_change(plist, &max_list, &min_btree ) >= 0)
			{
				Hashtable^ ht1 = gcnew Hashtable();
				ht1["List2BTree"] = max_list;
				ht1["BTree2List"] = min_btree;
				ht["SohmPhaseChange"] = ht1;
			}
			else {
				throw gcnew HDF5Exception("H5Pget_shared_mesg_phase_change failed!!!");
			}
		}
		finally
		{
			if (plist >= 0) {
				H5Pclose(plist);
			}
		}

        return ht;
    }

	String^ DriveInfo::Driver::get()
    {
		String^ result = nullptr;

		hid_t plist = -1;
     
		try
		{
			plist = H5Fget_access_plist(m_handle);
			if (plist < 0) {
				throw gcnew HDF5Exception("H5Fget_access_plist failed!!!");
			}

			hid_t driver_id = H5Pget_driver(plist);

			if (driver_id == H5FD_SEC2) {
				result = "H5FD_SEC2";
			}
			else if (driver_id == H5FD_DIRECT) {
				result = "H5FD_DIRECT";
			}
			else if (driver_id == H5FD_LOG) {
				result = "H5FD_LOG";
			}
			else if (driver_id == H5FD_WINDOWS) {
				result = "H5FD_WINDOWS";
			}
			else if (driver_id == H5FD_STDIO) {
				result = "H5FD_STDIO";
			}
			else if (driver_id == H5FD_CORE) {
				result = "H5FD_CORE";
			}
			else if (driver_id == H5FD_FAMILY) {
				result = "H5FD_FAMILY";
			}
			else if (driver_id == H5FD_MULTI) {
				result = "H5FD_MULTI";
			}
			else {
				result = "UNKNOWN";
			}
		}
		finally
		{
			if (plist >= 0) {
				H5Pclose(plist);
			}
		}

		return result;
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

        if (H5Fget_filesize(m_handle, &size) < 0) {
            throw gcnew Exception("H5Fget_filesize failed!!!");
        }

        return size;
    }

    hssize_t DriveInfo::FreeSpaceBytes::get()
    {
        hssize_t size = H5Fget_freespace(m_handle);
        if (size < 0) {
            throw gcnew Exception("H5Fget_freespace failed!!!");
        }

        return size;
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
        else {
            throw gcnew Exception("H5Fget_info failed!!!");
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
        else {
            throw gcnew HDF5Exception("H5Fget_mdc_config failed!!!");
        }

        return ht;
    }

    double DriveInfo::MdcHitRate::get()
    {
        double hit_rate = -1;

        if (H5Fget_mdc_hit_rate(m_handle, &hit_rate) < 0) {
            throw gcnew HDF5Exception("H5Fget_mdc_hit_rate failed!!!");
        }

        return hit_rate;
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
        else {
            throw gcnew HDF5Exception("H5Fget_mdc_size failed!!!");
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
