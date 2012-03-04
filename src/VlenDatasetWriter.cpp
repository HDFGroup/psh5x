
#include "HDF5Exception.h"
#include "VlenDatasetWriter.h"
#include "PSH5XException.h"
#include "ProviderUtils.h"

extern "C" {
#include "H5Dpublic.h"
#include "H5Spublic.h"
#include "H5Tpublic.h"
}

#include <cstring>
#include <vector>

using namespace std;

using namespace System;
using namespace System::Collections;
using namespace System::Collections::ObjectModel;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Provider;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
    VlenDatasetWriter::VlenDatasetWriter(hid_t h5file, System::String^ h5path)
        : m_h5file(h5file), m_h5path(h5path), m_array(nullptr)
    {
    }

    IList^ VlenDatasetWriter::Write(IList^ content)
    {
        char* name = (char*)(Marshal::StringToHGlobalAnsi(m_h5path)).ToPointer();

        hid_t dset = -1, fspace = -1, ftype = -1, ntype = -1, mtype = -1, base_type = -1;

        vector<hvl_t> wdata;

        hssize_t npoints = -1;

        try
        {
            dset = H5Dopen2(m_h5file, name, H5P_DEFAULT);
            if (dset < 0) {
                throw gcnew HDF5Exception("H5Dopen2 failed!");
            }

            fspace = H5Dget_space(dset);
            if (fspace < 0) {
                throw gcnew HDF5Exception("H5Dget_space failed!");
            }

            npoints = H5Sget_simple_extent_npoints(fspace);
            if (content->Count != safe_cast<int>(npoints)) {
                throw gcnew PSH5XException("Size mismatch!");
            }

            if (npoints > 0)
            {
                ftype = H5Dget_type(dset);
                if (ftype < 0) {
                    throw gcnew HDF5Exception("H5Dget_type failed!");
                }

                base_type = H5Tget_super(ftype);
                if (base_type < 0) {
                    throw gcnew HDF5Exception("H5Tget_super failed!");
                }

                vector<char> vchar;
                vector<short> vshort;
                vector<int>   vint;
                vector<long long> vllong;

                vector<unsigned char> vuchar;
                vector<unsigned short> vushort;
                vector<unsigned int>   vuint;
                vector<unsigned long long> vullong;


                Type^ t = ProviderUtils::H5Type2DotNet(base_type);
                if (t != nullptr)
                {
                    wdata = vector<hvl_t>(npoints);

                    if (H5Tget_class(base_type) == H5T_BITFIELD) {
                        ntype = H5Tget_native_type(base_type, H5T_DIR_DESCEND);
                    }
                    else {
                        ntype = H5Tget_native_type(base_type, H5T_DIR_ASCEND);
                    }
                    if (ntype < 0) {
                        throw gcnew HDF5Exception("H5Tget_native_type failed!");
                    }

                    mtype = H5Tvlen_create(ntype);
                    if (mtype < 0) {
                        throw gcnew HDF5Exception("H5Tvlen_create failed!");
                    }

                    IEnumerator^ ienum = content->GetEnumerator();
                    ienum->MoveNext();

                    array<int>^ len = gcnew array<int>(npoints);
                    array<int>^ offset = gcnew array<int>(npoints);
                    array<Object^>^ amaster = gcnew array<Object^>(npoints);

                    int total = 0;

                    for (int i = 0; i < npoints; ++i)
                    {

						Object^ obj = ProviderUtils::GetDotNetObject(ienum->Current);
						Array^ a = safe_cast<Array^>(obj);
                        
						offset[i] = total;
						len[i] = a->Length;
                        amaster[i] = a;

						total += len[i];

                        ienum->MoveNext();
                    }

                    array<char>^ achar = nullptr;
                    array<double>^ adouble = nullptr;
                    array<float>^ afloat = nullptr;
                    array<int>^ aint = nullptr;
                    array<long long>^ allong = nullptr;
                    array<short>^ ashort = nullptr;
                    array<unsigned char>^ auchar = nullptr;
                    array<unsigned int>^ auint = nullptr;
                    array<unsigned long long>^ aullong = nullptr;
                    array<unsigned short>^ aushort = nullptr;

                    pin_ptr<char> char_ptr;
                    pin_ptr<double> double_ptr;
                    pin_ptr<float> float_ptr;
                    pin_ptr<int> int_ptr;
                    pin_ptr<long long> llong_ptr;
                    pin_ptr<short> short_ptr;
                    pin_ptr<unsigned char> uchar_ptr;
                    pin_ptr<unsigned int> uint_ptr;
                    pin_ptr<unsigned long long> ullong_ptr;
                    pin_ptr<unsigned short> ushort_ptr;

                    H5T_class_t cls = H5Tget_class(base_type);
                    switch (cls)
                    {
                    case H5T_INTEGER:

#pragma region integer

                        if (t == SByte::typeid)
                        {
                            achar = gcnew array<char>(total);
                            char_ptr = &achar[0];
                            pin_ptr<char> p;
                            for (int i = 0; i < npoints; ++i) {
                                Array::Copy((Array^) amaster[i], 0, achar, offset[i], len[i]);
                                wdata[i].len = len[i];
                                p = &achar[offset[i]];
                                wdata[i].p = p;
                            }
                        }
                        else if (t == Int16::typeid)
                        {
                            ashort = gcnew array<short>(total);
                            short_ptr = &ashort[0];
                            pin_ptr<short> p;
                            for (int i = 0; i < npoints; ++i) {
                                Array::Copy((Array^) amaster[i], 0, ashort, offset[i], len[i]);
                                wdata[i].len = len[i];
                                p = &ashort[offset[i]];
                                wdata[i].p = p;
                            }
                        }
                        else if (t == Int32::typeid)
                        {
                            aint = gcnew array<int>(total);
                            int_ptr = &aint[0];
                            pin_ptr<int> p;
                            for (int i = 0; i < npoints; ++i) {
                                Array::Copy((Array^) amaster[i], 0, aint, offset[i], len[i]);
                                wdata[i].len = len[i];
                                p = &aint[offset[i]];
                                wdata[i].p = p;
                            }
                        }
                        else if (t == Int64::typeid)
                        {
                            allong = gcnew array<long long>(total);
                            llong_ptr = &allong[0];
                            pin_ptr<long long> p;
                            for (int i = 0; i < npoints; ++i) {
                                Array::Copy((Array^) amaster[i], 0, allong, offset[i], len[i]);
                                wdata[i].len = len[i];
                                p = &allong[offset[i]];
                                wdata[i].p = p;
                            }
                        }
                        else if (t == Byte::typeid)
                        {
                            auchar = gcnew array<unsigned char>(total);
                            uchar_ptr = &auchar[0];
                            pin_ptr<unsigned char> p;
                            for (int i = 0; i < npoints; ++i) {
                                Array::Copy((Array^) amaster[i], 0, auchar, offset[i], len[i]);
                                wdata[i].len = len[i];
                                p = &auchar[offset[i]];
                                wdata[i].p = p;
                            }
                        }
                        else if (t == UInt16::typeid)
                        {
                            aushort = gcnew array<unsigned short>(total);
                            ushort_ptr = &aushort[0];
                            pin_ptr<unsigned short> p;
                            for (int i = 0; i < npoints; ++i) {
                                Array::Copy((Array^) amaster[i], 0, aushort, offset[i], len[i]);
                                wdata[i].len = len[i];
                                p = &aushort[offset[i]];
                                wdata[i].p = p;
                            }
                        }
                        else if (t == UInt32::typeid)
                        {
                            auint = gcnew array<unsigned int>(total);
                            uint_ptr = &auint[0];
                            pin_ptr<unsigned int> p;
                            for (int i = 0; i < npoints; ++i) {
                                Array::Copy((Array^) amaster[i], 0, auint, offset[i], len[i]);
                                wdata[i].len = len[i];
                                p = &auint[offset[i]];
                                wdata[i].p = p;
                            }
                        }
                        else if (t == UInt64::typeid)
                        {
                            aullong = gcnew array<unsigned long long>(total);
                            ullong_ptr = &aullong[0];
                            pin_ptr<unsigned long long> p;
                            for (int i = 0; i < npoints; ++i) {
                                Array::Copy((Array^) amaster[i], 0, aullong, offset[i], len[i]);
                                wdata[i].len = len[i];
                                p = &aullong[offset[i]];
                                wdata[i].p = p;
                            }
                        }
                        else {
                            throw gcnew PSH5XException("Unknown integer type!");
                        }

#pragma endregion

                        break;

                    case H5T_FLOAT:

#pragma region float

                        if (t == Single::typeid)
                        {
                            afloat = gcnew array<float>(total);
                            float_ptr = &afloat[0];
                            pin_ptr<float> p;
                            for (int i = 0; i < npoints; ++i) {
                                Array::Copy((Array^) amaster[i], 0, afloat, offset[i], len[i]);
                                wdata[i].len = len[i];
                                p = &afloat[offset[i]];
                                wdata[i].p = p;
                            }
                        }
                        else if (t == Double::typeid)
                        {
                            adouble = gcnew array<double>(total);
                            double_ptr = &adouble[0];
                            pin_ptr<double> p;
                            for (int i = 0; i < npoints; ++i) {
                                Array::Copy((Array^) amaster[i], 0, adouble, offset[i], len[i]);
                                wdata[i].len = len[i];
                                p = &adouble[offset[i]];
                                wdata[i].p = p;
                            }
                        }
                        else {
                            throw gcnew PSH5XException("Unknown float type!");
                        }

#pragma endregion

                        break;

                    case H5T_BITFIELD:

#pragma region bitfield

                        if (t == Byte::typeid)
                        {
                            auchar = gcnew array<unsigned char>(total);
                            uchar_ptr = &auchar[0];
                            pin_ptr<unsigned char> p;
                            for (int i = 0; i < npoints; ++i) {
                                Array::Copy((Array^) amaster[i], 0, auchar, offset[i], len[i]);
                                wdata[i].len = len[i];
                                p = &auchar[offset[i]];
                                wdata[i].p = p;
                            }
                        }
                        else if (t == UInt16::typeid)
                        {
                            aushort = gcnew array<unsigned short>(total);
                            ushort_ptr = &aushort[0];
                            pin_ptr<unsigned short> p;
                            for (int i = 0; i < npoints; ++i) {
                                Array::Copy((Array^) amaster[i], 0, aushort, offset[i], len[i]);
                                wdata[i].len = len[i];
                                p = &aushort[offset[i]];
                                wdata[i].p = p;
                            }
                        }
                        else if (t == UInt32::typeid)
                        {
                            auint = gcnew array<unsigned int>(total);
                            uint_ptr = &auint[0];
                            pin_ptr<unsigned int> p;
                            for (int i = 0; i < npoints; ++i) {
                                Array::Copy((Array^) amaster[i], 0, auint, offset[i], len[i]);
                                wdata[i].len = len[i];
                                p = &auint[offset[i]];
                                wdata[i].p = p;
                            }
                        }
                        else if (t == UInt64::typeid)
                        {
                            aullong = gcnew array<unsigned long long>(total);
                            ullong_ptr = &aullong[0];
                            pin_ptr<unsigned long long> p;
                            for (int i = 0; i < npoints; ++i) {
                                Array::Copy((Array^) amaster[i], 0, aullong, offset[i], len[i]);
                                wdata[i].len = len[i];
                                p = &aullong[offset[i]];
                                wdata[i].p = p;
                            }
                        }
                        else {
                            throw gcnew PSH5XException("Unknown bitfield type!");
                        }

#pragma endregion

                        break;

                    default:

                        throw gcnew PSH5XException("Unsupported type!");
                        break;
                    }

                    if (H5Dwrite(dset, mtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, &wdata[0])) {
                        throw gcnew HDF5Exception("H5Dwrite failed!");
                    }
                }
            }
        }
        finally
        {
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
            if (dset >= 0) {
                H5Dclose(dset);
            }

            Marshal::FreeHGlobal(IntPtr(name));
        }

        return nullptr;
    }


}
