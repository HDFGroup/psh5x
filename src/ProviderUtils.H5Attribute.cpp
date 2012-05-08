
#include "AttributeInfo.h"
#include "HDF5Exception.h"
#include "ProviderUtils.h"
#include "PSH5XException.h"

extern "C" {
#include "H5Apublic.h"
#include "H5Dpublic.h"
#include "H5Spublic.h"
#include "H5Tpublic.h"
}

#include <cstring>

using namespace System;
using namespace System::Collections;
using namespace System::Management::Automation;
using namespace System::Runtime::InteropServices;
using namespace System::Web::Script::Serialization;

namespace PSH5X
{
    Hashtable^ ProviderUtils::H5Attribute(hid_t aid, String^ attributeName)
    {
        Hashtable^ ht = gcnew Hashtable();

        hid_t fspace = -1, ftype = -1, btype = -1, ntype = -1, mtype = -1;

        htri_t is_vlen = -1;

        char** rdata = NULL;
        char** vrdata = NULL;

        int i = 0;

        try
        {
            H5A_info_t info;
            if (H5Aget_info(aid, &info) >= 0)
            {
                AttributeInfo^ attrInfo = gcnew AttributeInfo(attributeName, &info);
                ht->Add("AttributeName", attrInfo->AttributeName);
                ht->Add("CreationOrderIsValid", attrInfo->CreationOrderIsValid);
                ht->Add("CreationOrder", attrInfo->CreationOrder);
                ht->Add("AttributeNameCharacterSet", attrInfo->AttributeNameCharacterSet);
                ht->Add("DataSizeBytes", attrInfo->DataSizeBytes);
                ht->Add("StorageSizeBytes", H5Aget_storage_size(aid));

#pragma region H5Aget_space

                fspace = H5Aget_space(aid);
                if (fspace < 0) {
                    throw gcnew HDF5Exception("H5Aget_space failed!!!");
                }

                H5S_class_t stype = H5Sget_simple_extent_type(fspace);

                switch (stype)
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

                hssize_t npoints = -1;

                if (stype == H5S_SIMPLE)
                {
                    int rank = H5Sget_simple_extent_ndims(fspace);
                    if (rank > 0)
                    {
                        ht->Add("Rank", rank);

                        npoints = H5Sget_simple_extent_npoints(fspace);
                        ht->Add("ElementCount", npoints);

                        rank = H5Sget_simple_extent_ndims(fspace);
                        if (rank >= 0)
                        {
							array<hsize_t>^ adims = gcnew array<hsize_t>(rank);
							pin_ptr<hsize_t> adims_ptr = &adims[0];
							rank = H5Sget_simple_extent_dims(fspace, adims_ptr, NULL);
                            ht->Add("Dimensions", adims);
                        }
                        else {
                            throw gcnew PSH5XException("Rank of simple dataspace must be positive!!!");
                        }
                    }
                    else {
                        throw gcnew PSH5XException("Rank of simple dataspace must be positive!!!");
                    }
                }
                else if (stype == H5S_SCALAR) {
                    npoints = 1;
                }

#pragma endregion

#pragma region H5Aget_type and H5Aread

                H5T_sign_t sign = H5T_SGN_NONE;

                if (npoints >= 1)
                {
                    if ((ftype = H5Aget_type(aid)) < 0) {
                        throw gcnew HDF5Exception("H5Aget_type failed!");
                    }

					JavaScriptSerializer^ serializer = gcnew JavaScriptSerializer();

                    ht["ElementType"] = serializer->Serialize(ProviderUtils::ParseH5Type(ftype));
                    size_t size = H5Tget_size(ftype);

					H5T_class_t cls = H5Tget_class(ftype);

                    switch (cls)
                    {
				    case H5T_BITFIELD:
					case H5T_ENUM:
                    case H5T_INTEGER:
						{
#pragma region HDF5 INTEGER & ENUM & BITFIELD

							if (cls == H5T_BITFIELD) {
								ht->Add("ElementTypeClass", "Bitfield");	
							}
							else if (cls == H5T_ENUM)
							{
								ht->Add("ElementTypeClass", "Enum");
								if ((btype = H5Tget_super(ftype)) < 0) {
									throw gcnew HDF5Exception("H5Tget_super failed!");
								}
								sign = H5Tget_sign(btype);
							}
							else if (cls == H5T_INTEGER) {
								ht->Add("ElementTypeClass", "Integer");
								sign = H5Tget_sign(ftype);
							}

							if (cls != H5T_BITFIELD) {
								if ((mtype = H5Tget_native_type(ftype, H5T_DIR_ASCEND)) < 0) {
									throw gcnew HDF5Exception("H5Tget_native_type failed!");
								}
							}
							else {
								if ((mtype = H5Tget_native_type(ftype, H5T_DIR_DESCEND)) < 0) {
									throw gcnew HDF5Exception("H5Tget_native_type failed!");
								}
							}

							if (sign == H5T_SGN_2)
							{
#pragma region signed
								if (size == 1)
								{
									array<char>^ a = gcnew array<char>(safe_cast<int>(npoints));
									pin_ptr<char> a_ptr = &a[0];
									if (H5Aread(aid, mtype, a_ptr) < 0) {
										throw gcnew HDF5Exception("H5Aread failed!");
									}
									else
									{
										if (npoints > 1) {
											ht->Add("Value", a);
										}
										else {
											ht->Add("Value", a[0]);
										}
									}
								}
								else if (size == 2)
								{
									array<short>^ a = gcnew array<short>(safe_cast<int>(npoints));
									pin_ptr<short> a_ptr = &a[0];
									if (H5Aread(aid, mtype, a_ptr) < 0) {
										throw gcnew HDF5Exception("H5Aread failed!");
									}
									else
									{
										if (npoints > 1) {
											ht->Add("Value", a);
										}
										else {
											ht->Add("Value", a[0]);
										}
									}
								}
								else if (size == 4)
								{
									array<int>^ a = gcnew array<int>(safe_cast<int>(npoints));
									pin_ptr<int> a_ptr = &a[0];
									if (H5Aread(aid, mtype, a_ptr) < 0) {
										throw gcnew Exception("H5Aread failed!");
									}
									else
									{
										if (npoints > 1) {
											ht->Add("Value", a);
										}
										else {
											ht->Add("Value", a[0]);
										}
									}
								}
								else if (size == 8)
								{
									array<long long>^ a = gcnew array<long long>(safe_cast<int>(npoints));
									pin_ptr<long long> a_ptr = &a[0];
									if (H5Aread(aid, mtype, a_ptr) < 0) {
										throw gcnew Exception("H5Aread failed!");
									}
									else
									{
										if (npoints > 1) {
											ht->Add("Value", a);
										}
										else {
											ht->Add("Value", a[0]);
										}
									}
								}
								else {
									throw gcnew PSH5XException("Unsupprted INTEGER or ENUM type!");
								}
#pragma endregion
							}
							else if (sign == H5T_SGN_NONE)
							{
#pragma region unsigned
								if (size == 1)
								{
									array<unsigned char>^ a = gcnew array<unsigned char>(safe_cast<int>(npoints));
									pin_ptr<unsigned char> a_ptr = &a[0];
									if (H5Aread(aid, mtype, a_ptr) < 0) {
										throw gcnew HDF5Exception("H5Aread failed!");
									}
									else
									{
										if (npoints > 1) {
											ht->Add("Value", a);
										}
										else {
											ht->Add("Value", a[0]);
										}
									}
								}
								else if (size == 2)
								{
									array<unsigned short>^ a = gcnew array<unsigned short>(safe_cast<int>(npoints));
									pin_ptr<unsigned short> a_ptr = &a[0];
									if (H5Aread(aid, mtype, a_ptr) < 0) {
										throw gcnew HDF5Exception("H5Aread failed!");
									}
									else
									{
										if (npoints > 1) {
											ht->Add("Value", a);
										}
										else {
											ht->Add("Value", a[0]);
										}
									}
								}
								else if (size == 4)
								{
									array<unsigned int>^ a = gcnew array<unsigned int>(safe_cast<int>(npoints));
									pin_ptr<unsigned int> a_ptr = &a[0];
									if (H5Aread(aid, mtype, a_ptr) < 0) {
										throw gcnew HDF5Exception("H5Aread failed!");
									}
									else
									{
										if (npoints > 1) {
											ht->Add("Value", a);
										}
										else {
											ht->Add("Value", a[0]);
										}
									}
								}
								else if (size == 8)
								{
									array<unsigned long long>^ a = gcnew array<unsigned long long>(safe_cast<int>(npoints));
									pin_ptr<unsigned long long> a_ptr = &a[0];
									if (H5Aread(aid, mtype, a_ptr) < 0) {
										throw gcnew HDF5Exception("H5Aread failed!");
									}
									else
									{
										if (npoints > 1) {
											ht->Add("Value", a);
										}
										else {
											ht->Add("Value", a[0]);
										}
									}
								}
								else {
									throw gcnew PSH5XException("Unsupprted INTEGER type!!!");
								}
#pragma endregion
							}
							else {
								throw gcnew PSH5XException("Unknown sign convention!!!");
							}
#pragma endregion
						}
                        break;

                    case H5T_FLOAT:
						{
#pragma region HDF5 FLOAT
							ht->Add("ElementTypeClass", "Float");

							if (size == 4)
							{
								array<float>^ afloat = gcnew array<float>(safe_cast<int>(npoints));
								pin_ptr<float> afloat_ptr = &afloat[0];
								mtype = H5Tcopy(H5T_NATIVE_FLOAT);
								if (H5Aread(aid, mtype, afloat_ptr) < 0) {
									throw gcnew HDF5Exception("H5Aread failed!");
								}
								else
								{
									if (npoints > 1) {
										ht->Add("Value", afloat);
									}
									else {
										ht->Add("Value", afloat[0]);
									}
								}
							}
							else if (size == 8)
							{
								array<double>^ adouble = gcnew array<double>(safe_cast<int>(npoints));
								pin_ptr<double> adouble_ptr = &adouble[0];
								mtype = H5Tcopy(H5T_NATIVE_DOUBLE);
								if (H5Aread(aid, mtype, adouble_ptr) < 0) {
									throw gcnew HDF5Exception("H5Aread failed!");
								}
								else
								{
									if (npoints > 1) {
										ht->Add("Value", adouble);
									}
									else {
										ht->Add("Value", adouble[0]);
									}
								}
							}
							else {
								throw gcnew PSH5XException("Unsupprted FLOAT type!");
							}
#pragma endregion
						}
                        break;

                    case H5T_STRING:
						{
#pragma region HDF5 STRING
							ht->Add("ElementTypeClass", "String");

							is_vlen = H5Tis_variable_str(ftype);

							if (is_vlen > 0)
							{
								ht->Add("IsVariableLength", true);

								vrdata = new char* [npoints];

								if ((mtype = H5Tcreate(H5T_STRING, H5T_VARIABLE)) < 0) {
									throw gcnew HDF5Exception("H5Tcreate failed!");
								}

								if (H5Aread(aid, mtype, vrdata) < 0) {
									throw gcnew HDF5Exception("H5Aread failed!");
								}
								else
								{
									if (npoints == 1) {
										ht->Add("Value", gcnew String(vrdata[0]));
									}
									else
									{
										array<String^>^ astring = gcnew array<String^>(safe_cast<int>(npoints));
										for (i = 0; i < npoints; ++i) {
											astring[i] = gcnew String(vrdata[i]);
										}
										ht->Add("Value", astring);
									}

									if (H5Dvlen_reclaim(mtype, fspace, H5P_DEFAULT, vrdata) < 0) {
										throw gcnew HDF5Exception("H5Dvlen_reclaim failed!");
									}
								}
							}
							else if (is_vlen == 0)
							{
								ht->Add("IsVariableLength", false);

								rdata = new char* [npoints];
								rdata[0] = new char [npoints*(size+1)];
								for (i = 1; i < npoints; ++i)
								{
									rdata[i] = rdata[0] + i * (size+1);
								}
								if ((mtype = H5Tcreate(H5T_STRING, size+1)) < 0) {
									throw gcnew HDF5Exception("H5Tcreate failed!");
								}
								if (H5Aread(aid, mtype, rdata[0]) < 0) {
									throw gcnew HDF5Exception("H5Aread failed!");
								}
								else
								{
									if (npoints == 1) {
										ht->Add("Value", gcnew String(rdata[0]));
									}
									else
									{
										array<String^>^ astring = gcnew array<String^>(safe_cast<int>(npoints));
										for (i = 0; i < npoints; ++i)
										{
											astring[i] = gcnew String(rdata[i]);
										}
										ht->Add("Value", astring);
									}
								}
							}
							else {
								throw gcnew PSH5XException("Unknown STRING type found!");
							}
#pragma endregion
						}
                        break;

					case H5T_OPAQUE:
                        ht->Add("ElementTypeClass", "Opaque");
                        break;
					case H5T_REFERENCE:
                        ht->Add("ElementTypeClass", "Reference");
                        break;
                    case H5T_ARRAY:
                        ht->Add("ElementTypeClass", "Array");
                        break;
                    case H5T_VLEN:
                        ht->Add("ElementTypeClass", "Vlen");
                        break;
					case H5T_COMPOUND:
                        ht->Add("ElementTypeClass", "Compound");
                        break;
					default:
                        ht->Add("ElementTypeClass", "UNKNOWN");;
                        break;
                    }
                }
#pragma endregion
            }
            else {
                throw gcnew HDF5Exception("H5Aget_info failed!");
            }
        }
        finally
        {
            if (vrdata != NULL) {
                delete [] vrdata;
            }
            if (rdata != NULL) {
                delete [] rdata[0];
                delete [] rdata;
            }
            if (mtype >= 0) {
                H5Tclose(mtype);
            }
			if (btype >= 0) {
                H5Tclose(btype);
            }
            if (ntype >= 0) {
                H5Tclose(ntype);
            }
            if (ftype >= 0) {
                H5Tclose(ftype);
            }
            if (fspace >= 0) {
                H5Sclose(fspace);
            }
        }

        return ht;
    }

	void ProviderUtils::SetScalarH5AttributeValue(hid_t aid, Object^ value)
    {
        hid_t fspace = -1, ftype = -1, ntype = -1, mtype = -1;

        htri_t is_vlen = -1;

        char* wdata = NULL;
        char* vwdata[1];
		vwdata[0] = NULL;

		try
		{

#pragma region sanity check

			if ((fspace = H5Aget_space(aid)) < 0) {
				throw gcnew HDF5Exception("H5Aget_space failed!");
			}

			H5S_class_t stype = H5Sget_simple_extent_type(fspace);
			if (stype != H5S_SCALAR) {
				throw gcnew PSH5XException("Scalar HDF5 attributes only!");
			}

            if ((ftype = H5Aget_type(aid)) < 0) {
                throw gcnew HDF5Exception("H5Aget_type failed!");
            }

            H5T_class_t cls = H5Tget_class(ftype);
            if (cls != H5T_INTEGER &&
                cls != H5T_FLOAT   &&
                cls != H5T_STRING) {
                    throw gcnew PSH5XException("Attribute type unsupported (currently)!");
            }

#pragma endregion
    
			size_t size = H5Tget_size(ftype);
			if ((ntype = H5Tget_native_type(ftype, H5T_DIR_ASCEND)) < 0) {
				throw gcnew HDF5Exception("H5Tget_native_type failed!");
			}

            H5T_sign_t sign = H5T_SGN_NONE;

            switch (H5Tget_class(ftype))
            {
            case H5T_INTEGER:
				{
#pragma region HDF5 INTEGER
					sign = H5Tget_sign(ftype);
					if (sign == H5T_SGN_2)
					{
#pragma region signed
						char achar;
						short ashort;
						int aint;
						long long allong;

						if (size == 1)
						{
							if (ProviderUtils::TryGetValue(value, achar))
							{
								if (H5Awrite(aid, ntype, &achar) < 0) {
									throw gcnew HDF5Exception("H5Awrite failed!");
								}
							}
							else {
								throw gcnew PSH5XException("Value size or type mismatch!");
							}
						}
						else if (size == 2)
						{
							if (ProviderUtils::TryGetValue(value, ashort))
							{
								if (H5Awrite(aid, ntype, &ashort) < 0) {
									throw gcnew HDF5Exception("H5Awrite failed!");
								}
							}
							else {
								throw gcnew PSH5XException("Value size or type mismatch!");
							}
						}
						else if (size == 4)
						{
							if (ProviderUtils::TryGetValue(value, aint))
							{
								if (H5Awrite(aid, ntype, &aint) < 0) {
									throw gcnew HDF5Exception("H5Awrite failed!");
								}
							}
							else {
								throw gcnew PSH5XException("Value size or type mismatch!");
							}
						}
						else if (size == 8)
						{
							if (ProviderUtils::TryGetValue(value, allong))
							{
								if (H5Awrite(aid, ntype, &allong) < 0) {
									throw gcnew HDF5Exception("H5Awrite failed!");
								}
							}
							else {
								throw gcnew PSH5XException("Value size or type mismatch!");
							}
						}
						else {
							throw gcnew PSH5XException("Unsupported INTEGER type!");
						}
#pragma endregion
					}
					else if (sign == H5T_SGN_NONE)
					{
#pragma region unsigned
						unsigned char auchar;
						unsigned short aushort;
						unsigned int auint;
						unsigned long long aullong;

						if (size == 1)
						{
							if (ProviderUtils::TryGetValue(value, auchar))
							{
								if (H5Awrite(aid, ntype, &auchar) < 0) {
									throw gcnew HDF5Exception("H5Awrite failed!");
								}
							}
							else {
								throw gcnew PSH5XException("Value size or type mismatch!");
							}
						}
						else if (size == 2)
						{
							if (ProviderUtils::TryGetValue(value, aushort))
							{
								if (H5Awrite(aid, ntype, &aushort) < 0) {
									throw gcnew HDF5Exception("H5Awrite failed!");
								}
							}
							else {
								throw gcnew PSH5XException("Value size or type mismatch!");
							}
						}
						else if (size == 4)
						{
							if (ProviderUtils::TryGetValue(value, auint))
							{
								if (H5Awrite(aid, ntype, &auint) < 0) {
									throw gcnew HDF5Exception("H5Awrite failed!");
								}
							}
							else {
								throw gcnew PSH5XException("Value size or type mismatch!");
							}
						}
						else if (size == 8)
						{
							if (ProviderUtils::TryGetValue(value, aullong))
							{
								if (H5Awrite(aid, ntype, &aullong) < 0) {
									throw gcnew HDF5Exception("H5Awrite failed!");
								}
							}
							else {
								throw gcnew PSH5XException("Value size or type mismatch!");
							}
						}
						else {
							throw gcnew PSH5XException("Unsupprted INTEGER type!");
						}
#pragma endregion
					}
					else {
						throw gcnew PSH5XException("Unknown sign convention!");
					}
#pragma endregion
				}
                break;

            case H5T_FLOAT:
				{
#pragma region HDF5 FLOAT
					if (size == 4)
					{
						float afloat;
						if (ProviderUtils::TryGetValue(value, afloat))
						{
							if (H5Awrite(aid, ntype, &afloat) < 0) {
								throw gcnew Exception("H5Awrite failed!");
							}
						}
						else {
							throw gcnew PSH5XException("Value size or type mismatch!");
						}
					}
					else if (size == 8)
					{
						double adouble;
						if (ProviderUtils::TryGetValue(value, adouble))
						{
							if (H5Awrite(aid, ntype, &adouble) < 0) {
								throw gcnew Exception("H5Awrite failed!");
							}
						}
						else {
							throw gcnew PSH5XException("Value size or type mismatch!");
						}
					}
					else {
						throw gcnew PSH5XException("Unsupprted FLOAT type!!!");
					}
#pragma endregion
				}
                break;

            case H5T_STRING:
				{
#pragma region HDF5 string

					String^ astring = nullptr;

					is_vlen = H5Tis_variable_str(ftype);

					if (is_vlen > 0)
					{
						if (ProviderUtils::TryGetValue(value, astring))
						{
							vwdata[0] = (char*) Marshal::StringToHGlobalAnsi(astring).ToPointer();

							if ((mtype = H5Tcreate(H5T_STRING, H5T_VARIABLE)) < 0) {
								throw gcnew HDF5Exception("H5Tcreate failed!");
							}
							if (H5Tset_cset(mtype, H5Tget_cset(ftype)) < 0) {
								throw gcnew HDF5Exception("H5Tset_cset failed!");
							}

							if (H5Awrite(aid, mtype, vwdata) < 0) {
								throw gcnew HDF5Exception("H5Awrite failed!");
							}
						}
						else {
							throw gcnew PSH5XException("Value type mismatch!");
						}
					}
					else if (is_vlen == 0)
					{
						H5T_str_t spad = H5Tget_strpad(ftype);

						if (ProviderUtils::TryGetValue(value, astring))
						{
							if ((spad == H5T_STR_NULLTERM && astring->Length > size-1) ||
									astring->Length > size) {
								throw gcnew PSH5XException("String too long!");
							}

							wdata = (char*) Marshal::StringToHGlobalAnsi(astring).ToPointer();

							if ((mtype = H5Tcreate(H5T_STRING, size)) < 0) {
								throw gcnew HDF5Exception("H5Tcreate failed!");
							}
							if (H5Tset_cset(mtype, H5Tget_cset(ftype)) < 0) {
								throw gcnew HDF5Exception("H5Tset_cset failed!");
							}

							if (H5Awrite(aid, mtype, wdata) < 0) {
								throw gcnew HDF5Exception("H5Awrite failed!");
							}
						}
						else {
							throw gcnew PSH5XException("Value size or type mismatch!");
						}
					}
					else {
						throw gcnew PSH5XException("Unknown STRING type found!!!");
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
			if (vwdata[0] != NULL) {
				Marshal::FreeHGlobal(IntPtr(vwdata[0]));
			}
            if (wdata != NULL) {
                Marshal::FreeHGlobal(IntPtr(wdata));
            }
            if (mtype >= 0) {
                H5Tclose(mtype);
            }
            if (ntype >= 0) {
                H5Tclose(ntype);
            }
            if (ftype >= 0) {
                H5Tclose(ftype);
            }
            if (fspace >= 0) {
                H5Sclose(fspace);
            }
        }

        return;
    }

    void ProviderUtils::SetH5AttributeValue(hid_t aid, Object^ value)
    {
        hid_t fspace = -1, ftype = -1, ntype = -1, mtype = -1;

        htri_t is_vlen = -1;

        char** wdata = NULL;
        char** vwdata = NULL;

        int i = 0;

        hssize_t npoints = -1;

        try
        {
            if ((fspace = H5Aget_space(aid)) < 0) {
                throw gcnew HDF5Exception("H5Aget_space failed!");
            }

            H5S_class_t stype = H5Sget_simple_extent_type(fspace);
            if (stype == H5S_NULL) {
                // nothing needs to be done
                return;
            }

            if (stype == H5S_SIMPLE) {
                npoints = H5Sget_simple_extent_npoints(fspace);       
            }
            else if (stype == H5S_SCALAR) {
                npoints = 1;
            }
            else {
                throw gcnew PSH5XException("Unknown dataspace type!");
            }

            if ((ftype = H5Aget_type(aid)) < 0) {
                throw gcnew HDF5Exception("H5Aget_type failed!");
            }

            H5T_class_t cls = H5Tget_class(ftype);
            if (cls != H5T_INTEGER && cls != H5T_FLOAT && cls != H5T_STRING) {
                    throw gcnew PSH5XException("Attribute type unsupported (currently)!");
            }

			if ((ntype = H5Tget_native_type(ftype, H5T_DIR_ASCEND)) < 0) {
				throw gcnew HDF5Exception("H5Tget_native_type failed!");
			}
			size_t size = H5Tget_size(ntype);

            H5T_sign_t sign = H5T_SGN_NONE;

            switch (H5Tget_class(ntype))
            {
            case H5T_INTEGER:
				{
#pragma region HDF5 INTEGER
					sign = H5Tget_sign(ntype);
					if (sign == H5T_SGN_2)
					{
#pragma region signed
						if (size == 1)
						{
							array<char>^ achar = gcnew array<char>(safe_cast<int>(npoints));
							if (ProviderUtils::TryGetValue(value, achar))
							{
								pin_ptr<char> achar_ptr = &achar[0];
								if (H5Awrite(aid, ntype, achar_ptr) < 0) {
									throw gcnew HDF5Exception("H5Awrite failed!");
								}
							}
							else {
								throw gcnew PSH5XException("Value size or type mismatch!");
							}
						}
						else if (size == 2)
						{
							array<short>^ ashort = gcnew array<short>(safe_cast<int>(npoints));
							if (ProviderUtils::TryGetValue(value, ashort))
							{
								pin_ptr<short> ashort_ptr = &ashort[0];
								if (H5Awrite(aid, ntype, ashort_ptr) < 0) {
									throw gcnew HDF5Exception("H5Awrite failed!");
								}
							}
							else {
								throw gcnew PSH5XException("Value size or type mismatch!");
							}
						}
						else if (size == 4)
						{
							array<int>^ aint = gcnew array<int>(safe_cast<int>(npoints));
							if (ProviderUtils::TryGetValue(value, aint))
							{
								pin_ptr<int> aint_ptr = &aint[0];
								if (H5Awrite(aid, ntype, aint_ptr) < 0) {
									throw gcnew HDF5Exception("H5Awrite failed!");
								}
							}
							else {
								throw gcnew PSH5XException("Value size or type mismatch!");
							}
						}
						else if (size == 8)
						{
							array<long long>^ allong = gcnew array<long long>(safe_cast<int>(npoints));
							if (ProviderUtils::TryGetValue(value, allong))
							{
								pin_ptr<long long> allong_ptr = &allong[0];
								if (H5Awrite(aid, ntype, allong_ptr) < 0) {
									throw gcnew HDF5Exception("H5Awrite failed!");
								}
							}
							else {
								throw gcnew PSH5XException("Value size or type mismatch!");
							}
						}
						else {
							throw gcnew PSH5XException("Unsupported INTEGER type!");
						}
#pragma endregion
					}
					else if (sign == H5T_SGN_NONE)
					{
#pragma region unsigned
						if (size == 1)
						{
							array<unsigned char>^ auchar = gcnew array<unsigned char>(safe_cast<int>(npoints));
							if (ProviderUtils::TryGetValue(value, auchar))
							{
								pin_ptr<unsigned char> auchar_ptr = &auchar[0];
								if (H5Awrite(aid, ntype, auchar_ptr) < 0) {
									throw gcnew HDF5Exception("H5Awrite failed!");
								}
							}
							else {
								throw gcnew PSH5XException("Value size or type mismatch!");
							}
						}
						else if (size == 2)
						{
							array<unsigned short>^ aushort = gcnew array<unsigned short>(safe_cast<int>(npoints));
							if (ProviderUtils::TryGetValue(value, aushort))
							{
								pin_ptr<unsigned short> aushort_ptr = &aushort[0];
								if (H5Awrite(aid, ntype, aushort_ptr) < 0) {
									throw gcnew HDF5Exception("H5Awrite failed!");
								}
							}
							else {
								throw gcnew PSH5XException("Value size or type mismatch!");
							}
						}
						else if (size == 4)
						{
							array<unsigned int>^ auint = gcnew array<unsigned int>(safe_cast<int>(npoints));
							if (ProviderUtils::TryGetValue(value, auint))
							{
								pin_ptr<unsigned int> auint_ptr = &auint[0];
								if (H5Awrite(aid, ntype, auint_ptr) < 0) {
									throw gcnew HDF5Exception("H5Awrite failed!");
								}
							}
							else {
								throw gcnew PSH5XException("Value size or type mismatch!");
							}
						}
						else if (size == 8)
						{
							array<unsigned long long>^ aullong = gcnew array<unsigned long long>(safe_cast<int>(npoints));
							if (ProviderUtils::TryGetValue(value, aullong))
							{
								pin_ptr<unsigned long long> aullong_ptr = &aullong[0];
								if (H5Awrite(aid, ntype, aullong_ptr) < 0) {
									throw gcnew HDF5Exception("H5Awrite failed!");
								}
							}
							else {
								throw gcnew PSH5XException("Value size or type mismatch!");
							}
						}
						else {
							throw gcnew PSH5XException("Unsupprted INTEGER type!");
						}
#pragma endregion
					}
					else {
						throw gcnew PSH5XException("Unknown sign convention!");
					}
#pragma endregion
				}
                break;

            case H5T_FLOAT:
				{
#pragma region HDF5 FLOAT
					if (size == 4)
					{
						array<float>^ afloat = gcnew array<float>(safe_cast<int>(npoints));
						if (ProviderUtils::TryGetValue(value, afloat))
						{
							pin_ptr<float> afloat_ptr = &afloat[0];
							if (H5Awrite(aid, ntype, afloat_ptr) < 0) {
								throw gcnew Exception("H5Awrite failed!");
							}
						}
						else {
							throw gcnew PSH5XException("Value size or type mismatch!");
						}
					}
					else if (size == 8)
					{
						array<double>^ adouble = gcnew array<double>(safe_cast<int>(npoints));
						if (ProviderUtils::TryGetValue(value, adouble))
						{
							pin_ptr<double> adouble_ptr = &adouble[0];
							if (H5Awrite(aid, ntype, adouble_ptr) < 0) {
								throw gcnew Exception("H5Awrite failed!");
							}
						}
						else {
							throw gcnew PSH5XException("Value size or type mismatch!");
						}
					}
					else {
						throw gcnew PSH5XException("Unsupprted FLOAT type!!!");
					}
#pragma endregion
				}
                break;

            case H5T_STRING:
				{
#pragma region HDF5 string

					is_vlen = H5Tis_variable_str(ftype);

					if (is_vlen > 0)
					{
						vwdata = new char* [npoints];

						array<String^>^ astring = gcnew array<String^>(safe_cast<int>(npoints));

						if (ProviderUtils::TryGetValue(value, astring))
						{
							for (i = 0; i < npoints; ++i)
							{
								vwdata[i] = (char*) Marshal::StringToHGlobalAnsi(astring[i]).ToPointer();
							}

							mtype = H5Tcopy(H5T_C_S1);
							if (H5Tset_size(mtype, H5T_VARIABLE) < 0) {
								throw gcnew HDF5Exception("H5Tset_size failed!");
							}
							if (H5Tset_cset(mtype, H5Tget_cset(ftype)) < 0) {
								throw gcnew HDF5Exception("H5Tset_cset failed!");
							}

							if (H5Awrite(aid, mtype, vwdata) < 0) {
								throw gcnew HDF5Exception("H5Awrite failed!");
							}
						}
						else {
							throw gcnew PSH5XException("Value type mismatch!");
						}
					}
					else if (is_vlen == 0)
					{
						H5T_str_t spad = H5Tget_strpad(ftype);

						wdata = new char* [npoints];
						wdata[0] = new char [npoints*(size+1)];
						for (i = 1; i < npoints; ++i) {
							wdata[i] = wdata[0] + i*(size+1);
						}

						array<String^>^ astring = gcnew array<String^>(safe_cast<int>(npoints));

						if (ProviderUtils::TryGetValue(value, astring))
						{
							for (i = 0; i < npoints; ++i)
							{
								if ((spad == H5T_STR_NULLTERM && astring[i]->Length > size-1) ||
									astring[i]->Length > size) {
									throw gcnew PSH5XException("String too long!");
								}
							}

							for (i = 0; i < npoints; ++i)
							{
								char* buf = (char*) Marshal::StringToHGlobalAnsi(astring[i]).ToPointer();
								memcpy((void*) wdata[i], (void*) buf, size);
								Marshal::FreeHGlobal(IntPtr(buf));
							}

							mtype = H5Tcopy(H5T_C_S1);
							if (H5Tset_size(mtype, size+1) < 0) {
								throw gcnew HDF5Exception("H5Tset_size failed!!!");
							}
							if (H5Tset_cset(mtype, H5Tget_cset(ftype)) < 0) {
								throw gcnew HDF5Exception("H5Tset_cset failed!");
							}

							if (H5Awrite(aid, mtype, wdata[0]) < 0) {
								throw gcnew HDF5Exception("H5Awrite failed!");
							}
						}
						else {
							throw gcnew PSH5XException("Value size or type mismatch!");
						}
					}
					else {
						throw gcnew PSH5XException("Unknown STRING type found!!!");
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
            if (vwdata != NULL) {
                for (i = 0; i < npoints; ++i) {   
                    Marshal::FreeHGlobal(IntPtr(vwdata[i]));
                }
                delete [] vwdata;
            }
            if (wdata != NULL) {
                delete [] wdata[0];
                delete [] wdata;
            }
            if (mtype >= 0) {
                H5Tclose(mtype);
            }
            if (ntype >= 0) {
                H5Tclose(ntype);
            }
            if (ftype >= 0) {
                H5Tclose(ftype);
            }
            if (fspace >= 0) {
                H5Sclose(fspace);
            }
        }

        return;
    }

	bool ProviderUtils::AttributeSizeOK(hid_t obj_id, hid_t type_id)
	{
		bool result = false;

		try
		{
			H5O_info_t info;
			if (H5Oget_info(obj_id, &info) < 0) {
				throw gcnew HDF5Exception("H5Oget_info failed!");
			}

			// HACK: how do I determine if we are in compact or dense storage?

			if (info.meta_size.attr.index_size > 0 && info.meta_size.attr.heap_size > 0) {
				result = true;
			}
			else {
				if (H5Tget_size(type_id) <= 64000) {
					result = true;
				}
			}
		}
		finally
		{
		}

		return result;
	}
}