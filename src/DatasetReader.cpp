
#include "DatasetReader.h"
#include "Providerutils.h"

extern "C" {
#include "H5Dpublic.h"
#include "H5Spublic.h"
#include "H5Tpublic.h"
}

using namespace System;
using namespace System::Collections;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Provider;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
    DatasetReader::DatasetReader(hid_t h5file, String^ h5path)
        : m_h5file(h5file), m_h5path(h5path), m_isCompound(false), m_array(nullptr), m_position(0)
    {
        if (ProviderUtils::IsH5Dataset(h5file, h5path))
        {
            char* name = (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();
            hid_t dset = H5Dopen2(h5file, name, H5P_DEFAULT);
            if (dset < 0) {
                throw gcnew ArgumentException("H5Dopen2 failed!");
            }

            hid_t file_type = H5Dget_type(dset);
            if (file_type < 0) {
                throw gcnew ArgumentException("H5Dget_type failed!");
            }
            hid_t mem_type = H5Tget_native_type(file_type, H5T_DIR_ASCEND);
            if (mem_type < 0) {
                throw gcnew ArgumentException("H5Tget_native_type failed!");
            }

            m_type = ProviderUtils::ParseH5Type(mem_type);
            if (((String^)m_type["Class"]) == "COMPOUND") {
                m_isCompound = true;
            }

            size_t size = H5Tget_size(mem_type);

            hid_t file_space = H5Dget_space(dset);
            if (file_space < 0) {
                throw gcnew ArgumentException("H5Dget_space failed!");
            }
            hssize_t npoints = H5Sget_simple_extent_npoints(file_space);
            if (npoints > 0)
            {
                hsize_t dims[1];
                dims[0] = static_cast<hsize_t>(npoints);
                hid_t mem_space = H5Screate_simple(1, dims, NULL);

                char* buf = new char [npoints*size];

                if (H5Dread(dset, mem_type, mem_space, H5S_ALL, H5P_DEFAULT, buf) < 0)
                {
                    delete [] buf;
                    throw gcnew ArgumentException("H5Dread failed!");
                }

                delete [] buf;

                m_array = gcnew array<PSObject^>(dims[0]);
                if (m_isCompound)
                {
                    for (long long i = 0; i < m_array->LongLength; ++i)
                    {
                        m_array[i] = gcnew PSObject();
                        for each (String^ key in ((Hashtable^)m_type["Members"])->Keys)
                        {
                            m_array[i]->Properties->Add(gcnew PSNoteProperty(key,i));
                        }
                    }
                }
                else
                {
                    for (long long i = 0; i < m_array->LongLength; ++i)
                    {
                        m_array[i] = gcnew PSObject(i);
                    }
                }
            }
            else
            {
                m_array = gcnew array<PSObject^>(0);
            }

            if (H5Sclose(file_space) < 0) { // TODO
            }
            if (H5Tclose(mem_type) < 0) { // TODO
            }
            if (H5Tclose(file_type) < 0) { // TODO
            }
            if (H5Dclose(dset) < 0) { // TODO
            }
        }
        else {
            throw gcnew ArgumentException(
                String::Format("'{0}' is not an HDF5 dataset.", h5path));
        }
    }

    DatasetReader::DatasetReader(hid_t h5file, String^ h5path,
        array<hsize_t>^ start, array<hsize_t>^ stride,
        array<hsize_t>^ count, array<hsize_t>^ block)
        : m_h5file(h5file), m_h5path(h5path), m_isCompound(false), m_array(nullptr), m_position(0)
    {
    }

    DatasetReader::DatasetReader(hid_t h5file, String^ h5path, array<hsize_t>^ coord)
        : m_h5file(h5file), m_h5path(h5path), m_isCompound(false), m_array(nullptr), m_position(0)
    {
    }


    IList^ DatasetReader::Read(long long readCount)
    {
        array<PSObject^>^ result = nullptr;

        long long remaining = m_array->LongLength - m_position;
        if (remaining > 0)
        {
            long long length = 0;
            if (readCount > remaining) { length = remaining; }
            else { length = readCount; }

            result = gcnew array<PSObject^>(length);
    
            long long pos = m_position;
            for (long long i = 0; i < length; ++i)
            {
                result[i] = m_array[pos++];
            }

            m_position += length;
        }

        return result;
    }
}
