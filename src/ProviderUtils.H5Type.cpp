
#include "HDF5Exception.h"
#include "ProviderUtils.h"
#include "PSH5XException.h"

extern "C" {
#include "H5Spublic.h"
#include "H5Tpublic.h"
}

#include <cstdlib>
#include <cstring>

using namespace Microsoft::CSharp;

using namespace System;
using namespace System::CodeDom::Compiler;
using namespace System::Collections;
using namespace System::Collections::Generic;
using namespace System::Management::Automation;
using namespace System::Reflection;
using namespace System::Runtime::InteropServices;
using namespace System::Text;

namespace PSH5X
{
    Dictionary<String^,Object^>^ ProviderUtils::ParseH5Type(hid_t type)
    {
		Dictionary<String^,Object^>^ result = gcnew Dictionary<String^,Object^>();

		Dictionary<String^,Object^>^ members = nullptr;

		ArrayList^ member = nullptr;

        hid_t unwnd = -1, ntype = -1;
        size_t size = 0, precision, offset;
        int i, nmembers = 0, rank;
        unsigned ui;
        char* name;
        String^ s;
        htri_t ierr;
        H5T_cset_t cset;
        H5T_str_t strpad;
        H5T_pad_t lsb, msb;
        H5T_sign_t sign;
        size_t spos, epos, esize, ebias, mpos, msize;

        try
        {            
            switch (H5Tget_class(type))
            {
            case H5T_INTEGER:
				{
#pragma region Integer

					result->Add("Class", "Integer");
					size = H5Tget_size(type);
					result->Add("Size", size);
					precision = H5Tget_precision(type);
					result->Add("Precision", precision);
					i = H5Tget_offset(type);
					result->Add("BitOffset", i);

					if (H5Tget_pad(type, &lsb, &msb) >= 0)
					{
						switch (lsb)
						{
						case H5T_PAD_ZERO:
							result->Add("LSBitPadding", "Zero");
							break;
						case H5T_PAD_ONE:
							result->Add("LSBitPadding", "One");
							break;
						case H5T_PAD_BACKGROUND:
							result->Add("LSBitPadding", "Background");
							break;
						default:
							result->Add("LSBitPadding", "UNKONWN");
							break;
						}

						switch (msb)
						{
						case H5T_PAD_ZERO:
							result->Add("MSBitPadding", "Zero");
							break;
						case H5T_PAD_ONE:
							result->Add("MSBitPadding", "One");
							break;
						case H5T_PAD_BACKGROUND:
							result->Add("MSBitPadding", "Background");
							break;
						default:
							result->Add("MSBitPadding", "UNKNOWN");
							break;
						}
					}
					else {
						throw gcnew HDF5Exception("H5Tget_pad failed!");
					}

					H5T_sign_t sign = H5Tget_sign(type);
					switch (sign)
					{
					case H5T_SGN_NONE:
						result->Add("SignType", "None");
						break;
					case H5T_SGN_2:
						result->Add("SignType", "2");
						break;
					default:
						result->Add("SignType", "UNKNOWN");
						break;
					}

					H5T_order_t order = H5Tget_order(type);
					switch (order)
					{
					case H5T_ORDER_LE:
						result->Add("ByteOrder", "LE");
						break;
					case H5T_ORDER_BE:
						result->Add("ByteOrder", "BE");
						break;
					case H5T_ORDER_VAX:
						result->Add("ByteOrder", "VAX");
						break;
					case H5T_ORDER_MIXED:
						result->Add("ByteOrder", "Mixed");
						break;
					case H5T_ORDER_NONE:
						result->Add("ByteOrder", "None");
						break;
					default:
						result->Add("ByteOrder", "UNKNOWN");
						break;
					}

					switch (size)
					{
					case 1:
						{
							if (sign == H5T_SGN_NONE) {
								if (order == H5T_ORDER_LE) {
									if (H5Tequal(type, H5T_STD_U8LE) > 0) {
										result->Add("StandardType", "H5T_STD_U8LE");
									}
								}
								else if (order == H5T_ORDER_BE) {
									if (H5Tequal(type, H5T_STD_U8BE) > 0) {
										result->Add("StandardType", "H5T_STD_U8BE");
									}
								}
							}
							else {
								if (order == H5T_ORDER_LE) {
									if (H5Tequal(type, H5T_STD_I8LE) > 0) {
										result->Add("StandardType", "H5T_STD_I8LE");
									}
								}
								else if (order == H5T_ORDER_BE) {
									if (H5Tequal(type, H5T_STD_I8BE) > 0) {
										result->Add("StandardType", "H5T_STD_I8BE");
									}
								}
							}
						}
						break;
					case 2:
						{
							if (sign == H5T_SGN_NONE) {
								if (order == H5T_ORDER_LE) {
									if (H5Tequal(type, H5T_STD_U16LE) > 0) {
										result->Add("StandardType", "H5T_STD_U16LE");
									}
								}
								else if (order == H5T_ORDER_BE) {
									if (H5Tequal(type, H5T_STD_U16BE) > 0) {
										result->Add("StandardType", "H5T_STD_U16BE");
									}
								}
							}
							else {
								if (order == H5T_ORDER_LE) {
									if (H5Tequal(type, H5T_STD_I16LE) > 0) {
										result->Add("StandardType", "H5T_STD_I16LE");
									}
								}
								else if (order == H5T_ORDER_BE) {
									if (H5Tequal(type, H5T_STD_I16BE) > 0) {
										result->Add("StandardType", "H5T_STD_I16BE");
									}
								}
							}
						}
						break;
					case 4:
						{
							if (sign == H5T_SGN_NONE) {
								if (order == H5T_ORDER_LE) {
									if (H5Tequal(type, H5T_STD_U32LE) > 0) {
										result->Add("StandardType", "H5T_STD_U32LE");
									}
								}
								else if (order == H5T_ORDER_BE) {
									if (H5Tequal(type, H5T_STD_U32BE) > 0) {
										result->Add("StandardType", "H5T_STD_U32BE");
									}
								}
							}
							else {
								if (order == H5T_ORDER_LE) {
									if (H5Tequal(type, H5T_STD_I32LE) > 0) {
										result->Add("StandardType", "H5T_STD_I32LE");
									}
								}
								else if (order == H5T_ORDER_BE) {
									if (H5Tequal(type, H5T_STD_I32BE) > 0) {
										result->Add("StandardType", "H5T_STD_I32BE");
									}
								}
							}
						}
						break;
					case 8:
						{
							if (sign == H5T_SGN_NONE) {
								if (order == H5T_ORDER_LE) {
									if (H5Tequal(type, H5T_STD_U64LE) > 0) {
										result->Add("StandardType", "H5T_STD_U64LE");
									}
								}
								else if (order == H5T_ORDER_BE) {
									if (H5Tequal(type, H5T_STD_U64BE) > 0) {
										result->Add("StandardType", "H5T_STD_U64BE");
									}
								}
							}
							else {
								if (order == H5T_ORDER_LE) {
									if (H5Tequal(type, H5T_STD_I64LE) > 0) {
										result->Add("StandardType", "H5T_STD_I64LE");
									}
								}
								else if (order == H5T_ORDER_BE) {
									if (H5Tequal(type, H5T_STD_I64BE) > 0) {
										result->Add("StandardType", "H5T_STD_I64BE");
									}
								}
							}
						}
						break;
					default:
						break;
					}
#pragma endregion
				}
                break;

            case H5T_FLOAT:
				{
#pragma region Float

					result->Add("Class", "Float");
					size = H5Tget_size(type);
					result->Add("Size", size);
					precision = H5Tget_precision(type);
					result->Add("Precision", precision);
					i = H5Tget_offset(type);
					result->Add("BitOffset", i);

					if (H5Tget_pad(type, &lsb, &msb) >= 0)
					{
						switch (lsb)
						{
						case H5T_PAD_ZERO:
							result->Add("LSBitPadding", "Zero");
							break;
						case H5T_PAD_ONE:
							result->Add("LSBitPadding", "One");
							break;
						case H5T_PAD_BACKGROUND:
							result->Add("LSBitPadding", "Background");
							break;
						default:
							result->Add("LSBitPadding", "UNKNOWN");
							break;
						}

						switch (msb)
						{
						case H5T_PAD_ZERO:
							result->Add("MSBitPadding", "Zero");
							break;
						case H5T_PAD_ONE:
							result->Add("MSBitPadding", "One");
							break;
						case H5T_PAD_BACKGROUND:
							result->Add("MSBitPadding", "Background");
							break;
						default:
							result->Add("MSBitPadding", "UNKNOWN");
							break;
						}
					}
					else {
						throw gcnew HDF5Exception("H5Tget_pad failed!");
					}

					switch (H5Tget_inpad(type))
					{
					case H5T_PAD_ZERO:
						result->Add("IntlBitPadding", "Zero");
						break;
					case H5T_PAD_ONE:
						result->Add("IntlBitPadding", "One");
						break;
					case H5T_PAD_BACKGROUND:
						result->Add("IntlBitPadding", "Background");
						break;
					default:
						result->Add("IntlBitPadding", "UNKNOWN");
						break;
					}

					H5T_order_t order = H5Tget_order(type);
					switch (order)
					{
					case H5T_ORDER_LE:
						result->Add("ByteOrder", "LE");
						break;
					case H5T_ORDER_BE:
						result->Add("ByteOrder", "BE");
						break;
					case H5T_ORDER_VAX:
						result->Add("ByteOrder", "VAX");
						break;
					case H5T_ORDER_MIXED:
						result->Add("ByteOrder", "Mixed");
						break;
					case H5T_ORDER_NONE:
						result->Add("ByteOrder", "None");
						break;
					default:
						result->Add("ByteOrder", "UNKNOWN");
						break;
					}

					if (H5Tget_fields(type, &spos, &epos, &esize, &mpos, &msize) >= 0)
					{
						result->Add("SignBitPos", spos);
						result->Add("ExpBitPos", epos);
						result->Add("ExpBits", esize);
						result->Add("MantBitPos", mpos);
						result->Add("MantBits", msize);
					}
					else {
						throw gcnew HDF5Exception("H5Tget_fields failed!");
					}

					ebias = H5Tget_ebias(type);
					result->Add("ExpBias", ebias);

					switch (H5Tget_norm(type))
					{
					case H5T_NORM_IMPLIED:
						result->Add("MantNorm", "Implied");
						break;
					case H5T_NORM_MSBSET:
						result->Add("MantNorm", "MsbSet");
						break;
					case H5T_NORM_NONE:
						result->Add("MantNorm", "None");
						break;
					default:
						result->Add("MantNorm", "UNKNOWN");
						break;
					}

					switch (size)
					{
					case 4:
						{
							if (order == H5T_ORDER_LE) {
								if (H5Tequal(type, H5T_IEEE_F32LE) > 0) {
									result->Add("StandardType", "H5T_IEEE_F32LE");
								}
							}
							else if (order == H5T_ORDER_BE) {
								if (H5Tequal(type, H5T_IEEE_F32BE) > 0) {
									result->Add("StandardType", "H5T_IEEE_F32BE");
								}
							}
						}
						break;

					case 8:
						{
							if (order == H5T_ORDER_LE) {
								if (H5Tequal(type, H5T_IEEE_F64LE) > 0) {
									result->Add("StandardType", "H5T_IEEE_F64LE");
								}
							}
							else if (order == H5T_ORDER_BE) {
								if (H5Tequal(type, H5T_IEEE_F64BE) > 0) {
									result->Add("StandardType", "H5T_IEEE_F64BE");
								}
							}
						}
						break;

					default:
						break;
					}

#pragma endregion
				}
                break;

            case H5T_STRING:
				{
#pragma region String

					result->Add("Class", "String");
					cset = H5Tget_cset(type);
					switch (cset)
					{
					case H5T_CSET_ASCII:
						result->Add("Encoding", "Ascii");
						break;
					case H5T_CSET_UTF8:
						result->Add("Encoding", "Utf8");
						break;
					default:
						result->Add("Encoding", "UNKNOWN");
						break;
					}

					ierr = H5Tis_variable_str(type);
					if (ierr == 0)
					{
						size = H5Tget_size(type);
						result->Add("Length", size);
						strpad = H5Tget_strpad(type);
						switch (strpad)
						{
						case H5T_STR_NULLTERM:
							result->Add("StrPad", "Nullterm");
							break;
						case H5T_STR_NULLPAD:
							result->Add("StrPad", "Nullpad");
							break;
						case H5T_STR_SPACEPAD:
							result->Add("StrPad", "SpacePad");
							break;
						default:
							result->Add("StrPad", "UNKNOWN");
							break;
						}
						result->Add("IsVariableLength", false);
					}
					else if (ierr > 0) {
						result->Add("IsVariableLength", true);
					}
					else {
						throw gcnew HDF5Exception("H5Tis_variable_str failed!!!");
					}

#pragma endregion
				}
				break;

            case H5T_BITFIELD:
				{
#pragma region Bitfield

					result->Add("Class", "Bitfield");
					size = H5Tget_size(type);
					result->Add("Size", size);
					precision = H5Tget_precision(type);
					result->Add("Precision", precision);
					i = H5Tget_offset(type);
					result->Add("BitOffset", i);

					if (H5Tget_pad(type, &lsb, &msb) >= 0)
					{
						switch (lsb)
						{
						case H5T_PAD_ZERO:
							result->Add("LSBitPadding", "Zero");
							break;
						case H5T_PAD_ONE:
							result->Add("LSBitPadding", "One");
							break;
						case H5T_PAD_BACKGROUND:
							result->Add("LSBitPadding", "Background");
							break;
						default:
							result->Add("LSBitPadding", "UNKNOWN");
							break;
						}

						switch (msb)
						{
						case H5T_PAD_ZERO:
							result->Add("MSBitPadding", "Zero");
							break;
						case H5T_PAD_ONE:
							result->Add("MSBitPadding", "One");
							break;
						case H5T_PAD_BACKGROUND:
							result->Add("MSBitPadding", "Background");
							break;
						default:
							result->Add("MSBitPadding", "UNKNOWN");
							break;
						}
					}
					else {
						throw gcnew HDF5Exception("H5Tget_pad failed!");
					}

					H5T_order_t order = H5Tget_order(type);
					switch (order)
					{
					case H5T_ORDER_LE:
						result->Add("ByteOrder", "LE");
						break;
					case H5T_ORDER_BE:
						result->Add("ByteOrder", "BE");
						break;
					case H5T_ORDER_VAX:
						result->Add("ByteOrder", "VAX");
						break;
					case H5T_ORDER_MIXED:
						result->Add("ByteOrder", "Mixed");
						break;
					case H5T_ORDER_NONE:
						result->Add("ByteOrder", "None");
						break;
					default:
						result->Add("ByteOrder", "UNKNOWN");
						break;
					}

					switch (size)
					{
					case 1:
						{
							if (order == H5T_ORDER_LE) {
								if (H5Tequal(type, H5T_STD_B8LE) > 0) {
									result->Add("StandardType", "H5T_STD_B8LE");
								}
							}
							else if (order == H5T_ORDER_BE) {
								if (H5Tequal(type, H5T_STD_B8BE) > 0) {
									result->Add("StandardType", "H5T_STD_B8BE");
								}
							}
						}
						break;
					case 2:
						{
							if (order == H5T_ORDER_LE) {
								if (H5Tequal(type, H5T_STD_B16LE) > 0) {
									result->Add("StandardType", "H5T_STD_B16LE");
								}
							}
							else if (order == H5T_ORDER_BE) {
								if (H5Tequal(type, H5T_STD_B16BE) > 0) {
									result->Add("StandardType", "H5T_STD_B16BE");
								}
							}
						}
						break;
					case 4:
						{
							if (order == H5T_ORDER_LE) {
								if (H5Tequal(type, H5T_STD_B32LE) > 0) {
									result->Add("StandardType", "H5T_STD_B32LE");
								}
							}
							else if (order == H5T_ORDER_BE) {
								if (H5Tequal(type, H5T_STD_B32BE) > 0) {
									result->Add("StandardType", "H5T_STD_B32BE");
								}
							}
						}
						break;
					case 8:
						{
							if (order == H5T_ORDER_LE) {
								if (H5Tequal(type, H5T_STD_B64LE) > 0) {
									result->Add("StandardType", "H5T_STD_B64LE");
								}
							}
							else if (order == H5T_ORDER_BE) {
								if (H5Tequal(type, H5T_STD_B64BE) > 0) {
									result->Add("StandardType", "H5T_STD_B64BE");
								}
							}
						}
						break;
					default:
						break;
					}

#pragma endregion
				}
                break;

            case H5T_OPAQUE:
				{
#pragma region Opaque
				
					result->Add("Class", "Opaque");
					name = H5Tget_tag(type);
					result->Add("Tag", gcnew String(name));
					result->Add("Size", H5Tget_size(type));
				
#pragma endregion
				}
                break;


            case H5T_COMPOUND:
				{
#pragma region Compound

					result->Add("Class", "Compound");
					size = H5Tget_size(type);
					result->Add("Size", size);

					members = gcnew Dictionary<String^,Object^>();

					nmembers = H5Tget_nmembers(type);
					for (ui = 0; ui < safe_cast<unsigned>(nmembers); ++ui)
					{
						name = H5Tget_member_name(type, ui);
						s = gcnew String(name);

						member = gcnew ArrayList();

						unwnd = H5Tget_member_type(type, ui);
						if (unwnd >= 0)
						{
							offset = H5Tget_member_offset(type, ui);

							member->Add(offset);
							member->Add(ProviderUtils::ParseH5Type(unwnd));

							if (H5Tclose(unwnd) < 0) {
								throw gcnew HDF5Exception("H5Tclose failed!");
							}
							else {
								unwnd = -1;
							}
						}

						members->Add(s, member);
					}

					result->Add("Members", members);
#pragma endregion
				}
                break;

            case H5T_ENUM:
				{
#pragma region Enum
					result->Add("Class", "Enum");
					size = H5Tget_size(type);
					if (size != 1 && size != 2 && size != 4 && size != 8) {
						throw gcnew PSH5XException("Unsupported enum size!");
					}
					unwnd = H5Tget_super(type);
					result->Add("Base", ProviderUtils::ParseH5Type(unwnd));
					sign = H5Tget_sign(unwnd);

					members = gcnew Dictionary<String^,Object^>();

					nmembers = H5Tget_nmembers(type);
					if (nmembers < 0) {
						throw gcnew HDF5Exception("H5Tget_nmembers failed!");
					}

					if ((ntype = H5Tget_native_type(type, H5T_DIR_ASCEND)) < 0) {
						throw gcnew HDF5Exception("H5Tget_native_type failed!");
					}

					char b;
					unsigned char B;
					short h;
					unsigned short H;
					int i;
					unsigned int I;
					long long l;
					unsigned long long L;

					for (ui = 0; ui < safe_cast<unsigned>(nmembers); ++ui)
					{
						name = H5Tget_member_name(type, ui);
						s = gcnew String(name);

						if (sign == H5T_SGN_NONE)
						{
							switch (size)
							{
							case 1:

								if (H5Tget_member_value(ntype, ui, &B) < 0) {
									throw gcnew HDF5Exception("H5Tget_member_value failed!");
								}
								members->Add(s, B);                            
								break;

							case 2:

								if (H5Tget_member_value(ntype, ui, &H) < 0) {
									throw gcnew HDF5Exception("H5Tget_member_value failed!");
								}
								members->Add(s, H);
								break;

							case 4:

								if (H5Tget_member_value(ntype, ui, &I) < 0) {
									throw gcnew HDF5Exception("H5Tget_member_value failed!");
								}
								members->Add(s, I);
								break;

							case 8:

								if (H5Tget_member_value(ntype, ui, &L) < 0) {
									throw gcnew HDF5Exception("H5Tget_member_value failed!");
								}
								members->Add(s, L);
								break;
							}
						}
						else if (sign == H5T_SGN_2)
						{
							switch (size)
							{
							case 1:

								if (H5Tget_member_value(ntype, ui, &b) < 0) {
									throw gcnew HDF5Exception("H5Tget_member_value failed!");
								}
								members->Add(s, b);
								break;

							case 2:

								if (H5Tget_member_value(ntype, ui, &h) < 0) {
									throw gcnew HDF5Exception("H5Tget_member_value failed!");
								}
								members->Add(s, h);            
								break;

							case 4:

								if (H5Tget_member_value(ntype, ui, &i) < 0) {
									throw gcnew HDF5Exception("H5Tget_member_value failed!");
								}
								members->Add(s, i);

								break;

							case 8:

								if (H5Tget_member_value(ntype, ui, &l) < 0) {
									throw gcnew HDF5Exception("H5Tget_member_value failed!");
								}
								members->Add(s, l);

								break;
							}
						}
						else {
							throw gcnew HDF5Exception("Unknown sign type!");
						}
					}
					result->Add("Members", members);
#pragma endregion
				}
                break;

            case H5T_VLEN:
				{
#pragma region Vlen
					result->Add("Class", "Vlen");
					unwnd = H5Tget_super(type);
					if (unwnd >= 0)
					{
						result->Add("Base", ProviderUtils::ParseH5Type(unwnd));
					}
#pragma endregion
				}
                break;

            case H5T_REFERENCE:
				{
#pragma region Reference
					result->Add("Class", "Reference");

					if (H5Tequal(type, H5T_STD_REF_OBJ) > 0) {
						result->Add("Kind", "Object");
						result->Add("StandardType", "H5T_STD_REF_OBJ");
					}
					else if (H5Tequal(type, H5T_STD_REF_DSETREG) > 0) {
						result->Add("Kind", "Region");
						result->Add("StandardType", "H5T_STD_REF_DSETREG");
					}
					else {
						result->Add("Kind", "UNKNOWN");
					}
#pragma endregion
				}
                break;

            case H5T_ARRAY:
				{
#pragma region Array
					result->Add("Class", "Array");
					if ((rank = H5Tget_array_ndims(type)) < 0) {
						throw gcnew HDF5Exception("H5Tget_array_ndims failed!");
					}
					array<hsize_t>^ dims = gcnew array<hsize_t>(rank);
					pin_ptr<hsize_t> dims_ptr = &dims[0];
					if ((rank = H5Tget_array_dims2(type, dims_ptr)) < 0) {
						throw gcnew HDF5Exception("H5Tget_array_dims2 failed!");
					}
					result->Add("Dims", dims);

					unwnd = H5Tget_super(type);
					if (unwnd >= 0) {
						result->Add("Base", ProviderUtils::ParseH5Type(unwnd));
					}
					else {
						throw gcnew HDF5Exception("H5Tget_super failed!");
					}
#pragma endregion
				}
                break;        

            default:
                throw gcnew PSH5XException("Unknown HDF5 datatype class!");
                break;
            }
        }
        finally
        {
            if (unwnd >= 0) {    
                H5Tclose(unwnd);
            }
			if (ntype >= 0) {    
                H5Tclose(ntype);
            }
        }

        return result;
    }

	hid_t ProviderUtils::ParseH5Type(Object^ obj)
    {
        hid_t result = -1;

		Dictionary<String^,Object^>^ dict = nullptr;

        try
        {
            if (TryGetValue(obj, dict))
            {
                result = ProviderUtils::ParseH5Type(dict);
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
        }

        return result;
    }
	
	hid_t ProviderUtils::ParseH5Type(Dictionary<String^,Object^>^ dict)
	{
		hid_t result = -1, base_type = -1, dtype = -1;

		char *label = NULL, *name = NULL;

		try
		{
			if (dict->ContainsKey("Class"))
			{
				String^ type_class = dict["Class"]->ToString()->ToUpper();

				if (type_class == "INTEGER")
				{
#pragma region INTEGER

					size_t precision = 0;
					if (dict->ContainsKey("Precision")) {
						if (!TryGetValue(dict["Precision"], precision)) {
							throw gcnew PSH5XException("INTEGER: Unable to convert the 'Precision' value into size_t.");
						}
					}
					else {
						throw gcnew PSH5XException("INTEGER: No 'Precision' key found.");
					}

					String^ s = nullptr;

					H5T_order_t order = H5T_ORDER_LE;
					if (dict->ContainsKey("ByteOrder")) {
						if (!TryGetValue(dict["ByteOrder"], s)) {
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
					if (dict->ContainsKey("BitOffset")) {
						if (!TryGetValue(dict["BitOffset"], offset)) {
							throw gcnew PSH5XException("INTEGER: Unable to convert the 'BitOffset' value into size_t.");
						}
					}

					H5T_sign_t sign = H5T_SGN_2;
					if (dict->ContainsKey("SignType")) {
						if (!TryGetValue(dict["SignType"], s)) {
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
					if (dict->ContainsKey("MSBitPadding")) {
						if (!TryGetValue(dict["MSBitPadding"], s)) {
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
					if (dict->ContainsKey("LSBitPadding")) {
						if (!TryGetValue(dict["LSBitPadding"], s)) {
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

#pragma endregion
				}
				else if (type_class == "FLOAT")
				{
#pragma region FLOAT

					size_t offset = 0;
					if (dict->ContainsKey("BitOffset")) {
						if (!TryGetValue(dict["BitOffset"], offset)) {
							throw gcnew PSH5XException("FLOAT: Unable to convert the 'BitOffset' value into size_t.");
						}
					}

					size_t precision = 0;
					if (dict->ContainsKey("Precision")) {
						if (!TryGetValue(dict["Precision"], precision)) {
							throw gcnew PSH5XException("FLOAT: Unable to convert the 'Precision' value into size_t.");
						}
					}
					else {
						throw gcnew PSH5XException("FLOAT: No 'Precision' key found.");
					}

					String^ s = nullptr;

					H5T_order_t order = H5T_ORDER_LE;
					if (dict->ContainsKey("ByteOrder")) {
						if (!TryGetValue(dict["ByteOrder"], s)) {
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
					if (dict->ContainsKey("MSBitPadding")) {
						if (!TryGetValue(dict["MSBitPadding"], s)) {
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
					if (dict->ContainsKey("LSBitPadding")) {
						if (!TryGetValue(dict["LSBitPadding"], s)) {
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
					if (dict->ContainsKey("IntlBitPadding")) {
						if (!TryGetValue(dict["IntlBitPadding"], s)) {
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
					if (dict->ContainsKey("SignBitPos")) {
						if (!TryGetValue(dict["SignBitPos"], spos)) {
							throw gcnew PSH5XException("FLOAT: Unable to convert the 'SignBitPos' value into size_t.");
						}
					}
					size_t epos = 23;
					if (dict->ContainsKey("ExpBitPos")) {
						if (!TryGetValue(dict["ExpBitPos"], epos)) {
							throw gcnew PSH5XException("FLOAT: Unable to convert the 'ExpBitPos' value into size_t.");
						}
					}
					size_t esize = 8;
					if (dict->ContainsKey("ExpBits")) {
						if (!TryGetValue(dict["ExpBits"], esize)) {
							throw gcnew PSH5XException("FLOAT: Unable to convert the 'ExpBits' value into size_t.");
						}
					}
					size_t mpos = 0;
					if (dict->ContainsKey("MantBitPos")) {
						if (!TryGetValue(dict["MantBitPos"], mpos)) {
							throw gcnew PSH5XException("FLOAT: Unable to convert the 'MantBitPos' value into size_t.");
						}
					}
					size_t msize = 23;
					if (dict->ContainsKey("MantBits")) {
						if (!TryGetValue(dict["MantBits"], msize)) {
							throw gcnew PSH5XException("FLOAT: Unable to convert the 'MantBits' value into size_t.");
						}
					}

					size_t ebias = 127;
					if (dict->ContainsKey("ExpBias")) {
						if (!TryGetValue(dict["ExpBias"], ebias)) {
							throw gcnew PSH5XException("FLOAT: Unable to convert the 'ExpBias' value into size_t.");
						}
					}

					H5T_norm_t norm = H5T_NORM_IMPLIED;
					if (dict->ContainsKey("MantNorm")) {
						if (!TryGetValue(dict["MantNorm"], s)) {
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

#pragma endregion
				}
				else if (type_class == "STRING")
				{
#pragma region STRING

					if (dict->ContainsKey("Length"))
					{
						size_t length = 0;
						if (TryGetValue(dict["Length"], length))
						{
							result = H5Tcreate(H5T_STRING, length);
							if (result < 0) {
								throw gcnew HDF5Exception("STRING: H5Tcreate failed.");
							}
						}
						else {
							throw gcnew PSH5XException("STRING: Invalid string length!");
						}
					}
					else  // variable length string
					{
						result = H5Tcreate(H5T_STRING, H5T_VARIABLE);
						if (result < 0) {
							throw gcnew HDF5Exception("STRING: H5Tcreate failed.");
						}
					}

					if (dict->ContainsKey("Encoding"))
					{
						String^ cset = dict["Encoding"]->ToString()->ToUpper();
						if (cset != "ASCII" && cset != "UTF-8") {
							throw gcnew PSH5XException(
								String::Format("STRING: Invalid encoding '{0}'. ASCII or UTF-8 only!",
								dict["Encoding"]->ToString()));
						}
						else
						{
							if (cset == "UTF-8") {
								if (H5Tset_cset(result, H5T_CSET_UTF8) < 0) {
									throw gcnew HDF5Exception("STRING: H5Tset_cset failed.");
								}
							}
							else {
								if (H5Tset_cset(result, H5T_CSET_ASCII) < 0) {
									throw gcnew HDF5Exception("STRING: H5Tset_cset failed.");
								}
							}
						}
					}
					else {
						throw gcnew HDF5Exception("STRING: No character set specified.");
					}

					if (dict->ContainsKey("StrPad"))
					{
						String^ strPad = dict["StrPad"]->ToString()->ToUpper();
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

#pragma endregion
				}
				else if (type_class == "BITFIELD")
				{
#pragma region BITFIELD

					size_t precision = 0;
					if (dict->ContainsKey("Precision")) {
						if (!TryGetValue(dict["Precision"], precision)) {
							throw gcnew PSH5XException("BITFIELD: Unable to convert the 'Precision' value into size_t.");
						}
					}
					else {
						throw gcnew PSH5XException("BITFIELD: No 'Precision' key found.");
					}

					String^ s = nullptr;

					H5T_order_t order = H5T_ORDER_LE;
					if (dict->ContainsKey("ByteOrder")) {
						if (!TryGetValue(dict["ByteOrder"], s)) {
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
					if (dict->ContainsKey("BitOffset")) {
						if (!TryGetValue(dict["BitOffset"], offset)) {
							throw gcnew PSH5XException("BITFIELD: Unable to convert the 'BitOffset' value into size_t.");
						}
					}

					H5T_pad_t lsb = H5T_PAD_ZERO, msb = H5T_PAD_ZERO;
					if (dict->ContainsKey("MSBitPadding")) {
						if (!TryGetValue(dict["MSBitPadding"], s)) {
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
					if (dict->ContainsKey("LSBitPadding")) {
						if (!TryGetValue(dict["LSBitPadding"], s)) {
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

#pragma endregion
				}
				else if (type_class == "OPAQUE")
				{
#pragma region OPAQUE
					size_t size = 0;
					if (dict->ContainsKey("Size")) // TODO: check size < 64KB
					{
						if (TryGetValue(dict["Size"], size))
						{
							if (dict->ContainsKey("Tag"))
							{
								String^ tag = dict["Tag"]->ToString();
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

					if (dict->ContainsKey("Size"))
					{
						if (ProviderUtils::TryGetValue(dict["Size"], size))
						{
							result = H5Tcreate(H5T_COMPOUND, size);
							if (result < 0) {
								throw gcnew HDF5Exception("COMPOUND: H5Tcreate failed.");
							}
						}
						else {
							throw gcnew PSH5XException("COMPOUND: Unable to convert the 'Size' value into size_t.");
						}
					}
					else {
						throw gcnew PSH5XException("COMPOUND: No 'Size' key found.");
					}

					if (dict->ContainsKey("Members"))
					{
						Dictionary<String^,Object^>^ components = nullptr;

						if (TryGetValue(dict["Members"], components))
						{
							for each (String^ key in components->Keys)
							{
								name = (char*)(Marshal::StringToHGlobalAnsi(key)).ToPointer();

								array<Object^>^ comp = nullptr;

								if (TryGetValue(components[key], comp))
								{
									if (comp->Length != 2) {
										throw gcnew HDF5Exception(
											String::Format("COMPOUND: Invalid member definition for key '{0}' found!", key));
									}

									size_t offset = 0;
									if (ProviderUtils::TryGetValue(comp[0], offset))
									{
										dtype = ParseH5Type(comp[1]);
										if (dtype >= 0)
										{
											if (H5Tinsert(result, name, offset, dtype) < 0)
											{
												throw gcnew HDF5Exception(
													String::Format("COMPOUND: H5Tinsert failed on member '{0}'!", key));
											}
										}
										else {
											throw gcnew PSH5XException(
												String::Format("COMPOUND: Unsupported member type '{0}' for component '{1}'",
												comp[1]->ToString(), key));
										}

										if (H5Tclose(dtype)) {
											throw gcnew HDF5Exception("H5Tclose failed!");
										}
										dtype = -1;
									}
									else {
										throw gcnew PSH5XException(
											String::Format("COMPOUND: Unable to determine the member offset for '{0}'", key));
									}


									Marshal::FreeHGlobal(IntPtr(name));
									name = NULL;
								}
							}

						}
						else {
							throw gcnew PSH5XException("COMPOUND: No 'Members' key found.");
						}
					}

#pragma endregion
				}
				else if (type_class == "REFERENCE")
				{
#pragma region REFERENCE

					if (dict->ContainsKey("Kind"))
					{
						String^ ref_type = dict["Kind"]->ToString()->ToUpper();
						if (ref_type == "OBJECT") {
							result = H5Tcopy(H5T_STD_REF_OBJ);
							if (result < 0) {
								throw gcnew HDF5Exception("H5Tcopy failed!");
							}
						}
						else if (ref_type == "REGION") {
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
					// TODO: finish this and support arbitrary native integer types

					if (dict->ContainsKey("Base"))
					{
						String^ ref_type = dict["Base"]->ToString()->ToUpper();
						base_type = ParseH5Type(dict["Base"]);
						if (base_type < 0) {
							throw gcnew PSH5XException(
								String::Format("ENUM: Invalid base type '{0}'!", ref_type));
						}

						if (H5Tget_class(base_type) != H5T_INTEGER) {
							throw gcnew PSH5XException("ENUM: 'Base' must be an integer type!");
						}
					}
					else {
						throw gcnew PSH5XException("ENUM: No 'Base' key found.");
					}

					if (dict->ContainsKey("Members"))
					{
						result = H5Tenum_create(base_type);
						if (result < 0) {
							throw gcnew HDF5Exception("H5Tenum_create failed!");
						}

						Dictionary<String^,Object^>^ members = nullptr;
						if (TryGetValue(dict["Members"], members))
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

					if (dict->ContainsKey("Base"))
					{
						String^ ref_type = dict["Base"]->ToString()->ToUpper();
						base_type = ParseH5Type(dict["Base"]);
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
						throw gcnew PSH5XException("VLEN: No 'Base' key found.");
					}

#pragma endregion
				}
				else if (type_class == "ARRAY")
				{
#pragma region ARRAY

					if (dict->ContainsKey("Base"))
					{
						base_type = ParseH5Type(dict["Base"]);
						if (base_type < 0) {
							throw gcnew PSH5XException("ARRAY: No 'Base' key found.");
						}

						if (dict->ContainsKey("Dims"))
						{
							array<hsize_t>^ dims = nullptr;

							if (ProviderUtils::TryGetValue(dict["Dims"], dims))
							{
								if (dims->Length > 0 && dims->Length <= H5S_MAX_RANK)
								{
									unsigned rank = safe_cast<unsigned>(dims->Length);
									hsize_t tmp[H5S_MAX_RANK];
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
										String::Format("ARRAY: The 'Dims' array's rank must not exceed '{0}'.", H5S_MAX_RANK));
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
							String::Format("ARRAY: Unsupported base type: '{0}'", dict["Type"]));
					}

#pragma endregion
				}
			}
			else {
				throw gcnew PSH5XException("No 'Class' name found.");
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

    hid_t ProviderUtils::H5Type(String^ type)
    {
        hid_t result = -1;

        String^ t = type->Trim()->ToUpper();

		bool isString = false;

        if (m_known_types->ContainsKey(t))
        {
            result = H5Tcopy((hid_t) m_known_types[t]);
            if (result < 0) {
                throw gcnew HDF5Exception("H5Tcopy failed!");
            }
        }
		else if (t->StartsWith("CSTRING"))
        {
#pragma region fixed-length C-style ASCII string

			if (t == "CSTRING") {
				throw gcnew HDF5Exception("You must specify a string length including the null terminator!");
			}
			else
			{
				size_t size = 0;
				if (ProviderUtils::TryGetValue(t->Substring(7), size))
				{
					if (size >= 1)
					{
						result = H5Tcreate(H5T_STRING, size);
						if (result < 0) {
							throw gcnew HDF5Exception("H5Tcreate failed!");
						}
						isString = true;
						// C
						if (H5Tset_strpad(result, H5T_STR_NULLTERM) < 0) {
							throw gcnew HDF5Exception("H5Tset_strpad failed!"); 
						}
					}
					else {
						throw gcnew PSH5XException("Fixed-length strings must be at least 1 character long!");
					}
				}
			}

#pragma endregion
		}
		else if (t->StartsWith("FSTRING"))
		{
#pragma region fixed-length FORTRAN-style ASCII string

			if (t == "FSTRING") {
				throw gcnew HDF5Exception("You must specify a (non-zero) string length!");
			}
			else
			{
				size_t size = 0;
				if (ProviderUtils::TryGetValue(t->Substring(7), size))
				{
					if (size >= 1)
					{
						result = H5Tcreate(H5T_STRING, size);
						if (result < 0) {
							throw gcnew HDF5Exception("H5Tcreate failed!");
						}
						isString = true;
						// C
						if (H5Tset_strpad(result, H5T_STR_SPACEPAD) < 0) {
							throw gcnew HDF5Exception("H5Tset_strpad failed!"); 
						}
					}
					else {
						throw gcnew PSH5XException("Fixed-length strings must be at least 1 character long!");
					}
				}
			}

#pragma endregion
		}
		else if (t->StartsWith("STRING"))
        {
#pragma region fixed-length FORTRAN or variable-length ASCII string

			if (t == "STRING")
            {
                result = H5Tcreate(H5T_STRING, H5T_VARIABLE);
                if (result < 0) {
                    throw gcnew HDF5Exception("H5Tcreate failed!");
                }
				isString = true;
            }
			else
			{
				size_t size = 0;
				if (ProviderUtils::TryGetValue(t->Substring(6), size))
				{
					if (size >= 1)
					{
						result = H5Tcreate(H5T_STRING, size);
						if (result < 0) {
							throw gcnew HDF5Exception("H5Tcreate failed!");
						}
						isString = true;
						// FORTRAN!!!
						if (H5Tset_strpad(result, H5T_STR_SPACEPAD) < 0) {
							throw gcnew HDF5Exception("H5Tset_strpad failed!"); 
						}
					}
					else {
						throw gcnew PSH5XException("Fixed-length strings must be at least 1 character long!");
					}
				}
			}

#pragma endregion
		}
		else if (t->StartsWith("UCSTRING"))
        {
#pragma region fixed-length C-style UTF-8 string

			if (t == "UCSTRING") {
				throw gcnew HDF5Exception("You must specify a string length including the null terminator!");
			}
			else
			{
				size_t size = 0;
				if (ProviderUtils::TryGetValue(t->Substring(8), size))
				{
					if (size >= 1)
					{
						result = H5Tcreate(H5T_STRING, size);
						if (result < 0) {
							throw gcnew HDF5Exception("H5Tcreate failed!");
						}
						isString = true;
						// C
						if (H5Tset_strpad(result, H5T_STR_NULLTERM) < 0) {
							throw gcnew HDF5Exception("H5Tset_strpad failed!"); 
						}
						if (H5Tset_cset(result, H5T_CSET_UTF8) < 0) {
							throw gcnew HDF5Exception("H5Tset_cset failed!");
						}
					}
					else {
						throw gcnew PSH5XException("Fixed-length strings must be at least 1 character long!");
					}
				}
			}

#pragma endregion
		}
		else if (t->StartsWith("UFSTRING"))
        {
#pragma region fixed-length FORTRAN-style UTF-8 string

			if (t == "UFSTRING") {
                throw gcnew HDF5Exception("You must specify a (non-zero) string length!");
            }
			else
			{
				size_t size = 0;
				if (ProviderUtils::TryGetValue(t->Substring(8), size))
				{
					if (size >= 1)
					{
						result = H5Tcreate(H5T_STRING, size);
						if (result < 0) {
							throw gcnew HDF5Exception("H5Tcreate failed!");
						}
						isString = true;
						// FORTRAN !!!
						if (H5Tset_strpad(result, H5T_STR_SPACEPAD) < 0) {
							throw gcnew HDF5Exception("H5Tset_strpad failed!");
						}
						if (H5Tset_cset(result, H5T_CSET_UTF8) < 0) {
							throw gcnew HDF5Exception("H5Tset_cset failed!");
						}
					}
					else {
						throw gcnew PSH5XException("Fixed-length strings must be at least 1 character long!");
					}
				}
			}

#pragma endregion
		}
		else if (t->StartsWith("USTRING"))
        {
#pragma region fixed-length FORTRAN or variable-length UTF-8 string

            if (t == "USTRING") // variable-length UTF-8 string
            {
                result = H5Tcreate(H5T_STRING, H5T_VARIABLE);
                if (result < 0) {
                    throw gcnew HDF5Exception("H5Tcreate failed!");
                }
				isString = true;
            }
			else
			{
				size_t size = 0;
				if (ProviderUtils::TryGetValue(t->Substring(7), size))
				{
					if (size >= 1)
					{
						result = H5Tcreate(H5T_STRING, size);
						if (result < 0) {
							throw gcnew HDF5Exception("H5Tcreate failed!");
						}
						isString = true;
						// FORTRAN !!!
						if (H5Tset_strpad(result, H5T_STR_SPACEPAD) < 0) {
							throw gcnew HDF5Exception("H5Tset_strpad failed!");
						}
					}
					else {
						throw gcnew PSH5XException("Fixed-length strings must be at least 1 characters long!");
					}
				}
			}

			if (H5Tset_cset(result, H5T_CSET_UTF8) < 0) {
				throw gcnew HDF5Exception("H5Tset_cset failed!");
			}

#pragma endregion
		}
		else if (t->StartsWith("OPAQUE"))
		{
			size_t size = 0;
			String^ tag = nullptr;

			if (t->IndexOf('[') != -1 && t->EndsWith("]"))
			{
				if (ProviderUtils::TryGetValue(t->Substring(6, t->IndexOf('[')-6), size))
				{
					tag = type->Trim()->Substring(t->IndexOf('[')+1, t->LastIndexOf(']')-t->IndexOf('[')-1);
					Console::WriteLine(tag);
					if (tag->Length > H5T_OPAQUE_TAG_MAX) {
						throw gcnew PSH5XException(
							String::Format("The tag length must not exceed {0}!", H5T_OPAQUE_TAG_MAX));
					}
					result = H5Tcreate(H5T_OPAQUE, size);
					if (result < 0) {
						throw gcnew HDF5Exception("H5Tcreate failed!");
					}
					if (H5Tset_tag(result, (char*) Marshal::StringToHGlobalAnsi(tag).ToPointer()) < 0) {
						throw gcnew HDF5Exception("H5Tset_tag failed!");
					}
				}
				else {
					throw gcnew PSH5XException("Unable to determine size of opaque type! Syntax error?");
				}
			}
			else {
				if (ProviderUtils::TryGetValue(t->Substring(6), size))
				{
					result = H5Tcreate(H5T_OPAQUE, size);
					if (result < 0) {
						throw gcnew HDF5Exception("H5Tcreate failed!");
					}
				}
				else {
					throw gcnew PSH5XException("Unable to determine size of opaque type! Syntax error?");
				}
			}

			Console::WriteLine(size);
		}

        if (result < 0) {
            throw gcnew PSH5XException(String::Format("Unknown type: '{0}'", t));
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

        hid_t ntype = -1, super = -1, atype = -1;

        try
        {
			switch (cls)
			{
			case H5T_INTEGER:
			case H5T_BITFIELD:
			case H5T_ENUM:
				{
					if (cls == H5T_INTEGER) {
						ntype = H5Tget_native_type(dtype, H5T_DIR_ASCEND);
					}
					else if (cls == H5T_BITFIELD) {
						ntype = H5Tget_native_type(dtype, H5T_DIR_DESCEND);
					}
					else if (cls == H5T_ENUM)  {
						super = H5Tget_super(dtype);
						if (super < 0) {
							throw gcnew HDF5Exception("H5Tget_super failed!");
						}
						ntype = H5Tget_native_type(super, H5T_DIR_ASCEND);
					}

					if (ntype < 0) {
						throw gcnew HDF5Exception("H5Tget_native_type failed!");
					}

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
					else if (H5Tequal(ntype, H5T_NATIVE_B8)  > 0) { result = Byte::typeid;   }
					else if (H5Tequal(ntype, H5T_NATIVE_B16) > 0) { result = UInt16::typeid; }
					else if (H5Tequal(ntype, H5T_NATIVE_B32) > 0) { result = UInt32::typeid; }
					else if (H5Tequal(ntype, H5T_NATIVE_B64) > 0) { result = UInt64::typeid; }
				}
				break;

			case H5T_FLOAT:
				{
					ntype = H5Tget_native_type(dtype, H5T_DIR_ASCEND);
					if (ntype < 0) {
						throw gcnew HDF5Exception("H5Tget_native_type failed!");
					}

					if      (H5Tequal(ntype, H5T_NATIVE_FLOAT) > 0)   { result = Single::typeid; }
					else if (H5Tequal(ntype, H5T_NATIVE_DOUBLE) > 0)  { result = Double::typeid; }
					else if (H5Tequal(ntype, H5T_NATIVE_LDOUBLE) > 0) { result = Double::typeid; }
				}
				break;

			case H5T_STRING:
				{
					result = String::typeid;
				}
				break;

			case H5T_OPAQUE:
				{
					result = array<unsigned char>::typeid;
				}
				break;

			case H5T_REFERENCE:
				{
					if (H5Tequal(dtype, H5T_STD_REF_OBJ) > 0) {
						result = String::typeid;
					}
					else if (H5Tequal(dtype, H5T_STD_REF_DSETREG) > 0) {
						result = KeyValuePair<String^,Array^>::typeid;
					}
					else {
						throw gcnew PSH5XException("Unsupported reference type found!");
					}
				}
				break;

			case H5T_ARRAY:
				{
					super = H5Tget_super(dtype);

					Type^ t = H5Type2DotNet(super);
					if (t != nullptr)
					{
						int rank = H5Tget_array_ndims(dtype);
						if (rank < 0) {
							throw gcnew HDF5Exception("H5Tget_array_ndims failed!!!");
						}

						array<hsize_t>^ adims = gcnew array<hsize_t>(rank);
						pin_ptr<hsize_t> adims_ptr = &adims[0];
						rank = H5Tget_array_dims2(dtype, adims_ptr);
						if (rank < 0) {
							throw gcnew HDF5Exception("H5Tget_array_dims2 failed!!!");
						}
						
						array<int>^ dims = gcnew array<int>(rank);
						for (int i = 0; i < rank; ++i) {
							dims[i] = safe_cast<int>(adims[i]);
						}

						Array^ dummy = Array::CreateInstance(t, dims);
						result = dummy->GetType();
					}
					else {
						throw gcnew PSH5XException("Unsupported array base type!");
					}
				}
				break;

                case H5T_VLEN:
				{
					super = H5Tget_super(dtype);

					Type^ t = H5Type2DotNet(super);
					if (t != nullptr)
					{
						Array^ dummy = Array::CreateInstance(t, 0);
						result = dummy->GetType();
					}
					else {
						throw gcnew PSH5XException("Unsupported vlen base type!");
					}
				}
				break;

			default:

				throw gcnew PSH5XException("Unsupported type class!");
				break;
			}
        }
        finally
        {
			if (atype >= 0) {
                H5Tclose(atype);
            }
            if (ntype >= 0) {
                H5Tclose(ntype);
            }
            if (super >= 0) {
                H5Tclose(super);
            }
        }

        return result;
    }

    __wchar_t ProviderUtils::TypeCode(Type^ t, H5T_class_t cls)
    {
		__wchar_t result = '\0';

		switch (cls)
		{
		case H5T_INTEGER:
		case H5T_BITFIELD:
		case H5T_ENUM:
			{
				if (t == System::SByte::typeid) {
					result = 'b';
				}
				else if (t == System::Byte::typeid) {
					result = 'B';
				}
				else if (t == System::Int16::typeid) {
					result = 'h';
				}
				else if (t == System::UInt16::typeid) {
					result = 'H';
				}
				else if (t == System::Int32::typeid) {
					result = 'i';
				}
				else if (t == System::UInt32::typeid) {
					result = 'I';
				}
				else if (t == System::Int64::typeid) {
					result = 'l';
				}
				else if (t == System::UInt64::typeid) {
					result = 'L';
				}
				else {
					throw gcnew PSH5XException("Cannot find type code for this type!");
				}
			}
			break;

		case H5T_FLOAT:
			{
				if (t == System::Single::typeid) {
					result = 'f';
				}
				else if (t == System::Double::typeid) {
					result = 'd';
				}
				else {
					throw gcnew PSH5XException("Cannot find type code for this type!");
				}
			}
			break;

		case H5T_STRING:
			{
				if (t == System::String::typeid) {
					result = 's';
				}
				else {
					throw gcnew PSH5XException("Cannot find type code for this type!");
				}
			}
			break;

		case H5T_ARRAY:
			{
				if (t->IsArray) {
					return 'A';
				}
				else {
					throw gcnew PSH5XException("Cannot find type code for this type!");
				}
			}
			break;

		default:
			throw gcnew PSH5XException("Cannot find type code for this class!");
			break;
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

	MethodInfo^ ProviderUtils::BinaryReaderMethod(hid_t type_id)
    {
        MethodInfo^ minfo = nullptr;

        Type^ magicType = System::IO::BinaryReader::typeid;

        hid_t super = -1;

        try
        {
			Type^ t = ProviderUtils::H5Type2DotNet(type_id);

            H5T_class_t cls = H5Tget_class(type_id);
            switch (cls)
            {
            case H5T_INTEGER:

				if (t == SByte::typeid) {
                    minfo = magicType->GetMethod("ReadSByte");
                }
                else if (t == Int16::typeid) {
                    minfo = magicType->GetMethod("ReadInt16");
                }
                else if (t == Int32::typeid) {
                    minfo = magicType->GetMethod("ReadInt32");
                }
                else if (t == Int64::typeid) {
                    minfo = magicType->GetMethod("ReadInt64");
                }
				else if (t == Byte::typeid) {
                    minfo = magicType->GetMethod("ReadByte");
                }
                else if (t == UInt16::typeid) {
                    minfo = magicType->GetMethod("ReadUInt16");
                }
				else if (t == UInt32::typeid) {
                    minfo = magicType->GetMethod("ReadUInt32");
                }
                else if (t == UInt64::typeid) {
                    minfo = magicType->GetMethod("ReadUInt64");
                }
                
                break;

            case H5T_FLOAT:

                if (t == Double::typeid) {
                    minfo = magicType->GetMethod("ReadDouble");
                }
                else if (t == Single::typeid) {
                    minfo = magicType->GetMethod("ReadSingle");
                }
                break;

			case H5T_BITFIELD:

				if (t == Byte::typeid) {
                    minfo = magicType->GetMethod("ReadByte");
                }
                else if (t == UInt16::typeid) {
                    minfo = magicType->GetMethod("ReadUInt16");
                }
				else if (t == UInt32::typeid) {
                    minfo = magicType->GetMethod("ReadUInt32");
                }
                else if (t == UInt64::typeid) {
                    minfo = magicType->GetMethod("ReadUInt64");
                }

				break;

            case H5T_ENUM:

                super = H5Tget_super(type_id);

				t = ProviderUtils::H5Type2DotNet(super);

                if (t == SByte::typeid) {
                    minfo = magicType->GetMethod("ReadSByte");
                }
                else if (t == Int16::typeid) {
                    minfo = magicType->GetMethod("ReadInt16");
                }
                else if (t == Int32::typeid) {
                    minfo = magicType->GetMethod("ReadInt32");
                }
                else if (t == Int64::typeid) {
                    minfo = magicType->GetMethod("ReadInt64");
                }
				else if (t == Byte::typeid) {
                    minfo = magicType->GetMethod("ReadByte");
                }
                else if (t == UInt16::typeid) {
                    minfo = magicType->GetMethod("ReadUInt16");
                }
				else if (t == UInt32::typeid) {
                    minfo = magicType->GetMethod("ReadUInt32");
                }
                else if (t == UInt64::typeid) {
                    minfo = magicType->GetMethod("ReadUInt64");
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

        //size_t size = H5Tget_size(type_id);

        int member_count = H5Tget_nmembers(type_id);
        if (member_count < 0) {
            throw gcnew HDF5Exception("H5Tget_nmembers failed!");
        }

        array<String^>^ member_name = gcnew array<String^>(member_count);
        array<String^>^ member_type = gcnew array<String^>(member_count);
        array<int>^ member_size = gcnew array<int>(member_count);
        array<int>^ member_offset = gcnew array<int>(member_count);
        array<__wchar_t>^ member_tcode = gcnew array<__wchar_t>(member_count);
		array<bool>^ member_is_array = gcnew array<bool>(member_count);
		array<String^>^ array_member_dims = gcnew array<String^>(member_count);

        StringBuilder^ sbname = gcnew StringBuilder();
        StringBuilder^ sbconstr = gcnew StringBuilder("(");

        char* name = NULL;

        hid_t mtype = -1, stype = -1, ntype = -1;

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

				H5T_class_t cls = H5Tget_class(mtype);

				if (cls == H5T_ARRAY)
				{
					StringBuilder^ sb = gcnew StringBuilder();
					member_is_array[i] = true;
					int rank = H5Tget_array_ndims(mtype);
					array<hsize_t>^ adims = gcnew array<hsize_t>(rank);
					pin_ptr<hsize_t> adims_ptr = &adims[0];
					rank = H5Tget_array_dims2(mtype, adims_ptr);
					sb->Append("[");
					for (int r = 0; r < rank; ++r) {
						if (r > 0) {
							sb->Append("," + safe_cast<int>(adims[r]).ToString());
						}
						else {
							sb->Append(safe_cast<int>(adims[r]).ToString());
						}
					}
					sb->Append("]");
					array_member_dims[i] = sb->ToString();
				}

                if (cls == H5T_BITFIELD) {
                    ntype = H5Tget_native_type(mtype, H5T_DIR_DESCEND);
                }
				else if (cls == H5T_ENUM)
				{
					stype = H5Tget_super(mtype);
					if (stype < 0) {
						throw gcnew HDF5Exception("H5Tget_super failed!");
					}

                    ntype = H5Tget_native_type(stype, H5T_DIR_ASCEND);

					if (H5Tclose(stype) < 0) {
						throw gcnew HDF5Exception("H5Tclose failed!");
					}
					stype = -1;
                }
                else {
                    ntype = H5Tget_native_type(mtype, H5T_DIR_ASCEND);
                }

                if (ntype < 0) {
                    throw gcnew HDF5Exception("H5Tget_native_type failed!");
                }

				if (!ProviderUtils::IsH5SimpleType(mtype))
				{
					if (cls == H5T_OPAQUE || cls == H5T_VLEN || cls == H5T_COMPOUND) {
						throw gcnew PSH5XException("Unsupported compound member type!");
					}
					else
					{
						stype = H5Tget_super(mtype);
						if (stype < 0) {
							throw gcnew HDF5Exception("H5Tget_super failed!");
						}

						if (!ProviderUtils::IsH5SimpleType(stype)) {
							throw gcnew PSH5XException("Unsupported compound member type!");
						}

						if (H5Tclose(stype) < 0) {
							throw gcnew HDF5Exception("H5Tclose failed!");
						}
						stype = -1;
					}
				}

                member_size[i] = safe_cast<int>(H5Tget_size(ntype));

                Type^ t = ProviderUtils::H5Type2DotNet(ntype);
                if (t != nullptr)
                {
                    member_type[i]  = t->ToString();
                    member_tcode[i] = ProviderUtils::TypeCode(t, cls);
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

			/*
			// patch member offsets for objects (arrays and strings)

			int mod = (IntPtr::Size == 4) ? 4 : 8;

			for (int i = 0; i < member_count; ++i)
            {
				int shift = 0;

				if (member_is_array[i] || member_tcode[i] == 's')
				{
					member_offset[i] += shift;
					int res = member_offset[i]%mod;

					if (res != 0)
					{
						shift += res;
						member_offset[i] += res;
					}
				}
			}
			*/

            sbconstr->Append(") {");

            String^ class_name = sbname->ToString();

            StringBuilder^ sbcode = gcnew StringBuilder();
            //sbcode->Append("using System.Runtime.InteropServices; ");
            //sbcode->Append("[StructLayout(LayoutKind.Explicit,Size= " + size + ",CharSet=CharSet.Ansi)] ");
            sbcode->Append("public class " + class_name + " { ");

            StringBuilder^ sb_def_constr = gcnew StringBuilder();
            sb_def_constr->Append(" public " + class_name + "() {");

            for (int i = 0; i < member_count; ++i)
            {
                //sbcode->Append("[FieldOffset(" + member_offset[i] + ")]");
				if (!member_is_array[i]) {
					sbcode->Append(" public " + member_type[i] + " " + member_name[i] + ";");
				}
				else {
					String^ arrString = member_type[i]->Substring(0, member_type[i]->IndexOf('['))
						+ array_member_dims[i];
					sbcode->Append(" public " + member_type[i] + " " + member_name[i] + " = new " + arrString + ";");
				}

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
			if (stype >= 0) {
                H5Tclose(stype);
            }
        }

        return result;
    }

    bool ProviderUtils::IsH5SimpleType(hid_t dtype)
    {
        H5T_class_t cls = H5Tget_class(dtype);

        return (cls != H5T_OPAQUE && cls != H5T_COMPOUND && cls != H5T_VLEN && cls != H5T_ARRAY);
    }

	hid_t ProviderUtils::GetH5MemoryType(Type^ t, hid_t ftype)
	{
		hid_t result = -1, ntype = -1, stype = -1, fcmtype = -1, mcmtype = -1;
		H5T_class_t cls = H5Tget_class(ftype);

		try
		{
			switch (cls)
			{
			case H5T_INTEGER:
#pragma region integer
				{
					if (!t->IsPrimitive) {
						throw gcnew PSH5XException("INTEGER: Unable to map non-primitive type to HDF5 integer type!");
					}

					ntype = H5Tget_native_type(ftype, H5T_DIR_ASCEND);

					switch (H5Tget_sign(ftype))
					{
					case H5T_SGN_NONE:
						{
							if (t != Byte::typeid && t != UInt16::typeid && t != UInt32::typeid && t != UInt64::typeid) {
								throw gcnew PSH5XException("INTEGER: Primitive type is not a supported unsigned integer type!");
							}

							if (H5Tequal(ntype, H5T_NATIVE_UCHAR))
							{
								if (t == Byte::typeid) {
									result = H5Tcopy(H5T_NATIVE_UCHAR);
								}
								else {
									throw gcnew PSH5XException("INTEGER: Unable to map primitive type!");
								}
							}
							else if (H5Tequal(ntype, H5T_NATIVE_USHORT))
							{
								if (t == Byte::typeid) {
									result = H5Tcopy(H5T_NATIVE_UCHAR);
								}
								else if (t == UInt16::typeid) {
									result = H5Tcopy(H5T_NATIVE_USHORT);
								}
								else {
									throw gcnew PSH5XException("INTEGER: Unable to map primitive type!");
								}
							}
							else if (H5Tequal(ntype, H5T_NATIVE_UINT))
							{
								if (t == Byte::typeid) {
									result = H5Tcopy(H5T_NATIVE_UCHAR);
								}
								else if (t == UInt16::typeid) {
									result = H5Tcopy(H5T_NATIVE_USHORT);
								}
								else if (t == UInt32::typeid) {
									result = H5Tcopy(H5T_NATIVE_UINT);
								}
								else {
									throw gcnew PSH5XException("INTEGER: Unable to map primitive type!");
								}
							}
							else if (H5Tequal(ntype, H5T_NATIVE_ULONG))
							{
								if (t == Byte::typeid) {
									result = H5Tcopy(H5T_NATIVE_UCHAR);
								}
								else if (t == UInt16::typeid) {
									result = H5Tcopy(H5T_NATIVE_USHORT);
								}
								else if (t == UInt32::typeid) {
									result = H5Tcopy(H5T_NATIVE_UINT);
								}
								else {
									throw gcnew PSH5XException("INTEGER: Unable to map primitive type!");
								}
							}
							else if (H5Tequal(ntype, H5T_NATIVE_ULLONG))
							{
								if (t == Byte::typeid) {
									result = H5Tcopy(H5T_NATIVE_UCHAR);
								}
								else if (t == UInt16::typeid) {
									result = H5Tcopy(H5T_NATIVE_USHORT);
								}
								else if (t == UInt32::typeid) {
									result = H5Tcopy(H5T_NATIVE_UINT);
								}
								else if (t == UInt64::typeid) {
									result = H5Tcopy(H5T_NATIVE_ULLONG);
								}
								else {
									throw gcnew PSH5XException("INTEGER: Unable to map primitive type!");
								}
							}
							else {
								throw gcnew PSH5XException("INTEGER: Unsupported HDF5 integer datatype!");
							}
						}
						break;
					case H5T_SGN_2:
						{
							if (t != SByte::typeid && t != Int16::typeid && t != Int32::typeid && t != Int64::typeid) {
								throw gcnew PSH5XException("INTEGER: Primitive type is not a supported signed integer type!");
							}

							if (H5Tequal(ntype, H5T_NATIVE_SCHAR))
							{
								if (t == SByte::typeid) {
									result = H5Tcopy(H5T_NATIVE_SCHAR);
								}
								else {
									throw gcnew PSH5XException("INTEGER: Unable to map primitive type!");
								}
							}
							else if (H5Tequal(ntype, H5T_NATIVE_SHORT))
							{
								if (t == SByte::typeid) {
									result = H5Tcopy(H5T_NATIVE_SCHAR);
								}
								else if (t == Int16::typeid) {
									result = H5Tcopy(H5T_NATIVE_SHORT);
								}
								else {
									throw gcnew PSH5XException("INTEGER: Unable to map primitive type!");
								}
							}
							else if (H5Tequal(ntype, H5T_NATIVE_INT))
							{
								if (t == SByte::typeid) {
									result = H5Tcopy(H5T_NATIVE_SCHAR);
								}
								else if (t == Int16::typeid) {
									result = H5Tcopy(H5T_NATIVE_SHORT);
								}
								else if (t == Int32::typeid) {
									result = H5Tcopy(H5T_NATIVE_INT);
								}
								else {
									throw gcnew PSH5XException("INTEGER: Unable to map primitive type!");
								}
							}
							else if (H5Tequal(ntype, H5T_NATIVE_LONG))
							{
								if (t == SByte::typeid) {
									result = H5Tcopy(H5T_NATIVE_SCHAR);
								}
								else if (t == Int16::typeid) {
									result = H5Tcopy(H5T_NATIVE_SHORT);
								}
								else if (t == Int32::typeid) {
									result = H5Tcopy(H5T_NATIVE_INT);
								}
								else {
									throw gcnew PSH5XException("INTEGER: Unable to map primitive type!");
								}
							}
							else if (H5Tequal(ntype, H5T_NATIVE_LLONG))
							{
								if (t == SByte::typeid) {
									result = H5Tcopy(H5T_NATIVE_SCHAR);
								}
								else if (t == Int16::typeid) {
									result = H5Tcopy(H5T_NATIVE_SHORT);
								}
								else if (t == Int32::typeid) {
									result = H5Tcopy(H5T_NATIVE_INT);
								}
								else if (t == Int64::typeid) {
									result = H5Tcopy(H5T_NATIVE_LLONG);
								}
								else {
									throw gcnew PSH5XException("INTEGER: Unable to map primitive type!");
								}
							}
							else {
								throw gcnew PSH5XException("INTEGER: Unsupported HDF5 integer datatype!");
							}
						}
						break;

					default:
						throw gcnew PSH5XException("INTEGER: Unknown sign type!");
						break;
					}
				}
#pragma endregion
				break;


			case H5T_FLOAT:
#pragma region float
				{
					if (!t->IsPrimitive) {
						throw gcnew PSH5XException("FLOAT: Unable to map non-primitive type to HDF5 floating point type!");
					}
					if (t != Single::typeid && t != Double::typeid) {
						throw gcnew PSH5XException("FLOAT: Primitive type is not a floating point type!");
					}

					ntype = H5Tget_native_type(ftype, H5T_DIR_ASCEND);

					if (H5Tequal(ntype, H5T_NATIVE_FLOAT))
					{
						if (t == Single::typeid) {
							result = H5Tcopy(H5T_NATIVE_FLOAT);
						}
						else {
							throw gcnew PSH5XException("FLOAT: Unable to map primitive type!");
						}
					}
					else if (H5Tequal(ntype, H5T_NATIVE_DOUBLE))
					{
						if (t == Single::typeid) {
							result = H5Tcopy(H5T_NATIVE_FLOAT);
						}
						else if (t == Double::typeid) {
							result = H5Tcopy(H5T_NATIVE_DOUBLE);
						}
						else {
							throw gcnew PSH5XException("FLOAT: Unable to map primitive type!");
						}
					}
					else if (H5Tequal(ntype, H5T_NATIVE_LDOUBLE))
					{
						if (t == Single::typeid) {
							result = H5Tcopy(H5T_NATIVE_FLOAT);
						}
						else if (t == Double::typeid) {
							result = H5Tcopy(H5T_NATIVE_DOUBLE);
						}
						else {
							throw gcnew PSH5XException("FLOAT: Unable to map primitive type!");
						}
					}
					else {
						throw gcnew PSH5XException("FLOAT: Unsupported HDF5 floating point datatype!");
					}
				}
#pragma endregion
				break;

			case H5T_BITFIELD:
#pragma region bitfield
				{
					if (!t->IsPrimitive) {
						throw gcnew PSH5XException("BITFIELD: Unable to map non-primitive type to HDF5 integer type!");
					}

					ntype = H5Tget_native_type(ftype, H5T_DIR_DESCEND);

					if (H5Tequal(ntype, H5T_NATIVE_B8))
					{
						if (t == Byte::typeid) {
							result = H5Tcopy(H5T_NATIVE_B8);
						}
						else {
							throw gcnew PSH5XException("BITFIELD: Unable to map primitive type!");
						}
					}
					else if (H5Tequal(ntype, H5T_NATIVE_B16))
					{
						if (t == Byte::typeid) {
							result = H5Tcopy(H5T_NATIVE_B8);
						}
						else if (t == UInt16::typeid) {
							result = H5Tcopy(H5T_NATIVE_B16);
						}
						else {
							throw gcnew PSH5XException("BITFIELD: Unable to map primitive type!");
						}
					}
					else if (H5Tequal(ntype, H5T_NATIVE_B32))
					{
						if (t == Byte::typeid) {
							result = H5Tcopy(H5T_NATIVE_B8);
						}
						else if (t == UInt16::typeid) {
							result = H5Tcopy(H5T_NATIVE_B16);
						}
						else if (t == UInt32::typeid) {
							result = H5Tcopy(H5T_NATIVE_B32);
						}
						else {
							throw gcnew PSH5XException("BITFIELD: Unable to map primitive type!");
						}
					}
					else if (H5Tequal(ntype, H5T_NATIVE_B64))
					{
						if (t == Byte::typeid) {
							result = H5Tcopy(H5T_NATIVE_B8);
						}
						else if (t == UInt16::typeid) {
							result = H5Tcopy(H5T_NATIVE_B16);
						}
						else if (t == UInt32::typeid) {
							result = H5Tcopy(H5T_NATIVE_B32);
						}
						else if (t == UInt64::typeid) {
							result = H5Tcopy(H5T_NATIVE_B64);
						}
						else {
							throw gcnew PSH5XException("BITFIELD: Unable to map primitive type!");
						}
					}
					else {
						throw gcnew PSH5XException("BITFIELD: Unsupported HDF5 bitfield datatype!");
					}

				}
#pragma endregion
				break;

			case H5T_ENUM:
#pragma region enum
				{
					if (!t->IsPrimitive) {
						throw gcnew PSH5XException("Enum: Unable to map non-primitive type to HDF5 enum type!");
					}

					stype = H5Tget_super(ftype);
					if (stype < 0) {
						throw gcnew HDF5Exception("H5Tget_super failed!");
					}
					ntype = H5Tget_native_type(stype, H5T_DIR_ASCEND);

					if (Marshal::SizeOf(t) > H5Tget_size(ntype)) {
						throw gcnew PSH5XException("Primitve type too large for enum!");
					}

					result = H5Tget_native_type(ftype, H5T_DIR_ASCEND);

					switch (H5Tget_sign(stype))
					{
					case H5T_SGN_NONE:
						{
							if (t != Byte::typeid && t != UInt16::typeid && t != UInt32::typeid && t != UInt64::typeid) {
								throw gcnew PSH5XException("Enum: Primitive type is not a supported unsigned integer type!");
							}
						}
						break;
					case H5T_SGN_2:
						{
							if (t != SByte::typeid && t != Int16::typeid && t != Int32::typeid && t != Int64::typeid) {
								throw gcnew PSH5XException("Enum: Primitive type is not a supported signed integer type!");
							}
						}
						break;

					default:
						throw gcnew PSH5XException("Enum: Unknown sign type!");
						break;
					}
				}
#pragma endregion
				break;

			case H5T_STRING:
#pragma region string
				{
					if (t != String::typeid) {
						throw gcnew PSH5XException("STRING: Unable to map type to HDF5 string type!");
					}
					htri_t is_vlen_str = H5Tis_variable_str(ftype);
					if (is_vlen_str < 0) {
						throw gcnew HDF5Exception("H5Tis_variable_str failed!");
					}

					if (is_vlen_str > 0) {
						result = H5Tcreate(H5T_STRING, H5T_VARIABLE);
					}
					else {
						size_t size = H5Tget_size(ftype);
						if (H5Tget_strpad(ftype) == H5T_STR_NULLTERM) {
							result = H5Tcreate(H5T_STRING, size);
						}
						else {
							result = H5Tcreate(H5T_STRING, size+1);
						}
						if (H5Tset_cset(result, H5Tget_cset(ftype)) < 0) {
							throw gcnew HDF5Exception("H5Tset_cset failed!");
						}
					}
				}
#pragma endregion
				break;

			case H5T_VLEN:
#pragma region vlen
				{
					if (!(t->IsArray || t->HasElementType)) {
						throw gcnew PSH5XException("VLEN: Unable to map non-array type to HDF5 variable-length type!");
					}
					Type^ et = t->GetElementType();
					stype = H5Tget_super(ftype);
					if (stype < 0) {
						throw gcnew HDF5Exception("H5Tget_super failed!");
					}
					result = H5Tvlen_create(GetH5MemoryType(et, stype));
				}
#pragma endregion
				break;

			case H5T_ARRAY:
#pragma region array
				{
					if (!(t->IsArray || t->HasElementType)) {
						throw gcnew PSH5XException("ARRAY: Unable to map non-array type to HDF5 array type!");
					}
					Type^ et = t->GetElementType();
					stype = H5Tget_super(ftype);
					int rank = H5Tget_array_ndims(ftype);
					if (rank < 0) {
						throw gcnew HDF5Exception("H5Tget_array_ndims failed!");
					}
					array<hsize_t>^ dims = gcnew array<hsize_t>(rank);
					pin_ptr<hsize_t> dims_ptr = &dims[0];
					rank =  H5Tget_array_dims2(ftype, dims_ptr);
					result = H5Tarray_create2(GetH5MemoryType(et, stype), rank, dims_ptr);
				}
#pragma endregion
				break;

			case H5T_COMPOUND:
#pragma region compound
				{
					array<FieldInfo^>^ info = t->GetFields(BindingFlags::Public|BindingFlags::Instance);
					if (info == nullptr) {
						throw gcnew PSH5XException("The .NET type has no public instance fields!");
					}
					if (info->Length != H5Tget_nmembers(ftype)) {
						throw gcnew PSH5XException("Field count / member count mismatch!");
					}

					size_t size = 0;
					array<size_t>^ offset = gcnew array<size_t>(info->Length);

					// we have to run through this loop twice to get the total size

					for (int m = 0; m < info->Length; ++m)
					{
						Type^ field_type = info[m]->FieldType;
						fcmtype = H5Tget_member_type(ftype, safe_cast<unsigned int>(m));
						mcmtype = ProviderUtils::GetH5MemoryType(field_type, fcmtype);

						offset[m] = size;
						size += H5Tget_size(mcmtype);

						H5Tclose(mcmtype);
						mcmtype = -1;
						H5Tclose(fcmtype);
						fcmtype = -1;
					}

					result = H5Tcreate(H5T_COMPOUND, size);
					if (result < 0) {
						throw gcnew HDF5Exception("H5Tcreate failed!");
					}

					for (int m = 0; m < info->Length; ++m)
					{
						char* name = H5Tget_member_name(ftype, safe_cast<unsigned int>(m));
						
						Type^ field_type = info[m]->FieldType;
						fcmtype = H5Tget_member_type(ftype, safe_cast<unsigned int>(m));
						mcmtype = ProviderUtils::GetH5MemoryType(field_type, fcmtype);
						
						if (H5Tinsert(result, name, offset[m], mcmtype) < 0) {
							throw gcnew HDF5Exception("H5Tinsert failed!");
						}
						
						H5Tclose(mcmtype);
						mcmtype = -1;
						H5Tclose(fcmtype);
						fcmtype = -1;
					}
				}
#pragma endregion
				break;

			default:

				throw gcnew PSH5XException("Unsupported type class!");
				break;
			}
		}
		finally
		{
			if (mcmtype >= 0) {
				H5Tclose(mcmtype);
			}
			if (fcmtype >= 0) {
				H5Tclose(fcmtype);
			}
			if (stype >= 0) {
				H5Tclose(stype);
			}
			if (ntype >= 0) {
				H5Tclose(ntype);
			}
		}

		return result;
	}

	bool ProviderUtils::TryGetFillValue(hid_t dtype, Object^ value, array<unsigned char>^% fill)
	{
		bool result = false;
		fill = nullptr;

		hid_t ntype = -1;

		IntPtr iptr = IntPtr::Zero;

		try
		{
			Object^ obj = ProviderUtils::GetDotNetObject(value);

			if ((ntype = H5Tget_native_type(dtype, H5T_DIR_ASCEND)) < 0) {
				throw gcnew HDF5Exception("H5Tget_native_type failed!");
			}

			size_t size = H5Tget_size(ntype);

			switch (H5Tget_class(ntype))
			{
			case H5T_INTEGER:
				{
#pragma region integer

					switch (H5Tget_sign(ntype))
					{
					case H5T_SGN_NONE:
						{
							switch (size)
							{
							case 1:
								{
									unsigned char B;
									if (ProviderUtils::TryGetValue(value, B))
									{
										fill = BitConverter::GetBytes(B);
										result = true;
									}
								}
								break;
							case 2:
								{
									unsigned short S;
									if (ProviderUtils::TryGetValue(value, S))
									{
										fill = BitConverter::GetBytes(S);
										result = true;
									}
								}
								break;
							case 4:
								{
									unsigned int I;
									if (ProviderUtils::TryGetValue(value, I))
									{
										fill = BitConverter::GetBytes(I);
										result = true;
									}
								}
								break;
							case 8:
								{
									unsigned long long L;
									if (ProviderUtils::TryGetValue(value, L))
									{
										fill = BitConverter::GetBytes(L);
										result = true;
									}
								}
								break;

							default:
								break;
							}
						}
						break;

					case H5T_SGN_2:
						{
							switch (size)
							{
							case 1:
								{
									char b;
									if (ProviderUtils::TryGetValue(value, b))
									{
										fill = BitConverter::GetBytes(b);
										result = true;
									}
								}
								break;
							case 2:
								{
									short s;
									if (ProviderUtils::TryGetValue(value, s))
									{
										fill = BitConverter::GetBytes(s);
										result = true;
									}
								}
								break;
							case 4:
								{
									int i;
									if (ProviderUtils::TryGetValue(value, i))
									{
										fill = BitConverter::GetBytes(i);
										result = true;
									}
								}
								break;
							case 8:
								{
									long long l;
									if (ProviderUtils::TryGetValue(value, l))
									{
										fill = BitConverter::GetBytes(l);
										result = true;
									}
								}
								break;

							default:
								break;
							}
						}
						break;

					default:
						break;
					}
#pragma endregion
				}
				break;

			case H5T_BITFIELD:
				{
#pragma region bitfield

					switch (size)
					{
					case 1:
						{
							unsigned char B;
							if (ProviderUtils::TryGetValue(value, B))
							{
								fill = BitConverter::GetBytes(B);
								result = true;
							}
						}
						break;
					case 2:
						{
							unsigned short S;
							if (ProviderUtils::TryGetValue(value, S))
							{
								fill = BitConverter::GetBytes(S);
								result = true;
							}
						}
						break;
					case 4:
						{
							unsigned int I;
							if (ProviderUtils::TryGetValue(value, I))
							{
								fill = BitConverter::GetBytes(I);
								result = true;
							}
						}
						break;
					case 8:
						{
							unsigned long long L;
							if (ProviderUtils::TryGetValue(value, L))
							{
								fill = BitConverter::GetBytes(L);
								result = true;
							}
						}
						break;

					default:
						break;
					}
#pragma endregion
				}
				break;

			case H5T_FLOAT:
				{
#pragma region float
					if (size == 4)
					{
						float f;
						if (ProviderUtils::TryGetValue(obj, f))
						{
							fill = BitConverter::GetBytes(f);
							result = true;
						}
					}
					else if (size == 8)
					{
						double d;
						if (ProviderUtils::TryGetValue(obj, d))
						{
							fill = BitConverter::GetBytes(d);
							result = true;
						}
					}
#pragma endregion
				}
				break;

			case H5T_STRING:
				{
#pragma region string
					String^ s;
					if (ProviderUtils::TryGetValue(obj, s))
					{
						// length check for fixed-length strings
						if (H5Tis_variable_str(ntype) == 0)
						{
							if (H5Tget_strpad(ntype) == H5T_STR_NULLTERM)
							{
								if (s->Length > safe_cast<int>(size-1)) {
									throw gcnew PSH5XException("The string fill value is too long for this type!");
								}
							}
							else {
								if (s->Length > safe_cast<int>(size)) {
									throw gcnew PSH5XException("The string fill value is too long for this type!");
								}
							}
						}

						iptr = Marshal::StringToHGlobalAnsi(s);
						fill = gcnew array<unsigned char>(s->Length);
						Marshal::Copy(iptr, fill, 0, s->Length);
						result = true;
					}
#pragma endregion
				}
				break;

			default:
				break;
			}
		}
		finally
		{
			if (iptr != IntPtr::Zero) {
				Marshal::FreeHGlobal(iptr);
			}
			if (ntype >= 0) {
				H5Tclose(ntype);
			}
		}
		
		return result;
	}

	bool ProviderUtils::TryGetFillValue(hid_t dtype, array<unsigned char>^ fill, Object^% value)
	{
		bool result = false;
		fill = nullptr;




		return result;
	}
}