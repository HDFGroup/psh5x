
#include "AttributeInfo.h"
#include "HDF5Exception.h"
#include "Provider.h"
#include "ProviderUtils.h"
#include "PSH5XException.h"

extern "C" {
#include "H5Gpublic.h"
#include "H5Spublic.h"
#include "H5Tpublic.h"
}

#include <cstdlib>

using namespace System;
using namespace System::Collections;
using namespace System::Collections::Generic;
using namespace System::Collections::ObjectModel;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Provider;
using namespace System::Runtime::InteropServices;
using namespace System::Web::Script::Serialization;

namespace PSH5X
{
	void Provider::NewProperty(String^ path, String^ propertyName, String^ propertyTypeName,
		Object^ value)
	{
		WriteVerbose(
			String::Format("HDF5Provider::NewProperty(Path = '{0}',PropertyName='{1}',propertyTypeName = '{2}')",
			path, propertyName, propertyTypeName));

		hid_t ftype = -1, fspace = -1, oid = -1, aid = -1;

		char *path_str = NULL, *attr_name = NULL, *obj_path = NULL;

		hsize_t* current_size = NULL;

		try
		{

#pragma region sanity check

			DriveInfo^ drive = nullptr;
			String^ h5path = nullptr;
			if (!ProviderUtils::TryGetDriveEtH5Path(path, ProviderInfo, drive, h5path))
			{
				throw gcnew PSH5XException("Ill-formed HDF5 path name and/or unable to obtain drive name!");
			}

			if (drive->ReadOnly)
			{
				throw gcnew PSH5XException("The drive is read-only and cannot be modified!");
			}

			if (!ProviderUtils::IsResolvableH5Path(drive->FileHandle, h5path))
			{
				throw gcnew PSH5XException(String::Format("HDF5 object '{0}' does not exist", path));
			}

#pragma endregion

			attr_name = (char*)(Marshal::StringToHGlobalAnsi(propertyName)).ToPointer();
			obj_path = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

			RuntimeDefinedParameterDictionary^ dynamicParameters =
				(RuntimeDefinedParameterDictionary^) DynamicParameters;

#pragma region mandatory parameters

			String^ elemType = (String^) dynamicParameters["ElementType"]->Value;

			if (!elemType->Trim()->StartsWith("{")) // not JSON, alias or predefined type
			{
				if (elemType->StartsWith("/")) {
					if (ProviderUtils::IsH5DatatypeObject(drive->FileHandle, elemType))
					{
						path_str = (char*)(Marshal::StringToHGlobalAnsi(elemType)).ToPointer();
						ftype = H5Topen2(drive->FileHandle, path_str, H5P_DEFAULT);
						if (ftype < 0) {
							throw gcnew HDF5Exception("H5Topen2 failed!!!");
						}
					}
					else {
						throw gcnew PSH5XException("The HDF5 path name specified does not refer to an datatype object.");
					}
				}
				else
				{
					ftype = ProviderUtils::H5Type(elemType);
					if (ftype < 0) {
						throw gcnew PSH5XException("Invalid HDF5 datatype specified!");
					}
				}
			}
			else
			{
				JavaScriptSerializer^ serializer = gcnew JavaScriptSerializer();
				Dictionary<String^,Object^>^ type = nullptr;
				try
				{
					type = safe_cast<Dictionary<String^,Object^>^>(serializer->DeserializeObject(elemType));

				}
				catch (...)
				{
					throw gcnew PSH5XException("JSON format error.");
				}

				ftype = ProviderUtils::ParseH5Type(type);
				if (ftype < 0) {
					throw gcnew PSH5XException("Invalid HDF5 datatype specified!");
				}
			}

#pragma endregion

#pragma region optional parameters

			String^ shape = "scalar";

			bool isNull = (dynamicParameters["Null"]->IsSet);
			bool isSimple = (dynamicParameters["Simple"]->Value != nullptr);

			if (isNull && isSimple) {
				throw gcnew PSH5XException("The -Null and -Simple options are incompatible. Choose one!");
			}
			if (isNull)   { shape = "Null";   }
			if (isSimple) { shape = "Simple"; }

			oid = H5Oopen(drive->FileHandle, obj_path, H5P_DEFAULT);
			if (oid < 0) {
				throw gcnew HDF5Exception("H5Oopen failed!");
			}

			if (H5Aexists(oid, attr_name) > 0) {
				throw gcnew PSH5XException(String::Format(
					"Item '{0}' already has a property named '{1}'", path, propertyName));
			}

			if (!ProviderUtils::AttributeSizeOK(oid, ftype)) {
				throw gcnew PSH5XException("Attribute too large! (Max. 64K in compact storage)");
			}

			if (shape->ToUpper() == "NULL")
			{
				fspace = H5Screate(H5S_NULL);
				if (fspace < 0) {
					throw gcnew HDF5Exception("H5Screate failed!");
				}
			}
			else if (shape->ToUpper() == "SCALAR")
			{
				fspace = H5Screate(H5S_SCALAR);
				if (fspace < 0) {
					throw gcnew HDF5Exception("H5Screate failed!");
				}
			}
			else if (shape->ToUpper() == "SIMPLE")
			{
				fspace = H5Screate(H5S_SIMPLE);
				if (fspace < 0) {
					throw gcnew Exception("H5Screate failed!");
				}

				array<hsize_t>^ dims = (array<hsize_t>^) dynamicParameters["Simple"]->Value;

				int rank = dims->Length;
				current_size = new hsize_t [rank];
				for (int i = 0; i < rank; ++i) { current_size[i] = dims[i]; }

				if (H5Sset_extent_simple(fspace, rank, current_size, current_size) < 0) {
					throw gcnew HDF5Exception("H5Sset_extent_simple failed!");
				}
			}
			else {
				throw gcnew PSH5XException("Invalid shape!");
			}

			bool setValue = false;
			Object^ attrValue = nullptr;
			if (value != nullptr)
			{
				attrValue = ProviderUtils::GetDotNetObject(value);
				setValue = true;
			}

#pragma endregion

			if (this->ShouldProcess(h5path,
				String::Format("HDF5 attribute '{0}' does not exist, create it",
				propertyName)))
			{
				aid = H5Acreate2(oid, attr_name, ftype, fspace, H5P_DEFAULT, H5P_DEFAULT);
				if (aid < 0) {
					throw gcnew HDF5Exception("H5Acreate2 failed!");
				}

				if (setValue)
				{
					if (shape->ToUpper() == "SIMPLE") {
						ProviderUtils::SetH5AttributeValue(aid, attrValue);
					}
					else if (shape->ToUpper() == "SCALAR") {
						ProviderUtils::SetScalarH5AttributeValue(aid, attrValue);
					}
				}

				if (H5Fflush(aid, H5F_SCOPE_LOCAL) < 0) {
					throw gcnew HDF5Exception("H5Fflush failed!");
				}

				// no -PassThru
				// Hashtable^ ht = ProviderUtils::H5Attribute(aid, propertyName);
				// WritePropertyObject(ht, path);
			}
		}
		finally
		{
			if (aid >= 0) {
				H5Aclose(aid);
			}

			if (current_size != NULL) {
				delete [] current_size;
			}

			if (fspace >= 0) {
				H5Sclose(fspace);
			}

			if (oid >= 0) {
				H5Oclose(oid);
			}

			if (ftype >= 0) {
				H5Tclose(ftype);
			}

			if (path_str != NULL) {
				Marshal::FreeHGlobal(IntPtr(path_str));
			}

			if (attr_name != NULL) {
				Marshal::FreeHGlobal(IntPtr(attr_name));
			}

			if (obj_path != NULL) {
				Marshal::FreeHGlobal(IntPtr(obj_path));
			}
		}

		return;
	}

	Object^ Provider::NewPropertyDynamicParameters(String^ path, String^ propertyName,
		String^ propertyTypeName, Object^ value)
	{
		WriteVerbose(
			String::Format("HDF5Provider::NewPropertyDynamicParameters(Path = '{0}', PropertyName='{1}',propertyTypeName = '{2}')",
			path, propertyName, propertyTypeName));

		RuntimeDefinedParameterDictionary^ dict = gcnew RuntimeDefinedParameterDictionary();

		// ElementType

		ParameterAttribute^ attr1 = gcnew ParameterAttribute();
		attr1->Mandatory = true;
		attr1->ValueFromPipeline = false;

		RuntimeDefinedParameter^ paramElementType = gcnew RuntimeDefinedParameter();
		paramElementType->Name = "ElementType";
		// can be String or Hashtable
		paramElementType->ParameterType = String::typeid;
		paramElementType->Attributes->Add(attr1);

		dict->Add("ElementType", paramElementType);

		// Simple

		ParameterAttribute^ attr2 = gcnew ParameterAttribute();
		attr2->Mandatory = false;
		attr2->ValueFromPipeline = false;

		RuntimeDefinedParameter^ paramSimple = gcnew RuntimeDefinedParameter();
		paramSimple->Name = "Simple";
		paramSimple->ParameterType = array<hsize_t>::typeid;
		paramSimple->Attributes->Add(attr2);

		dict->Add("Simple", paramSimple);

		// Null

		ParameterAttribute^ attr3 = gcnew ParameterAttribute();
		attr3->Mandatory = false;
		attr3->ValueFromPipeline = false;

		RuntimeDefinedParameter^ paramNull = gcnew RuntimeDefinedParameter();
		paramNull->Name = "Null";
		paramNull->ParameterType = SwitchParameter::typeid;
		paramNull->Attributes->Add(attr3);

		dict->Add("Null", paramNull);

		return dict;
	}

}