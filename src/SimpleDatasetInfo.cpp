
#include "HDF5Exception.h"
#include "PSH5XException.h"
#include "SimpleDatasetInfo.h"

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
    SimpleDatasetInfo::SimpleDatasetInfo(hid_t dset) : DatasetInfo(dset)
    {
        hid_t dspace = -1;

        try
        {
            dspace = H5Dget_space(dset);
            if (dspace < 0) {
                throw gcnew HDF5Exception("H5Dget_space failed!");
            }

            H5S_class_t type = H5Sget_simple_extent_type(dspace);
            if (type != H5S_SIMPLE) {
                throw gcnew PSH5XException("This is not a simple dataspace!");
            }

            vector<hsize_t> dims(m_rank);
            vector<hsize_t> maxdims(m_rank);

            int rank = H5Sget_simple_extent_dims(dspace, &dims[0], &maxdims[0]);
            if (rank > 0)
            {
                m_dims    = gcnew array<hsize_t>(m_rank);
                m_maxdims = gcnew array<long long>(m_rank);

                for (int i = 0; i < m_rank; ++i)
                {
                    m_dims[i]    = dims[i];
                    m_maxdims[i] = safe_cast<long long>(maxdims[i]);
                }
            }
        }
        finally
        {
            if (dspace >= 0) {
                H5Sclose(dspace);
            }
        }
    }
}
