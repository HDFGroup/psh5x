
#include "CompoundDatasetReader.h"
#include "HDF5Exception.h"
#include "ProviderUtils.h"
#include "PSH5XException.h"

extern "C" {
#include "H5Dpublic.h"
#include "H5Spublic.h"
#include "H5Tpublic.h"
}

#include <cstdlib>

using namespace Microsoft::CSharp;

using namespace System;
using namespace System::CodeDom::Compiler;
using namespace System::Collections;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Provider;
using namespace System::Reflection;
using namespace System::Runtime::InteropServices;
using namespace System::Text;

namespace PSH5X
{
    CompoundDatasetReader::CompoundDatasetReader(hid_t h5file, String^ h5path)
        : m_array(nullptr), m_position(0)
    {
        char* name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();
        char* mname = NULL;

        unsigned char* rdata = NULL;

        hid_t dset = -1, ftype = -1, ntype = -1, mtype = -1, memtype = -1, fspace = -1;

        try
        {
            dset = H5Dopen2(h5file, name, H5P_DEFAULT);
            if (dset < 0) {
                throw gcnew HDF5Exception("H5Dopen2 failed!");
            }

            fspace = H5Dget_space(dset);
            if (fspace < 0) {
                throw gcnew HDF5Exception("H5Dget_space failed!");
            }

            ftype = H5Dget_type(dset);
            if (ftype < 0) {
                throw gcnew HDF5Exception("H5Dget_type failed!");
            }

            size_t size = H5Tget_size(ftype);
                
            hssize_t npoints = H5Sget_simple_extent_npoints(fspace);
            if (npoints > 0)
            {
                memtype = H5Tcreate(H5T_COMPOUND, size);
                if (memtype < 0) {
                    throw gcnew HDF5Exception("H5Tcreate failed!");
                }

                int mcount = H5Tget_nmembers(ftype);

                array<String^>^ member_name = gcnew array<String^>(mcount);
                array<H5T_class_t>^ member_class = gcnew array<H5T_class_t>(mcount);
                array<String^>^ member_type = gcnew array<String^>(mcount);
                array<int>^ member_size = gcnew array<int>(mcount);
                array<int>^ member_offset = gcnew array<int>(mcount);
                array<MethodInfo^>^ member_info = gcnew array<MethodInfo^>(mcount);

                for (int i = 0; i < mcount; ++i)
                {
                    size_t offset = H5Tget_member_offset(ftype, i);

                    member_offset[i] = safe_cast<int>(offset);
                    mname = H5Tget_member_name(ftype, safe_cast<unsigned>(i));
                    mtype = H5Tget_member_type(ftype, safe_cast<unsigned>(i));
                    if (mtype < 0) {
                        throw gcnew HDF5Exception("H5Tget_member_type failed!");
                    }
                    member_class[i] = H5Tget_class(mtype);

                    if (member_class[i]) {
                        ntype = H5Tget_native_type(mtype, H5T_DIR_DESCEND);
                    }
                    else {
                        ntype = H5Tget_native_type(mtype, H5T_DIR_ASCEND);
                    }
                    if (ntype < 0) {
                        throw gcnew HDF5Exception("H5Tget_native_type failed!");
                    }

                    if (H5Tinsert(memtype, mname, offset, ntype) < 0) {
                        throw gcnew HDF5Exception("H5Tinsert failed!");
                    }
                    
                    member_size[i] = safe_cast<int>(H5Tget_size(ntype));
                    member_info[i] = ProviderUtils::BitConverterMethod(ntype);

                    Type^ t = ProviderUtils::H5Type2DotNet(ntype);
                    if (t != nullptr)
                    {
                        member_type[i]  = t->ToString();
                    }
                    else {
                        throw gcnew PSH5XException("Unsupported member type in compound!");
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

                m_type = ProviderUtils::GetCompundDotNetType(ftype);

                array<ConstructorInfo^>^ cinfo = m_type->GetConstructors();
                if (cinfo->Length != 2) {
                    throw gcnew PSH5XException("Constructor code generation error!");
                }

                array<unsigned char>^ mbuf = gcnew array<unsigned char>(npoints*size);

                pin_ptr<unsigned char> p = &mbuf[0];
                unsigned char* buf = p;

                if (H5Dread(dset, memtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf) < 0) {
                    throw gcnew HDF5Exception("H5Dread failed!");
                }

                m_array = gcnew array<Object^>(npoints);

                array<Object^>^ args = gcnew array<Object^>(mcount);
                
                array<unsigned char>^ row = gcnew array<unsigned char>(size);

                for (long long i = 0; i < npoints; ++i)
                {
                    Array::Copy(mbuf, size*i, row, 0, size);

                    for (int m = 0; m < mcount; ++m)
                    {
                        if (member_info[m] != nullptr) {
                            args[m] = member_info[m]->Invoke(
                                nullptr, gcnew array<Object^>{row, member_offset[m]});
                        }
                        else {
                            if (member_type[m] == "System.Byte") {
                                args[m] = System::Byte(row[member_offset[m]]);
                            }
                            else if(member_class[m] == H5T_STRING)
                            {
                                args[m] = Marshal::PtrToStringAnsi(IntPtr(buf+size*i+member_offset[m]));
                            }
                            else {
                                args[m] = "\"" + BitConverter::ToString(row, member_offset[m], member_size[m]) + "\""; 
                            }
                        }
                    }

                    m_array->SetValue(cinfo[1]->Invoke(args), i);
                }
            }
            else
            {
                m_array = gcnew array<Object^>(0);
            }
        }
        finally
        {
            if (rdata != NULL) {
                delete [] rdata;
            }
            if (fspace >= 0) {
                H5Sclose(fspace);
            }
            if (ftype >= 0) {
                H5Tclose(ftype);
            }
            if (memtype >= 0) {
                H5Tclose(memtype);
            }
            if (ntype >= 0) {
                H5Tclose(ntype);
            }
            if (mtype >= 0) {
                H5Tclose(mtype);
            }
            if (dset >= 0) {
                H5Dclose(dset);
            }
            if (name != NULL) {
                Marshal::FreeHGlobal(IntPtr(name));
            }
        }
    }

    IList^ CompoundDatasetReader::Read(long long readCount)
    {
        array<Object^>^ result = nullptr;

        long long remaining = m_array->LongLength - m_position;
        if (remaining > 0)
        {
            long long length = 0;

            if (readCount > remaining)
            {
                length = remaining;
            }
            else
            {
                if (readCount > 0)
                {
                    length = readCount;
                }
                else
                {
                    length = m_array->LongLength;
                }
            }

            result = gcnew array<Object^>(length);

            Array::Copy((Array^) m_array, m_position, (Array^) result, (long long) 0, length);

            m_position += length;
        }

        return result;
    }
}