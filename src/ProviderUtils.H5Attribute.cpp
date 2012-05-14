
#include "AttributeInfo.h"
#include "H5ArrayT.h"
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

                if ((fspace = H5Aget_space(aid)) < 0) {
                    throw gcnew HDF5Exception("H5Aget_space failed!!!");
                }

                H5S_class_t stype = H5Sget_simple_extent_type(fspace);
				array<hsize_t>^ dims = gcnew array<hsize_t>(1);
				dims[0] = 1;

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

				int rank = -1;
                hssize_t npoints = -1;

                if (stype == H5S_SIMPLE)
				{
					if ((rank = H5Sget_simple_extent_ndims(fspace)) <= 0) {
						throw gcnew PSH5XException("Rank of simple dataspace must be positive!");
					}
					ht->Add("Rank", rank);

					npoints = H5Sget_simple_extent_npoints(fspace);
					ht->Add("ElementCount", npoints);

					if ((rank = H5Sget_simple_extent_ndims(fspace)) <= 0) {
						throw gcnew HDF5Exception("H5Sget_simple_extent_ndims failed!");
					}

					dims = gcnew array<hsize_t>(rank);
					pin_ptr<hsize_t> dims_ptr = &dims[0];
					rank = H5Sget_simple_extent_dims(fspace, dims_ptr, NULL);
					ht->Add("Dimensions", dims);

				}
                else if (stype == H5S_SCALAR) {
					rank = 0;
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
								if (cls != H5T_ENUM) {
									if ((mtype = H5Tget_native_type(ftype, H5T_DIR_ASCEND)) < 0) {
										throw gcnew HDF5Exception("H5Tget_native_type failed!");
									}
								}
								else {
									if ((mtype = H5Tget_native_type(btype, H5T_DIR_ASCEND)) < 0) {
										throw gcnew HDF5Exception("H5Tget_native_type failed!");
									}
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
									H5Array<char>^ h5a = gcnew H5Array<char>(dims);
									pin_ptr<char> ptr = h5a->GetHandle();
									if (H5Aread(aid, mtype, ptr) < 0) {
										throw gcnew HDF5Exception("H5Aread failed!");
									}
									else
									{
										if (rank >= 1) {
											ht->Add("Value", h5a->GetArray());
										}
										else {
											ht->Add("Value", ptr[0]);
										}
									}
								}
								else if (size == 2)
								{
									H5Array<short>^ h5a = gcnew H5Array<short>(dims);
									pin_ptr<short> ptr = h5a->GetHandle();
									if (H5Aread(aid, mtype, ptr) < 0) {
										throw gcnew HDF5Exception("H5Aread failed!");
									}
									else
									{
										if (rank >= 1) {
											ht->Add("Value", h5a->GetArray());
										}
										else {
											ht->Add("Value", ptr[0]);
										}
									}
								}
								else if (size == 4)
								{
									H5Array<int>^ h5a = gcnew H5Array<int>(dims);
									pin_ptr<int> ptr = h5a->GetHandle();
									if (H5Aread(aid, mtype, ptr) < 0) {
										throw gcnew HDF5Exception("H5Aread failed!");
									}
									else
									{
										if (rank >= 1) {
											ht->Add("Value", h5a->GetArray());
										}
										else {
											ht->Add("Value", ptr[0]);
										}
									}
								}
								else if (size == 8)
								{
									H5Array<long long>^ h5a = gcnew H5Array<long long>(dims);
									pin_ptr<long long> ptr = h5a->GetHandle();
									if (H5Aread(aid, mtype, ptr) < 0) {
										throw gcnew HDF5Exception("H5Aread failed!");
									}
									else
									{
										if (rank >= 1) {
											ht->Add("Value", h5a->GetArray());
										}
										else {
											ht->Add("Value", ptr[0]);
										}
									}
								}
								else {
									throw gcnew PSH5XException("Unsupprted INTEGER, ENUM, or BITFIELD type!");
								}
#pragma endregion
							}
							else if (sign == H5T_SGN_NONE)
							{
#pragma region unsigned
								if (size == 1)
								{
									H5Array<unsigned char>^ h5a = gcnew H5Array<unsigned char>(dims);
									pin_ptr<unsigned char> ptr = h5a->GetHandle();
									if (H5Aread(aid, mtype, ptr) < 0) {
										throw gcnew HDF5Exception("H5Aread failed!");
									}
									else
									{
										if (rank >= 1) {
											ht->Add("Value", h5a->GetArray());
										}
										else {
											ht->Add("Value", ptr[0]);
										}
									}
								}
								else if (size == 2)
								{
									H5Array<unsigned short>^ h5a = gcnew H5Array<unsigned short>(dims);
									pin_ptr<unsigned short> ptr = h5a->GetHandle();
									if (H5Aread(aid, mtype, ptr) < 0) {
										throw gcnew HDF5Exception("H5Aread failed!");
									}
									else
									{
										if (rank >= 1) {
											ht->Add("Value", h5a->GetArray());
										}
										else {
											ht->Add("Value", ptr[0]);
										}
									}
								}
								else if (size == 4)
								{
									H5Array<unsigned int>^ h5a = gcnew H5Array<unsigned int>(dims);
									pin_ptr<unsigned int> ptr = h5a->GetHandle();
									if (H5Aread(aid, mtype, ptr) < 0) {
										throw gcnew HDF5Exception("H5Aread failed!");
									}
									else
									{
										if (rank >= 1) {
											ht->Add("Value", h5a->GetArray());
										}
										else {
											ht->Add("Value", ptr[0]);
										}
									}
								}
								else if (size == 8)
								{
									H5Array<unsigned long long>^ h5a = gcnew H5Array<unsigned long long>(dims);
									pin_ptr<unsigned long long> ptr = h5a->GetHandle();
									if (H5Aread(aid, mtype, ptr) < 0) {
										throw gcnew HDF5Exception("H5Aread failed!");
									}
									else
									{
										if (rank >= 1) {
											ht->Add("Value", h5a->GetArray());
										}
										else {
											ht->Add("Value", ptr[0]);
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

							if ((mtype = H5Tget_native_type(ftype, H5T_DIR_ASCEND)) < 0) {
								throw gcnew HDF5Exception("H5Tget_native_type failed!");
							}

							if (size == 4)
							{
								H5Array<float>^ h5a = gcnew H5Array<float>(dims);
								pin_ptr<float> ptr = h5a->GetHandle();
								if (H5Aread(aid, mtype, ptr) < 0) {
									throw gcnew HDF5Exception("H5Aread failed!");
								}
								else
								{
									if (rank >= 1) {
										ht->Add("Value", h5a->GetArray());
									}
									else {
										ht->Add("Value", ptr[0]);
									}
								}
							}
							else if (size == 8)
							{
								H5Array<double>^ h5a = gcnew H5Array<double>(dims);
								pin_ptr<double> ptr = h5a->GetHandle();
								if (H5Aread(aid, mtype, ptr) < 0) {
									throw gcnew HDF5Exception("H5Aread failed!");
								}
								else
								{
									if (rank >= 1) {
										ht->Add("Value", h5a->GetArray());
									}
									else {
										ht->Add("Value", ptr[0]);
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
								if (H5Tset_cset(mtype, H5Tget_cset(ftype)) < 0) {
									throw gcnew HDF5Exception("H5Tset_cset failed!");
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
										H5Array<String^>^ h5a = gcnew H5Array<String^>(dims);
								        pin_ptr<String^> ptr = h5a->GetHandle();
										for (i = 0; i < npoints; ++i) {
											ptr[i] = gcnew String(vrdata[i]);
										}
										ht->Add("Value", h5a->GetArray());
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
								for (i = 1; i < npoints; ++i) {
									rdata[i] = rdata[0] + i * (size+1);
								}
								if ((mtype = H5Tcreate(H5T_STRING, size+1)) < 0) {
									throw gcnew HDF5Exception("H5Tcreate failed!");
								}
								if (H5Tset_cset(mtype, H5Tget_cset(ftype)) < 0) {
									throw gcnew HDF5Exception("H5Tset_cset failed!");
								}
								if (H5Tset_strpad(mtype, H5Tget_strpad(ftype)) < 0) {
									throw gcnew HDF5Exception("H5Tset_strpad failed!");
								}

								if (H5Aread(aid, mtype, rdata[0]) < 0) {
									throw gcnew HDF5Exception("H5Aread failed!");
								}
								else
								{
									if (npoints == 1) {
										ht->Add("Value", Marshal::PtrToStringAnsi(IntPtr(rdata[0]), safe_cast<int>(size)));
									}
									else
									{
										H5Array<String^>^ h5a = gcnew H5Array<String^>(dims);
								        pin_ptr<String^> ptr = h5a->GetHandle();
										for (i = 0; i < npoints; ++i) {
											ptr[i] = Marshal::PtrToStringAnsi(IntPtr(rdata[i]), safe_cast<int>(size));
										}
										ht->Add("Value", h5a->GetArray());
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

	void ProviderUtils::SetH5AttributeValue(hid_t aid, Object^ value)
	{
		hid_t fspace = -1, ftype = -1, btype = -1, ntype = -1, mtype = -1;

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
            if (cls == H5T_COMPOUND || cls == H5T_VLEN || cls == H5T_ARRAY ||
				cls == H5T_REFERENCE || cls == H5T_OPAQUE) {
                    throw gcnew PSH5XException("Attribute type unsupported (currently)!");
            }

            switch (cls)
			{
			case H5T_BITFIELD:
			case H5T_ENUM:
			case H5T_INTEGER:
				{
#pragma region HDF5 INTEGER & ENUM & BITFIELD
					
					size_t size = H5Tget_size(ftype);
					H5T_sign_t sign = H5T_SGN_NONE;

					if (cls == H5T_ENUM)
					{
						if ((btype = H5Tget_super(ftype)) < 0) {
							throw gcnew HDF5Exception("H5Tget_super failed!");
						}
						sign = H5Tget_sign(btype);
					}
					else if (cls == H5T_INTEGER) {
						sign = H5Tget_sign(ftype);
					}

					if (cls != H5T_BITFIELD) {
						if ((ntype = H5Tget_native_type(ftype, H5T_DIR_ASCEND)) < 0) {
							throw gcnew HDF5Exception("H5Tget_native_type failed!");
						}
					}
					else {
						if ((ntype = H5Tget_native_type(ftype, H5T_DIR_DESCEND)) < 0) {
							throw gcnew HDF5Exception("H5Tget_native_type failed!");
						}
					}

					if (sign == H5T_SGN_2)
					{
#pragma region signed
						if (size == 1)
						{
							pin_ptr<char> a_ptr = nullptr;
							array<char>^ a = nullptr;
							char av;
							if (stype == H5S_SCALAR) {
								if (!ProviderUtils::TryGetValue(value, av)) {
									throw gcnew PSH5XException("Value size or type mismatch!");
								}
								a_ptr = &av;
							}
							else {
								a = gcnew array<char>(safe_cast<int>(npoints));
								if (!ProviderUtils::TryGetValue(value, a)) {
									throw gcnew PSH5XException("Value size or type mismatch!");
								}
								a_ptr = &a[0];
							}
							if (H5Awrite(aid, ntype, a_ptr) < 0) {
								throw gcnew Exception("H5Awrite failed!");
							}
						}
						else if (size == 2)
						{
							pin_ptr<short> a_ptr = nullptr;
							array<short>^ a = nullptr;
							short av;
							if (stype == H5S_SCALAR) {
								if (!ProviderUtils::TryGetValue(value, av)) {
									throw gcnew PSH5XException("Value size or type mismatch!");
								}
								a_ptr = &av;
							}
							else {
								a = gcnew array<short>(safe_cast<int>(npoints));
								if (!ProviderUtils::TryGetValue(value, a)) {
									throw gcnew PSH5XException("Value size or type mismatch!");
								}
								a_ptr = &a[0];
							}
							if (H5Awrite(aid, ntype, a_ptr) < 0) {
								throw gcnew Exception("H5Awrite failed!");
							}
						}
						else if (size == 4)
						{
							pin_ptr<int> a_ptr = nullptr;
							array<int>^ a = nullptr;
							int av;
							if (stype == H5S_SCALAR) {
								if (!ProviderUtils::TryGetValue(value, av)) {
									throw gcnew PSH5XException("Value size or type mismatch!");
								}
								a_ptr = &av;
							}
							else {
								a = gcnew array<int>(safe_cast<int>(npoints));
								if (!ProviderUtils::TryGetValue(value, a)) {
									throw gcnew PSH5XException("Value size or type mismatch!");
								}
								a_ptr = &a[0];
							}
							if (H5Awrite(aid, ntype, a_ptr) < 0) {
								throw gcnew Exception("H5Awrite failed!");
							}
						}
						else if (size == 8)
						{
							pin_ptr<long long> a_ptr = nullptr;
							array<long long>^ a = nullptr;
							long long av;
							if (stype == H5S_SCALAR) {
								if (!ProviderUtils::TryGetValue(value, av)) {
									throw gcnew PSH5XException("Value size or type mismatch!");
								}
								a_ptr = &av;
							}
							else {
								a = gcnew array<long long>(safe_cast<int>(npoints));
								if (!ProviderUtils::TryGetValue(value, a)) {
									throw gcnew PSH5XException("Value size or type mismatch!");
								}
								a_ptr = &a[0];
							}
							if (H5Awrite(aid, ntype, a_ptr) < 0) {
								throw gcnew Exception("H5Awrite failed!");
							}
						}
						else {
							throw gcnew PSH5XException("Unsupported INTEGER, ENUM, or BITFIELD type!");
						}
#pragma endregion
					}
					else if (sign == H5T_SGN_NONE)
					{
#pragma region unsigned
						if (size == 1)
						{
							pin_ptr<unsigned char> a_ptr = nullptr;
							array<unsigned char>^ a = nullptr;
							unsigned char av;
							if (stype == H5S_SCALAR) {
								if (!ProviderUtils::TryGetValue(value, av)) {
									throw gcnew PSH5XException("Value size or type mismatch!");
								}
								a_ptr = &av;
							}
							else {
								a = gcnew array<unsigned char>(safe_cast<int>(npoints));
								if (!ProviderUtils::TryGetValue(value, a)) {
									throw gcnew PSH5XException("Value size or type mismatch!");
								}
								a_ptr = &a[0];
							}
							if (H5Awrite(aid, ntype, a_ptr) < 0) {
								throw gcnew Exception("H5Awrite failed!");
							}
						}
						else if (size == 2)
						{
							pin_ptr<unsigned short> a_ptr = nullptr;
							array<unsigned short>^ a = nullptr;
							unsigned short av;
							if (stype == H5S_SCALAR) {
								if (!ProviderUtils::TryGetValue(value, av)) {
									throw gcnew PSH5XException("Value size or type mismatch!");
								}
								a_ptr = &av;
							}
							else {
								a = gcnew array<unsigned short>(safe_cast<int>(npoints));
								if (!ProviderUtils::TryGetValue(value, a)) {
									throw gcnew PSH5XException("Value size or type mismatch!");
								}
								a_ptr = &a[0];
							}
							if (H5Awrite(aid, ntype, a_ptr) < 0) {
								throw gcnew Exception("H5Awrite failed!");
							}
						}
						else if (size == 4)
						{
							pin_ptr<unsigned int> a_ptr = nullptr;
							array<unsigned int>^ a = nullptr;
							unsigned int av;
							if (stype == H5S_SCALAR) {
								if (!ProviderUtils::TryGetValue(value, av)) {
									throw gcnew PSH5XException("Value size or type mismatch!");
								}
								a_ptr = &av;
							}
							else {
								a = gcnew array<unsigned int>(safe_cast<int>(npoints));
								if (!ProviderUtils::TryGetValue(value, a)) {
									throw gcnew PSH5XException("Value size or type mismatch!");
								}
								a_ptr = &a[0];
							}
							if (H5Awrite(aid, ntype, a_ptr) < 0) {
								throw gcnew Exception("H5Awrite failed!");
							}
						}
						else if (size == 8)
						{
							pin_ptr<unsigned long long> a_ptr = nullptr;
							array<unsigned long long>^ a = nullptr;
							unsigned long long av;
							if (stype == H5S_SCALAR) {
								if (!ProviderUtils::TryGetValue(value, av)) {
									throw gcnew PSH5XException("Value size or type mismatch!");
								}
								a_ptr = &av;
							}
							else {
								a = gcnew array<unsigned long long>(safe_cast<int>(npoints));
								if (!ProviderUtils::TryGetValue(value, a)) {
									throw gcnew PSH5XException("Value size or type mismatch!");
								}
								a_ptr = &a[0];
							}
							if (H5Awrite(aid, ntype, a_ptr) < 0) {
								throw gcnew Exception("H5Awrite failed!");
							}
						}
						else {
							throw gcnew PSH5XException("Unsupported INTEGER, ENUM, or BITFIELD type!");
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

					size_t size = H5Tget_size(ftype);
					ntype = H5Tget_native_type(ftype, H5T_DIR_ASCEND);

					if (size == 4)
					{
						pin_ptr<float> a_ptr = nullptr;
						array<float>^ a = nullptr;
						float av;
						if (stype == H5S_SCALAR) {
							if (!ProviderUtils::TryGetValue(value, av)) {
								throw gcnew PSH5XException("Value size or type mismatch!");
							}
							a_ptr = &av;
						}
						else {
							a = gcnew array<float>(safe_cast<int>(npoints));
							if (!ProviderUtils::TryGetValue(value, a)) {
								throw gcnew PSH5XException("Value size or type mismatch!");
							}
							a_ptr = &a[0];
						}
						if (H5Awrite(aid, ntype, a_ptr) < 0) {
							throw gcnew Exception("H5Awrite failed!");
						}
					}
					else if (size == 8)
					{
						pin_ptr<double> a_ptr = nullptr;
						array<double>^ a = nullptr;
						double av;
						if (stype == H5S_SCALAR) {
							if (!ProviderUtils::TryGetValue(value, av)) {
								throw gcnew PSH5XException("Value size or type mismatch!");
							}
							a_ptr = &av;
						}
						else {
							a = gcnew array<double>(safe_cast<int>(npoints));
							if (!ProviderUtils::TryGetValue(value, a)) {
								throw gcnew PSH5XException("Value size or type mismatch!");
							}
							a_ptr = &a[0];
						}
						if (H5Awrite(aid, ntype, a_ptr) < 0) {
							throw gcnew Exception("H5Awrite failed!");
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
#pragma region HDF5 STRING
					is_vlen = H5Tis_variable_str(ftype);

					if (is_vlen > 0)
					{
						vwdata = new char* [npoints];

						if (stype == H5S_SIMPLE)
						{
							array<String^>^ a = gcnew array<String^>(safe_cast<int>(npoints));

							if (ProviderUtils::TryGetValue(value, a)) {
								for (i = 0; i < npoints; ++i) {
									vwdata[i] = (char*) Marshal::StringToHGlobalAnsi(a[i]).ToPointer();
								}
							}
							else {
								throw gcnew PSH5XException("Value type mismatch!");
							}
						}
						else //scalar
						{
							String^ s = nullptr;
							if (ProviderUtils::TryGetValue(value, s)) {
								vwdata[0] = (char*) Marshal::StringToHGlobalAnsi(s).ToPointer();
							}
							else {
								throw gcnew PSH5XException("Value type mismatch!");
							}
						}

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
					else if (is_vlen == 0)
					{
						size_t size = H5Tget_size(ftype);
						H5T_str_t spad = H5Tget_strpad(ftype);

						wdata = new char* [npoints];
						wdata[0] = new char [npoints*size];
						for (i = 1; i < npoints; ++i) {
							wdata[i] = wdata[0] + i*size;
						}

						if (stype == H5S_SIMPLE)
						{
							array<String^>^ a = gcnew array<String^>(safe_cast<int>(npoints));

							if (ProviderUtils::TryGetValue(value, a))
							{
								for (i = 0; i < npoints; ++i) {
									if ((spad == H5T_STR_NULLTERM && a[i]->Length > size-1) || a[i]->Length > size) {
											throw gcnew PSH5XException("String too long!");
									}
								}

								for (i = 0; i < npoints; ++i)
								{
									char* buf = (char*) Marshal::StringToHGlobalAnsi(a[i]).ToPointer();
									memcpy((void*) wdata[i], (void*) buf, size);
									Marshal::FreeHGlobal(IntPtr(buf));
								}
							}
							else {
								throw gcnew PSH5XException("Value size or type mismatch!");
							}
						}
						else // scalar
						{
							String^ s = nullptr;
							if (ProviderUtils::TryGetValue(value, s))
							{
								if ((spad == H5T_STR_NULLTERM && s->Length > size-1) || s->Length > size) {
										throw gcnew PSH5XException("String too long!");
								}

								char* buf = (char*) Marshal::StringToHGlobalAnsi(s).ToPointer();
								memcpy((void*) wdata[0], (void*) buf, size);
								Marshal::FreeHGlobal(IntPtr(buf));
							}
							else {
								throw gcnew PSH5XException("Value size or type mismatch!");
							}
						}

						if ((mtype = H5Tcreate(H5T_STRING, size)) < 0) {
							throw gcnew HDF5Exception("H5Tcreate failed!");
						}
						if (H5Tset_cset(mtype, H5Tget_cset(ftype)) < 0) {
							throw gcnew HDF5Exception("H5Tset_cset failed!");
						}
						if (H5Tset_strpad(mtype, spad) < 0) {
							throw gcnew HDF5Exception("H5Tset_strpad failed!");
						}
						if (H5Awrite(aid, mtype, wdata[0]) < 0) {
							throw gcnew HDF5Exception("H5Awrite failed!");
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
			if (btype >= 0) {
                H5Tclose(btype);
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