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
    public ref class VlenDatasetWriterT
        : System::Management::Automation::Provider::IContentWriter
    {
    public:

        VlenDatasetWriterT(hid_t h5file, System::String^ h5path,
			System::Management::Automation::RuntimeDefinedParameterDictionary^ dict)
            : m_h5file(h5file), m_h5path(h5path), m_dict(dict), m_array(nullptr), m_position(0)
        {
        }

        ~VlenDatasetWriterT() { this->!VlenDatasetWriterT(); }

        !VlenDatasetWriterT() {}

        virtual void Close() {}

        virtual System::Collections::IList^ Write(System::Collections::IList^ content)
        {
            char* name = (char*)(Marshal::StringToHGlobalAnsi(m_h5path)).ToPointer();

            hid_t dset = -1, ftype = -1, mtype = -1, fspace = -1, mspace = H5S_ALL;

			bool sel_flag = false;

			std::vector<hvl_t> wdata;

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

				ftype = H5Dget_type(dset);
				if (ftype < 0) {
					throw gcnew HDF5Exception("H5Dget_type failed!");
				}
				mtype = H5Tget_native_type(ftype, H5T_DIR_ASCEND);
				if (mtype < 0) {
					throw gcnew HDF5Exception("H5Tget_native_type failed!");
				}

				wdata = std::vector<hvl_t>(npoints);

				IEnumerator^ ienum = content->GetEnumerator();
				ienum->MoveNext();

				// count the elements per array and initialize offsets into a global array

				array<int>^ len = gcnew array<int>(safe_cast<int>(npoints));
				array<int>^ offset = gcnew array<int>(safe_cast<int>(npoints));
				array<Object^>^ amaster = gcnew array<Object^>(safe_cast<int>(npoints));

				int total = 0;

				for (int i = 0; i < npoints; ++i)
				{
					if (ienum->Current != nullptr)
					{
						Object^ obj = ProviderUtils::GetDotNetObject(ienum->Current);
						Array^ a = safe_cast<Array^>(obj);

						offset[i] = total;
						len[i] = a->Length;
						amaster[i] = a;

						total += len[i];	
					}
					else {
						throw gcnew PSH5XException(String::Format("Uninitialized array found at position {0}!", npoints));
					}

					ienum->MoveNext();
				}

				++total;

				if (total > 1)  // there's nothing to write if total == 1
				{
					// copy stuff into a single global array and initialize the hvl_t structures

					array<T>^ arr = gcnew array<T>(total);
					pin_ptr<T> arr_ptr = &arr[0];
					pin_ptr<T> p;

					for (int i = 0; i < npoints; ++i) {
						if (len[i] > 0) {
							Array::Copy((Array^) amaster[i], 0, arr, offset[i], len[i]);
						}
						wdata[i].len = len[i];
						p = &arr[offset[i]];
						wdata[i].p = p;
					}

					if (H5Dwrite(dset, mtype, mspace, fspace, H5P_DEFAULT, &wdata[0])) {
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
