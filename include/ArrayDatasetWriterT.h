#pragma once

extern "C" {
#include "H5Dpublic.h"
#include "H5Spublic.h"
#include "H5Tpublic.h"
}

#include <vector>

namespace PSH5X
{
    template <typename T>
    public ref class ArrayDatasetWriterT
        : System::Management::Automation::Provider::IContentWriter
    {
    public:

        ArrayDatasetWriterT(hid_t h5file, System::String^ h5path)
            : m_h5file(h5file), m_h5path(h5path), m_array(nullptr), m_position(0)
        {
        }

        // TODO: deal with hyperslabs and point sets

        /*

        ArrayDatasetWriterT(hid_t h5file, System::String^ h5path,
        array<hsize_t>^ start, array<hsize_t>^ stride,
        array<hsize_t>^ count, array<hsize_t>^ block);

        ArrayDatasetWriterT(hid_t h5file, System::String^ h5path, array<hsize_t>^ coord);
        */

        ~ArrayDatasetWriterT() { this->!ArrayDatasetWriterT(); }

        !ArrayDatasetWriterT() {}

        virtual void Close() {}

        virtual System::Collections::IList^ Write(System::Collections::IList^ content)
        {
            char* name = (char*)(Marshal::StringToHGlobalAnsi(m_h5path)).ToPointer();

            hid_t dset = -1, fspace = -1, ftype = -1, ntype = -1;

			std::vector<hsize_t> adims;

            try
            {
                dset = H5Dopen2(m_h5file, name, H5P_DEFAULT);
                if (dset < 0) {
                    throw gcnew HDF5Exception("H5Dopen2 failed!");
                }
				ftype = H5Dget_type(dset);
                if (ftype < 0) {
                    throw gcnew HDF5Exception("H5Dget_type failed!");
                }
                fspace = H5Dget_space(dset);
                if (fspace < 0) {
                    throw gcnew HDF5Exception("H5Dget_space failed!");
                }

                hssize_t npoints = 1;
				if (H5Sget_simple_extent_type(fspace) == H5S_SIMPLE) {
				    npoints = H5Sget_simple_extent_npoints(fspace);
				}

                if (content->Count != safe_cast<int>(npoints))
                {
                    throw gcnew PSH5XException("Size mismatch!");
                }
                
				if (npoints > 0)
				{
					// get the array dimensions

					int rank = H5Tget_array_ndims(ftype);
					if (rank < 0) {
						throw gcnew HDF5Exception("H5Tget_array_ndims failed!");
					}

					adims = std::vector<hsize_t>(rank);
					if (H5Tget_array_dims2(ftype, &adims[0]) < 0) {
						throw gcnew HDF5Exception("H5Tget_array_dims2 failed!");
					}
					hsize_t arrayLength = 1;
					for (int i = 0; i < rank; ++i) {
						arrayLength *= adims[i];
					}

					m_array = gcnew array<T>(content->Count*arrayLength);
					size_t count = 0;

					IEnumerator^ ienum = content->GetEnumerator();
					ienum->MoveNext();
					
					Array^ a = nullptr;

					for (int i = 0; i < npoints; ++i)
					{
						if (ienum->Current != nullptr)
						{
							Object^ obj = ProviderUtils::GetDotNetObject(ienum->Current);
							a = safe_cast<Array^>(obj);
						}
						else {
							throw gcnew PSH5XException(String::Format("Uninitialized array found at position {0}!", npoints));
						}

						IEnumerator^ aenum = a->GetEnumerator();
						while ((aenum->MoveNext()) && (aenum->Current != nullptr)) {
							m_array[count++] = safe_cast<T>(aenum->Current);
						}

						ienum->MoveNext();
					}

					pin_ptr<T> ptr = &m_array[0];

					ntype = H5Tget_native_type(ftype, H5T_DIR_ASCEND);
					if (ntype < 0) {
						throw gcnew ArgumentException("H5Tget_native_type failed!");
					}

					if (H5Dwrite(dset, ntype, H5S_ALL, H5S_ALL, H5P_DEFAULT, ptr) < 0) {
						throw gcnew HDF5Exception("H5Dwrite failed!");
					}

					if (H5Fflush(dset, H5F_SCOPE_LOCAL) < 0) {
						throw gcnew ArgumentException("H5Fflush failed!");
					}
				}

                m_position += content->Count;
            }
            finally
            {
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

    virtual void Seek(long long offset, System::IO::SeekOrigin origin)
    {
        throw gcnew PSH5XException("DatasetWriter::Seek() not implemented!");
    }

private:

    hid_t m_h5file;

    System::String^ m_h5path;

    array<T>^ m_array;

    long long m_position;

};

}
