
#include "HDF5Exception.h"
#include "ArrayDatasetWriter.h"
#include "PSH5XException.h"
#include "ProviderUtils.h"

extern "C" {
#include "H5Dpublic.h"
#include "H5Spublic.h"
#include "H5Tpublic.h"
}

#include <cstring>
#include <vector>

using namespace std;

using namespace System;
using namespace System::Collections;
using namespace System::Collections::ObjectModel;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Provider;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
	ArrayDatasetWriter::ArrayDatasetWriter(hid_t h5file, System::String^ h5path)
        : m_h5file(h5file), m_h5path(h5path), m_array(nullptr)
    {
    }

	IList^ ArrayDatasetWriter::Write(IList^ content)
    {
        char* name = (char*)(Marshal::StringToHGlobalAnsi(m_h5path)).ToPointer();

        hid_t dset = -1, fspace = -1, ftype = -1, ntype = -1, atype = -1, base_type = -1;

        array<unsigned char>^ wdata;

        hssize_t npoints = -1;

		vector<hsize_t> adims;

        try
		{
			dset = H5Dopen2(m_h5file, name, H5P_DEFAULT);
			if (dset < 0) {
				throw gcnew HDF5Exception("H5Dopen2 failed!");
			}

			fspace = H5Dget_space(dset);
			if (fspace < 0) {
				throw gcnew HDF5Exception("H5Dget_space failed!");
			}

			npoints = 1;
			if (H5Sget_simple_extent_type(fspace) == H5S_SIMPLE) {
				npoints = H5Sget_simple_extent_npoints(fspace);
			}

			if (content->Count != safe_cast<int>(npoints)) {
				throw gcnew PSH5XException("Size mismatch!");
			}

			if (npoints > 0)
			{
				ftype = H5Dget_type(dset);
				if (ftype < 0) {
					throw gcnew HDF5Exception("H5Dget_type failed!");
				}

				int rank = H5Tget_array_ndims(ftype);
				if (rank < 0) {
					throw gcnew HDF5Exception("H5Tget_array_ndims failed!");
				}

				adims = vector<hsize_t>(rank);
				if (H5Tget_array_dims2(ftype, &adims[0]) < 0) {
					throw gcnew HDF5Exception("H5Tget_array_dims2 failed!");
				}
				hsize_t count = 1;
				for (int i = 0; i < rank; ++i)
				{
					count *= adims[i];
				}

				base_type = H5Tget_super(ftype);
				if (base_type < 0) {
					throw gcnew HDF5Exception("H5Tget_super failed!");
				}

				Type^ t = ProviderUtils::H5Type2DotNet(base_type);
				if (t != nullptr)
				{
					size_t size = H5Tget_size(base_type);

					//wdata = gcnew array<unsigned char>(npoints*count*size);

					if (H5Tget_class(base_type) == H5T_BITFIELD) {
						ntype = H5Tget_native_type(base_type, H5T_DIR_DESCEND);
					}
					else {
						ntype = H5Tget_native_type(base_type, H5T_DIR_ASCEND);
					}
					if (ntype < 0) {
						throw gcnew HDF5Exception("H5Tget_native_type failed!");
					}

					atype = H5Tarray_create2(ntype, safe_cast<unsigned>(rank), &adims[0]);
					if (atype < 0) {
						throw gcnew HDF5Exception("H5Tarray_create2 failed!");
					}

					IEnumerator^ ienum = content->GetEnumerator();
					ienum->MoveNext();

					array<Object^>^ amaster = gcnew array<Object^>(npoints);

					for (int i = 0; i < npoints; ++i)
					{
						if (ienum->Current != nullptr)
						{
							Object^ obj = ProviderUtils::GetDotNetObject(ienum->Current);
							amaster[i] = safe_cast<Array^>(obj);
						}
						else {
							throw gcnew PSH5XException(String::Format("Uninitialized array found at position {0}!", npoints));
						}

						ienum->MoveNext();
					}

					H5T_class_t cls = H5Tget_class(base_type);
					switch (cls)
					{
					case H5T_INTEGER:

						wdata = gcnew array<int>(npoints*count);
						size_t count = 0;

						IEnumerator^ enumerator = nullptr;

						for (int i = 0; i < npoints; ++i)
						{
							 enumerator = ((Array^) amaster[i])->GetEnumerator();
							 while ((enumerator->MoveNext()) && (enumerator->Current != nullptr))
							 {
								 if (!TryGetValue(enumerator->Current, wdata[count++])) {
									 throw gcnew PSH5XException("Failed to get array value!");
								 }
							 }
						}

						break;

					case H5T_FLOAT:

						for (int i = 0; i < npoints; ++i) {
							//Array::Copy((Array^) amaster[i], 0, wdata, i*npoints*size, size);
						}

						break;

					case H5T_BITFIELD:

						for (int i = 0; i < npoints; ++i) {
							//Array::Copy((Array^) amaster[i], 0, wdata, i*npoints*size, size);
						}

						break;

					// TODO: handle strings and enums

					default:

						throw gcnew PSH5XException("Unsupported base type in array type!");
						break;
					}

					pin_ptr<unsigned char> wdata_ptr = &wdata[0];

					if (H5Dwrite(dset, atype, H5S_ALL, H5S_ALL, H5P_DEFAULT, wdata_ptr)) {
						throw gcnew HDF5Exception("H5Dwrite failed!");
					}
				}
			}
		}
        finally
        {
            if (atype >= 0) {
                H5Tclose(atype);
            }
            if (ntype >= 0) {
                H5Tclose(ntype);
            }
            if (ftype >= 0) {
                H5Tclose(ftype);
            }
            if (fspace >= 0) {
                H5Sclose(fspace);
            }
            if (dset >= 0) {
                H5Dclose(dset);
            }

            Marshal::FreeHGlobal(IntPtr(name));
        }

        return nullptr;
    }

}