
#include "AttributeInfo.h"
#include "ProviderUtils.h"

extern "C" {
#include "H5Apublic.h"
#include "H5Dpublic.h"
#include "H5Spublic.h"
#include "H5Tpublic.h"
}

using namespace System;
using namespace System::Collections;
using namespace System::Management::Automation;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{

    Hashtable^ ProviderUtils::H5Attribute(hid_t aid, String^ attributeName)
    {
        Hashtable^ ht = gcnew Hashtable();

        Exception^ ex = nullptr;

        hid_t fspace = -1, ftype = -1, ntype = -1, mtype = -1;

        hsize_t* dims = NULL;
        hsize_t* maxdims = NULL;

        htri_t is_vlen = -1;

        char** rdata = NULL;
        char** vrdata = NULL;

        int i = 0;

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
                ex = gcnew Exception("H5Aget_space failed!!!");
                goto error;
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

                    dims = new hsize_t [rank];
                    maxdims = new hsize_t [rank];
                    rank = H5Sget_simple_extent_dims(fspace, dims, maxdims);
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
                        ht->Add("MaxDimensions", maxadims);
                    }
                    else {
                        ex = gcnew Exception("Rank of simple dataspace must be positive!!!");
                        goto error;
                    }
                }
                else {
                    ex = gcnew Exception("Rank of simple dataspace must be positive!!!");
                    goto error;
                }
            }
            else if (stype == H5S_SCALAR)
            {
                npoints = 1;
            }

#pragma endregion

#pragma region H5Aget_type

            array<char>^ achar = nullptr;
            pin_ptr<char> achar_ptr = nullptr;

            array<double>^ adouble = nullptr;
            pin_ptr<double> adouble_ptr = nullptr;

            array<float>^ afloat = nullptr;
            pin_ptr<float> afloat_ptr = nullptr;

            array<int>^ aint = nullptr;
            pin_ptr<int> aint_ptr = nullptr;

            array<long long>^ allong = nullptr;
            pin_ptr<long long> allong_ptr = nullptr;

            array<short>^ ashort = nullptr;
            pin_ptr<short> ashort_ptr = nullptr;

            array<String^>^ astring = nullptr;

            H5T_sign_t sign = H5T_SGN_NONE;

            if (npoints >= 1)
            {
                ftype = H5Aget_type(aid);
                if (ftype < 0) {
                    ex = gcnew Exception("H5Aget_type failed!!!");
                    goto error;
                }

                ht["ElementType"] = ProviderUtils::ParseH5Type(ftype);
                size_t size = H5Tget_size(ftype);

                switch (H5Tget_class(ftype))
                {
                case H5T_INTEGER:

#pragma region HDF5 INTEGER

                    ht->Add("ElementTypeClass", "Integer");
                    ntype = H5Tget_native_type(ftype, H5T_DIR_ASCEND);
                    sign = H5Tget_sign(ntype);
                    if (sign == H5T_SGN_2)
                    {
                        if (size == 1)
                        {
                            achar = gcnew array<char>(npoints);
                            achar_ptr = &achar[0];
                            mtype = H5Tcopy(H5T_NATIVE_CHAR);
                            if (H5Aread(aid, mtype, achar_ptr) < 0)
                            {
                                ex = gcnew Exception("H5Aread failed!!!");
                                goto error;
                            }
                            else
                            {
                                if (npoints > 1) {
                                    ht->Add("Value", achar);
                                }
                                else {
                                    ht->Add("Value", achar[0]);
                                }
                            }
                        }
                        else if (size == 2)
                        {
                            ashort = gcnew array<short>(npoints);
                            ashort_ptr = &ashort[0];
                            mtype = H5Tcopy(H5T_NATIVE_SHORT);
                            if (H5Aread(aid, mtype, ashort_ptr) < 0)
                            {
                                ex = gcnew Exception("H5Aread failed!!!");
                                goto error;
                            }
                            else
                            {
                                if (npoints > 1) {
                                    ht->Add("Value", ashort);
                                }
                                else {
                                    ht->Add("Value", ashort[0]);
                                }
                            }
                        }
                        else if (size == 4)
                        {
                            aint = gcnew array<int>(npoints);
                            aint_ptr = &aint[0];
                            mtype = H5Tcopy(H5T_NATIVE_INT);
                            if (H5Aread(aid, mtype, aint_ptr) < 0)
                            {
                                ex = gcnew Exception("H5Aread failed!!!");
                                goto error;
                            }
                            else
                            {
                                if (npoints > 1) {
                                    ht->Add("Value", aint);
                                }
                                else {
                                    ht->Add("Value", aint[0]);
                                }
                            }
                        }
                        else if (size == 8)
                        {
                            allong = gcnew array<long long>(npoints);
                            allong_ptr = &allong[0];
                            mtype = H5Tcopy(H5T_NATIVE_LLONG);
                            if (H5Aread(aid, mtype, allong_ptr) < 0)
                            {
                                ex = gcnew Exception("H5Aread failed!!!");
                                goto error;
                            }
                            else
                            {
                                if (npoints > 1) {
                                    ht->Add("Value", allong);
                                }
                                else {
                                    ht->Add("Value", allong[0]);
                                }
                            }
                        }
                        else {
                            ex = gcnew Exception("Unsupprted INTEGER type!!!");
                            goto error;
                        }
                    }
                    else if (sign == H5T_SGN_NONE)
                    {

                    }
                    else {
                        ex = gcnew Exception("Unknown sign convention!!!");
                        goto error;
                    }

#pragma endregion

                    break;

                case H5T_FLOAT:

#pragma region HDF5 FLOAT

                    ht->Add("ElementTypeClass", "Float");

                    if (size == 4)
                    {
                        afloat = gcnew array<float>(npoints);
                        afloat_ptr = &afloat[0];
                        mtype = H5Tcopy(H5T_NATIVE_FLOAT);
                        if (H5Aread(aid, mtype, afloat_ptr) < 0)
                        {
                            ex = gcnew Exception("H5Aread failed!!!");
                            goto error;
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
                        adouble = gcnew array<double>(npoints);
                        adouble_ptr = &adouble[0];
                        mtype = H5Tcopy(H5T_NATIVE_DOUBLE);
                        if (H5Aread(aid, mtype, adouble_ptr) < 0)
                        {
                            ex = gcnew Exception("H5Aread failed!!!");
                            goto error;
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
                        ex = gcnew Exception("Unsupprted FLOAT type!!!");
                        goto error;
                    }

#pragma endregion

                    break;

                case H5T_STRING:
                    
#pragma region HDF5 string

                    ht->Add("ElementTypeClass", "String");

                    is_vlen = H5Tis_variable_str(ftype);

                    if (is_vlen > 0)
                    {
                        vrdata = new char* [npoints];
                        mtype = H5Tcopy(H5T_C_S1);
                        if (H5Tset_size(mtype, H5T_VARIABLE) < 0) {
                            ex = gcnew Exception("H5Tset_size failed!!!");
                            goto error;
                        }
                        if (H5Aread(aid, mtype, vrdata) < 0) {
                            ex = gcnew Exception("H5Aread failed!!!");
                            goto error;
                        }
                        else
                        {
                            if (npoints == 1) {
                                ht->Add("Value", gcnew String(vrdata[0]));
                            }
                            else
                            {
                                astring = gcnew array<String^>(npoints);
                                for (i = 0; i < npoints; ++i)
                                {
                                    astring[i] = gcnew String(vrdata[i]);
                                }
                                ht->Add("Value", astring);
                            }

                            if (H5Dvlen_reclaim(mtype, fspace, H5P_DEFAULT, vrdata) < 0) {
                                ex = gcnew Exception("H5Dvlen_reclaim failed!!!");
                                goto error;
                            }
                        }
                    }
                    else if (is_vlen == 0)
                    {
                        rdata = new char* [npoints];
                        rdata[0] = new char [npoints*(size+1)];
                        for (i = 1; i < npoints; ++i)
                        {
                            rdata[i] = rdata[0] + i * (size+1);
                        }
                        mtype = H5Tcopy(H5T_C_S1);
                        if (H5Tset_size(mtype, size+1) < 0) {
                            ex = gcnew Exception("H5Tset_size failed!!!");
                            goto error;
                        }
                        if (H5Aread(aid, mtype, rdata[0]) < 0) {
                            ex = gcnew Exception("H5Aread failed!!!");
                            goto error;
                        }
                        else
                        {
                            if (npoints == 1) {
                                ht->Add("Value", gcnew String(rdata[0]));
                            }
                            else
                            {
                                astring = gcnew array<String^>(npoints);
                                for (i = 0; i < npoints; ++i)
                                {
                                    astring[i] = gcnew String(rdata[i]);
                                }
                                ht->Add("Value", astring);
                            }
                        }
                    }
                    else {
                        ex = gcnew Exception("Unknown STRING type found!!!");
                        goto error;
                    }

#pragma endregion

                    break;

                case H5T_BITFIELD:
                    ht->Add("ElementTypeClass", "Bitfield");
                    break;
                case H5T_REFERENCE:
                    ht->Add("ElementTypeClass", "Reference");
                    break;
                case H5T_OPAQUE:
                    ht->Add("ElementTypeClass", "Opaque");
                    break;

                case H5T_COMPOUND:
                    ht->Add("ElementTypeClass", "Compound");
                    goto error;
                    break;
                case H5T_ENUM:
                    ht->Add("ElementTypeClass", "Enum");
                    goto error;
                    break;
                case H5T_VLEN:
                    ht->Add("ElementTypeClass", "Vlen");
                    goto error;
                    break;
                case H5T_ARRAY:
                    ht->Add("ElementTypeClass", "Array");
                    goto error;
                    break;
                default:
                    ht->Add("ElementTypeClass", "UNKNOWN");
                    goto error;
                    break;
                }
            }

#pragma endregion

        }
        else {
            ex = gcnew Exception("H5Aget_info failed!!!");
            goto error;
        }

error:

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

        if (ntype >= 0) {
            H5Tclose(ntype);
        }

        if (ftype >= 0) {
            H5Tclose(ftype);
        }

        if (dims != NULL) {
            delete [] dims;
        }

        if (maxdims != NULL) {
            delete [] maxdims;
        };

        if (fspace >= 0) {
            H5Sclose(fspace);
        }

        if (ex != nullptr) {
            throw ex;
        }

        return ht;
    }
}