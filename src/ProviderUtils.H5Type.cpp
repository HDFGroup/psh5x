
#include "HDF5Exception.h"
#include "ProviderUtils.h"
#include "PSH5XException.h"

extern "C" {
#include "H5Tpublic.h"
}

#include <cstdlib>
#include <cstring>

using namespace Microsoft::CSharp;

using namespace System;
using namespace System::CodeDom::Compiler;
using namespace System::Collections;
using namespace System::Management::Automation;
using namespace System::Reflection;
using namespace System::Runtime::InteropServices;
using namespace System::Text;

namespace PSH5X
{
    Hashtable^ ProviderUtils::ParseH5Type(hid_t type)
    {
        Hashtable^ result = gcnew Hashtable();
        Hashtable^ ht = nullptr;

        hid_t unwnd = -1;
        size_t size = 0, offset;
        int nmembers = 0, rank;
        unsigned ui;
        char* name;
        String^ s;
        hsize_t* dims = NULL;
        array<hsize_t>^ d;
        htri_t ierr;
        H5T_cset_t cset;
        H5T_str_t strpad;
        H5T_pad_t lsb, msb;
        H5T_sign_t sign;
        size_t spos, epos, esize, mpos, msize;
        
        char b;
        unsigned char B;
        short h;
        unsigned short H;
        int i;
        unsigned int I;
        long long l;
        unsigned long long L;

        try
        {            
            switch (H5Tget_class(type))
            {
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
                else {
                    throw gcnew HDF5Exception("H5Tget_pad failed!");
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

#pragma endregion

                break;

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
                else {
                    throw gcnew HDF5Exception("H5Tget_pad failed!");
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
                else {
                    throw gcnew HDF5Exception("H5Tget_fields failed!");
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

#pragma endregion

                break;

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
                    result["IsVariableLength"] = false;
                }
                else if (ierr > 0) {
                    result["IsVariableLength"] = true;
                }
                else {
                    throw gcnew HDF5Exception("H5Tis_variable_str faile!");
                }

#pragma endregion

                break;

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
                else {
                    throw gcnew HDF5Exception("H5Tget_pad failed!");
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

#pragma endregion

                break;

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

#pragma endregion

                break;


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
                        if (H5Tclose(unwnd) < 0) {
                            throw gcnew HDF5Exception("H5Tclose failed!");
                        }
                        else {
                            unwnd = -1;
                        }
                    }
                }
                result["Members"] = ht;

#pragma endregion

                break;

            case H5T_ENUM:

#pragma region Enum

                result["Class"] = "ENUM";
                size = H5Tget_size(type);
                if (size != 1 && size != 2 && size != 4 && size != 8) {
                    throw gcnew PSH5XException("Unsupported enum size!");
                }
                result["Size"] = size;
                unwnd = H5Tget_super(type);
                result["Type"] = ProviderUtils::ParseH5Type(unwnd);
                sign = H5Tget_sign(unwnd);
                
                ht = gcnew Hashtable();
                nmembers = H5Tget_nmembers(type);
                if (nmembers < 0) {
                    throw gcnew HDF5Exception("H5Tget_nmembers failed!");
                }
                for (ui = 0; ui < safe_cast<unsigned>(nmembers); ++ui)
                {
                    name = H5Tget_member_name(type, ui);
                    s = gcnew String(name);

                    if (sign == H5T_SGN_NONE)
                    {
                        switch (size)
                        {
                        case 1:

                            if (H5Tget_member_value(type, ui, &B) < 0) {
                                throw gcnew HDF5Exception("H5Tget_member_value failed!");
                            }
                            ht[s] = B;
                            
                            break;

                        case 2:

                            if (H5Tget_member_value(type, ui, &H) < 0) {
                                throw gcnew HDF5Exception("H5Tget_member_value failed!");
                            }
                            ht[s] = H;
                            
                            break;

                        case 4:

                            if (H5Tget_member_value(type, ui, &I) < 0) {
                                throw gcnew HDF5Exception("H5Tget_member_value failed!");
                            }
                            ht[s] = I;
                            
                            break;

                        case 8:
                            
                            if (H5Tget_member_value(type, ui, &L) < 0) {
                                throw gcnew HDF5Exception("H5Tget_member_value failed!");
                            }
                            ht[s] = L;
                            
                            break;
                        }
                    }
                    else if (sign == H5T_SGN_2)
                    {
                        switch (size)
                        {
                        case 1:

                            if (H5Tget_member_value(type, ui, &b) < 0) {
                                throw gcnew HDF5Exception("H5Tget_member_value failed!");
                            }
                            ht[s] = b;
                            
                            break;

                        case 2:

                            if (H5Tget_member_value(type, ui, &h) < 0) {
                                throw gcnew HDF5Exception("H5Tget_member_value failed!");
                            }
                            ht[s] = h;
                            
                            break;

                        case 4:

                            if (H5Tget_member_value(type, ui, &i) < 0) {
                                throw gcnew HDF5Exception("H5Tget_member_value failed!");
                            }
                            ht[s] = i;
                            
                            break;

                        case 8:
                            
                            if (H5Tget_member_value(type, ui, &l) < 0) {
                                throw gcnew HDF5Exception("H5Tget_member_value failed!");
                            }
                            ht[s] = l;
                            
                            break;
                        }
                    }
                    else {
                        throw gcnew HDF5Exception("Unknown sign type!");
                    }
                }
                result["Members"] = ht;

#pragma endregion

                break;

            case H5T_VLEN:

#pragma region Vlen

                result["Class"] = "VLEN";

                unwnd = H5Tget_super(type);
                if (unwnd >= 0)
                {
                    result["Type"] = ProviderUtils::ParseH5Type(unwnd);
                    
                }

#pragma endregion

                break;

            case H5T_REFERENCE:

#pragma region Reference

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

#pragma endregion

                break;


            case H5T_ARRAY:

#pragma region Array

                result["Class"] = "ARRAY";
                rank = H5Tget_array_ndims(type);
                dims = new hsize_t [rank];
                rank = H5Tget_array_dims2(type, dims);
                d = gcnew array<hsize_t>(rank);
                for (i = 0; i < rank; ++i) {
                    d[i] = dims[i];
                }
                result["Dims"] = d;
                unwnd = H5Tget_super(type);
                if (unwnd >= 0)
                {
                    result["Type"] = ProviderUtils::ParseH5Type(unwnd);
                }
                else {
                    throw gcnew HDF5Exception("H5Tget_super failed!");
                }

#pragma endregion

                break;        

            default:
                throw gcnew PSH5XException("Unknown HDF5 datatype class!");
                break;
            }
        }
        finally
        {
            if (dims != NULL) {
                delete [] dims;
            }
            if (unwnd >= 0) {    
                H5Tclose(unwnd);
            }
        }

        return result;
    }

    hid_t ProviderUtils::ParseH5Type(Object^ obj)
    {
        hid_t result = -1, base_type = -1, dtype = -1;

        char *label = NULL, *name = NULL;

        Hashtable^ ht = nullptr;

        try
        {
            if (TryGetValue(obj, ht))
            {
                if (ht->ContainsKey("Class"))
                {
                    String^ type_class = ht["Class"]->ToString()->ToUpper();

                    if (Array::BinarySearch(m_type_classes, type_class) >= 0)
                    {
                        if (type_class == "INTEGER")
                        {
#pragma region INTEGER
                            if (ht->ContainsKey("Type")) {
                                result = ProviderUtils::ParseH5Type(ht["Type"]);
                                if (H5Tget_class(result) != H5T_INTEGER) {
                                    throw gcnew PSH5XException("INTEGER: The type specified is not of class INTEGER.");
                                }
                            }
                            else
                            {
                                size_t precision = 0;
                                if (ht->ContainsKey("Precision")) {
                                    if (!TryGetValue(ht["Precision"], precision)) {
                                        throw gcnew PSH5XException("INTEGER: Unable to convert the 'Precision' value into size_t.");
                                    }
                                }
                                else {
                                    throw gcnew PSH5XException("INTEGER: No 'Precision' key found.");
                                }

                                String^ s = nullptr;

                                H5T_order_t order = H5T_ORDER_LE;
                                if (ht->ContainsKey("ByteOrder")) {
                                    if (!TryGetValue(ht["ByteOrder"], s)) {
                                        throw gcnew PSH5XException("INTEGER: Unable to convert the 'ByteOrder' value into a string.");
                                    }

                                    if (s != nullptr)
                                    {
                                        s = s->Trim()->ToUpper();
                                        if (s != "BE" && s != "LE") {
                                            throw gcnew PSH5XException("INTEGER: Invalid 'ByteOrder' value. Must be 'BE' or 'LE'!");
                                        }
                                        if (s == "BE") {
                                            order = H5T_ORDER_BE;
                                        }
                                    }
                                    else {
                                        throw gcnew PSH5XException("INTEGER: Invalid 'ByteOrder' value. Must be 'BE' or 'LE'!");
                                    }
                                }

                                size_t offset = 0;
                                if (ht->ContainsKey("BitOffset")) {
                                    if (!TryGetValue(ht["BitOffset"], offset)) {
                                        throw gcnew PSH5XException("INTEGER: Unable to convert the 'BitOffset' value into size_t.");
                                    }
                                }

                                H5T_sign_t sign = H5T_SGN_2;
                                if (ht->ContainsKey("SignType")) {
                                    if (!TryGetValue(ht["SignType"], s)) {
                                        throw gcnew PSH5XException("INTEGER: Unable to convert the 'SignType' value into a string.");
                                    }

                                    if (s != nullptr)
                                    {
                                        s = s->Trim()->ToUpper();
                                        if (s != "NONE" && s != "2") {
                                            throw gcnew PSH5XException("INTEGER: Invalid 'SignType' value. Must be '2' or 'None'!");
                                        }
                                        if (s == "NONE") {
                                            sign = H5T_SGN_NONE;
                                        }
                                    }
                                    else {
                                        throw gcnew PSH5XException("INTEGER: Invalid 'SignType' value. Must be '2' or 'None'!");
                                    }
                                }

                                H5T_pad_t lsb = H5T_PAD_ZERO, msb = H5T_PAD_ZERO;
                                if (ht->ContainsKey("MSBitPadding")) {
                                    if (!TryGetValue(ht["MSBitPadding"], s)) {
                                        throw gcnew PSH5XException("INTEGER: Unable to convert the 'MSBitPadding' value into a string.");
                                    }

                                    if (s != nullptr)
                                    {
                                        s = s->Trim()->ToUpper();
                                        if (s != "ZERO" && s != "ONE" && s != "BACKGROUND") {
                                            throw gcnew PSH5XException("INTEGER: Invalid 'MSBitPadding' value. Must be 'Zero' or 'One' or 'Background'!");
                                        }

                                        if (s == "ONE") {
                                            msb = H5T_PAD_ONE;
                                        }
                                        else if (s == "BACKGROUND") {
                                            msb = H5T_PAD_BACKGROUND;
                                        }
                                    }
                                    else {
                                        throw gcnew PSH5XException("INTEGER: Invalid 'MSBitPadding' value. Must be 'Zero' or 'One' or 'Background'!");
                                    }
                                }
                                if (ht->ContainsKey("LSBitPadding")) {
                                    if (!TryGetValue(ht["LSBitPadding"], s)) {
                                        throw gcnew PSH5XException("INTEGER: Unable to convert the 'LSBitPadding' value into a string.");
                                    }

                                    if (s != nullptr)
                                    {
                                        s = s->Trim()->ToUpper();
                                        if (s != "ZERO" && s != "ONE" && s != "BACKGROUND") {
                                            throw gcnew PSH5XException("INTEGER: Invalid 'LSBitPadding' value. Must be 'Zero' or 'One' or 'Background'!");
                                        }

                                        if (s == "ONE") {
                                            msb = H5T_PAD_ONE;
                                        }
                                        else if (s == "BACKGROUND") {
                                            msb = H5T_PAD_BACKGROUND;
                                        }
                                    }
                                    else {
                                        throw gcnew PSH5XException("INTEGER: Invalid 'LSBitPadding' value. Must be 'Zero' or 'One' or 'Background'!");
                                    }
                                }

                                if (sign == H5T_SGN_2) {
                                    result = H5Tcopy(H5T_NATIVE_CHAR);
                                    if (result < 0) {
                                        throw gcnew HDF5Exception("H5Tcopy failed!");
                                    }
                                }
                                else {
                                    result = H5Tcopy(H5T_NATIVE_UCHAR);
                                    if (result < 0) {
                                        throw gcnew HDF5Exception("H5Tcopy failed!");
                                    }
                                }

                                if (H5Tset_precision(result, precision) < 0) {
                                    throw gcnew HDF5Exception("H5Tset_precision failed!");
                                }
                                if (H5Tset_offset(result, offset) < 0) {
                                    throw gcnew HDF5Exception("H5Tset_offset failed!");
                                }
                                if (H5Tset_order(result, order) < 0) {
                                    throw gcnew HDF5Exception("H5Tset_order failed!");
                                }
                                if (H5Tset_pad(result, lsb, msb) < 0) {
                                    throw gcnew HDF5Exception("H5Tset_pad failed!");
                                }
                            }

#pragma endregion
                        }
                        else if (type_class == "FLOAT")
                        {
#pragma region FLOAT
                            if (ht->ContainsKey("Type")) {
                                result = ProviderUtils::ParseH5Type(ht["Type"]);
                                if (H5Tget_class(result) != H5T_FLOAT) {
                                    throw gcnew PSH5XException("FLOAT: The type specified is not of class FLOAT.");
                                }
                            }
                            else
                            {

                                size_t offset = 0;
                                if (ht->ContainsKey("BitOffset")) {
                                    if (!TryGetValue(ht["BitOffset"], offset)) {
                                        throw gcnew PSH5XException("FLOAT: Unable to convert the 'BitOffset' value into size_t.");
                                    }
                                }

                                size_t precision = 0;
                                if (ht->ContainsKey("Precision")) {
                                    if (!TryGetValue(ht["Precision"], precision)) {
                                        throw gcnew PSH5XException("FLOAT: Unable to convert the 'Precision' value into size_t.");
                                    }
                                }
                                else {
                                    throw gcnew PSH5XException("FLOAT: No 'Precision' key found.");
                                }

                                String^ s = nullptr;

                                H5T_order_t order = H5T_ORDER_LE;
                                if (ht->ContainsKey("ByteOrder")) {
                                    if (!TryGetValue(ht["ByteOrder"], s)) {
                                        throw gcnew PSH5XException("FLOAT: Unable to convert the 'ByteOrder' value into a string.");
                                    }

                                    if (s != nullptr)
                                    {
                                        s = s->Trim()->ToUpper();
                                        if (s != "BE" && s != "LE") {
                                            throw gcnew PSH5XException("FLOAT: Invalid 'ByteOrder' value. Must be 'BE' or 'LE'!");
                                        }
                                        if (s == "BE") {
                                            order = H5T_ORDER_BE;
                                        }
                                    }
                                    else {
                                        throw gcnew PSH5XException("FLOAT: Invalid 'ByteOrder' value. Must be 'BE' or 'LE'!");
                                    }
                                }

                                H5T_pad_t lsb = H5T_PAD_ZERO, msb = H5T_PAD_ZERO, inp = H5T_PAD_ZERO;
                                if (ht->ContainsKey("MSBitPadding")) {
                                    if (!TryGetValue(ht["MSBitPadding"], s)) {
                                        throw gcnew PSH5XException("FLOAT: Unable to convert the 'MSBitPadding' value into a string.");
                                    }

                                    if (s != nullptr)
                                    {
                                        s = s->Trim()->ToUpper();
                                        if (s != "ZERO" && s != "ONE" && s != "BACKGROUND") {
                                            throw gcnew PSH5XException("FLOAT: Invalid 'MSBitPadding' value. Must be 'Zero' or 'One' or 'Background'!");
                                        }

                                        if (s == "ONE") {
                                            msb = H5T_PAD_ONE;
                                        }
                                        else if (s == "BACKGROUND") {
                                            msb = H5T_PAD_BACKGROUND;
                                        }
                                    }
                                    else {
                                        throw gcnew PSH5XException("FLOAT: Invalid 'MSBitPadding' value. Must be 'Zero' or 'One' or 'Background'!");
                                    }
                                }
                                if (ht->ContainsKey("LSBitPadding")) {
                                    if (!TryGetValue(ht["LSBitPadding"], s)) {
                                        throw gcnew PSH5XException("FLOAT: Unable to convert the 'LSBitPadding' value into a string.");
                                    }

                                    if (s != nullptr)
                                    {
                                        s = s->Trim()->ToUpper();
                                        if (s != "ZERO" && s != "ONE" && s != "BACKGROUND") {
                                            throw gcnew PSH5XException("FLOAT: Invalid 'LSBitPadding' value. Must be 'Zero' or 'One' or 'Background'!");
                                        }

                                        if (s == "ONE") {
                                            msb = H5T_PAD_ONE;
                                        }
                                        else if (s == "BACKGROUND") {
                                            msb = H5T_PAD_BACKGROUND;
                                        }
                                    }
                                    else {
                                        throw gcnew PSH5XException("FLOAT: Invalid 'LSBitPadding' value. Must be 'Zero' or 'One' or 'Background'!");
                                    }
                                }
                                if (ht->ContainsKey("IntlBitPadding")) {
                                    if (!TryGetValue(ht["IntlBitPadding"], s)) {
                                        throw gcnew PSH5XException("FLOAT: Unable to convert the 'IntlBitPadding' value into a string.");
                                    }

                                    if (s != nullptr)
                                    {
                                        s = s->Trim()->ToUpper();
                                        if (s != "ZERO" && s != "ONE" && s != "BACKGROUND") {
                                            throw gcnew PSH5XException("FLOAT: Invalid 'IntlBitPadding' value. Must be 'Zero' or 'One' or 'Background'!");
                                        }

                                        if (s == "ONE") {
                                            msb = H5T_PAD_ONE;
                                        }
                                        else if (s == "BACKGROUND") {
                                            msb = H5T_PAD_BACKGROUND;
                                        }
                                    }
                                    else {
                                        throw gcnew PSH5XException("FLOAT: Invalid 'IntlBitPadding' value. Must be 'Zero' or 'One' or 'Background'!");
                                    }
                                }

                                size_t spos = 31;
                                if (ht->ContainsKey("SignBitPos")) {
                                    if (!TryGetValue(ht["SignBitPos"], spos)) {
                                        throw gcnew PSH5XException("FLOAT: Unable to convert the 'SignBitPos' value into size_t.");
                                    }
                                }
                                size_t epos = 23;
                                if (ht->ContainsKey("ExpBitPos")) {
                                    if (!TryGetValue(ht["ExpBitPos"], epos)) {
                                        throw gcnew PSH5XException("FLOAT: Unable to convert the 'ExpBitPos' value into size_t.");
                                    }
                                }
                                size_t esize = 8;
                                if (ht->ContainsKey("ExpBits")) {
                                    if (!TryGetValue(ht["ExpBits"], esize)) {
                                        throw gcnew PSH5XException("FLOAT: Unable to convert the 'ExpBits' value into size_t.");
                                    }
                                }
                                size_t mpos = 0;
                                if (ht->ContainsKey("MantBitPos")) {
                                    if (!TryGetValue(ht["MantBitPos"], mpos)) {
                                        throw gcnew PSH5XException("FLOAT: Unable to convert the 'MantBitPos' value into size_t.");
                                    }
                                }
                                size_t msize = 23;
                                if (ht->ContainsKey("MantBits")) {
                                    if (!TryGetValue(ht["MantBits"], msize)) {
                                        throw gcnew PSH5XException("FLOAT: Unable to convert the 'MantBits' value into size_t.");
                                    }
                                }

                                size_t ebias = 127;
                                if (ht->ContainsKey("ExpBias")) {
                                    if (!TryGetValue(ht["ExpBias"], ebias)) {
                                        throw gcnew PSH5XException("FLOAT: Unable to convert the 'ExpBias' value into size_t.");
                                    }
                                }

                                H5T_norm_t norm = H5T_NORM_IMPLIED;
                                if (ht->ContainsKey("MantNorm")) {
                                    if (!TryGetValue(ht["MantNorm"], s)) {
                                        throw gcnew PSH5XException("FLOAT: Unable to convert the 'MantNorm' value into a string.");
                                    }

                                    if (s != nullptr)
                                    {
                                        s = s->Trim()->ToUpper();
                                        if (s != "IMPLIED" && s != "MSBSET" && s != "NONE") {
                                            throw gcnew PSH5XException("FLOAT: Invalid 'MantNorm' value. Must be 'Implied', 'MsbSet' or 'None'!");
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
                                        throw gcnew PSH5XException("FLOAT: Invalid 'MantNorm' value. Must be 'Implied', 'MsbSet' or 'None'!");
                                    }
                                }

                                // TODO: check constraints

                                result = H5Tcopy(H5T_NATIVE_FLOAT);
                                if (result < 0) {
                                    throw gcnew HDF5Exception("H5Tcopy failed!");
                                }

                                if (H5Tset_precision(result, precision) < 0) {
                                    throw gcnew HDF5Exception("H5Tset_precision failed!");
                                }
                                if (H5Tset_offset(result, offset) < 0) {
                                    throw gcnew HDF5Exception("H5Tset_offset failed!");
                                }
                                if (H5Tset_order(result, order) < 0) {
                                    throw gcnew HDF5Exception("H5Tset_order failed!");
                                }
                                if (H5Tset_pad(result, lsb, msb) < 0) {
                                    throw gcnew HDF5Exception("H5Tset_pad failed!");
                                }
                                if (H5Tset_inpad(result, inp) < 0) {
                                    throw gcnew HDF5Exception("H5Tset_inpad failed!");
                                }
                                if (H5Tset_fields(result, spos, epos, esize, mpos, msize) < 0) {
                                    throw gcnew HDF5Exception("H5Tset_fields failed!");
                                }
                                if (H5Tset_ebias(result, ebias) < 0) {
                                    throw gcnew HDF5Exception("H5Tset_ebias failed!");
                                }
                                if (H5Tset_norm(result, norm) < 0) {
                                    throw gcnew HDF5Exception("H5Tset_norm failed!");
                                }
                            }

#pragma endregion
                        }
                        else if (type_class == "STRING")
                        {
#pragma region STRING

                            if (ht->ContainsKey("CharSet") && ht["CharSet"]->ToString()->ToUpper() != "ASCII") {
                                throw gcnew PSH5XException(
                                    String::Format("STRING: Invalid character set '{0}'. ASCII only!",
                                    ht["CharSet"]->ToString()));
                            }

                            result = H5Tcopy(H5T_C_S1);
                            if (result < 0) {
                                throw gcnew HDF5Exception("STRING: H5Tcopy failed.");
                            }

                            if (ht->ContainsKey("StrPad"))
                            {
                                String^ strPad = ht["StrPad"]->ToString()->ToUpper();
                                if (strPad == "NULLTERM") {
                                    if (H5Tset_strpad(result, H5T_STR_NULLTERM) < 0) {
                                        throw gcnew HDF5Exception("STRING: H5Tset_strpad failed.");
                                    }
                                }
                                else if (strPad == "NULLPAD") {
                                    if (H5Tset_strpad(result, H5T_STR_NULLPAD) < 0) {                                       
                                        throw gcnew HDF5Exception("STRING: H5Tset_strpad failed.");
                                    }
                                }
                                else if (strPad == "SPACEPAD") {
                                    if (H5Tset_strpad(result, H5T_STR_SPACEPAD) < 0) {
                                        throw gcnew HDF5Exception("STRING: H5Tset_strpad failed.");
                                    }
                                }
                                else {
                                    throw gcnew PSH5XException(
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
                                        throw gcnew HDF5Exception("STRING: H5Tset_size failed.");
                                    }
                                }
                                else {
                                    throw gcnew PSH5XException("STRING: Invalid string length!");
                                }
                            }
                            else  // variable length string
                            {
                                if (H5Tset_size(result, H5T_VARIABLE) < 0) {
                                    throw gcnew HDF5Exception("STRING: H5Tset_size failed.");
                                }
                            }

#pragma endregion
                        }
                        else if (type_class == "BITFIELD")
                        {
#pragma region BITFIELD

                            if (ht->ContainsKey("Type")) {
                                result = ProviderUtils::ParseH5Type(ht["Type"]);
                                if (H5Tget_class(result) != H5T_BITFIELD) {
                                    throw gcnew PSH5XException("BITFIELD: The type specified is not of class BITFIELD.");
                                }
                            }
                            else
                            {
                                size_t precision = 0;
                                if (ht->ContainsKey("Precision")) {
                                    if (!TryGetValue(ht["Precision"], precision)) {
                                        throw gcnew PSH5XException("BITFIELD: Unable to convert the 'Precision' value into size_t.");
                                    }
                                }
                                else {
                                    throw gcnew PSH5XException("BITFIELD: No 'Precision' key found.");
                                }

                                String^ s = nullptr;

                                H5T_order_t order = H5T_ORDER_LE;
                                if (ht->ContainsKey("ByteOrder")) {
                                    if (!TryGetValue(ht["ByteOrder"], s)) {
                                        throw gcnew PSH5XException("BITFIELD: Unable to convert the 'ByteOrder' value into a string.");
                                    }

                                    if (s != nullptr)
                                    {
                                        s = s->Trim()->ToUpper();
                                        if (s != "BE" && s != "LE") {
                                            throw gcnew PSH5XException("BITFIELD: Invalid 'ByteOrder' value. Must be 'BE' or 'LE'!");
                                        }
                                        if (s == "BE") {
                                            order = H5T_ORDER_BE;
                                        }
                                    }
                                    else {
                                        throw gcnew PSH5XException("BITFIELD: Invalid 'ByteOrder' value. Must be 'BE' or 'LE'!");
                                    }
                                }

                                size_t offset = 0;
                                if (ht->ContainsKey("BitOffset")) {
                                    if (!TryGetValue(ht["BitOffset"], offset)) {
                                        throw gcnew PSH5XException("BITFIELD: Unable to convert the 'BitOffset' value into size_t.");
                                    }
                                }

                                H5T_pad_t lsb = H5T_PAD_ZERO, msb = H5T_PAD_ZERO;
                                if (ht->ContainsKey("MSBitPadding")) {
                                    if (!TryGetValue(ht["MSBitPadding"], s)) {
                                        throw gcnew PSH5XException("BITFIELD: Unable to convert the 'MSBitPadding' value into a string.");
                                    }

                                    if (s != nullptr)
                                    {
                                        s = s->Trim()->ToUpper();
                                        if (s != "ZERO" && s != "ONE" && s != "BACKGROUND") {
                                            throw gcnew PSH5XException("BITFIELD: Invalid 'MSBitPadding' value. Must be 'Zero' or 'One' or 'Background'!");
                                        }

                                        if (s == "ONE") {
                                            msb = H5T_PAD_ONE;
                                        }
                                        else if (s == "BACKGROUND") {
                                            msb = H5T_PAD_BACKGROUND;
                                        }
                                    }
                                    else {
                                        throw gcnew PSH5XException("BITFIELD: Invalid 'MSBitPadding' value. Must be 'Zero' or 'One' or 'Background'!");
                                    }
                                }
                                if (ht->ContainsKey("LSBitPadding")) {
                                    if (!TryGetValue(ht["LSBitPadding"], s)) {
                                        throw gcnew PSH5XException("BITFIELD: Unable to convert the 'LSBitPadding' value into a string.");
                                    }

                                    if (s != nullptr)
                                    {
                                        s = s->Trim()->ToUpper();
                                        if (s != "ZERO" && s != "ONE" && s != "BACKGROUND") {
                                            throw gcnew PSH5XException("BITFIELD: Invalid 'LSBitPadding' value. Must be 'Zero' or 'One' or 'Background'!");
                                        }

                                        if (s == "ONE") {
                                            msb = H5T_PAD_ONE;
                                        }
                                    else if (s == "BACKGROUND") {
                                        msb = H5T_PAD_BACKGROUND;
                                    }
                                }
                                else {
                                    throw gcnew PSH5XException("BITFIELD: Invalid 'LSBitPadding' value. Must be 'Zero' or 'One' or 'Background'!");
                                }
                            }

                            result = H5Tcopy(H5T_NATIVE_B8);
                            if (result < 0) {
                                throw gcnew HDF5Exception("H5Tcopy failed!");
                            }

                            if (H5Tset_precision(result, precision) < 0) {
                                throw gcnew HDF5Exception("H5Tset_precision failed!");
                            }
                            if (H5Tset_offset(result, offset) < 0) {
                                throw gcnew HDF5Exception("H5Tset_offset failed!");
                            }
                            if (H5Tset_order(result, order) < 0) {
                                throw gcnew HDF5Exception("H5Tset_order failed!");
                            }
                            if (H5Tset_pad(result, lsb, msb) < 0) {
                                throw gcnew HDF5Exception("H5Tset_pad failed!");
                            }
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
                                        label = (char*)(Marshal::StringToHGlobalAnsi(tag)).ToPointer();
                                        if (strlen(label) < H5T_OPAQUE_TAG_MAX)
                                        {
                                            result = H5Tcreate(H5T_OPAQUE, size);
                                            if (result < 0) {
                                                throw gcnew HDF5Exception("OPAQUE: H5Tcreate failed.");
                                            }
                                            if (H5Tset_tag(result, label) < 0) {
                                                throw gcnew HDF5Exception("OPAQUE: H5Tset_tag failed.");
                                            }
                                        }
                                        else {
                                            throw gcnew PSH5XException(String::Format("OPAQUE: Invalid tag found: '{0}'", tag));
                                        }
                                    }
                                    else {
                                        result = H5Tcreate(H5T_OPAQUE, size);
                                        if (result < 0) {
                                            throw gcnew HDF5Exception("OPAQUE: H5Tcreate failed.");
                                        }
                                    }
                                }
                                else {
                                    throw gcnew PSH5XException("OPAQUE: Unable to convert the 'Bytes' value into size_t.");
                                }
                            }
                            else {
                                throw gcnew PSH5XException("OPAQUE: No 'Bytes' key found.");
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
                                    if (result < 0) {
                                        throw gcnew HDF5Exception("COMPOUND: H5Tcreate failed.");
                                    }

                                    if (ht->ContainsKey("Members"))
                                    {
                                        Hashtable^ components = nullptr;

                                        if (TryGetValue(ht["Members"], components))
                                        {
                                            for each (String^ key in components->Keys)
                                            {
                                                name = (char*)(Marshal::StringToHGlobalAnsi(key)).ToPointer();

                                                Hashtable^ comp = nullptr;

                                                if (TryGetValue(components[key], comp))
                                                {
                                                    size_t offset = 0;
                                                    if (comp->ContainsKey("MemberOffset"))
                                                    {
                                                        if (ProviderUtils::TryGetValue(comp["MemberOffset"], offset))
                                                        {
                                                            dtype = ParseH5Type(comp);
                                                            if (dtype >= 0)
                                                            {
                                                                if (H5Tinsert(result, name, offset, dtype) < 0)
                                                                {
                                                                    throw gcnew HDF5Exception(
                                                                        String::Format("COMPOUND: H5Tinsert failed on component '{0}'!", key));
                                                                }
                                                            }
                                                            else {
                                                                throw gcnew PSH5XException(
                                                                    String::Format("COMPOUND: Unsupported component type '{0}' for component '{1}'",
                                                                    comp, key));
                                                            }
                                                            
                                                            if (H5Tclose(dtype)) {
                                                                throw gcnew HDF5Exception("H5Tclose failed!");
                                                            }
                                                            dtype = -1;
                                                        }
                                                        else {
                                                            throw gcnew PSH5XException(
                                                                String::Format("COMPOUND: Unable to convert the 'MemberOffset' value into size_t for component '{0}'", key));
                                                        }
                                                    }
                                                    else {
                                                        throw gcnew PSH5XException(
                                                            String::Format("COMPOUND: No 'MemberOffset' key found for component '{0}'.", key));
                                                    }
                                                }
                                                else {
                                                    throw gcnew PSH5XException(
                                                        String::Format("COMPOUND: Member '{0}' does not contain a hashtable.", key));
                                                }

                                                Marshal::FreeHGlobal(IntPtr(name));
                                                name = NULL;
                                            }
                                        }
                                        else {
                                            throw gcnew PSH5XException("COMPOUND: 'Members' does not contain a hashtable.");
                                        }
                                    }
                                    else {
                                        throw gcnew PSH5XException("COMPOUND: No 'Members' key found.");
                                    }
                                }
                                else {
                                    throw gcnew PSH5XException("COMPOUND: Unable to convert the 'Size' value into size_t.");
                                }
                            }
                            else {
                                throw gcnew PSH5XException("COMPOUND: No 'Size' key found.");
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
                                    if (result < 0) {
                                        throw gcnew HDF5Exception("H5Tcopy failed!");
                                    }
                                }
                                else if (ref_type == "REF_DSETREG") {
                                    result = H5Tcopy(H5T_STD_REF_DSETREG);
                                    if (result < 0) {
                                        throw gcnew HDF5Exception("H5Tcopy failed!");
                                    }
                                }
                                else {
                                    throw gcnew PSH5XException(
                                        String::Format("REFERENCE: Invalid reference type '{0}'.", ref_type));
                                }
                            }
                            else {
                                throw gcnew PSH5XException("REFERENCE: No 'Type' key found.");
                            }

#pragma endregion
                        }
                        else if (type_class == "ENUM")
                        {
#pragma region ENUM
                            // TODO: finish this

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
                                            name = (char*)(Marshal::StringToHGlobalAnsi(key)).ToPointer();
                                            if (H5Tenum_insert(result, name, &value) < 0)
                                            {
                                            }
                                            Marshal::FreeHGlobal(IntPtr(name));
                                            name = NULL;
                                        }
                                        else {
                                            throw gcnew PSH5XException(
                                                String::Format("ENUM: The value of '{0}' is not integer.", key));
                                        }
                                    }
                                }
                                else {
                                    throw gcnew PSH5XException("ENUM: The value of the 'Members' key is not a hashtable.");
                                }
                            }
                            else {
                                throw gcnew PSH5XException("ENUM: No 'Members' key found.");
                            }

#pragma endregion
                        }                        
                        else if (type_class == "VLEN")
                        {
#pragma region VLEN

                            if (ht->ContainsKey("Type"))
                            {
                                String^ ref_type = ht["Type"]->ToString()->ToUpper();
                                base_type = ParseH5Type(ht["Type"]);
                                if (base_type < 0) {
                                    throw gcnew PSH5XException(
                                        String::Format("VLEN: Invalid base type '{0}'!", ref_type));
                                }

                                result = H5Tvlen_create(base_type);
                                if (result < 0) {
                                    throw gcnew HDF5Exception("H5Tvlen_create failed!");
                                }
                            }
                            else {
                                throw gcnew PSH5XException("VLEN: No 'Type' key found.");
                            }

#pragma endregion
                        }
                        else if (type_class == "ARRAY")
                        {
#pragma region ARRAY

                            if (ht->ContainsKey("Type"))
                            {
                                base_type = ParseH5Type(ht["Type"]);
                                if (base_type < 0) {
                                    throw gcnew PSH5XException("ARRAY: No 'Type' key found.");
                                }

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

                                                if (dims[i] == 0) {
                                                    throw gcnew PSH5XException(
                                                        "ARRAY: Dimensions must be positive.");
                                                }
                                            }
                                            result = H5Tarray_create2(base_type, rank, tmp);
                                            if (result < 0) {
                                                throw gcnew HDF5Exception("H5Tarray_create2 failed!");
                                            }
                                        }
                                        else {
                                            throw gcnew PSH5XException(
                                                "ARRAY: The 'Dims' array's rank must not exceed 4.");
                                        }
                                    }
                                    else {
                                        throw gcnew PSH5XException(
                                            "ARRAY: 'Dims' key has invalid value.");
                                    }
                                }
                                else {
                                    throw gcnew PSH5XException("ARRAY: No 'Dims' key found.");
                                }
                            }
                            else {
                                throw gcnew PSH5XException(
                                    String::Format("ARRAY: Unsupported base type: '{0}'", ht["Type"]));
                            }

#pragma endregion
                        }
                    }
                    else {
                        throw gcnew PSH5XException(
                            String::Format("Unsupported type class '{0}' found.", type_class));
                    }
                }
                else if (ht->ContainsKey("Type"))
                {
                    result = ProviderUtils::ParseH5Type(ht["Type"]);
                    if (result < 0) {
                        throw gcnew PSH5XException("Invalid type!");
                    }
                }
                else {
                    throw gcnew PSH5XException("No 'Class' or 'Type' key found.");
                }
            }
            else
            {
                String^ str = nullptr;
                if (ProviderUtils::TryGetValue(obj, str))
                {
                    result = H5Type(str);
                    if (result < 0) {
                        throw gcnew PSH5XException("Invalid type!");
                    }
                }
            }

            if (result < 0) {
                throw gcnew PSH5XException("The argument provided cannot be converted into an HDF5 datatype type.");
            }
        }
        finally
        {
            if (base_type >= 0) {
                H5Tclose(base_type);
            }
            if (dtype >= 0) {
                H5Tclose(dtype);
            }
            if (label != NULL) {
                Marshal::FreeHGlobal(IntPtr(label));
            }
            if (name != NULL) {
                Marshal::FreeHGlobal(IntPtr(name));
            }
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
            if (result < 0) {
                throw gcnew HDF5Exception("H5Tcopy failed!");
            }
        }
        else if (t->StartsWith("S"))
        {
            if (t == "STRING") // default is variable-length C-string
            {
                result = H5Tcopy(H5T_C_S1);
                if (result < 0) {
                    throw gcnew HDF5Exception("H5Tcopy failed!");
                }
                if (H5Tset_size(result, H5T_VARIABLE) < 0) {
                    throw gcnew HDF5Exception("H5Tset_size failed!");
                }
            }
        }

        if (result < 0) {
            throw gcnew PSH5XException(
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

    Type^ ProviderUtils::H5Type2DotNet(hid_t dtype)
    {
        Type^ result = nullptr;

        H5T_class_t cls = H5Tget_class(dtype);

        hid_t ntype = -1, super = -1;

        try
        {
            if (cls == H5T_BITFIELD) {
                ntype = H5Tget_native_type(dtype, H5T_DIR_DESCEND);
            }
            else {
                ntype = H5Tget_native_type(dtype, H5T_DIR_ASCEND);
            }
            if (ntype < 0) {
                throw gcnew HDF5Exception("H5Tget_native_type failed!");
            }

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
                result = Byte::typeid;
            }
            else if (cls == H5T_ENUM)
            {
                super = H5Tget_super(ntype);

                if      (H5Tequal(super, H5T_NATIVE_CHAR)   > 0) { result = SByte::typeid;  }
                else if (H5Tequal(super, H5T_NATIVE_SHORT)  > 0) { result = Int16::typeid;  }
                else if (H5Tequal(super, H5T_NATIVE_INT)    > 0) { result = Int32::typeid;  }
                else if (H5Tequal(super, H5T_NATIVE_LONG)   > 0) { result = Int32::typeid;  }
                else if (H5Tequal(super, H5T_NATIVE_LLONG)  > 0) { result = Int64::typeid;  }
                else if (H5Tequal(super, H5T_NATIVE_UCHAR)  > 0) { result = Byte::typeid;   }
                else if (H5Tequal(super, H5T_NATIVE_USHORT) > 0) { result = UInt16::typeid; }
                else if (H5Tequal(super, H5T_NATIVE_UINT)   > 0) { result = UInt32::typeid; }
                else if (H5Tequal(super, H5T_NATIVE_ULONG)  > 0) { result = UInt32::typeid; }
                else if (H5Tequal(super, H5T_NATIVE_ULLONG) > 0) { result = UInt64::typeid; }
            }
        }
        finally
        {
            if (ntype >= 0) {
                H5Tclose(ntype);
            }
            if (super >= 0) {
                H5Tclose(super);
            }
        }

        return result;
    }

    __wchar_t ProviderUtils::TypeCode(Type^ t)
    {
        if (t == System::String::typeid) {
            return 's';
        }
        else if (t == System::Single::typeid) {
            return 'f';
        }
        else if (t == System::Double::typeid) {
            return 'd';
        }
        else if (t == System::SByte::typeid) {
            return 'b';
        }
        else if (t == System::Byte::typeid) {
            return 'B';
        }
        if (t == System::Int16::typeid) {
            return 'h';
        }
        else if (t == System::UInt16::typeid) {
            return 'H';
        }
        if (t == System::Int32::typeid) {
            return 'i';
        }
        else if (t == System::UInt32::typeid) {
            return 'I';
        }
        if (t == System::Int64::typeid) {
            return 'l';
        }
        else if (t == System::UInt64::typeid) {
            return 'L';
        }
        else {
            return '\0';
        }
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

    Array^ ProviderUtils::GetPSObjectArray(long long length,
            array<String^>^ mname, array<Type^>^ mtype)
    {
        Exception^ ex = nullptr;

        Array^ result = nullptr;

        if (mname->Length != mtype->Length) {
            ex = gcnew ArgumentException("Argument length mismatch!!!");
            goto error;
        }
        
        CompilerParameters^ params = gcnew CompilerParameters();
        params->GenerateInMemory = true;
        params->TreatWarningsAsErrors = false;
        params->GenerateExecutable = false;
        params->CompilerOptions = "/optimize";

        String^ code = "public class Point { public double x {get; set;} public double y {get; set;} public Point() {x=0;y=0;} }";

        CSharpCodeProvider^ provider = gcnew CSharpCodeProvider();
        CompilerResults^ compilate = provider->CompileAssemblyFromSource(params, code);

        Assembly^ assembly = compilate->CompiledAssembly;
        Type^ t = assembly->GetType("Point");
        
        result = Array::CreateInstance(t, length);
        for (long long i = 0; i < length; ++i)
        {
            result->SetValue(Activator::CreateInstance(t), i);
        }

error:

        if (ex != nullptr) {
            throw ex;
        }

        return result;
    }

    MethodInfo^ ProviderUtils::BitConverterMethod(hid_t type_id)
    {
        MethodInfo^ minfo = nullptr;

        Type^ magicType = System::BitConverter::typeid;

        hid_t super = -1;

        try
        {
            H5T_class_t cls = H5Tget_class(type_id);
            switch (cls)
            {
            case H5T_INTEGER:

                if (ProviderUtils::H5Type2DotNet(type_id) == Int32::typeid) {
                    minfo = magicType->GetMethod("ToInt32");
                }
                else if (ProviderUtils::H5Type2DotNet(type_id) == Int64::typeid) {
                    minfo = magicType->GetMethod("ToInt64");
                }
                else if (ProviderUtils::H5Type2DotNet(type_id) == Int16::typeid) {
                    minfo = magicType->GetMethod("ToInt16");
                }
                if (ProviderUtils::H5Type2DotNet(type_id) == UInt32::typeid) {
                    minfo = magicType->GetMethod("ToUInt32");
                }
                else if (ProviderUtils::H5Type2DotNet(type_id) == UInt64::typeid) {
                    minfo = magicType->GetMethod("ToUInt64");
                }
                else if (ProviderUtils::H5Type2DotNet(type_id) == UInt16::typeid) {
                    minfo = magicType->GetMethod("ToUInt16");
                }
                break;

            case H5T_FLOAT:

                if (ProviderUtils::H5Type2DotNet(type_id) == Double::typeid) {
                    minfo = magicType->GetMethod("ToDouble");
                }
                else if (ProviderUtils::H5Type2DotNet(type_id) == Single::typeid) {
                    minfo = magicType->GetMethod("ToSingle");
                }
                break;

            case H5T_ENUM:

                super = H5Tget_super(type_id);

                if (ProviderUtils::H5Type2DotNet(super) == Int32::typeid) {
                    minfo = magicType->GetMethod("ToInt32");
                }
                else if (ProviderUtils::H5Type2DotNet(super) == Int64::typeid) {
                    minfo = magicType->GetMethod("ToInt64");
                }
                else if (ProviderUtils::H5Type2DotNet(super) == Int16::typeid) {
                    minfo = magicType->GetMethod("ToInt16");
                }
                if (ProviderUtils::H5Type2DotNet(super) == UInt32::typeid) {
                    minfo = magicType->GetMethod("ToUInt32");
                }
                else if (ProviderUtils::H5Type2DotNet(super) == UInt64::typeid) {
                    minfo = magicType->GetMethod("ToUInt64");
                }
                else if (ProviderUtils::H5Type2DotNet(super) == UInt16::typeid) {
                    minfo = magicType->GetMethod("ToUInt16");
                }
                break;

            default:
                break;
            }
        }
        finally
        {
            if (super >= 0) {
                H5Tclose(super);
            }
        }

        return minfo;
    }

    Type^ ProviderUtils::GetCompundDotNetType(hid_t type_id)
    {
        Type^ result = nullptr;

        if (H5Tget_class(type_id) != H5T_COMPOUND) {
            throw gcnew PSH5XException("This is not a compound type");
        }

        size_t size = H5Tget_size(type_id);

        int member_count = H5Tget_nmembers(type_id);
        if (member_count < 0) {
            throw gcnew HDF5Exception("H5Tget_nmembers failed!");
        }

        array<String^>^ member_name = gcnew array<String^>(member_count);
        array<String^>^ member_type = gcnew array<String^>(member_count);
        array<int>^ member_size = gcnew array<int>(member_count);
        array<int>^ member_offset = gcnew array<int>(member_count);
        array<__wchar_t>^ member_tcode = gcnew array<__wchar_t>(member_count);

        StringBuilder^ sbname = gcnew StringBuilder();
        StringBuilder^ sbconstr = gcnew StringBuilder("(");

        char* name = NULL;

        hid_t mtype = -1, ntype = -1;

        try
        {
            for (int i = 0; i < member_count; ++i)
            {
                size_t offset = H5Tget_member_offset(type_id, i);

                member_offset[i] = safe_cast<int>(offset);

                name = H5Tget_member_name(type_id, safe_cast<unsigned>(i));
                if (name == NULL) {
                    throw gcnew HDF5Exception("H5Tget_member_name failed!");
                }

                mtype = H5Tget_member_type(type_id, safe_cast<unsigned>(i));
                if (mtype < 0) {
                    throw gcnew HDF5Exception("H5Tget_member_type failed!");
                }

                if (H5Tget_class(mtype) == H5T_BITFIELD) {
                    ntype = H5Tget_native_type(mtype, H5T_DIR_DESCEND);
                }
                else {
                    ntype = H5Tget_native_type(mtype, H5T_DIR_ASCEND);
                }
                if (ntype < 0) {
                    throw gcnew HDF5Exception("H5Tget_native_type failed!");
                }

                member_size[i] = safe_cast<int>(H5Tget_size(ntype));

                Type^ t = ProviderUtils::H5Type2DotNet(ntype);
                if (t != nullptr)
                {
                    member_type[i]  = t->ToString();
                    member_tcode[i] = ProviderUtils::TypeCode(t);
                    sbname->Append(member_tcode[i]);
                    member_name[i]  = member_tcode[i] + Convert::ToString(i);
                }
                else {
                    throw gcnew PSH5XException("Unsupported member type in compound!");
                }

                sbconstr->Append(member_type[i] + " " + "param" + Convert::ToString(i));
                if (i < member_count-1) {
                    sbconstr->Append(", ");
                }

                if (H5Tclose(ntype) < 0) {
                    throw gcnew HDF5Exception("H5Tclose failed!");
                }
                ntype = -1;

                if (H5Tclose(mtype) < 0) {
                    throw gcnew HDF5Exception("H5Tclose failed!");
                }
                mtype = -1;
            }

            sbconstr->Append(") {");

            String^ class_name = sbname->ToString();

            StringBuilder^ sbcode = gcnew StringBuilder();
            sbcode->Append("using System.Runtime.InteropServices; ");
            sbcode->Append("[StructLayout(LayoutKind.Explicit,Size= " + size + ",CharSet=CharSet.Ansi)] ");
            sbcode->Append("public class " + class_name + " { ");

            StringBuilder^ sb_def_constr = gcnew StringBuilder();
            sb_def_constr->Append(" public " + class_name + "() {");

            for (int i = 0; i < member_count; ++i)
            {
                sbcode->Append("[FieldOffset(" + member_offset[i] + ")]");
                sbcode->Append(" public " + member_type[i] + " " + member_name[i] + ";"); // + " {get;set;} ");

                if (member_tcode[i] == 's') {
                    sb_def_constr->Append(" " + member_name[i] + " = \"\"; ");
                }

                sbconstr->Append(member_name[i] + " = param" + Convert::ToString(i) + ";");
            }

            sb_def_constr->Append(" }");
            sbconstr->Append(" }");

            sbcode->Append(sb_def_constr->ToString());
            sbcode->Append(" public " + class_name + sbconstr->ToString());

            String^ code = sbcode->ToString() + "}";

            //Console::WriteLine(code);

            CompilerParameters^ params = gcnew CompilerParameters();
            params->GenerateInMemory = true;
            params->TreatWarningsAsErrors = false;
            params->GenerateExecutable = false;
            params->CompilerOptions = "/optimize";

            CSharpCodeProvider^ provider = gcnew CSharpCodeProvider();
            CompilerResults^ compilate = provider->CompileAssemblyFromSource(params, code);

            Assembly^ assembly = compilate->CompiledAssembly;
            result = assembly->GetType(sbname->ToString());
        }
        finally
        {
            if (mtype >= 0) {
                H5Tclose(mtype);
            }
            if (ntype >= 0) {
                H5Tclose(ntype);
            }
        }

        return result;
    }


    bool ProviderUtils::IsH5SimpleType(hid_t dtype)
    {
        H5T_class_t cls = H5Tget_class(dtype);

        return (cls != H5T_COMPOUND && cls != H5T_VLEN && cls != H5T_ARRAY);
    }

    bool ProviderUtils::IsH5IntegerType(hid_t dtype)
    {
        H5T_class_t cls = H5Tget_class(dtype);

        return (cls == H5T_INTEGER);
    }

    bool ProviderUtils::IsH5FloatType(hid_t dtype)
    {
        H5T_class_t cls = H5Tget_class(dtype);

        return (cls == H5T_FLOAT);
    }

    bool ProviderUtils::IsH5StringType(hid_t dtype)
    {
        H5T_class_t cls = H5Tget_class(dtype);

        return (cls == H5T_STRING);
    }

    bool ProviderUtils::IsH5BitfieldType(hid_t dtype)
    {
        H5T_class_t cls = H5Tget_class(dtype);

        return (cls == H5T_BITFIELD);
    }

    bool ProviderUtils::IsH5OpaqueType(hid_t dtype)
    {
        H5T_class_t cls = H5Tget_class(dtype);

        return (cls == H5T_OPAQUE);
    }

    bool ProviderUtils::IsH5CompoundType(hid_t dtype)
    {
        H5T_class_t cls = H5Tget_class(dtype);

        return (cls == H5T_COMPOUND);
    }

    bool ProviderUtils::IsH5ReferenceType(hid_t dtype)
    {
        H5T_class_t cls = H5Tget_class(dtype);

        return (cls == H5T_REFERENCE);
    }

    bool ProviderUtils::IsH5EnumType(hid_t dtype)
    {
        H5T_class_t cls = H5Tget_class(dtype);

        return (cls == H5T_ENUM);
    }

    bool ProviderUtils::IsH5VlenType(hid_t dtype)
    {
        H5T_class_t cls = H5Tget_class(dtype);

        return (cls == H5T_VLEN);
    }

    bool ProviderUtils::IsH5ArrayType(hid_t dtype)
    {
        H5T_class_t cls = H5Tget_class(dtype);

        return (cls == H5T_ARRAY);
    }

    
}