
#include "AttributeInfo.h"
#include "ProviderUtils.h"

extern "C" {
#include "H5Apublic.h"
#include "H5Spublic.h"
#include "H5Tpublic.h"
}

using namespace System;
using namespace System::Collections;
using namespace System::Management::Automation;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{

    Hashtable^ ProviderUtils::H5Attribute(hid_t attr_id, String^ attributeName)
    {
        Hashtable^ ht = gcnew Hashtable();

        H5A_info_t info;
        if (H5Aget_info(attr_id, &info) >= 0)
        {
            AttributeInfo^ attrInfo = gcnew AttributeInfo(attributeName, &info);
            ht->Add("AttributeName", attrInfo->AttributeName);
            ht->Add("CreationOrderIsValid", attrInfo->CreationOrderIsValid);
            ht->Add("CreationOrder", attrInfo->CreationOrder);
            ht->Add("AttributeNameCharacterSet", attrInfo->AttributeNameCharacterSet);
            ht->Add("DataSizeBytes", attrInfo->DataSizeBytes);
            ht->Add("StorageSizeBytes", H5Aget_storage_size(attr_id));

#pragma region H5Aget_space

            hid_t space_id = H5Aget_space(attr_id);
            if (space_id >= 0)
            {
                switch (H5Sget_simple_extent_type(space_id))
                {
                case H5S_SCALAR:
                    ht->Add("SimpleExtentType", "Scalar");
                    break;
                case H5S_SIMPLE:
                    ht->Add("SimpleExtentType", "Simple");
                    break;
                case H5S_NULL:
                    ht->Add("SimpleExtentType", "Null");
                    break;
                default:
                    ht->Add("SimpleExtentType", "UNKNOWN");
                    break;
                }

                int rank = H5Sget_simple_extent_ndims(space_id);
                if (rank > 0)
                {
                    ht->Add("Rank", rank);
                    ht->Add("ElementCount", H5Sget_simple_extent_npoints(space_id));

                    hsize_t* dims = new hsize_t [rank];
                    hsize_t* maxdims = new hsize_t [rank];
                    rank = H5Sget_simple_extent_dims(space_id, dims, maxdims);
                    if (rank >= 0)
                    {
                        array<hsize_t>^ adims    = gcnew array<hsize_t>(rank);
                        array<hsize_t>^ maxadims = gcnew array<hsize_t>(rank);
                        for (int i = 0; i < rank; ++i)
                        {
                            adims[i]    = dims[i];
                            maxadims[i] = maxdims[i];
                        }
                        ht->Add("Dimensions", adims);
                        //ht->Add("MaxDimensions", maxadims);
                    }
                    else { // TODO
                    }
                    delete [] maxdims;
                    delete [] dims;
                }
                else { // TODO
                }

                if (H5Sclose(space_id) < 0) { // TODO
                }
            }
            else { // TODO
            }

#pragma endregion Retrieve attribute shape and maximum extent

#pragma region H5Aget_type

            hid_t type_id = H5Aget_type(attr_id);

            if (type_id >= 0)
            {
                switch (H5Tget_class(type_id))
                {
                case H5T_INTEGER:
                    ht->Add("ElementTypeClass", "Integer");
                    break;
                case H5T_FLOAT:
                    ht->Add("ElementTypeClass", "Float");
                    break;
                case H5T_STRING:
                    ht->Add("ElementTypeClass", "String");
                    break;
                case H5T_BITFIELD:
                    ht->Add("ElementTypeClass", "Bitfield");
                    break;
                case H5T_OPAQUE:
                    ht->Add("ElementTypeClass", "Opaque");
                    break;
                case H5T_COMPOUND:
                    ht->Add("ElementTypeClass", "Compound");
                    break;
                case H5T_REFERENCE:
                    ht->Add("ElementTypeClass", "Reference");
                    break;
                case H5T_ENUM:
                    ht->Add("ElementTypeClass", "Enum");
                    break;
                case H5T_VLEN:
                    ht->Add("ElementTypeClass", "Vlen");
                    break;
                case H5T_ARRAY:
                    ht->Add("ElementTypeClass", "Array");
                    break;
                default:
                    ht->Add("ElementTypeClass", "UNKNOWN");
                    break;
                }

                ht["ElementType"] = ProviderUtils::ParseH5Type(type_id);
            }
            else { // TODO
            }

#pragma endregion Retrieve attribute type information (and value)

        }
        else { // TODO
        }

        return ht;
    }
}