
#include "ArrayDatasetReaderT.h"
#include "ArrayDatasetWriterT.h"
#include "CompoundDatasetReader.h"
#include "CompoundDatasetWriter.h"
#include "DatasetReaderT.h"
#include "DriveInfo.h"
#include "HDF5Exception.h"
#include "Provider.h"
#include "ProviderUtils.h"
#include "PSH5XException.h"
#include "PrimitiveTypeDatasetWriter.h"
#include "StringDatasetReader.h"
#include "StringDatasetWriter.h"
#include "VlenDatasetReader.h"
#include "VlenDatasetWriterT.h"

extern "C" {
#include "H5public.h"
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
    void Provider::ClearContent(String ^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::ClearContent(Path = '{0}')", path));
        return;
    }

    Object^ Provider::ClearContentDynamicParameters(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::ClearContentDynamicParameters(Path = '{0}')", path));
        return nullptr;
    }

    IContentReader^ Provider::GetContentReader(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::GetContentReader(Path = '{0}')", path));

        IContentReader^ result = nullptr;

        hid_t dset = -1, ftype = -1, base_type = -1, fspace = -1, mspace = H5S_ALL;

        char *name = NULL;

		bool sel_flag = false;

        try
        {
            DriveInfo^ drive = nullptr;
            String^ h5path = nullptr;
            if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path)) {
                throw gcnew PSH5XException("Ill-formed HDF5 path name and/or unable to obtain drive name!");
            }
            if (!ProviderUtils::IsH5Dataset(drive->FileHandle, h5path)) {
                throw gcnew PSH5XException("This is not an HDF5 dataset!");
            }

            name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

            dset = H5Dopen2(drive->FileHandle, name, H5P_DEFAULT);
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
			H5S_class_t scls = H5Sget_simple_extent_type(fspace);

#pragma region check for selections

			if (scls == H5S_SIMPLE)
			{
				RuntimeDefinedParameterDictionary^ dynamicParameters =
					(RuntimeDefinedParameterDictionary^) DynamicParameters;

				int rank = H5Sget_simple_extent_ndims(fspace);
				if (rank < 0) {
					throw gcnew HDF5Exception("H5Sget_simple_extent_ndims failed!");
				}

				array<hsize_t>^ start = nullptr;
				if (dynamicParameters["Start"]->Value != nullptr) {
					start = (array<hsize_t>^) dynamicParameters["Start"]->Value;
					sel_flag = true;
					if (sel_flag) {
						if (rank != start->Length) {
							throw gcnew PSH5XException("The length of the 'Start' array must match the dataset rank!");
						}
					}
				}
				array<hsize_t>^ stride = nullptr;
				if (dynamicParameters["Stride"]->Value != nullptr) {
					stride = (array<hsize_t>^) dynamicParameters["Stride"]->Value;
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
				if (dynamicParameters["Count"]->Value != nullptr) {
					count = (array<hsize_t>^) dynamicParameters["Count"]->Value;
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
				if (dynamicParameters["Block"]->Value != nullptr) {
					block = (array<hsize_t>^) dynamicParameters["Block"]->Value;
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

					for (int i = 0; i < rank; ++i) {
						dims[i] = block[i]*count[i];
					}

					mspace = H5Screate_simple(rank, dims_ptr, NULL);
					if (mspace < 0) {
						throw gcnew HDF5Exception("H5Screate_simple failed!");
					}
				}
			}

#pragma endregion

			if (scls != H5S_NULL)
			{
				Type^ t = nullptr;

				H5T_class_t cls = H5Tget_class(ftype);

				H5T_class_t bcls = H5T_NO_CLASS;
				
				switch (cls)
				{
				case H5T_INTEGER:
				case H5T_ENUM:
				case H5T_BITFIELD:
#pragma region HDF5 integer, enum, bitfield

					t = ProviderUtils::H5Type2DotNet(ftype);
					if (t == SByte::typeid) {
						result = gcnew DatasetReaderT<SByte>(dset, ftype, fspace, mspace);
					}
					else if (t == Int16::typeid) {
						result = gcnew DatasetReaderT<Int16>(dset, ftype, fspace, mspace);
					}
					else if (t == Int32::typeid) {
						result = gcnew DatasetReaderT<Int32>(dset, ftype, fspace, mspace);
					}
					else if (t == Int64::typeid) {
						result = gcnew DatasetReaderT<Int64>(dset, ftype, fspace, mspace);
					}
					else if (t == Byte::typeid) {
						result = gcnew DatasetReaderT<Byte>(dset, ftype, fspace, mspace);
					}
					else if (t == UInt16::typeid) {
						result = gcnew DatasetReaderT<UInt16>(dset, ftype, fspace, mspace);
					}
					else if (t ==  UInt32::typeid) {
						result = gcnew DatasetReaderT<UInt32>(dset, ftype, fspace, mspace);
					}
					else if (t ==  UInt64::typeid) {
						result = gcnew DatasetReaderT<UInt64>(dset, ftype, fspace, mspace);
					}
					else {
						throw gcnew PSH5XException("Unsupported integer, enum, or bitfield type!");
					}

#pragma endregion
					break;

				case H5T_FLOAT:
#pragma region HDF5 float

					t = ProviderUtils::H5Type2DotNet(ftype);
					if (t == Single::typeid) {
						result = gcnew DatasetReaderT<Single>(dset, ftype, fspace, mspace);
					}
					else if (t == Double::typeid) {
						result = gcnew DatasetReaderT<Double>(dset, ftype, fspace, mspace);
					}
					else {
						throw gcnew PSH5XException("Unsupported float type!");
					}

#pragma endregion
					break;

				case H5T_STRING:

					result = gcnew StringDatasetReader(dset, ftype, fspace, mspace);
					break;

				case H5T_COMPOUND:

					result = gcnew CompoundDatasetReader(dset, ftype, fspace, mspace);
					break;

				case H5T_VLEN:

					result = gcnew VlenDatasetReader(dset, ftype, fspace, mspace);
					break;

				case H5T_ARRAY:

					base_type = H5Tget_super(ftype);
					bcls = H5Tget_class(base_type);

					switch (bcls)
					{
					case H5T_INTEGER:
					case H5T_ENUM:
					case H5T_BITFIELD:
#pragma region HDF5 integer, enum, bitfield

						t = ProviderUtils::H5Type2DotNet(base_type);
						if (t == SByte::typeid) {
							result = gcnew ArrayDatasetReaderT<SByte>(dset, ftype, fspace, mspace);
						}
						else if (t == Int16::typeid) {
							result = gcnew ArrayDatasetReaderT<Int16>(dset, ftype, fspace, mspace);
						}
						else if (t ==  Int32::typeid) {
							result = gcnew ArrayDatasetReaderT<Int32>(dset, ftype, fspace, mspace);
						}
						else if (t ==  Int64::typeid) {
							result = gcnew ArrayDatasetReaderT<Int64>(dset, ftype, fspace, mspace);
						}
						else if (t == Byte::typeid) {
							result = gcnew ArrayDatasetReaderT<Byte>(dset, ftype, fspace, mspace);
						}
						else if (t == UInt16::typeid) {
							result = gcnew ArrayDatasetReaderT<UInt16>(dset, ftype, fspace, mspace);
						}
						else if (t ==  UInt32::typeid) {
							result = gcnew ArrayDatasetReaderT<UInt32>(dset, ftype, fspace, mspace);
						}
						else if (t ==  UInt64::typeid) {
							result = gcnew ArrayDatasetReaderT<UInt64>(dset, ftype, fspace, mspace);
						}
						else {
							throw gcnew PSH5XException("Unsupported integer, enum, or bitfield type!");
						}
#pragma endregion
						break;

					case H5T_FLOAT:
#pragma region HDF5 float

						t = ProviderUtils::H5Type2DotNet(base_type);
						if (t == Single::typeid) {
							result = gcnew ArrayDatasetReaderT<Single>(dset, ftype, fspace, mspace);
						}
						else if (t == Double::typeid) {
							result = gcnew ArrayDatasetReaderT<Double>(dset, ftype, fspace, mspace);
						}
						else {
							throw gcnew PSH5XException("Unsupported float type!");
						}

#pragma endregion
						break;

					default:

						throw gcnew PSH5XException("Unsupported base type in array!");
						break;
					}

					break;

				default:

					throw gcnew PSH5XException("Unsupported type class!");
					break;
				}
			}
		}
        finally
        {
			if (mspace != H5S_ALL) {
                H5Sclose(mspace); 
            }
			if (fspace >= 0) {
                H5Sclose(fspace); 
            }
			if (base_type >= 0) {
                H5Tclose(base_type); 
            }
            if (ftype >= 0) {
                H5Tclose(ftype); 
            }
            if (dset >= 0) {
                H5Dclose(dset);
            }
            if (name != NULL) {
                Marshal::FreeHGlobal(IntPtr(name));
            }
        }

        return result;
    }

    Object^ Provider::GetContentReaderDynamicParameters(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::GetContentReaderDynamicParameters(Path = '{0}')", path));

		RuntimeDefinedParameterDictionary^ dict = gcnew RuntimeDefinedParameterDictionary();

		// Start

		ParameterAttribute^ attr1 = gcnew ParameterAttribute();
		attr1->Mandatory = false;
		attr1->ValueFromPipeline = false;

		RuntimeDefinedParameter^ paramStart = gcnew RuntimeDefinedParameter();
		paramStart->Name = "Start";
		paramStart->ParameterType = array<hsize_t>::typeid;
		paramStart->Attributes->Add(attr1);

		dict->Add("Start", paramStart);

		// Stride

		ParameterAttribute^ attr2 = gcnew ParameterAttribute();
		attr2->Mandatory = false;
		attr2->ValueFromPipeline = false;

		RuntimeDefinedParameter^ paramStride = gcnew RuntimeDefinedParameter();
		paramStride->Name = "Stride";
		paramStride->ParameterType = array<hsize_t>::typeid;
		paramStride->Attributes->Add(attr2);

		dict->Add("Stride", paramStride);

		// Count

		ParameterAttribute^ attr3 = gcnew ParameterAttribute();
		attr3->Mandatory = false;
		attr3->ValueFromPipeline = false;

		RuntimeDefinedParameter^ paramCount = gcnew RuntimeDefinedParameter();
		paramCount->Name = "Count";
		paramCount->ParameterType = array<hsize_t>::typeid;
		paramCount->Attributes->Add(attr3);

		dict->Add("Count", paramCount);

		// Block

		ParameterAttribute^ attr4 = gcnew ParameterAttribute();
		attr4->Mandatory = false;
		attr4->ValueFromPipeline = false;

		RuntimeDefinedParameter^ paramBlock = gcnew RuntimeDefinedParameter();
		paramBlock->Name = "Block";
		paramBlock->ParameterType = array<hsize_t>::typeid;
		paramBlock->Attributes->Add(attr4);

		dict->Add("Block", paramBlock);

		// Elements

		ParameterAttribute^ attr5 = gcnew ParameterAttribute();
		attr5->Mandatory = false;
		attr5->ValueFromPipeline = false;

		RuntimeDefinedParameter^ paramElements = gcnew RuntimeDefinedParameter();
		paramElements->Name = "Elements";
		paramElements->ParameterType = array<hsize_t>::typeid;
		paramElements->Attributes->Add(attr5);

		dict->Add("Elements", paramElements);

        return dict;
    }

    IContentWriter^ Provider::GetContentWriter(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::GetContentWriter(Path = '{0}')", path));

        IContentWriter^ result = nullptr;

        hid_t dset = -1, ftype = -1, base_type = -1, fspace = -1;

        char *name = NULL;

		bool sel_flag = false;

        try
        {
            DriveInfo^ drive = nullptr;
            String^ h5path = nullptr;
            if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path)) {
                throw gcnew PSH5XException("Ill-formed HDF5 path name and/or unable to obtain drive name!");
            }
            if (drive->ReadOnly) {
                throw gcnew PSH5XException("The drive is read-only and cannot be modified!");
            }
            if (!ProviderUtils::IsH5Dataset(drive->FileHandle, h5path)) {
                throw gcnew PSH5XException("This is not an HDF5 dataset!");
            }

            name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

            dset = H5Dopen2(drive->FileHandle, name, H5P_DEFAULT);
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

			Type^ t = nullptr;

			H5T_class_t cls = H5Tget_class(ftype);

			H5T_class_t bcls = H5T_NO_CLASS;

			switch (cls)
			{
			case H5T_INTEGER:
			case H5T_FLOAT:
			case H5T_BITFIELD:
			case H5T_ENUM:

				result = gcnew PrimitiveTypeDatasetWriter(drive->FileHandle, h5path,
					(RuntimeDefinedParameterDictionary^) DynamicParameters);
				break;

			case H5T_STRING:

				result = gcnew StringDatasetWriter(drive->FileHandle, h5path);
				break;

			case H5T_COMPOUND:

				result = gcnew CompoundDatasetWriter(drive->FileHandle, h5path);
				break;

			case H5T_VLEN:

				base_type = H5Tget_super(ftype);
				bcls = H5Tget_class(base_type);

				switch (bcls)
				{
				case H5T_INTEGER:
				case H5T_ENUM:
				case H5T_BITFIELD:
#pragma region HDF5 integer, enum, bitfield
					{
						t = ProviderUtils::H5Type2DotNet(base_type);
						if (t == SByte::typeid) {
							result = gcnew VlenDatasetWriterT<SByte>(drive->FileHandle, h5path);
						}
						else if (t == Int16::typeid) {
							result = gcnew VlenDatasetWriterT<Int16>(drive->FileHandle, h5path);
						}
						else if (t == Int32::typeid) {
							result = gcnew VlenDatasetWriterT<Int32>(drive->FileHandle, h5path);
						}
						else if (t == Int64::typeid) {
							result = gcnew VlenDatasetWriterT<Int64>(drive->FileHandle, h5path);
						}
						else if (t == Byte::typeid) {
							result = gcnew VlenDatasetWriterT<Byte>(drive->FileHandle, h5path);
						}
						else if (t == UInt16::typeid) {
							result = gcnew VlenDatasetWriterT<UInt16>(drive->FileHandle, h5path);
						}
						else if (t ==  UInt32::typeid) {
							result = gcnew VlenDatasetWriterT<UInt32>(drive->FileHandle, h5path);
						}
						else if (t ==  UInt64::typeid) {
							result = gcnew VlenDatasetWriterT<UInt64>(drive->FileHandle, h5path);
						}
						else {
							throw gcnew PSH5XException("Unsupported integer, enum, or bitfield type!");
						}
					}
#pragma endregion
					break;

				case H5T_FLOAT:
#pragma region HDF5 float

					t = ProviderUtils::H5Type2DotNet(base_type);
					if (t == Single::typeid) {
						result = gcnew VlenDatasetWriterT<Single>(drive->FileHandle, h5path);
					}
					else if (t == Double::typeid) {
						result = gcnew VlenDatasetWriterT<Double>(drive->FileHandle, h5path);
					}
					else {
						throw gcnew PSH5XException("Unsupported float type!");
					}

#pragma endregion
					break;

				default:

					throw gcnew PSH5XException("Unsupported base type in vlen!");
					break;
				}

				break;

			case H5T_ARRAY:

				base_type = H5Tget_super(ftype);
				bcls = H5Tget_class(base_type);

				switch (bcls)
				{
				case H5T_INTEGER:
				case H5T_ENUM:
				case H5T_BITFIELD:
#pragma region HDF5 integer, enum, bitfield
					{
						t = ProviderUtils::H5Type2DotNet(base_type);
						if (t == SByte::typeid) {
							result = gcnew ArrayDatasetWriterT<SByte>(drive->FileHandle, h5path);
						}
						else if (t == Int16::typeid) {
							result = gcnew ArrayDatasetWriterT<Int16>(drive->FileHandle, h5path);
						}
						else if (t == Int32::typeid) {
							result = gcnew ArrayDatasetWriterT<Int32>(drive->FileHandle, h5path);
						}
						else if (t == Int64::typeid) {
							result = gcnew ArrayDatasetWriterT<Int64>(drive->FileHandle, h5path);
						}
						else if (t == Byte::typeid) {
							result = gcnew ArrayDatasetWriterT<Byte>(drive->FileHandle, h5path);
						}
						else if (t == UInt16::typeid) {
							result = gcnew ArrayDatasetWriterT<UInt16>(drive->FileHandle, h5path);
						}
						else if (t ==  UInt32::typeid) {
							result = gcnew ArrayDatasetWriterT<UInt32>(drive->FileHandle, h5path);
						}
						else if (t ==  UInt64::typeid) {
							result = gcnew ArrayDatasetWriterT<UInt64>(drive->FileHandle, h5path);
						}
						else {
							throw gcnew PSH5XException("Unsupported integer, enum, or bitfield type!");
						}
					}
#pragma endregion
					break;

				case H5T_FLOAT:
#pragma region HDF5 float
					{
						t = ProviderUtils::H5Type2DotNet(base_type);
						if (t == Single::typeid) {
							result = gcnew ArrayDatasetWriterT<Single>(drive->FileHandle, h5path);
						}
						else if (t == Double::typeid) {
							result = gcnew ArrayDatasetWriterT<Double>(drive->FileHandle, h5path);
						}
						else {
							throw gcnew PSH5XException("Unsupported float type!");
						}
					}
#pragma endregion
					break;

				default:

					throw gcnew PSH5XException("Unsupported base type in array!");
					break;
				}

				break;

			default:

				throw gcnew PSH5XException("Unsupported type class!");
				break;
			}
        }
        finally
        {
			if (fspace >= 0) {
                H5Sclose(fspace); 
            }
			if (base_type >= 0) {
                H5Tclose(base_type); 
            }
            if (ftype >= 0) {
                H5Tclose(ftype); 
            }
            if (dset >= 0) {
                H5Dclose(dset);
            }
            if (name != NULL) {
                Marshal::FreeHGlobal(IntPtr(name));
            }
        }

        return result;
    }

    Object^ Provider::GetContentWriterDynamicParameters(String^ path)
    {
        WriteVerbose(String::Format("HDF5Provider::GetContentWriterDynamicParameters(Path = '{0}')", path));
       
		RuntimeDefinedParameterDictionary^ dict = gcnew RuntimeDefinedParameterDictionary();

		// Start

		ParameterAttribute^ attr1 = gcnew ParameterAttribute();
		attr1->Mandatory = false;
		attr1->ValueFromPipeline = false;

		RuntimeDefinedParameter^ paramStart = gcnew RuntimeDefinedParameter();
		paramStart->Name = "Start";
		paramStart->ParameterType = array<hsize_t>::typeid;
		paramStart->Attributes->Add(attr1);

		dict->Add("Start", paramStart);

		// Stride

		ParameterAttribute^ attr2 = gcnew ParameterAttribute();
		attr2->Mandatory = false;
		attr2->ValueFromPipeline = false;

		RuntimeDefinedParameter^ paramStride = gcnew RuntimeDefinedParameter();
		paramStride->Name = "Stride";
		paramStride->ParameterType = array<hsize_t>::typeid;
		paramStride->Attributes->Add(attr2);

		dict->Add("Stride", paramStride);

		// Count

		ParameterAttribute^ attr3 = gcnew ParameterAttribute();
		attr3->Mandatory = false;
		attr3->ValueFromPipeline = false;

		RuntimeDefinedParameter^ paramCount = gcnew RuntimeDefinedParameter();
		paramCount->Name = "Count";
		paramCount->ParameterType = array<hsize_t>::typeid;
		paramCount->Attributes->Add(attr3);

		dict->Add("Count", paramCount);

		// Block

		ParameterAttribute^ attr4 = gcnew ParameterAttribute();
		attr4->Mandatory = false;
		attr4->ValueFromPipeline = false;

		RuntimeDefinedParameter^ paramBlock = gcnew RuntimeDefinedParameter();
		paramBlock->Name = "Block";
		paramBlock->ParameterType = array<hsize_t>::typeid;
		paramBlock->Attributes->Add(attr4);

		dict->Add("Block", paramBlock);

		// Elements

		ParameterAttribute^ attr5 = gcnew ParameterAttribute();
		attr5->Mandatory = false;
		attr5->ValueFromPipeline = false;

		RuntimeDefinedParameter^ paramElements = gcnew RuntimeDefinedParameter();
		paramElements->Name = "Elements";
		paramElements->ParameterType = array<hsize_t>::typeid;
		paramElements->Attributes->Add(attr5);

		dict->Add("Elements", paramElements);

        return dict;
    }
}