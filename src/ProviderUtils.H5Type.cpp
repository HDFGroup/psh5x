
#include "ProviderUtils.h"

extern "C" {
#include "H5Tpublic.h"
}

#include <cstdlib>
#include <cstring>

using namespace System;
using namespace System::Collections;
using namespace System::Management::Automation;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
    Hashtable^ ProviderUtils::ParseH5Type(hid_t type)
    {
        Hashtable^ result = gcnew Hashtable();
        Hashtable^ ht = nullptr;

        hid_t unwnd = -1;
        size_t size = 0, offset;
        int i, nmembers = 0, rank;
        unsigned ui;
        char* name;
        String^ s;
        hsize_t* dims;
        array<hsize_t>^ d;
        htri_t ierr;
        H5T_cset_t cset;
        H5T_str_t strpad;
        H5T_pad_t lsb, msb;
        size_t spos, epos, esize, mpos, msize;
        
        switch (H5Tget_class(type))
        {
        case H5T_COMPOUND:

#pragma region Compound

            result["Class"] = "COMPOUND";
            size = H5Tget_size(type);
            result["Size"] = size;
            ht = gcnew Hashtable();
            nmembers = H5Tget_nmembers(type);
            for (ui = 0; ui < safe_cast<unsigned>(nmembers); ++ui)
            {
                name = H5Tget_member_name(type, ui);
                s = gcnew String(name);
                unwnd = H5Tget_member_type(type, ui);
                if (unwnd >= 0)
                {
                    ht[s] = ProviderUtils::ParseH5Type(unwnd);
                    offset = H5Tget_member_offset(type, ui);
                    ((Hashtable^) ht[s])->Add("MemberOffset", offset);
                    if (H5Tclose(unwnd) < 0) { // TODO
                    }
                }
            }
            result["Members"] = ht;
            break;

#pragma endregion

        case H5T_ENUM:

            break;

        case H5T_VLEN:

            result["Class"] = "VLEN";

            unwnd = H5Tget_super(type);
            if (unwnd >= 0)
            {
                result["Type"] = ProviderUtils::ParseH5Type(unwnd);
                if (H5Tclose(unwnd) < 0) { // TODO
                }
            }
            break;

        case H5T_ARRAY:

#pragma region Array

            result["Class"] = "ARRAY";
            rank = H5Tget_array_ndims(type);
            dims = new hsize_t [rank];
            rank = H5Tget_array_dims2(type, dims);
            d = gcnew array<hsize_t>(rank);
            for (i = 0; i < rank; ++i) { d[i] = dims[i]; }
            delete [] dims;
            result["Dims"] = d;
            unwnd = H5Tget_super(type);
            if (unwnd >= 0)
            {
                result["Type"] = ProviderUtils::ParseH5Type(unwnd);
                if (H5Tclose(unwnd) < 0) { // TODO
                }
            }
            break;

#pragma endregion

        case H5T_STRING:

#pragma region String

            result["Class"] = "STRING";
            cset = H5Tget_cset(type);
            switch (cset)
            {
            case H5T_CSET_ASCII:
                result["CharSet"] = "Ascii";
                break;
            case H5T_CSET_UTF8:
                result["CharSet"] = "Utf8";
                break;
            default:
                result["CharSet"] = "UNKNOWN";
                break;
            }

            ierr = H5Tis_variable_str(type);
            if (ierr == 0)
            {
                size = H5Tget_size(type);
                result["Length"] = size;
                strpad = H5Tget_strpad(type);
                switch (strpad)
                {
                case H5T_STR_NULLTERM:
                    result["StrPad"] = "Nullterm";
                    break;
                case H5T_STR_NULLPAD:
                    result["StrPad"] = "Nullpad";
                    break;
                case H5T_STR_SPACEPAD:
                    result["StrPad"] = "Spacepad";
                    break;
                default:
                    result["StrPad"] = "UNKNOWN";
                    break;
                }
            }
 
            break;

#pragma endregion

        case H5T_INTEGER:

#pragma region Integer

            result["Class"] = "INTEGER";
            size = H5Tget_size(type);
            result["Size"] = size;
            size = H5Tget_precision(type);
            result["Precision"] = size;
            i = H5Tget_offset(type);
            result["BitOffset"] = i;

            if (H5Tget_pad(type, &lsb, &msb) >= 0)
            {
                switch (lsb)
                {
                case H5T_PAD_ZERO:
                    result["LSBitPadding"] = "Zero";
                    break;
                case H5T_PAD_ONE:
                    result["LSBitPadding"] = "One";
                    break;
                case H5T_PAD_BACKGROUND:
                    result["LSBitPadding"] = "Background";
                    break;
                default:
                    result["LSBitPadding"] = "UNKNOWN";
                    break;
                }

                switch (msb)
                {
                case H5T_PAD_ZERO:
                    result["MSBitPadding"] = "Zero";
                    break;
                case H5T_PAD_ONE:
                    result["MSBitPadding"] = "One";
                    break;
                case H5T_PAD_BACKGROUND:
                    result["MSBitPadding"] = "Background";
                    break;
                default:
                    result["MSBitPadding"] = "UNKNOWN";
                    break;
                }
            }
            else { // TODO
            }

            switch (H5Tget_sign(type))
            {
            case H5T_SGN_NONE:
                result["SignType"] = "None";
                break;
            case H5T_SGN_2:
                result["SignType"] = "2";
                break;
            default:
                result["SignType"] = "UNKNOWN";
                break;
            }

            switch (H5Tget_order(type))
            {
            case H5T_ORDER_LE:
                result["ByteOrder"] = "LE";
                break;
            case H5T_ORDER_BE:
                result["ByteOrder"] = "BE";
                break;
            case H5T_ORDER_VAX:
                result["ByteOrder"] = "VAX";
                break;
            case H5T_ORDER_MIXED:
                result["ByteOrder"] = "Mixed";
                break;
            case H5T_ORDER_NONE:
                result["ByteOrder"] = "None";
                break;
            default:
                result["ByteOrder"] = "UNKNOWN";
                break;
            }

            break;

#pragma endregion

        case H5T_FLOAT:
            
#pragma region Float

            result["Class"] = "FLOAT";
            size = H5Tget_size(type);
            result["Size"] = size;
            size = H5Tget_precision(type);
            result["Precision"] = size;
            i = H5Tget_offset(type);
            result["BitOffset"] = i;

            if (H5Tget_pad(type, &lsb, &msb) >= 0)
            {
                switch (lsb)
                {
                case H5T_PAD_ZERO:
                    result["LSBitPadding"] = "Zero";
                    break;
                case H5T_PAD_ONE:
                    result["LSBitPadding"] = "One";
                    break;
                case H5T_PAD_BACKGROUND:
                    result["LSBitPadding"] = "Background";
                    break;
                default:
                    result["LSBitPadding"] = "UNKNOWN";
                    break;
                }

                switch (msb)
                {
                case H5T_PAD_ZERO:
                    result["MSBitPadding"] = "Zero";
                    break;
                case H5T_PAD_ONE:
                    result["MSBitPadding"] = "One";
                    break;
                case H5T_PAD_BACKGROUND:
                    result["MSBitPadding"] = "Background";
                    break;
                default:
                    result["MSBitPadding"] = "UNKNOWN";
                    break;
                }
            }
            else { // TODO
            }

            switch (H5Tget_inpad(type))
            {
            case H5T_PAD_ZERO:
                result["IntlBitPadding"] = "Zero";
                break;
            case H5T_PAD_ONE:
                result["IntlBitPadding"] = "One";
                break;
            case H5T_PAD_BACKGROUND:
                result["IntlBitPadding"] = "Background";
                break;
            default:
                result["IntlBitPadding"] = "UNKNOWN";
                break;
            }

            switch (H5Tget_order(type))
            {
            case H5T_ORDER_LE:
                result["ByteOrder"] = "LE";
                break;
            case H5T_ORDER_BE:
                result["ByteOrder"] = "BE";
                break;
            case H5T_ORDER_VAX:
                result["ByteOrder"] = "VAX";
                break;
            case H5T_ORDER_MIXED:
                result["ByteOrder"] = "Mixed";
                break;
            case H5T_ORDER_NONE:
                result["ByteOrder"] = "None";
                break;
            default:
                result["ByteOrder"] = "UNKNOWN";
                break;
            }

            if (H5Tget_fields(type, &spos, &epos, &esize, &mpos, &msize) >= 0)
            {
                result["SignBitPos"] = spos;
                result["ExpBitPos"] = epos;
                result["ExpBits"] = esize;
                result["MantBitPos"] = mpos;
                result["MantBits"] = msize;
            }

            size = H5Tget_ebias(type);
            result["ExpBias"] = size;

            switch (H5Tget_norm(type))
            {
            case H5T_NORM_IMPLIED:
                result["MantNorm"] = "Implied";
                break;
            case H5T_NORM_MSBSET:
                result["MantNorm"] = "MsbSet";
                break;
            case H5T_NORM_NONE:
                result["MantNorm"] = "None";
                break;
            default:
                result["MantNorm"] = "UNKNOWN";
                break;
            }

            break;

#pragma endregion

        case H5T_BITFIELD:
            
#pragma region Bitfield

            result["Class"] = "BITFIELD";
            size = H5Tget_size(type);
            result["Size"] = size;
            size = H5Tget_precision(type);
            result["Precision"] = size;
            i = H5Tget_offset(type);
            result["BitOffset"] = i;

            if (H5Tget_pad(type, &lsb, &msb) >= 0)
            {
                switch (lsb)
                {
                case H5T_PAD_ZERO:
                    result["LSBitPadding"] = "Zero";
                    break;
                case H5T_PAD_ONE:
                    result["LSBitPadding"] = "One";
                    break;
                case H5T_PAD_BACKGROUND:
                    result["LSBitPadding"] = "Background";
                    break;
                default:
                    result["LSBitPadding"] = "UNKNOWN";
                    break;
                }

                switch (msb)
                {
                case H5T_PAD_ZERO:
                    result["MSBitPadding"] = "Zero";
                    break;
                case H5T_PAD_ONE:
                    result["MSBitPadding"] = "One";
                    break;
                case H5T_PAD_BACKGROUND:
                    result["MSBitPadding"] = "Background";
                    break;
                default:
                    result["MSBitPadding"] = "UNKNOWN";
                    break;
                }
            }
            else { // TODO
            }

            switch (H5Tget_order(type))
            {
            case H5T_ORDER_LE:
                result["ByteOrder"] = "LE";
                break;
            case H5T_ORDER_BE:
                result["ByteOrder"] = "BE";
                break;
            case H5T_ORDER_VAX:
                result["ByteOrder"] = "VAX";
                break;
            case H5T_ORDER_MIXED:
                result["ByteOrder"] = "Mixed";
                break;
            case H5T_ORDER_NONE:
                result["ByteOrder"] = "None";
                break;
            default:
                result["ByteOrder"] = "UNKNOWN";
                break;
            }

            break;

#pragma endregion

        case H5T_OPAQUE:

#pragma region Opaque

            result["Class"] = "OPAQUE";
            name = H5Tget_tag(type);
            result["Tag"] = gcnew String(name);
            
            switch (H5Tget_order(type))
            {
            case H5T_ORDER_LE:
                result["ByteOrder"] = "LE";
                break;
            case H5T_ORDER_BE:
                result["ByteOrder"] = "BE";
                break;
            case H5T_ORDER_VAX:
                result["ByteOrder"] = "VAX";
                break;
            case H5T_ORDER_MIXED:
                result["ByteOrder"] = "Mixed";
                break;
            case H5T_ORDER_NONE:
                result["ByteOrder"] = "None";
                break;
            default:
                result["ByteOrder"] = "UNKNOWN";
                break;
            }

            break;

#pragma endregion

        case H5T_REFERENCE:

            result["Class"] = "REFERENCE";
            if (H5Tequal(type, H5T_STD_REF_OBJ) > 0) {
                result["Type"] = "REF_OBJ";
            }
            else if (H5Tequal(type, H5T_STD_REF_DSETREG) > 0) {
                result["Type"] = "REF_DSETREG";
            }
            else {
                result["Type"] = "UNKNOWN";
            }
            break;

        default:
            break;
        }

        return result;
    }

    hid_t ProviderUtils::ParseH5Type(Object^ obj)
    {
        hid_t result = -1;

        Hashtable^ ht = nullptr;

        if (TryGetValue(obj, ht))
        {
            if (ht->ContainsKey("Class"))
            {
                String^ type_class = ht["Class"]->ToString()->ToUpper();

                if (Array::BinarySearch(m_type_classes, type_class) >= 0)
                {
                    if (type_class == "ARRAY")
                    {
#pragma region ARRAY

                        if (ht->ContainsKey("Type"))
                        {
                            hid_t base_type = ParseH5Type(ht["Type"]);
                            
                            if (base_type >= 0)
                            {
                                if (ht->ContainsKey("Dims"))
                                {
                                    array<hsize_t>^ dims = nullptr;
                                    
                                    if (ProviderUtils::TryGetValue(ht["Dims"], dims))
                                    {
                                        if (dims->Length > 0 && dims->Length <= 4)
                                        {
                                            unsigned rank = safe_cast<unsigned>(dims->Length);
                                            hsize_t tmp[4];
                                            for (int i = 0; i < dims->Length; ++i) {
                                                tmp[i] = dims[i];
                                            }
                                            result = H5Tarray_create2(base_type, rank, tmp);
                                        }
                                        else {
                                            throw gcnew ArgumentException("ARRAY: The 'Dims' array's rank must not exceed 4.");
                                        }
                                    }
                                    else {
                                        throw gcnew ArgumentException("ARRAY: 'Dims' key has invalid value.");
                                    }
                                }
                                else {
                                    throw gcnew ArgumentException("ARRAY: No 'Dims' key found.");
                                }
                            }
                            else {
                                throw gcnew ArgumentException(
                                    String::Format("ARRAY: Unsupported base type: '{0}'", ht["Type"]));
                            }
                        }
                        else {
                            throw gcnew ArgumentException("ARRAY: No 'Type' key found.");
                        }

#pragma endregion
                    }
                    else if (type_class == "BITFIELD")
                    {
#pragma region BITFIELD

                        if (ht->ContainsKey("Type")) {
                            result = ProviderUtils::ParseH5Type(ht["Type"]);
                            if (H5Tget_class(result) != H5T_BITFIELD) {
                                   throw gcnew ArgumentException("BITFIELD: The type specified is not of class BITFIELD.");
                            }
                            return result;
                        }

                        size_t precision = 0;
                        if (ht->ContainsKey("Precision")) {
                            if (!TryGetValue(ht["Precision"], precision)) {
                                throw gcnew ArgumentException("BITFIELD: Unable to convert the 'Precision' value into size_t.");
                            }
                        }
                        else {
                            throw gcnew NotImplementedException("BITFIELD: No 'Precision' key found.");
                        }

                        String^ s = nullptr;

                        H5T_order_t order = H5T_ORDER_LE;
                        if (ht->ContainsKey("ByteOrder")) {
                            if (!TryGetValue(ht["ByteOrder"], s)) {
                                throw gcnew ArgumentException("BITFIELD: Unable to convert the 'ByteOrder' value into a string.");
                            }

                            if (s != nullptr)
                            {
                                s = s->Trim()->ToUpper();
                                if (s != "BE" && s != "LE") {
                                    throw gcnew ArgumentException("BITFIELD: Invalid 'ByteOrder' value. Must be 'BE' or 'LE'!");
                                }

                                if (s == "BE")
                                {
                                    order = H5T_ORDER_BE;
                                }
                            }
                            else {
                                throw gcnew ArgumentException("BITFIELD: Invalid 'ByteOrder' value. Must be 'BE' or 'LE'!");
                            }
                        }

                        size_t offset = 0;
                        if (ht->ContainsKey("BitOffset")) {
                            if (!TryGetValue(ht["BitOffset"], offset)) {
                                throw gcnew ArgumentException("BITFIELD: Unable to convert the 'BitOffset' value into size_t.");
                            }
                        }

                        H5T_pad_t lsb = H5T_PAD_ZERO, msb = H5T_PAD_ZERO;
                        if (ht->ContainsKey("MSBitPadding")) {
                            if (!TryGetValue(ht["MSBitPadding"], s)) {
                                throw gcnew ArgumentException("BITFIELD: Unable to convert the 'MSBitPadding' value into a string.");
                            }

                            if (s != nullptr)
                            {
                                s = s->Trim()->ToUpper();
                                if (s != "ZERO" && s != "ONE" && s != "BACKGROUND") {
                                    throw gcnew ArgumentException("BITFIELD: Invalid 'MSBitPadding' value. Must be 'Zero' or 'One' or 'Background'!");
                                }

                                if (s == "ONE") {
                                    msb = H5T_PAD_ONE;
                                }
                                else if (s == "BACKGROUND") {
                                    msb = H5T_PAD_BACKGROUND;
                                }
                            }
                            else {
                                throw gcnew ArgumentException("BITFIELD: Invalid 'MSBitPadding' value. Must be 'Zero' or 'One' or 'Background'!");
                            }
                        }
                        if (ht->ContainsKey("LSBitPadding")) {
                            if (!TryGetValue(ht["LSBitPadding"], s)) {
                                throw gcnew ArgumentException("BITFIELD: Unable to convert the 'LSBitPadding' value into a string.");
                            }

                            if (s != nullptr)
                            {
                                s = s->Trim()->ToUpper();
                                if (s != "ZERO" && s != "ONE" && s != "BACKGROUND") {
                                    throw gcnew ArgumentException("BITFIELD: Invalid 'LSBitPadding' value. Must be 'Zero' or 'One' or 'Background'!");
                                }

                                if (s == "ONE") {
                                    msb = H5T_PAD_ONE;
                                }
                                else if (s == "BACKGROUND") {
                                    msb = H5T_PAD_BACKGROUND;
                                }
                            }
                            else {
                                throw gcnew ArgumentException("BITFIELD: Invalid 'LSBitPadding' value. Must be 'Zero' or 'One' or 'Background'!");
                            }
                        }

                        result = H5Tcopy(H5T_NATIVE_B8);
                        
                        if (H5Tset_precision(result, precision) < 0) { // TODO
                        }
                        if (H5Tset_offset(result, offset) < 0) { // TODO
                        }
                        if (H5Tset_order(result, order) < 0) { // TODO
                        }
                        if (H5Tset_pad(result, lsb, msb) < 0) { // TODO
                        }
#pragma endregion
                    }
                    else if (type_class == "COMPOUND")
                    {
#pragma region COMPOUND

                        size_t size = 0;
                        
                        if (ht->ContainsKey("Size"))
                        {
                            if (ProviderUtils::TryGetValue(ht["Size"], size))
                            {
                                result = H5Tcreate(H5T_COMPOUND, size);

                                if (ht->ContainsKey("Components"))
                                {
                                    Hashtable^ components = nullptr;
                                    
                                    if (TryGetValue(ht["Components"], components))
                                    {
                                        for each (String^ key in components->Keys)
                                        {
                                            char* name = (char*)(Marshal::StringToHGlobalAnsi(key)).ToPointer();
                                            
                                            Hashtable^ comp = nullptr;

                                            if (TryGetValue(components[key], comp))
                                            {
                                                size_t offset = 0;
                                                if (comp->ContainsKey("MemberOffset"))
                                                {
                                                    if (ProviderUtils::TryGetValue(comp["MemberOffset"], offset))
                                                    {
                                                        hid_t dtype = ParseH5Type(comp);
                                                        if (dtype >= 0)
                                                        {
                                                            if (H5Tinsert(result, name, offset, dtype) < 0)
                                                            {
                                                                if (H5Tclose(result) < 0) { // TODO
                                                                }
                                                                result = -1;
                                                                throw gcnew ArgumentException(
                                                                    String::Format("COMPOUND: Failed to insert component '{0}'", key));
                                                            }
                                                        }
                                                        else {
                                                            throw gcnew ArgumentException(
                                                                String::Format("COMPOUND: Unsupported component type '{0}' for component '{1}'",
                                                                comp, key));
                                                        }
                                                    }
                                                    else {
                                                        throw gcnew ArgumentException(
                                                            String::Format("COMPOUND: Unable to convert the 'MemberOffset' value into size_t for component '{0}'", key));
                                                    }
                                                }
                                                else {
                                                    throw gcnew ArgumentException(
                                                        String::Format("COMPOUND: No 'MemberOffset' key found for component '{0}'.", key));
                                                }
                                            }
                                            else {
                                                throw gcnew ArgumentException(
                                                    String::Format("COMPOUND: Component '{0}' does not contain a hashtable.", key));
                                            }
                                        }
                                    }
                                    else {
                                        throw gcnew ArgumentException("COMPOUND: 'Components' does not contain a hashtable.");
                                    }
                                }
                                else {
                                    throw gcnew ArgumentException("COMPOUND: No 'Components' key found.");
                                }
                            }
                            else {
                                throw gcnew ArgumentException("COMPOUND: Unable to convert the 'Size' value into size_t.");
                            }
                        }
                        else {
                            throw gcnew ArgumentException("COMPOUND: No 'Size' key found.");
                        }

#pragma endregion
                    }
                    else if (type_class == "ENUM")
                    {
#pragma region ENUM

                        if (ht->ContainsKey("Members"))
                        {
                            result = H5Tenum_create(H5Tcopy(H5T_NATIVE_INT));

                            Hashtable^ members = nullptr;
                            if (TryGetValue(ht["Members"], members))
                            {
                                for each (String^ key in members->Keys)
                                {
                                    int value = -1;
                                    if (ProviderUtils::TryGetValue(members[key], value))
                                    {
                                        char* name = (char*)(Marshal::StringToHGlobalAnsi(key)).ToPointer();
                                        if (H5Tenum_insert(result, name, &value) < 0)
                                        {
                                        }
                                    }
                                    else {
                                        throw gcnew ArgumentException(
                                            String::Format("ENUM: The value of '{0}' is not integer.", key));
                                    }
                                }
                            }
                            else {
                                throw gcnew ArgumentException("ENUM: The value of the 'Members' key is not a hashtable.");
                            }
                        }
                        else {
                            throw gcnew ArgumentException("ENUM: No 'Members' key found.");
                        }

#pragma endregion
                    }
                    else if (type_class == "FLOAT")
                    {
#pragma region FLOAT

                        if (ht->ContainsKey("Type")) {
                            result = ProviderUtils::ParseH5Type(ht["Type"]);
                            if (H5Tget_class(result) != H5T_FLOAT) {
                                   throw gcnew ArgumentException("FLOAT: The type specified is not of class FLOAT.");
                            }
                            return result;
                        }

                        size_t offset = 0;
                        if (ht->ContainsKey("BitOffset")) {
                            if (!TryGetValue(ht["BitOffset"], offset)) {
                                throw gcnew ArgumentException("FLOAT: Unable to convert the 'BitOffset' value into size_t.");
                            }
                        }

                        size_t precision = 0;
                        if (ht->ContainsKey("Precision")) {
                            if (!TryGetValue(ht["Precision"], precision)) {
                                throw gcnew ArgumentException("FLOAT: Unable to convert the 'Precision' value into size_t.");
                            }
                        }
                        else {
                            throw gcnew NotImplementedException("FLOAT: No 'Precision' key found.");
                        }

                        String^ s = nullptr;

                        H5T_order_t order = H5T_ORDER_LE;
                        if (ht->ContainsKey("ByteOrder")) {
                            if (!TryGetValue(ht["ByteOrder"], s)) {
                                throw gcnew ArgumentException("FLOAT: Unable to convert the 'ByteOrder' value into a string.");
                            }

                            if (s != nullptr)
                            {
                                s = s->Trim()->ToUpper();
                                if (s != "BE" && s != "LE") {
                                    throw gcnew ArgumentException("FLOAT: Invalid 'ByteOrder' value. Must be 'BE' or 'LE'!");
                                }

                                if (s == "BE")
                                {
                                    order = H5T_ORDER_BE;
                                }
                            }
                            else {
                                throw gcnew ArgumentException("FLOAT: Invalid 'ByteOrder' value. Must be 'BE' or 'LE'!");
                            }
                        }

                        H5T_pad_t lsb = H5T_PAD_ZERO, msb = H5T_PAD_ZERO, inp = H5T_PAD_ZERO;
                        if (ht->ContainsKey("MSBitPadding")) {
                            if (!TryGetValue(ht["MSBitPadding"], s)) {
                                throw gcnew ArgumentException("FLOAT: Unable to convert the 'MSBitPadding' value into a string.");
                            }

                            if (s != nullptr)
                            {
                                s = s->Trim()->ToUpper();
                                if (s != "ZERO" && s != "ONE" && s != "BACKGROUND") {
                                    throw gcnew ArgumentException("FLOAT: Invalid 'MSBitPadding' value. Must be 'Zero' or 'One' or 'Background'!");
                                }

                                if (s == "ONE") {
                                    msb = H5T_PAD_ONE;
                                }
                                else if (s == "BACKGROUND") {
                                    msb = H5T_PAD_BACKGROUND;
                                }
                            }
                            else {
                                throw gcnew ArgumentException("FLOAT: Invalid 'MSBitPadding' value. Must be 'Zero' or 'One' or 'Background'!");
                            }
                        }
                        if (ht->ContainsKey("LSBitPadding")) {
                            if (!TryGetValue(ht["LSBitPadding"], s)) {
                                throw gcnew ArgumentException("FLOAT: Unable to convert the 'LSBitPadding' value into a string.");
                            }

                            if (s != nullptr)
                            {
                                s = s->Trim()->ToUpper();
                                if (s != "ZERO" && s != "ONE" && s != "BACKGROUND") {
                                    throw gcnew ArgumentException("FLOAT: Invalid 'LSBitPadding' value. Must be 'Zero' or 'One' or 'Background'!");
                                }

                                if (s == "ONE") {
                                    msb = H5T_PAD_ONE;
                                }
                                else if (s == "BACKGROUND") {
                                    msb = H5T_PAD_BACKGROUND;
                                }
                            }
                            else {
                                throw gcnew ArgumentException("FLOAT: Invalid 'LSBitPadding' value. Must be 'Zero' or 'One' or 'Background'!");
                            }
                        }
                        if (ht->ContainsKey("IntlBitPadding")) {
                            if (!TryGetValue(ht["IntlBitPadding"], s)) {
                                throw gcnew ArgumentException("FLOAT: Unable to convert the 'IntlBitPadding' value into a string.");
                            }

                            if (s != nullptr)
                            {
                                s = s->Trim()->ToUpper();
                                if (s != "ZERO" && s != "ONE" && s != "BACKGROUND") {
                                    throw gcnew ArgumentException("FLOAT: Invalid 'IntlBitPadding' value. Must be 'Zero' or 'One' or 'Background'!");
                                }

                                if (s == "ONE") {
                                    msb = H5T_PAD_ONE;
                                }
                                else if (s == "BACKGROUND") {
                                    msb = H5T_PAD_BACKGROUND;
                                }
                            }
                            else {
                                throw gcnew ArgumentException("FLOAT: Invalid 'IntlBitPadding' value. Must be 'Zero' or 'One' or 'Background'!");
                            }
                        }

                        size_t spos = 31;
                        if (ht->ContainsKey("SignBitPos")) {
                            if (!TryGetValue(ht["SignBitPos"], spos)) {
                                throw gcnew ArgumentException("FLOAT: Unable to convert the 'SignBitPos' value into size_t.");
                            }
                        }
                        size_t epos = 23;
                        if (ht->ContainsKey("ExpBitPos")) {
                            if (!TryGetValue(ht["ExpBitPos"], epos)) {
                                throw gcnew ArgumentException("FLOAT: Unable to convert the 'ExpBitPos' value into size_t.");
                            }
                        }
                        size_t esize = 8;
                        if (ht->ContainsKey("ExpBits")) {
                            if (!TryGetValue(ht["ExpBits"], esize)) {
                                throw gcnew ArgumentException("FLOAT: Unable to convert the 'ExpBits' value into size_t.");
                            }
                        }
                        size_t mpos = 0;
                        if (ht->ContainsKey("MantBitPos")) {
                            if (!TryGetValue(ht["MantBitPos"], mpos)) {
                                throw gcnew ArgumentException("FLOAT: Unable to convert the 'MantBitPos' value into size_t.");
                            }
                        }
                        size_t msize = 23;
                        if (ht->ContainsKey("MantBits")) {
                            if (!TryGetValue(ht["MantBits"], msize)) {
                                throw gcnew ArgumentException("FLOAT: Unable to convert the 'MantBits' value into size_t.");
                            }
                        }

                        size_t ebias = 127;
                        if (ht->ContainsKey("ExpBias")) {
                            if (!TryGetValue(ht["ExpBias"], ebias)) {
                                throw gcnew ArgumentException("FLOAT: Unable to convert the 'ExpBias' value into size_t.");
                            }
                        }

                        H5T_norm_t norm = H5T_NORM_IMPLIED;
                        if (ht->ContainsKey("MantNorm")) {
                            if (!TryGetValue(ht["MantNorm"], s)) {
                                throw gcnew ArgumentException("FLOAT: Unable to convert the 'MantNorm' value into a string.");
                            }

                            if (s != nullptr)
                            {
                                s = s->Trim()->ToUpper();
                                if (s != "IMPLIED" && s != "MSBSET" && s != "NONE") {
                                    throw gcnew ArgumentException("FLOAT: Invalid 'MantNorm' value. Must be 'Implied', 'MsbSet' or 'None'!");
                                }

                                if (s == "IMPLIED") {
                                    norm = H5T_NORM_IMPLIED;
                                }
                                else if (s == "MSBSET") {
                                    norm = H5T_NORM_MSBSET;
                                }
                                else {
                                    norm = H5T_NORM_NONE;
                                }
                            }
                            else {
                                throw gcnew ArgumentException("FLOAT: Invalid 'MantNorm' value. Must be 'Implied', 'MsbSet' or 'None'!");
                            }
                        }

                        // TODO: check constraints

                        result = H5Tcopy(H5T_NATIVE_FLOAT);

                        if (H5Tset_precision(result, precision) < 0) { // TODO
                        }
                        if (H5Tset_offset(result, offset) < 0) { // TODO
                        }
                        if (H5Tset_order(result, order) < 0) { // TODO
                        }
                        if (H5Tset_pad(result, lsb, msb) < 0) { // TODO
                        }
                        if (H5Tset_inpad(result, inp) < 0) { // TODO
                        }
                        if (H5Tset_fields(result, spos, epos, esize, mpos, msize) < 0) { // TODO
                        }
                        if (H5Tset_ebias(result, ebias) < 0) { // TODO
                        }
                        if (H5Tset_norm(result, norm) < 0) { // TODO
                        }
                        
#pragma endregion
                    }
                    else if (type_class == "INTEGER")
                    {
#pragma region INTEGER
                        if (ht->ContainsKey("Type")) {
                            result = ProviderUtils::ParseH5Type(ht["Type"]);
                            if (H5Tget_class(result) != H5T_INTEGER) {
                                   throw gcnew ArgumentException("INTEGER: The type specified is not of class INTEGER.");
                            }
                            return result;
                        }
                        
                        size_t precision = 0;
                        if (ht->ContainsKey("Precision")) {
                            if (!TryGetValue(ht["Precision"], precision)) {
                                throw gcnew ArgumentException("INTEGER: Unable to convert the 'Precision' value into size_t.");
                            }
                        }
                        else {
                            throw gcnew NotImplementedException("INTEGER: No 'Precision' key found.");
                        }

                        String^ s = nullptr;

                        H5T_order_t order = H5T_ORDER_LE;
                        if (ht->ContainsKey("ByteOrder")) {
                            if (!TryGetValue(ht["ByteOrder"], s)) {
                                throw gcnew ArgumentException("INTEGER: Unable to convert the 'ByteOrder' value into a string.");
                            }

                            if (s != nullptr)
                            {
                                s = s->Trim()->ToUpper();
                                if (s != "BE" && s != "LE") {
                                    throw gcnew ArgumentException("INTEGER: Invalid 'ByteOrder' value. Must be 'BE' or 'LE'!");
                                }

                                if (s == "BE")
                                {
                                    order = H5T_ORDER_BE;
                                }
                            }
                            else {
                                throw gcnew ArgumentException("INTEGER: Invalid 'ByteOrder' value. Must be 'BE' or 'LE'!");
                            }
                        }

                        size_t offset = 0;
                        if (ht->ContainsKey("BitOffset")) {
                            if (!TryGetValue(ht["BitOffset"], offset)) {
                                throw gcnew ArgumentException("INTEGER: Unable to convert the 'BitOffset' value into size_t.");
                            }
                        }

                        H5T_sign_t sign = H5T_SGN_2;
                        if (ht->ContainsKey("SignType")) {
                            if (!TryGetValue(ht["SignType"], s)) {
                                throw gcnew ArgumentException("INTEGER: Unable to convert the 'SignType' value into a string.");
                            }

                            if (s != nullptr)
                            {
                                s = s->Trim()->ToUpper();
                                if (s != "NONE" && s != "2") {
                                    throw gcnew ArgumentException("INTEGER: Invalid 'SignType' value. Must be '2' or 'None'!");
                                }

                                if (s == "NONE")
                                {
                                    sign = H5T_SGN_NONE;
                                }
                            }
                            else {
                                throw gcnew ArgumentException("INTEGER: Invalid 'SignType' value. Must be '2' or 'None'!");
                            }
                        }

                        H5T_pad_t lsb = H5T_PAD_ZERO, msb = H5T_PAD_ZERO;
                        if (ht->ContainsKey("MSBitPadding")) {
                            if (!TryGetValue(ht["MSBitPadding"], s)) {
                                throw gcnew ArgumentException("INTEGER: Unable to convert the 'MSBitPadding' value into a string.");
                            }

                            if (s != nullptr)
                            {
                                s = s->Trim()->ToUpper();
                                if (s != "ZERO" && s != "ONE" && s != "BACKGROUND") {
                                    throw gcnew ArgumentException("INTEGER: Invalid 'MSBitPadding' value. Must be 'Zero' or 'One' or 'Background'!");
                                }

                                if (s == "ONE") {
                                    msb = H5T_PAD_ONE;
                                }
                                else if (s == "BACKGROUND") {
                                    msb = H5T_PAD_BACKGROUND;
                                }
                            }
                            else {
                                throw gcnew ArgumentException("INTEGER: Invalid 'MSBitPadding' value. Must be 'Zero' or 'One' or 'Background'!");
                            }
                        }
                        if (ht->ContainsKey("LSBitPadding")) {
                            if (!TryGetValue(ht["LSBitPadding"], s)) {
                                throw gcnew ArgumentException("INTEGER: Unable to convert the 'LSBitPadding' value into a string.");
                            }

                            if (s != nullptr)
                            {
                                s = s->Trim()->ToUpper();
                                if (s != "ZERO" && s != "ONE" && s != "BACKGROUND") {
                                    throw gcnew ArgumentException("INTEGER: Invalid 'LSBitPadding' value. Must be 'Zero' or 'One' or 'Background'!");
                                }

                                if (s == "ONE") {
                                    msb = H5T_PAD_ONE;
                                }
                                else if (s == "BACKGROUND") {
                                    msb = H5T_PAD_BACKGROUND;
                                }
                            }
                            else {
                                throw gcnew ArgumentException("INTEGER: Invalid 'LSBitPadding' value. Must be 'Zero' or 'One' or 'Background'!");
                            }
                        }

                        if (sign == H5T_SGN_2) {
                            result = H5Tcopy(H5T_NATIVE_CHAR);
                        }
                        else {
                            result = H5Tcopy(H5T_NATIVE_UCHAR);
                        }


                        if (H5Tset_precision(result, precision) < 0) { // TODO
                        }
                        if (H5Tset_offset(result, offset) < 0) { // TODO
                        }
                        if (H5Tset_order(result, order) < 0) { // TODO
                        }
                        if (H5Tset_pad(result, lsb, msb) < 0) { // TODO
                        }

#pragma endregion
                    }
                    else if (type_class == "OPAQUE")
                    {
#pragma region OPAQUE
                        size_t size = 0;
                        if (ht->ContainsKey("Bytes")) // TODO: check size < 64KB
                        {
                            if (TryGetValue(ht["Bytes"], size))
                            {
                                if (ht->ContainsKey("Tag"))
                                {
                                    String^ tag = ht["Tag"]->ToString();
                                    char* label = (char*)(Marshal::StringToHGlobalAnsi(tag)).ToPointer();
                                    if (strlen(label) < H5T_OPAQUE_TAG_MAX)
                                    {
                                        result = H5Tcreate(H5T_OPAQUE, size);
                                        if (H5Tset_tag(result, label) < 0) {
                                            throw gcnew InvalidOperationException("OPAQUE: H5Tset_tag failed.");
                                        }
                                    }
                                    else {
                                        throw gcnew ArgumentException(String::Format("OPAQUE: Invalid tag found: '{0}'", tag));
                                    }
                                }
                                else {
                                    result = H5Tcreate(H5T_OPAQUE, size);
                                }
                            }
                            else {
                                throw gcnew ArgumentException("OPAQUE: Unable to convert the 'Bytes' value into size_t.");
                            }
                        }
                        else {
                            throw gcnew ArgumentException("OPAQUE: No 'Bytes' key found.");
                        }
#pragma endregion
                    }
                    else if (type_class == "REFERENCE")
                    {
#pragma region REFERENCE

                        if (ht->ContainsKey("Type"))
                        {
                            String^ ref_type = ht["Type"]->ToString()->ToUpper();
                            if (ref_type == "REF_OBJ") {
                                result = H5Tcopy(H5T_STD_REF_OBJ);
                            }
                            else if (ref_type == "REF_DSETREG") {
                                result = H5Tcopy(H5T_STD_REF_DSETREG);
                            }
                            else {
                                throw gcnew ArgumentException(
                                    String::Format("REFERENCE: Invalid reference type '{0}'.", ref_type));
                            }
                        }
                        else {
                            throw gcnew ArgumentException("REFERENCE: No 'Type' key found.");
                        }

#pragma endregion
                    }
                    else if (type_class == "STRING")
                    {
#pragma region STRING

                        if (ht->ContainsKey("CharSet") && ht["CharSet"]->ToString()->ToUpper() != "ASCII") {
                            throw gcnew ArgumentException(
                                    String::Format("STRING: Invalid character set '{0}'. ASCII only!",
                                    ht["CharSet"]->ToString()));
                        }

                        result = H5Tcopy(H5T_C_S1);
                        if (result < 0) {
                            throw gcnew ArgumentException("STRING: H5Tcopy failed.");
                        }

                        if (ht->ContainsKey("StrPad"))
                        {
                            String^ strPad = ht["StrPad"]->ToString()->ToUpper();
                            if (strPad == "NULLTERM") {
                                if (H5Tset_strpad(result, H5T_STR_NULLTERM) < 0) {
                                    if (H5Tclose(result) < 0) { // TODO
                                    }
                                    result = -1;
                                    throw gcnew ArgumentException("STRING: H5Tset_strpad failed.");
                                }
                            }
                            else if (strPad == "NULLPAD") {
                                if (H5Tset_strpad(result, H5T_STR_NULLPAD) < 0) {
                                    if (H5Tclose(result) < 0) { // TODO
                                    }
                                    result = -1;
                                    throw gcnew ArgumentException("STRING: H5Tset_strpad failed.");
                                }
                            }
                            else if (strPad == "SPACEPAD") {
                                if (H5Tset_strpad(result, H5T_STR_SPACEPAD) < 0) {
                                    if (H5Tclose(result) < 0) { // TODO
                                    }
                                    result = -1;
                                    throw gcnew ArgumentException("STRING: H5Tset_strpad failed.");
                                }
                            }
                            else {
                                if (H5Tclose(result) < 0) { // TODO
                                }
                                result = -1;
                                throw gcnew ArgumentException(
                                    String::Format("STRING: Invalid string padding '{0}'. Must be 'NULLTERM', 'NULLPAD', or 'SPACEPAD'!",
                                    strPad));
                            }
                        }

                        if (ht->ContainsKey("Length"))
                        {
                            size_t length = 0;
                            if (TryGetValue(ht["Length"], length))
                            {
                                if (H5Tset_size(result, length) < 0) {
                                    result = -1;
                                    throw gcnew ArgumentException("STRING: H5Tset_size failed.");
                                }
                            }
                            else {
                                throw gcnew ArgumentException("STRING: Invalid string length!");
                            }
                        }
                        else  // variable length string
                        {
                            if (H5Tset_size(result, H5T_VARIABLE) < 0) {
                                result = -1;
                                throw gcnew ArgumentException("STRING: H5Tset_size failed.");
                            }
                        }

#pragma endregion
                    }
                    else if (type_class == "VLEN")
                    {
#pragma region VLEN

                        if (ht->ContainsKey("Type"))
                        {
                            String^ ref_type = ht["Type"]->ToString()->ToUpper();
                            hid_t base_type = ParseH5Type(ht["Type"]);
                            if (base_type >= 0) {
                                result = H5Tvlen_create(base_type);
                            }
                            else {
                                throw gcnew ArgumentException(
                                    String::Format("VLEN: Unsupported base type: '{0}'", ref_type));
                            }
                        }
                        else {
                            throw gcnew ArgumentException("VLEN: No 'Type' key found.");
                        }

#pragma endregion
                    }
                }
                else {
                    throw gcnew ArgumentException(
                        String::Format("Unsupported type class '{0}' found.", type_class));
                }
            }
            else if (ht->ContainsKey("Type"))
            {
                result = ProviderUtils::ParseH5Type(ht["Type"]);
            }
            else {
                throw gcnew ArgumentException("No 'Class' or 'Type' key found.");
            }
        }
        else
        {
            String^ str = nullptr;
            if (ProviderUtils::TryGetValue(obj, str))
            {
                result = H5Type(str);
            }
        }

        if (result < 0) {
            throw gcnew ArgumentException("The argument provided cannot be converted into an HDF5 datatype type.");
        }
        
        return result;
    }


    bool ProviderUtils::IsSupportedH5Type(String^ type)
    {
        String^ t = type->ToUpper();

        if (t == "SYSTEM.COLLECTIONS.HASHTABLE")
        {
            return true;
        }

        if (m_known_types->ContainsKey(t))
        {
            return true;
        }

        return false;
    }

    hid_t ProviderUtils::H5Type(String^ type)
    {
        hid_t result = -1;

        String^ t = type->Trim()->ToUpper();

        if (m_known_types->ContainsKey(t))
        {
            result = H5Tcopy((hid_t) m_known_types[t]);
        }
        else if (t->StartsWith("S"))
        {
            if (t == "STRING") // default is variable-length C-string
            {
                result = H5Tcopy(H5T_C_S1);
                if (result >= 0) {
                    if (H5Tset_size(result, H5T_VARIABLE) < 0) { // TODO
                        if (H5Tclose(result) < 0) { // TODO
                        }
                        result = -1;
                    }
                }
                else { // TODO
                }
            }
        }

        if (result < 0) {
            throw gcnew ArgumentException(
                String::Format("Unknown type: '{0}'", t));
        }

        return result;
    }

    String^ ProviderUtils::H5NativeType2String(hid_t ntype)
    {
        String^ result = nullptr;

        if      (H5Tequal(ntype, H5T_NATIVE_CHAR) > 0)    { result = "CHAR";    }
        else if (H5Tequal(ntype, H5T_NATIVE_SHORT) > 0)   { result = "SHORT";   }
        else if (H5Tequal(ntype, H5T_NATIVE_INT) > 0)     { result = "INT";     }
        else if (H5Tequal(ntype, H5T_NATIVE_LONG) > 0)    { result = "LONG";    }
        else if (H5Tequal(ntype, H5T_NATIVE_LLONG) > 0)   { result = "LLONG";   }
        else if (H5Tequal(ntype, H5T_NATIVE_UCHAR) > 0)   { result = "UCHAR";   }
        else if (H5Tequal(ntype, H5T_NATIVE_USHORT) > 0)  { result = "USHORT";  }
        else if (H5Tequal(ntype, H5T_NATIVE_UINT) > 0)    { result = "UINT";    }
        else if (H5Tequal(ntype, H5T_NATIVE_ULONG) > 0)   { result = "ULONG";   }
        else if (H5Tequal(ntype, H5T_NATIVE_ULLONG) > 0)  { result = "ULLONG";  }
        else if (H5Tequal(ntype, H5T_NATIVE_FLOAT) > 0)   { result = "FLOAT";   }
        else if (H5Tequal(ntype, H5T_NATIVE_DOUBLE) > 0)  { result = "DOUBLE";  }
        else if (H5Tequal(ntype, H5T_NATIVE_LDOUBLE) > 0) { result = "LDOUBLE"; }
        else if (H5Tequal(ntype, H5T_NATIVE_B8) > 0)      { result = "B8";      }
        else if (H5Tequal(ntype, H5T_NATIVE_B16) > 0)     { result = "B16";     }
        else if (H5Tequal(ntype, H5T_NATIVE_B32) > 0)     { result = "B32";     }
        else if (H5Tequal(ntype, H5T_NATIVE_B64) > 0)     { result = "B64";     }

        return result;
    }

    Type^ ProviderUtils::H5NativeType2DotNet(hid_t ntype)
    {
        Type^ result = nullptr;

        H5T_class_t cls = H5Tget_class(ntype);

        if (cls == H5T_INTEGER)
        {
            if      (H5Tequal(ntype, H5T_NATIVE_CHAR)   > 0) { result = SByte::typeid;  }
            else if (H5Tequal(ntype, H5T_NATIVE_SHORT)  > 0) { result = Int16::typeid;  }
            else if (H5Tequal(ntype, H5T_NATIVE_INT)    > 0) { result = Int32::typeid;  }
            else if (H5Tequal(ntype, H5T_NATIVE_LONG)   > 0) { result = Int32::typeid;  }
            else if (H5Tequal(ntype, H5T_NATIVE_LLONG)  > 0) { result = Int64::typeid;  }
            else if (H5Tequal(ntype, H5T_NATIVE_UCHAR)  > 0) { result = Byte::typeid;   }
            else if (H5Tequal(ntype, H5T_NATIVE_USHORT) > 0) { result = UInt16::typeid; }
            else if (H5Tequal(ntype, H5T_NATIVE_UINT)   > 0) { result = UInt32::typeid; }
            else if (H5Tequal(ntype, H5T_NATIVE_ULONG)  > 0) { result = UInt32::typeid; }
            else if (H5Tequal(ntype, H5T_NATIVE_ULLONG) > 0) { result = UInt64::typeid; }
        }
        else if (cls == H5T_FLOAT)
        {
            if      (H5Tequal(ntype, H5T_NATIVE_FLOAT) > 0)   { result = Single::typeid; }
            else if (H5Tequal(ntype, H5T_NATIVE_DOUBLE) > 0)  { result = Double::typeid; }
            else if (H5Tequal(ntype, H5T_NATIVE_LDOUBLE) > 0) { result = Double::typeid; }

        }
        else if (cls == H5T_STRING) {
            result = String::typeid;
        }
        else if (cls == H5T_BITFIELD)
        {
            if      (H5Tequal(ntype, H5T_NATIVE_B8) > 0)  { result = Byte::typeid;   }
            else if (H5Tequal(ntype, H5T_NATIVE_B16) > 0) { result = UInt16::typeid; }
            else if (H5Tequal(ntype, H5T_NATIVE_B32) > 0) { result = UInt32::typeid; }
            else if (H5Tequal(ntype, H5T_NATIVE_B64) > 0) { result = UInt64::typeid; }
        }
        else if (cls == H5T_OPAQUE) {
            result = array<Byte>::typeid;
        }


        return result;
    }

    hid_t ProviderUtils::DotNetType2H5Native(Type^ ntype, bool isBitfield)
    {
        hid_t result = -1;

        if      (ntype == SByte::typeid)  { result = H5T_NATIVE_CHAR;   }
        else if (ntype == Int16::typeid)  { result = H5T_NATIVE_SHORT;  }
        else if (ntype == Int32::typeid)  { result = H5T_NATIVE_INT;    }
        else if (ntype == Int64::typeid)  { result = H5T_NATIVE_LLONG;  }
        else if (ntype == Byte::typeid)   { result = (isBitfield) ? H5T_NATIVE_B8 : H5T_NATIVE_UCHAR;  }
        else if (ntype == UInt16::typeid) { result = (isBitfield) ? H5T_NATIVE_B16: H5T_NATIVE_USHORT; }
        else if (ntype == UInt32::typeid) { result = (isBitfield) ? H5T_NATIVE_B32: H5T_NATIVE_UINT;   }
        else if (ntype == UInt64::typeid) { result = (isBitfield) ? H5T_NATIVE_B64: H5T_NATIVE_ULLONG; }
        else if (ntype == Single::typeid) { result = H5T_NATIVE_FLOAT;  }
        else if (ntype == Double::typeid) { result = H5T_NATIVE_DOUBLE; }
        
        return result;
    }
}