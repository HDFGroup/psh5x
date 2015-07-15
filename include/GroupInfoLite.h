#pragma once

#include "HDF5Exception.h"
#include "ObjectInfoLite.h"

extern "C" {
#include "H5Gpublic.h"
}

namespace PSH5X
{

    public ref class GroupInfoLite : ObjectInfoLite
    {
    public:

        property long long LinkCount
        {
            long long get() { return safe_cast<long long>(m_nlinks); }
        }

        GroupInfoLite(hid_t group) : ObjectInfoLite(group)
        {
            H5G_info_t grp_info;
            if (H5Gget_info(group, &grp_info) >= 0)
            {    
                m_nlinks = grp_info.nlinks;
            }
            else {
                throw gcnew HDF5Exception("H5Gget_info failed!");
            }
        }

    protected:

        hsize_t 	    m_nlinks;
    };

}