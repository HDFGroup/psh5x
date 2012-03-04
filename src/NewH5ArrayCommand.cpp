

#include "NewH5ArrayCommand.h"
#include "ProviderUtils.h"
#include "PSH5XException.h"

extern "C" {
#include "H5Tpublic.h"
}

using namespace System;
using namespace System::Collections;
using namespace System::Collections::Generic;
using namespace System::Collections::ObjectModel;
using namespace System::IO;
using namespace System::Management::Automation;
using namespace System::Reflection;
using namespace System::Runtime::InteropServices;
using namespace System::Web::Script::Serialization;

namespace PSH5X
{
    void NewH5ArrayCommand::BeginProcessing()
    {
        if (m_length <= 0) {
            throw gcnew ArgumentException("Length must be positive!!!");
        }

        return;
    }

    void NewH5ArrayCommand::ProcessRecord()
    {
        hid_t dtype = -1, ntype = -1, base_type = -1;
        Array^ result = nullptr;

        array<long long>^ dims = gcnew array<long long>(1);
        dims[0] = m_length;

		try
		{
			if (!m_type->Trim()->StartsWith("{")) // not JSON, alias or predefined type
			{

				dtype = ProviderUtils::H5Type(m_type);
				if (dtype < 0) {
					throw gcnew PSH5XException("Invalid HDF5 datatype specified!");
				}
			}
			else
			{
				JavaScriptSerializer^ serializer = gcnew JavaScriptSerializer();
				Dictionary<String^,Object^>^ type = nullptr;
				try
				{
					type = safe_cast<Dictionary<String^,Object^>^>(serializer->DeserializeObject(m_type));

				}
				catch (...)
				{
					throw gcnew PSH5XException("JSON format error.");
				}

				dtype = ProviderUtils::ParseH5Type(type);
				if (dtype < 0) {
					throw gcnew PSH5XException("Invalid HDF5 datatype specified!");
				}
			}

			if (dtype != -1)
			{
				if (ProviderUtils::IsH5SimpleType(dtype))
				{
					System::Type^ t = ProviderUtils::H5Type2DotNet(dtype);
					result = Array::CreateInstance(t, dims); 
				}
				else
				{
					ntype = H5Tget_native_type(dtype, H5T_DIR_ASCEND);

					if (ProviderUtils::IsH5ArrayType(ntype))
					{
#pragma region HDF5 array type

						base_type = H5Tget_super(ntype);
						System::Type^ t = ProviderUtils::H5Type2DotNet(base_type);

						if (t != nullptr)
						{
							int rank = H5Tget_array_ndims(ntype);
							array<hsize_t>^ adims = gcnew array<hsize_t>(rank);
                            pin_ptr<hsize_t> adims_ptr = &adims[0];
							rank = H5Tget_array_dims2(ntype, adims_ptr);
							array<int>^ dims = gcnew array<int>(rank);
							for (int i = 0; i < rank; ++i) {
								dims[i] = safe_cast<int>(adims[i]);
							}
							Array^ dummy = Array::CreateInstance(t, dims);
							result = Array::CreateInstance(dummy->GetType(), m_length);

							for (long long i = 0; i < m_length; ++i) {
								result->SetValue(Array::CreateInstance(t, dims), i);
							}
						}
						else {
							throw gcnew PSH5XException("Unsupported base datatype for ARRAY!");
						}

#pragma endregion
					}
					else if (ProviderUtils::IsH5CompoundType(ntype))
					{
#pragma region HDF5 compound type

						System::Type^ t = ProviderUtils::GetCompundDotNetType(ntype);
						if (t != nullptr)
						{
							result = Array::CreateInstance(t, m_length);

							array<ConstructorInfo^>^ cinfo = t->GetConstructors();
							if (cinfo->Length != 2) {
								throw gcnew PSH5XException("Constructor code generation error!");
							}

							for (long long i = 0; i < m_length; ++i) {
								result->SetValue(cinfo[0]->Invoke(nullptr), i);
							}
						}
						else {
							throw gcnew PSH5XException("Unsupported member datatype in COMPOUND!");
						}

#pragma endregion
					}
					else if (ProviderUtils::IsH5VlenType(ntype))
					{
#pragma region HDF5 variable-length array type

						base_type = H5Tget_super(ntype);
						System::Type^ t = ProviderUtils::H5Type2DotNet(base_type);

						if (t != nullptr)
						{
							Array^ dummy = Array::CreateInstance(t, 0);
							result = Array::CreateInstance(dummy->GetType(), m_length);
							for (long long i = 0; i < m_length; ++i) {
								result->SetValue(Array::CreateInstance(t, 0), i);
							}
						}
						else {
							throw gcnew PSH5XException("Unsupported base datatype for VLEN!");
						}

#pragma endregion
					}
					else {
						throw gcnew PSH5XException("Unrecognized type!!!");
					}
				}
			}
			else {
				throw gcnew PSH5XException("Internal error.");
			}
		}
		finally
		{
			if (base_type >= 0) {
				H5Tclose(base_type);
			}

			if (ntype >= 0) {
				H5Tclose(ntype);
			}

			if (dtype >= 0) {
				H5Tclose(dtype);
			}
		}

        WriteObject(result);
        return;
    }
}