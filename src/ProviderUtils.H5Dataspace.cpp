
#include "HDF5Exception.h"
#include "ProviderUtils.h"
#include "PSH5XException.h"

extern "C" {

#include "H5Ipublic.h"
#include "H5Spublic.h"
#include "H5Rpublic.h"
}

using namespace System;
using namespace System::Collections;
using namespace System::Collections::Generic;
using namespace System::Management::Automation;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
	bool ProviderUtils::WriterCheckSelection
		(
		hid_t% fspace,
		hid_t% mspace,
		hsize_t sel_count,
		RuntimeDefinedParameterDictionary^ dict
		)
	{
		bool sel_flag = false;

		try
		{
			H5S_class_t scls = H5Sget_simple_extent_type(fspace);

			if (scls == H5S_SIMPLE)
			{
				int rank = H5Sget_simple_extent_ndims(fspace);
				if (rank < 0) {
					throw gcnew HDF5Exception("H5Sget_simple_extent_ndims failed!");
				}

				array<hsize_t>^ start = nullptr;
				if (dict["Start"]->Value != nullptr) {
					start = (array<hsize_t>^) dict["Start"]->Value;
					sel_flag = true;
					if (sel_flag) {
						if (rank != start->Length) {
							throw gcnew PSH5XException("The length of the 'Start' array must match the dataset rank!");
						}
					}
				}
				array<hsize_t>^ stride = nullptr;
				if (dict["Stride"]->Value != nullptr) {
					stride = (array<hsize_t>^) dict["Stride"]->Value;
					if (sel_flag) {
						if (rank != stride->Length) {
							throw gcnew PSH5XException("The length of the 'Stride' array must match the dataset rank!");
						}
					}
					else {
						sel_flag = true;
					}
				}
				array<hsize_t>^ count = nullptr;
				if (dict["Count"]->Value != nullptr) {
					count = (array<hsize_t>^) dict["Count"]->Value;
					if (sel_flag) {
						if (rank != count->Length) {
							throw gcnew PSH5XException("The length of the 'Count' array must match the dataset rank!");
						}
					}
					else {
						sel_flag = true;
					}
				}
				array<hsize_t>^ block = nullptr;
				if (dict["Block"]->Value != nullptr) {
					block = (array<hsize_t>^) dict["Block"]->Value;
					if (sel_flag) {
						if (rank != block->Length) {
							throw gcnew PSH5XException("The length of the 'Block' array must match the dataset rank!");
						}
					}
					else {
						sel_flag = true;
					}
				}

				// initialize arrays

				if (sel_flag)
				{
					if (start == nullptr) {
						start = gcnew array<hsize_t>(rank);
					}
					if (stride == nullptr) {
						stride = gcnew array<hsize_t>(rank);
						for (int i = 0; i < rank; ++i) {
							stride[i] = 1;
						}
					}
					if (count == nullptr) {
						count = gcnew array<hsize_t>(rank);
						for (int i = 0; i < rank; ++i) {
							count[i] = 1;
						}
					}
					if (block == nullptr) {
						block = gcnew array<hsize_t>(rank);
						for (int i = 0; i < rank; ++i) {
							block[i] = 1;
						}
					}

					array<hsize_t>^ dims = gcnew array<hsize_t>(rank);
					pin_ptr<hsize_t> dims_ptr = &dims[0];
					rank = H5Sget_simple_extent_dims(fspace, dims_ptr, NULL);
					if (rank < 0) {
						throw gcnew HDF5Exception("H5Sget_simple_extent_dims failed!");
					}

					for (int i = 0; i < rank; ++i)
					{
						if (block[i] < 1) {
							throw gcnew PSH5XException("All block sizes must be positive!");
						}
						if (count[i] < 1) {
							throw gcnew PSH5XException("All block counts must be positive!");
						}
						if (count[i] > 1 && stride[i] < block[i]) {
							throw gcnew PSH5XException("If a block count is greater than 1, then the stride in that dimension must be greater or equal to the block size!");
						}
						if ((start[i] + block[i] + stride[i]*(count[i]-1)) > dims[i]) {
							throw gcnew PSH5XException("The hyperslab extends beyond the limits of the dataspace!");
						}
					}

					pin_ptr<hsize_t> start_ptr = &start[0], stride_ptr = &stride[0], count_ptr = &count[0], block_ptr = &block[0];
					if (H5Sselect_hyperslab(fspace, H5S_SELECT_SET, start_ptr, stride_ptr, count_ptr, block_ptr) < 0) {
						throw gcnew HDF5Exception("H5Sselect_hyperslab failed!");
					}

					hssize_t npoints = H5Sget_select_npoints(fspace);
					if (npoints != sel_count && sel_count != 1) {
						throw gcnew HDF5Exception("Selection count mismatch found!");
					}

					// select the entire content provided

					hsize_t mdims[1];
					mdims[0] = safe_cast<hsize_t>(npoints);

					mspace = H5Screate_simple(1, mdims, NULL);
					if (mspace < 0) {
						throw gcnew HDF5Exception("H5Screate_simple failed!");
					}
					if (H5Sselect_all(mspace) < 0) {
						throw gcnew HDF5Exception("H5Sselect_all failed!");
					}
				}
			}
		}
		finally
		{
		}

		return sel_flag;
	}

	String^ ProviderUtils::GetObjectReference(hid_t dset, void* ref)
	{
		String^ result;

		IntPtr name = Marshal::AllocHGlobal(256);
		ssize_t ret_val = 0;

		try
		{
			ret_val = H5Rget_name(dset, H5R_OBJECT, ref, (char*) name.ToPointer(), 255);
			if (ret_val < 0) {
				throw gcnew HDF5Exception("H5Rget_name failed!");
			}
			if (ret_val > 255)  // name is longer than 255 characters?
			{
				Marshal::FreeHGlobal(name);
				name = Marshal::AllocHGlobal(safe_cast<int>(ret_val+1));

				ret_val = H5Rget_name(dset, H5R_OBJECT, ref, (char*) name.ToPointer(), ret_val);
				if (ret_val < 0) {
					throw gcnew HDF5Exception("H5Rget_name failed!");
				}
			}

			result = Marshal::PtrToStringAnsi(name);
		}
		finally
		{
			Marshal::FreeHGlobal(name);
		}

		return result;
	}

	KeyValuePair<String^,Array^> ProviderUtils::GetRegionReference(hid_t dset, void* ref)
	{
		KeyValuePair<String^,Array^> result;

		hid_t sel = -1;

		IntPtr name = Marshal::AllocHGlobal(256);
		ssize_t ret_val = 0;

		try
		{
			sel = H5Rget_region(dset, H5R_DATASET_REGION, ref);
			if (sel < 0) {
				throw gcnew HDF5Exception("H5Rget_region failed!");
			}
			if (H5Sget_simple_extent_type(sel) != H5S_SIMPLE) {
				throw gcnew PSH5XException("Non-simple selection found!");
			}

			int rank = H5Sget_simple_extent_ndims(sel);
			if (rank <= 0) {
				throw gcnew HDF5Exception("H5Sget_simple_extent_ndims failed!");
			}

			Array^ arr = nullptr;

			array<int>^ result_dims = gcnew array<int>(2); 

			switch(H5Sget_select_type(sel))
			{
			case H5S_SEL_NONE:
				break;
			case H5S_SEL_POINTS:
				{
					hssize_t count = H5Sget_select_elem_npoints(sel);
					if (count > 0)
					{
						result_dims[0] = safe_cast<int>(count);
						result_dims[1] = rank;
						arr = Array::CreateInstance(hsize_t::typeid, result_dims);

						array<hsize_t>^ buf = gcnew array<hsize_t>(safe_cast<int>(count*rank));
						pin_ptr<hsize_t> buf_ptr = &buf[0];
						if (H5Sget_select_elem_pointlist(sel, 0, count, buf_ptr) < 0) {
							throw gcnew HDF5Exception("H5Sget_select_elem_pointlist failed!");
						}

						array<int>^ index = gcnew array<int>(2);

						for (hssize_t i = 0; i < count; ++i) {
							for (int r = 0; r < rank; ++r) {
								index[0] = safe_cast<int>(i);
								index[1] = r;
								arr->SetValue(buf[i*rank + r], index);
							}
						}
					}

					if (count < 0) {
						throw gcnew HDF5Exception("H5Sget_select_elem_npoints failed!");
					}
				}
				break;
			case H5S_SEL_HYPERSLABS:
				{
					hssize_t count =  H5Sget_select_hyper_nblocks(sel);
					if (count > 0)
					{
						result_dims[0] = safe_cast<int>(count);
						result_dims[1] = 2*rank;
						arr = Array::CreateInstance(hsize_t::typeid, result_dims);
						
						array<hsize_t>^ buf = gcnew array<hsize_t>(safe_cast<int>(count*2*rank));
						pin_ptr<hsize_t> buf_ptr = &buf[0];
						if (H5Sget_select_hyper_blocklist(sel, 0, count, buf_ptr) < 0) {
							throw gcnew HDF5Exception("H5Sget_select_hyper_blocklist failed!");
						}

						array<int>^ index = gcnew array<int>(2);

						for (hssize_t i = 0; i < count; ++i) {
							for (int r = 0; r < 2*rank; ++r) {
								index[0] = safe_cast<int>(i);
								index[1] = r;
								arr->SetValue(buf[i*2*rank + r], index);
							}
						}
					}

					if (count < 0) {
						throw gcnew HDF5Exception("H5Sget_select_hyper_nblocks failed!");
					}
				}
				break;
			case H5S_SEL_ALL:
				{
					// return the entire space as two corners (0,..,0) - (dims[0]-1, ..., dims[rank-1]-1)

					array<hsize_t>^ dims = gcnew array<hsize_t>(rank);
					pin_ptr<hsize_t> dims_ptr = &dims[0];
					rank = H5Sget_simple_extent_dims(sel, dims_ptr, NULL);
					if (rank < 0) {
						throw gcnew HDF5Exception("H5Sget_simple_extent_dims failed!");
					}

					result_dims[0] = safe_cast<int>(1);
					result_dims[1] = 2*rank;
					arr = Array::CreateInstance(hsize_t::typeid, result_dims);

					for (int r = 0; r < rank; ++r) {
						arr->SetValue(dims[r]-1, rank+r);
					}
				}
				break;
			}
			
			ret_val = H5Rget_name(dset, H5R_DATASET_REGION, ref, (char*) name.ToPointer(), 255);
			if (ret_val < 0) {
				throw gcnew HDF5Exception("H5Rget_name failed!");
			}
			if (ret_val > 255)  // name is longer than 255 characters?
			{
				Marshal::FreeHGlobal(name);
				name = Marshal::AllocHGlobal(safe_cast<int>(ret_val+1));

				ret_val = H5Rget_name(dset, H5R_OBJECT, ref, (char*) name.ToPointer(), ret_val);
				if (ret_val < 0) {
					throw gcnew HDF5Exception("H5Rget_name failed!");
				}
			}

			result = KeyValuePair<String^,Array^>(Marshal::PtrToStringAnsi(name), arr);
		}
		finally
		{
			Marshal::FreeHGlobal(name);

			if (sel != -1) {
				H5Sclose(sel);
			}
		}

		return result;
	}
}