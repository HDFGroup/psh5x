#pragma once

#include "DriveInfo.h"
#include "DatasetInfo.h"
#include "DatasetInfoLite.h"

extern "C" {
#include "H5public.h"
#include "H5Ipublic.h"
}

namespace PSH5X
{
    ref class ProviderUtils
    {
    public:

#pragma region Properties

        property static System::String^ HDF5_PATH_SEP
        {
            System::String^ get() { return m_hdf5_path_sep; }
        }

        property static System::String^ HDF5_DRIVE_SEP
        {
            System::String^ get() { return m_hdf5_drive_sep; }
        }

        property static System::String^ HDF5_PROVIDER_SEP
        {
            System::String^ get() { return m_hdf5_provider_sep; }
        }

        property static array<System::Char>^ DriveSeparator
        {
            array<wchar_t>^ get() { return m_drive_sep; }
        }

        property static System::Collections::Hashtable^ BitfieldH5Types
        {
            System::Collections::Hashtable^ get() { return m_bitfield_types; }
        }

        property static System::Collections::Hashtable^ FloatH5Types
        {
            System::Collections::Hashtable^ get() { return m_float_types; }
        }

        property static System::Collections::Hashtable^ IntegerH5Types
        {
            System::Collections::Hashtable^ get() { return m_integer_types; }
        }

        property static System::Collections::Hashtable^ NativeH5Types
        {
            System::Collections::Hashtable^ get() { return m_native_types; }
        }

        property static System::Collections::Hashtable^ PredefinedH5Types
        {
            System::Collections::Hashtable^ get() { return m_predefined_types; }
        }

        property static System::Collections::Hashtable^ ReferenceH5Types
        {
            System::Collections::Hashtable^ get() { return m_reference_types; }
        }

        property static System::Collections::Hashtable^ StandardH5Types
        {
            System::Collections::Hashtable^ get() { return m_standard_types; }
        }

#pragma endregion

#pragma region PS and HDF5 path names

        static DriveInfo^ GetDriveFromPath(System::String^ path,
            System::Management::Automation::ProviderInfo^ provider);
        
        static System::String^ NormalizePath(System::String^ path);

        static System::String^ PathNoDrive(System::String^ path);

        static bool TryGetDriveEtH5Path(System::String^ path,
            System::Management::Automation::ProviderInfo^ provider, DriveInfo^% drive,
            System::String^% h5path);

        static bool IsWellFormedH5Path(System::String^ h5path);
        
        // path is in use, but may end in a dangling link

        static bool IsValidH5Path(hid_t loc, System::String^ h5path);

		// path can be resolved to an HDF5 object

		static bool IsResolvableH5Path(hid_t loc, System::String^ h5path);

        static bool CanCreateItemAt(hid_t loc, System::String^ h5path);

        static bool CanForceCreateItemAt(hid_t loc, System::String^ h5path);

        static System::String^ ParentPath(System::String^ h5path);

        static System::String^ ChildName(System::String^ h5path);
        
#pragma endregion

        static System::DateTime^ UnixTime2DateTime(time_t secondsSinceJan1);

        static bool IsH5Object(hid_t file, System::String^ h5path);

        static bool IsH5Group(hid_t file, System::String^ h5path);

        static bool IsH5Dataset(hid_t file, System::String^ h5path);

        static bool IsH5ChunkedDataset(hid_t file, System::String^ h5path);

        static bool IsH5DatatypeObject(hid_t file, System::String^ h5path);

        static bool IsH5SymLink(hid_t file, System::String^ h5path);

        static bool IsH5SoftLink(hid_t file, System::String^ h5path);

        static bool IsH5ExternalLink(hid_t file, System::String^ h5path);

		static bool IsH5PacketTable(hid_t file, System::String^ h5path);

        static array<System::String^>^ GetObjectAttributeNames(hid_t obj_id);

        static array<System::String^>^ GetGroupLinkNames(hid_t group_id, bool recurse);

		static System::Object^ GetObjectRep(
			hid_t oid,
			System::Collections::Hashtable^ dropItem,
			bool detailed,
			bool% isContainer
			);

#pragma region HDF5 Dataset

        static DatasetInfoLite^ GetDatasetInfoLite(hid_t obj_id);

        static DatasetInfo^ GetDatasetInfo(hid_t obj_id);

#pragma endregion

#pragma region HDF5 Datatype

        static System::Collections::Generic::Dictionary<System::String^,System::Object^>^ ParseH5Type(hid_t type);

		static hid_t ParseH5Type(System::Collections::Generic::Dictionary<System::String^, System::Object^>^ dict);

        static hid_t ParseH5Type(System::Object^ obj);

        static hid_t H5Type(System::String^ type);

        static System::String^ H5NativeType2String(hid_t type_id);

        static System::Type^ ProviderUtils::H5Type2DotNet(hid_t type_id);

        static __wchar_t ProviderUtils::TypeCode(System::Type^ t, H5T_class_t cls);

        static hid_t ProviderUtils::DotNetType2H5Native(System::Type^ ntype, bool isBitfield);

        static System::Type^ GetArrayType(hid_t type_id);

		static System::Array^ ProviderUtils::GetArray(System::IO::BinaryReader^% reader, array<hsize_t>^ dims, hid_t type_id);

		static System::Array^ ProviderUtils::GetArray(void* buffer, array<hsize_t>^ dims, hid_t type_id);

        static System::Array^ ProviderUtils::GetArray(void* buffer, size_t nelems, hid_t type_id);

		static System::Reflection::MethodInfo^ BinaryReaderMethod(hid_t type_id);

        static System::Type^ GetCompundDotNetType(hid_t type_id);

        static bool IsH5SimpleType(hid_t dtype);

		static hid_t GetH5MemoryType(System::Type^ t, hid_t ftype);

		static bool TryGetFillValue(hid_t dtype, System::Object^ value, array<unsigned char>^% fill);

		static bool TryGetFillValue(hid_t dtype, array<unsigned char>^ fill, System::Object^% value);

#pragma endregion

#pragma region HDF5 Dataspace

		static bool WriterCheckSelection(hid_t% fspace, hid_t% mspace, hsize_t sel_count,
			System::Management::Automation::RuntimeDefinedParameterDictionary^ dict);

		static System::String^ GetObjectReference(hid_t dset, void* ref);

		static System::Collections::Generic::KeyValuePair<System::String^, System::Array^>
			GetRegionReference(hid_t dset, void* ref);

#pragma endregion

#pragma region HDF5 Attribute

        static System::Collections::Hashtable^ H5Attribute(hid_t attr_id, System::String^ attributeName, hid_t file);

		static void SetH5AttributeValue(hid_t attr_id, System::Object^ value, hid_t file);

		static bool AttributeSizeOK(hid_t obj_id, hid_t type_id);

#pragma endregion

#pragma region General Utilities

        static bool ResolveItemType(System::String^ str, System::String^% itemType);

		static System::Object^ GetDotNetObject(System::Object^ obj);

		static bool TryGetValue(System::Object^ obj, char& i);

		static bool TryGetValue(System::Object^ obj, unsigned char& i);

		static bool TryGetValue(System::Object^ obj, short& i);

		static bool TryGetValue(System::Object^ obj, unsigned short& i);

        static bool TryGetValue(System::Object^ obj, int& i);

		static bool TryGetValue(System::Object^ obj, unsigned int& i);

		static bool TryGetValue(System::Object^ obj, long long& i);

		static bool TryGetValue(System::Object^ obj, unsigned long long& i);

		static bool TryGetValue(System::Object^ obj, float& i);

		static bool TryGetValue(System::Object^ obj, double& i);

        static bool TryGetValue(System::Object^ obj, System::String^% str);

        static bool TryGetValue(System::Object^ obj, System::Collections::Hashtable^% ht);

        static bool TryGetValue(System::Object^ obj, array<hsize_t>^% arr);

        static bool TryGetValue(System::Object^ obj, array<System::String^>^% arr);
		
		static bool TryGetValue(System::Object^ obj, array<System::Object^>^% arr);

		static bool TryGetValue(System::Object^ obj,
			System::Collections::Generic::Dictionary<System::String^, System::Object^>^% dict);

        template<typename T>
        static bool TryGetValue(System::Object^ obj, array<T>^% arr)
        {
            bool result = false;

            Object^ bobj = GetDotNetObject(obj);

            try
            {
                Array^ tmp = safe_cast<Array^>(bobj);
                arr = gcnew array<T>(tmp->Length);

				if (tmp->Rank == 1) {
					Array::Copy(tmp, arr, tmp->Length);
				}
				else // take the slow way home
				{
					IEnumerator^ iter = tmp->GetEnumerator();
					int count = 0;
					while ((iter->MoveNext()) && (iter->Current != nullptr)) {
						arr[count++] = safe_cast<T>(iter->Current);
					}
				}

                result = true;
            }
            catch (...) {}

            return result;
        }

#pragma endregion

    private:

		static array<System::String^>^ GetLinkNames(System::String^ h5path);

        static ProviderUtils();
        
        static System::String^ m_hdf5_path_sep;

        static System::String^ m_hdf5_drive_sep;
        
        static System::String^ m_hdf5_provider_sep;
        
        static array<System::Char>^ m_drive_sep;
        
        static array<System::String^>^ m_type_classes;

        static System::Collections::Hashtable^ m_predefined_types;

        static System::Collections::Hashtable^ m_bitfield_types;

        static System::Collections::Hashtable^ m_float_types;

        static System::Collections::Hashtable^ m_integer_types;

        static System::Collections::Hashtable^ m_native_types;

        static System::Collections::Hashtable^ m_reference_types;

        static System::Collections::Hashtable^ m_standard_types;

        static System::Collections::Hashtable^ m_known_types;

        static System::Collections::Hashtable^ m_item_types;
    };
}