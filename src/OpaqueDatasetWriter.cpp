
#include "HDF5Exception.h"
#include "OpaqueDatasetWriter.h"
#include "PSH5XException.h"
#include "ProviderUtils.h"

extern "C" {
#include "H5Dpublic.h"
#include "H5Spublic.h"
#include "H5Tpublic.h"
}

using namespace System;
using namespace System::Collections;
using namespace System::Collections::ObjectModel;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Provider;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
	OpaqueDatasetWriter::OpaqueDatasetWriter(hid_t h5file, String^ h5path,
		RuntimeDefinedParameterDictionary^ dict)
		: m_h5file(h5file), m_h5path(h5path), m_dict(dict), m_array(nullptr), m_position(0)
	{
	}

	IList^ OpaqueDatasetWriter::Write(IList^ content)
	{
		char* name = (char*)(Marshal::StringToHGlobalAnsi(m_h5path)).ToPointer();

		hid_t dset = -1, ftype = -1, mtype = -1, fspace = -1, mspace = H5S_ALL;

		bool sel_flag = false;

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

			// get the opaque type size

			size_t osize = H5Tget_size(ftype);
			if (osize == 0) {
				throw gcnew PSH5XException("Zero-sized opaque type found!");
			}

			m_array = gcnew array<unsigned char>(content->Count*safe_cast<int>(osize));
			size_t count = 0;

			IEnumerator^ ienum = content->GetEnumerator();
			ienum->MoveNext();

			Array^ a = nullptr;

			for (int i = 0; i < npoints; ++i)
			{
				if (ienum->Current != nullptr)
				{
					Object^ obj = ProviderUtils::GetDotNetObject(ienum->Current);
					a = safe_cast<array<unsigned char>^>(obj);
				}
				else {
					throw gcnew PSH5XException(String::Format("Uninitialized array found at position {0}!", npoints));
				}

				IEnumerator^ aenum = a->GetEnumerator();
				while ((aenum->MoveNext()) && (aenum->Current != nullptr)) {
					((array<unsigned char>^)m_array)[count++] = safe_cast<unsigned char>(aenum->Current);
				}

				ienum->MoveNext();
			}

			pin_ptr<unsigned char> ptr = &((array<unsigned char>^)m_array)[0];

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

	void OpaqueDatasetWriter::Seek(long long offset, System::IO::SeekOrigin origin)
	{
		offset = 0;
		origin = System::IO::SeekOrigin::End;
		throw gcnew PSH5XException("OpaqueDatasetWriter::Seek() not implemented!");
	}

}