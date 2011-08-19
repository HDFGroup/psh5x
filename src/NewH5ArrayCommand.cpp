
#include "ProviderUtils.h"
#include "NewH5ArrayCommand.h"

extern "C" {
#include "H5Tpublic.h"
}

using namespace System;
using namespace System::Collections;
using namespace System::Collections::ObjectModel;
using namespace System::IO;
using namespace System::Management::Automation;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
    void NewH5ArrayCommand::BeginProcessing()
    {
        if (m_length <= 0)
        {
            throw gcnew ArgumentException("Length must be positive!!!");
        }

        if (ProviderUtils::TryGetValue(m_obj, m_ht))
        {
            m_is_ht = true;
        }
        else if (ProviderUtils::TryGetValue(m_obj, m_str))
        {
            m_is_str = true;
        }
        else {
            throw gcnew ArgumentException(
                "The type information provided cannot be parsed!!!");
        }

        return;
    }

    void NewH5ArrayCommand::ProcessRecord()
    {
        Exception^ ex = nullptr;

        hid_t dtype = -1, ntype = -1, base_type = -1;
        hsize_t* adims = NULL;

        Array^ result = nullptr;

        array<long long>^ dims = gcnew array<long long>(1);
        dims[0] = m_length;

        dtype = ProviderUtils::ParseH5Type(m_obj);
        if (dtype != -1)
        {
            if (m_is_str)
            {
                System::Type^ t = ProviderUtils::H5Type2DotNet(dtype);
                result = Array::CreateInstance(t, dims); 
            }
            else if (m_is_ht)
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
                        adims = new hsize_t [rank];
                        rank = H5Tget_array_dims2(ntype, adims);
                        int size = 1;
                        for (int i = 0; i < rank; ++i)
                        {
                            size *= safe_cast<int>(adims[i]);
                        }

                        if (size > 0)
                        {
                            result = Array::CreateInstance(t, m_length*size);
                        }
                        else { 
                            ex = gcnew ArgumentException("Invalid dimensions for ARRAY!");
                            goto error;
                        }
                    }
                    else {
                        ex = gcnew ArgumentException("Unsupported base datatype for ARRAY!");
                        goto error;
                    }

#pragma endregion
                }
                else if (ProviderUtils::IsH5CompoundType(ntype))
                {
#pragma region HDF5 compound type

                int mcount = H5Tget_nmembers(ntype);
                array<String^>^ mname = gcnew array<String^>(mcount);
                array<System::Type^>^ mtype = gcnew array<System::Type^>(mcount);
                
                for (int i = 0; i < mcount; ++i)
                {
                    char* name = H5Tget_member_name(ntype, safe_cast<unsigned>(i));
                    mname[i] = gcnew String(name);

                    dtype = H5Tget_member_type(ntype, safe_cast<unsigned>(i));
                    mtype[i] = ProviderUtils::H5Type2DotNet(dtype);
                    if (mtype[i] == nullptr) {
                        ex = gcnew ArgumentException("Unsupported member datatype in COMPOUND!");
                        goto error;
                    }
                }

                result = ProviderUtils::GetPSObjectArray(m_length, mname, mtype);


#pragma endregion
                }
            }
            else {
                ex = gcnew ArgumentException("Internal error!!!");
                goto error;
            }
        }
        else {
            ex = gcnew ArgumentException("Unrecognized type!!!");
            goto error;
        }

error:

        if (adims != NULL) {
            delete [] adims;
        }

        if (base_type >= 0) {
            H5Tclose(base_type);
        }

        if (ntype >= 0) {
            H5Tclose(ntype);
        }

        if (dtype >= 0) {
            H5Tclose(dtype);
        }
        
        if (ex != nullptr) {
            throw ex;
        }

        WriteObject(result);
        return;
    }
}