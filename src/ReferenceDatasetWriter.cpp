
#include "HDF5Exception.h"
#include "ReferenceDatasetWriter.h"
#include "PSH5XException.h"
#include "ProviderUtils.h"

extern "C" {
#include "H5Dpublic.h"
#include "H5Rpublic.h"
#include "H5Spublic.h"
#include "H5Tpublic.h"
}

using namespace System;
using namespace System::Collections;
using namespace System::Collections::Generic;
using namespace System::Collections::ObjectModel;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Provider;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
	ReferenceDatasetWriter::ReferenceDatasetWriter(hid_t h5file, String^ h5path,
		RuntimeDefinedParameterDictionary^ dict)
		: m_h5file(h5file), m_h5path(h5path), m_dict(dict), m_array(nullptr), m_position(0)
	{
	}

	System::Collections::IList^ ReferenceDatasetWriter::Write(System::Collections::IList^ content)
	{
		char* name = (char*)(Marshal::StringToHGlobalAnsi(m_h5path)).ToPointer();

		hid_t dset = -1, ftype = -1, mtype = -1, fspace = -1, mspace = H5S_ALL, robj = -1, rspace = -1;

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

			// get the reference type size
			size_t rsize = H5Tget_size(ftype);
			if (rsize == 0) {
				throw gcnew PSH5XException("Zero-sized reference type found!");
			}

			m_array = gcnew array<unsigned char>(content->Count*safe_cast<int>(rsize));
			pin_ptr<unsigned char> ptr = &((array<unsigned char>^)m_array)[0];

			System::Collections::IEnumerator^ ienum = content->GetEnumerator();
			ienum->MoveNext();

			// check if we are dealing with object or region references

			if (H5Tequal(ftype, H5T_STD_REF_OBJ) > 0)
			{
#pragma region object reference

				String^ path;

				for (int i = 0; i < npoints; ++i)
				{
					if (ienum->Current != nullptr)
					{
						Object^ obj = ProviderUtils::GetDotNetObject(ienum->Current);
						path = safe_cast<String^>(obj);
					}
					else {
						throw gcnew PSH5XException(String::Format("Empty string found at position {0}!", npoints));
					}

					IntPtr path_str = Marshal::StringToHGlobalAnsi(path);

					pin_ptr<unsigned char> ptr = &((array<unsigned char>^)m_array)[i*rsize];
					if (H5Rcreate(ptr, m_h5file, (char*) path_str.ToPointer(), H5R_OBJECT, -1) < 0) {
						throw gcnew HDF5Exception("H5Rcreate failed!");
					}
					Marshal::FreeHGlobal(path_str);

					ienum->MoveNext();
				}

#pragma endregion
			}
			else if (H5Tequal(ftype, H5T_STD_REF_DSETREG) > 0)
			{
#pragma region region reference

				KeyValuePair<String^, Array^> rref;

				for (int i = 0; i < npoints; ++i)
				{
					if (ienum->Current != nullptr)
					{
						Object^ obj = ProviderUtils::GetDotNetObject(ienum->Current);
						rref = safe_cast< KeyValuePair<String^,Array^> >(obj);
					}
					else {
						throw gcnew PSH5XException(String::Format("Empty KeyValuePair found at position {0}!", npoints));
					}

					IntPtr path_str = Marshal::StringToHGlobalAnsi(rref.Key);

					if ((robj = H5Oopen(m_h5file, (char*) path_str.ToPointer(), H5P_DEFAULT)) < 0) {
						throw gcnew HDF5Exception("H5Oopen failed!");
					}
					H5O_info_t info;
					if (H5Oget_info(robj, &info) < 0) {
						throw gcnew HDF5Exception("H5Oget_info failed!");
					}
					if (info.type != H5O_TYPE_DATASET) {
						throw gcnew PSH5XException(String::Format("The object at '{0}' is not an HDF5 dataset!", rref.Key));
					}
					if ((rspace = H5Dget_space(robj)) < 0) {
						throw gcnew HDF5Exception("H5Dget_space failed!");
					}
					if (H5Sget_simple_extent_type(rspace) != H5S_SIMPLE) {
						throw gcnew PSH5XException(String::Format("The object at '{0}' is not a simple HDF5 dataset!", rref.Key));
					}
					int rank = H5Sget_simple_extent_ndims(rspace);
					if (rank <= 0) {
						throw gcnew HDF5Exception("H5Sget_simple_extent_ndims failed!");
					}

					array<hsize_t,2>^ arr = (array<hsize_t,2>^) rref.Value;
					pin_ptr<hsize_t> arr_ptr = &arr[0,0];
					int rows = arr->GetLength(0);

					if (arr->GetLength(1) == rank)
					{
						if (H5Sselect_none(rspace) < 0) {
							throw gcnew HDF5Exception("H5Sselect_none failed!");
						}
						for (int r = 0; r < rows; ++r) {
							if (H5Sselect_elements(rspace, H5S_SELECT_APPEND, 1, (hsize_t*) (arr_ptr + r*rank)) < 0) {
								throw gcnew HDF5Exception("H5Sselect_elements failed!");
							}
						}
					}
					else if (arr->GetLength(1) == 2*rank)
					{
						if (H5Sselect_none(rspace) < 0) {
							throw gcnew HDF5Exception("H5Sselect_none failed!");
						}
					
						array<hsize_t>^ count = gcnew array<hsize_t>(rank);
						for (int k = 0; k < rank; ++k) {
							count[k] = 1;
						}
						pin_ptr<hsize_t> count_ptr = &count[0];

						array<hsize_t>^ block = gcnew array<hsize_t>(rank);
						pin_ptr<hsize_t> block_ptr = &block[0];

						for (int r = 0; r < rows; ++r)
						{
							for (int c = 0; c < rank; ++c) {
								block[c] = arr[r, c+rank] - arr[r,c] + 1;
								if (block[c] < 1) {
									throw gcnew PSH5XException("Block dimensions must be positive!");
								}
							}
							if (H5Sselect_hyperslab(rspace, H5S_SELECT_OR, (hsize_t*) (arr_ptr + r*2*rank), NULL, count_ptr, block_ptr) < 0) {
								throw gcnew HDF5Exception("H5Sselect_hyperslab failed!");
							}
						}
					}
					else {
						throw gcnew PSH5XException("Unexpected rank in selection!");
					}

					if (H5Rcreate(ptr+i*rsize, m_h5file, (char*) path_str.ToPointer(), H5R_DATASET_REGION , rspace) < 0) {
						throw gcnew HDF5Exception("H5Rcreate failed!");
					}

					H5Sclose(rspace);
					rspace = -1;
					H5Dclose(robj);
					robj = -1;
					Marshal::FreeHGlobal(path_str);

					ienum->MoveNext();
				}

#pragma endregion
			}
			else {
				throw gcnew PSH5XException("Unknown reference type found!");
			}

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
			if (rspace >= 0) {
				H5Sclose(rspace);
			}
			if (robj >= 0) {
				H5Dclose(robj);
			}
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
}