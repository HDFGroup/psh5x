
#include "HDF5Exception.h"
#include "PSH5XException.h"
#include "SimpleChunkedDatasetInfoLite.h"

extern "C" {
#include "H5Dpublic.h"
#include "H5Ppublic.h"
#include "H5Spublic.h"
#include "H5Tpublic.h"
}

#include <vector>

using namespace std;

using namespace System;
using namespace System::IO;
using namespace System::Management::Automation;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
    SimpleChunkedDatasetInfoLite::SimpleChunkedDatasetInfoLite(hid_t dset) : SimpleDatasetInfoLite(dset)
    {
        hid_t plist = -1;

        try
        {
            plist = H5Dget_create_plist(dset);
            if (plist < 0) {
                throw gcnew HDF5Exception("H5Dget_create_plist failed!");
            }

            H5D_layout_t lout = H5Pget_layout(plist);
            if (lout != H5D_CHUNKED) {
                throw gcnew PSH5XException("This is not a chunked dataset!");
            }

            vector<hsize_t> dims(m_rank);
            int rank = H5Pget_chunk(plist, m_rank, &dims[0]);
            if (rank == m_rank)
            {
                m_chunk = gcnew array<hsize_t>(m_rank);
                for (int i = 0; i < m_rank; ++i)
                    m_chunk[i] = dims[i];
            }
            else {
                throw gcnew HDF5Exception("Rank mismatch!");
            }
        }
        finally
        {
            if (plist >= 0) {
                H5Pclose(plist);
            }
        }
    }
}
