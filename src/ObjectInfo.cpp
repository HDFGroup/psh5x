
#include "ProviderUtils.h"
#include "ObjectInfo.h"

extern "C" {
#include "H5Dpublic.h"
#include "H5Gpublic.h"
#include "H5Opublic.h"
#include "H5Ppublic.h"
#include "H5Tpublic.h"
}

using namespace System;
using namespace System::Collections;
using namespace System::IO;
using namespace System::Management::Automation;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
    ObjectInfo::ObjectInfo(hid_t obj) : ObjectInfoLite(obj), m_obj_id(obj)
    {
        H5O_info_t info;

        hid_t cplist = -1;

        if (H5Oget_info(obj, &info) >= 0)
        {
            switch (info.type)
            {
            case H5O_TYPE_GROUP:
                cplist = H5Gget_create_plist(obj);
                break;
            case H5O_TYPE_DATASET:
                cplist = H5Dget_create_plist(obj);
                break;
            case H5O_TYPE_NAMED_DATATYPE:
                cplist = H5Tget_create_plist(obj);
                break;
            default:
                break;
            }

            m_fileno = info.fileno;
            m_addr = info.addr;
            m_rc = info.rc;
            
            m_header = gcnew Hashtable();

            m_header["Version"] = info.hdr.version;
            m_header["ObjectHeaderMessageCount"] = info.hdr.nmesgs;
            m_header["ObjectHeaderChunkCount"] = info.hdr.nchunks;
            m_header["ObjectHeaderStatus"] = info.hdr.flags;

            m_header["Space"] = gcnew Hashtable();
            ((Hashtable^) m_header["Space"])->Add("TotalBytes", info.hdr.space.total);
            ((Hashtable^) m_header["Space"])->Add("MetadataBytes", info.hdr.space.meta);
            ((Hashtable^) m_header["Space"])->Add("MessageBytes", info.hdr.space.mesg);
            ((Hashtable^) m_header["Space"])->Add("FreeBytes", info.hdr.space.free);

            m_header["MessageFlags"] = gcnew Hashtable();
            ((Hashtable^) m_header["MessageFlags"])->Add("Present", info.hdr.mesg.present);
            ((Hashtable^) m_header["MessageFlags"])->Add("Shared", info.hdr.mesg.shared);

            m_meta_size = gcnew Hashtable();

            m_meta_size["ForObjects"] = gcnew Hashtable();
            ((Hashtable^) m_meta_size["ForObjects"])->Add("IndexBytes", info.meta_size.obj.index_size);
            ((Hashtable^) m_meta_size["ForObjects"])->Add("HeapBytes", info.meta_size.obj.heap_size);

            m_meta_size["ForAttributes"] = gcnew Hashtable();
            ((Hashtable^) m_meta_size["ForAttributes"])->Add("IndexBytes", info.meta_size.attr.index_size);
            ((Hashtable^) m_meta_size["ForAttributes"])->Add("HeapBytes", info.meta_size.attr.heap_size);

            m_cplist = gcnew Hashtable();

            if (cplist >= 0)
            {
                unsigned max_compact, min_dense;
                if (H5Pget_attr_phase_change(cplist, &max_compact, &min_dense) >= 0)
                {
                    Hashtable^ ht = gcnew Hashtable();
                    ht["Compact2Dense"] = max_compact;
                    ht["Dense2Compact"] = min_dense;
                    m_cplist["AttributeStoragePhaseChangeThresholds"] = ht; 
                }
                else { // TODO
                }

                unsigned crt_order_flags;
                if (H5Pget_attr_creation_order(cplist, &crt_order_flags) >= 0)
                {
                    if (crt_order_flags == 0)
                    {
                        m_cplist["AttributeCreationOrder"] = "NotSet";
                    }
                    else if (crt_order_flags & H5P_CRT_ORDER_TRACKED)
                    {
                        if (crt_order_flags & H5P_CRT_ORDER_INDEXED)
                        {
                            m_cplist["AttributeCreationOrder"] = "Indexed";
                        }
                        else
                        {
                            m_cplist["AttributeCreationOrder"] = "Tracked";
                        }
                    }
                }
                else { // TODO
                }

                hbool_t track_times;
                if (H5Pget_obj_track_times(cplist, &track_times) >= 0)
                {
                    m_cplist["ObjectTimeIsTracked"] = (track_times > 0);
                }
                else { // TODO
                }

                if (H5Pclose(cplist) < 0) { // TODO
                }
            }
            else { // TODO
            }
        }
        else { // TODO
        }
    }

}
