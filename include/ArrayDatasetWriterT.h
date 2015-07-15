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

        ArrayDatasetWriterT(hid_t h5file, System::String^ h5path,
			System::Management::Automation::RuntimeDefinedParameterDictionary^ dict)
            : m_h5file(h5file), m_h5path(h5path), m_dict(dict), m_array(nullptr), m_position(0)
        {
        }

        ~ArrayDatasetWriterT() { this->!ArrayDatasetWriterT(); }

        !ArrayDatasetWriterT() {}

        virtual void Close() {}

        virtual System::Collections::IList^ Write(System::Collections::IList^ content)
        {
            char* name = (char*)(Marshal::StringToHGlobalAnsi(m_h5path)).ToPointer();

            hid_t dset = -1, ftype = -1, mtype = -1, fspace = -1, mspace = H5S_ALL;

			bool sel_flag = false;

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
				mtype = H5Tget_native_type(ftype, H5T_DIR_ASCEND);
				if (mtype < 0) {
					throw gcnew ArgumentException("H5Tget_native_type failed!");
				}
                fspace = H5Dget_space(dset);
                if (fspace < 0) {
                    throw gcnew HDF5Exception("H5Dget_space failed!");
				}

				sel_flag = ProviderUtils::WriterCheckSelection(fspace, mspace, safe_cast<hsize_t>(content->Count), m_dict);

				hssize_t npoints = 1;
				if (H5Sget_simple_extent_type(fspace) == H5S_SIMPLE) {
					if (sel_flag) {
						npoints = H5Sget_select_npoints(fspace);
					}
					else {
						npoints = H5Sget_simple_extent_npoints(fspace);
					}
				}

				if (content->Count != safe_cast<int>(npoints)) {
					throw gcnew PSH5XException("Size mismatch!");
				}

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

				m_array = gcnew array<T>(safe_cast<int>(content->Count*arrayLength));
				int count = 0;

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

				if (H5Dwrite(dset, mtype, mspace, fspace, H5P_DEFAULT, ptr) < 0) {
					throw gcnew HDF5Exception("H5Dwrite failed!");
				}

				if (H5Fflush(dset, H5F_SCOPE_LOCAL) < 0) {
					throw gcnew ArgumentException("H5Fflush failed!");
				}

                m_position += content->Count;
            }
            finally
            {
				if (mspace != H5S_ALL) {
                    H5Sclose(mspace);
                }
				if (fspace >= 0) {
                    H5Sclose(fspace);
                }
                if (mtype >= 0) {
                    H5Tclose(mtype);
                }
                if (ftype >= 0) {
                    H5Tclose(ftype);
                }
                if (dset >= 0) {
                    H5Dclose(dset);
                }

                Marshal::FreeHGlobal(IntPtr(name));
            }

        return content;
    }

    virtual void Seek(long long offset, System::IO::SeekOrigin origin)
    {
		offset = 0;
		origin = System::IO::SeekOrigin::End;
        throw gcnew PSH5XException("DatasetWriter::Seek() not implemented!");
    }

private:

    hid_t m_h5file;

    System::String^ m_h5path;

	System::Management::Automation::RuntimeDefinedParameterDictionary^ m_dict;

    array<T>^ m_array;

    long long m_position;

};

}
