
#pragma once

#include "PSH5XException.h"

namespace PSH5X
{
    template <typename T>
    public ref class H5Array
    {
    public:

        System::Array^ GetArray()
        {
            return m_handle;
        }

        interior_ptr<T> GetHandle()
        {
            interior_ptr<T> result = nullptr;

            if (m_handle->Rank < 1 || m_handle->Rank > 32)
            {
                System::String^ message = System::String::Format(
                    "H5Array data type does not support more than 32 dimension.\n"
                    " - attempting to create array with {0} dimensions.\n",
                    m_handle->Rank);
                throw gcnew PSH5XException(message);
            }

#pragma region Do not ruin your eyes!

            array<T,1>^ dim1;
            array<T,2>^ dim2;
            array<T,3>^ dim3;
            array<T,4>^ dim4;
            array<T,5>^ dim5;
            array<T,6>^ dim6;
            array<T,7>^ dim7;
            array<T,8>^ dim8;
            array<T,9>^ dim9;
            array<T,10>^ dim10;
            array<T,11>^ dim11;
            array<T,12>^ dim12;
            array<T,13>^ dim13;
            array<T,14>^ dim14;
            array<T,15>^ dim15;
            array<T,16>^ dim16;
            array<T,17>^ dim17;
            array<T,18>^ dim18;
            array<T,19>^ dim19;
            array<T,20>^ dim20;
            array<T,21>^ dim21;
            array<T,22>^ dim22;
            array<T,23>^ dim23;
            array<T,24>^ dim24;
            array<T,25>^ dim25;
            array<T,26>^ dim26;
            array<T,27>^ dim27;
            array<T,28>^ dim28;
            array<T,29>^ dim29;
            array<T,30>^ dim30;
            array<T,31>^ dim31;
            array<T,32>^ dim32;

            switch(m_handle->Rank)
            {
            case 1:
                dim1 = reinterpret_cast<array<T,1>^> (m_handle);
                result = interior_ptr<T>(&dim1[0]);
                break;

            case 2:
                dim2 = reinterpret_cast<array<T,2>^>(m_handle);
                result = interior_ptr<T>(&dim2[0,0]);
                break;

            case 3:
                dim3 = reinterpret_cast<array<T,3>^> (m_handle);
                result = interior_ptr<T>(&dim3[0,0,0]);
                break;

            case 4:
                dim4 = reinterpret_cast<array<T,4>^> (m_handle);
                result = interior_ptr<T>(&dim4[0,0,0,0]);
                break;

            case 5:
                dim5 = reinterpret_cast<array<T,5>^> (m_handle);
                result = interior_ptr<T>(&dim5[0,0,0,0,0]);
                break;

            case 6:
                dim6 = reinterpret_cast<array<T,6>^> (m_handle);
                result = interior_ptr<T>(&dim6[0,0,0,0,0,0]);
                break;

            case 7:
                dim7 = reinterpret_cast<array<T,7>^> (m_handle);
                result = interior_ptr<T>(&dim7[0,0,0,0,0,0,0]);
                break;

            case 8:
                dim8 = reinterpret_cast<array<T,8>^> (m_handle);
                result = interior_ptr<T>(&dim8[0,0,0,0,0,0,0,0]);
                break;

            case 9:
                dim9 = reinterpret_cast<array<T,9>^> (m_handle);
                result = interior_ptr<T>(&dim9[0,0,0,0,0,0,0,0,0]);
                break;

            case 10:
                dim10 = reinterpret_cast<array<T,10>^> (m_handle);
                result = interior_ptr<T>(&dim10[0,0,0,0,0,0,0,0,0,0]);
                break;

            case 11:
                dim11 = reinterpret_cast<array<T,11>^> (m_handle);
                result = interior_ptr<T>(&dim11[0,0,0,0,0,0,0,0,0,0,0]);
                break;

            case 12:
                dim12 = reinterpret_cast<array<T,12>^> (m_handle);
                result = interior_ptr<T>(&dim12[0,0,0,0,0,0,0,0,0,0,0,0]);
                break;

            case 13:
                dim13 = reinterpret_cast<array<T,13>^> (m_handle);
                result = interior_ptr<T>(&dim13[0,0,0,0,0,0,0,0,0,0,0,0,0]);
                break;

            case 14:
                dim14 = reinterpret_cast<array<T,14>^> (m_handle);
                result = interior_ptr<T>(&dim14[0,0,0,0,0,0,0,0,0,0,0,0,0,0]);
                break;

            case 15:
                dim15 = reinterpret_cast<array<T,15>^> (m_handle);
                result = interior_ptr<T>(&dim15[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]);
                break;

            case 16:
                dim16 = reinterpret_cast<array<T,16>^> (m_handle);
                result = interior_ptr<T>(&dim16[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]);
                break;

            case 17:
                dim17 = reinterpret_cast<array<T,17>^> (m_handle);
                result = interior_ptr<T>(&dim17[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]);
                break;

            case 18:
                dim18 = reinterpret_cast<array<T,18>^> (m_handle);
                result = interior_ptr<T>(&dim18[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]);
                break;

            case 19:
                dim19 = reinterpret_cast<array<T,19>^> (m_handle);
                result = interior_ptr<T>(&dim19[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]);
                break;

            case 20:
                dim20 = reinterpret_cast<array<T,20>^> (m_handle);
                result = interior_ptr<T>(&dim20[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]);
                break;

            case 21:
                dim21 = reinterpret_cast<array<T,21>^> (m_handle);
                result = interior_ptr<T>(
                    &dim21[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]);
                break;

            case 22:
                dim22 = reinterpret_cast<array<T,22>^> (m_handle);
                result = interior_ptr<T>(
                    &dim22[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]);
                break;
            
            case 23:
                dim23 = reinterpret_cast<array<T,23>^> (m_handle);
                result = interior_ptr<T>(
                    &dim23[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]);
                break;
            
            case 24:
                dim24 = reinterpret_cast<array<T,24>^> (m_handle);
                result = interior_ptr<T>(
                    &dim24[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]);
                break;

            case 25:
                dim25 = reinterpret_cast<array<T,25>^> (m_handle);
                result = interior_ptr<T>(
                    &dim25[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]);
                break;

            case 26:
                dim26 = reinterpret_cast<array<T,26>^> (m_handle);
                result = interior_ptr<T>(
                    &dim26[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]);
                break;

            case 27:
                dim27 = reinterpret_cast<array<T,27>^> (m_handle);
                result = interior_ptr<T>(
                    &dim27[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]);
                break;

            case 28:
                dim28 = reinterpret_cast<array<T,28>^> (m_handle);
                result = interior_ptr<T>(
                    &dim28[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]);
                break;

            case 29:
                dim29 = reinterpret_cast<array<T,29>^> (m_handle);
                result = interior_ptr<T>(
                    &dim29[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]);
                break;

            case 30:
                dim30 = reinterpret_cast<array<T,30>^> (m_handle);
                result = interior_ptr<T>(
                    &dim30[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]);
                break;
        
            case 31:
                dim31 = reinterpret_cast<array<T,31>^> (m_handle);
                result = interior_ptr<T>(
                    &dim31[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]);
                break;
                        
            case 32:
                dim32 = reinterpret_cast<array<T,32>^> (m_handle);
                result = interior_ptr<T>(
                    &dim32[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]);
                break;

            default:
                break;


            } 

#pragma endregion

            return result;
        }


        H5Array(array<hsize_t>^ dims)
        {
            if (dims->Length < 1 || dims->Length > 32)
            {
                System::String^ message = System::String::Format(
                    "H5Array does not support more than 32 dimension.\n"
                    " - attempting to create array with {0} dimensions.\n",
                    dims->Length);
                throw gcnew PSH5XException(message);
            }

            for (int i = 0; i < dims->Length; ++i) {
                if (dims[i] <= 0) {
                    throw gcnew PSH5XException("Dimensions must be positive!");
                }
            }

#pragma region Do not ruin your eyes!

            switch(dims->Length)
            {
            case 1:
                m_handle = gcnew array<T,1>(dims[0]);
                break;

            case 2:
                m_handle = gcnew array<T,2>(dims[0], dims[1]);
                break;

            case 3:
                m_handle = gcnew array<T,3>(dims[0], dims[1], dims[2]);
                break;

            case 4:
                m_handle = gcnew array<T,4>(
                    dims[0], dims[1], dims[2], dims[3]);
                break;

            case 5:
                m_handle = gcnew array<T,5>(
                    dims[0], dims[1], dims[2], dims[3], dims[4]);
                break;
            
            case 6:
                m_handle = gcnew array<T,6>(
                    dims[0], dims[1], dims[2], dims[3], dims[4], dims[5]);
                break;

            case 7:
                m_handle = gcnew array<T,7>(
                    dims[0], dims[1], dims[2], dims[3], dims[4],
                    dims[5], dims[6]);
                break;

            case 8:
                m_handle = gcnew array<T,8>(
                    dims[0], dims[1], dims[2], dims[3], dims[4],
                    dims[5], dims[6], dims[7]);
                break;

            case 9:
                m_handle = gcnew array<T,9>(
                    dims[0], dims[1], dims[2], dims[3], dims[4],
                    dims[5], dims[6], dims[7], dims[8]);
                break;

            case 10:
                m_handle = gcnew array<T,10>(
                    dims[0], dims[1], dims[2], dims[3], dims[4],
                    dims[5], dims[6], dims[7], dims[8], dims[9]);
                break;

            case 11:
                m_handle = gcnew array<T,11>(
                    dims[0], dims[1], dims[2], dims[3], dims[4],
                    dims[5], dims[6], dims[7], dims[8], dims[9],
                    dims[10]);
                break;

            case 12:
                m_handle = gcnew array<T,12>(
                    dims[0], dims[1], dims[2], dims[3], dims[4],
                    dims[5], dims[6], dims[7], dims[8], dims[9],
                    dims[10], dims[11]);
                break;

            case 13:
                m_handle = gcnew array<T,13>(
                    dims[0], dims[1], dims[2], dims[3], dims[4],
                    dims[5], dims[6], dims[7], dims[8], dims[9],
                    dims[10], dims[11], dims[12]);
                break;

            case 14:
                m_handle = gcnew array<T,14>(
                    dims[0], dims[1], dims[2], dims[3], dims[4],
                    dims[5], dims[6], dims[7], dims[8], dims[9],
                    dims[10], dims[11], dims[12], dims[13]);
                break;

            case 15:
                m_handle = gcnew array<T,15>(
                    dims[0], dims[1], dims[2], dims[3], dims[4],
                    dims[5], dims[6], dims[7], dims[8], dims[9],
                    dims[10], dims[11], dims[12], dims[13], dims[14]);
                break;
                
            case 16:
                m_handle = gcnew array<T,16>(
                    dims[0], dims[1], dims[2], dims[3], dims[4],
                    dims[5], dims[6], dims[7], dims[8], dims[9],
                    dims[10], dims[11], dims[12], dims[13], dims[14],
                    dims[15]);
                break;

            case 17:
                m_handle = gcnew array<T,17>(
                    dims[0], dims[1], dims[2], dims[3], dims[4],
                    dims[5], dims[6], dims[7], dims[8], dims[9],
                    dims[10], dims[11], dims[12], dims[13], dims[14],
                    dims[15], dims[16]);
                break;
                
            case 18:
                m_handle = gcnew array<T,18>(
                    dims[0], dims[1], dims[2], dims[3], dims[4],
                    dims[5], dims[6], dims[7], dims[8], dims[9],
                    dims[10], dims[11], dims[12], dims[13], dims[14],
                    dims[15], dims[16], dims[17]);
                break;
            
            case 19:
                m_handle = gcnew array<T,19>(
                    dims[0], dims[1], dims[2], dims[3], dims[4],
                    dims[5], dims[6], dims[7], dims[8], dims[9],
                    dims[10], dims[11], dims[12], dims[13], dims[14],
                    dims[15], dims[16], dims[17], dims[18]);
                break;
                
            case 20:
                m_handle = gcnew array<T,20>(
                    dims[0], dims[1], dims[2], dims[3], dims[4],
                    dims[5], dims[6], dims[7], dims[8], dims[9],
                    dims[10], dims[11], dims[12], dims[13], dims[14],
                    dims[15], dims[16], dims[17], dims[18], dims[19]);
                break;
                
            case 21:
                m_handle = gcnew array<T,21>(
                    dims[0], dims[1], dims[2], dims[3], dims[4],
                    dims[5], dims[6], dims[7], dims[8], dims[9],
                    dims[10], dims[11], dims[12], dims[13], dims[14],
                    dims[15], dims[16], dims[17], dims[18], dims[19],
                    dims[20]);
                break;
                
            case 22:
                m_handle = gcnew array<T,22>(
                    dims[0], dims[1], dims[2], dims[3], dims[4],
                    dims[5], dims[6], dims[7], dims[8], dims[9],
                    dims[10], dims[11], dims[12], dims[13], dims[14],
                    dims[15], dims[16], dims[17], dims[18], dims[19],
                    dims[20], dims[21]);
                break;
                
            case 23:
                m_handle = gcnew array<T,23>(
                    dims[0], dims[1], dims[2], dims[3], dims[4],
                    dims[5], dims[6], dims[7], dims[8], dims[9],
                    dims[10], dims[11], dims[12], dims[13], dims[14],
                    dims[15], dims[16], dims[17], dims[18], dims[19],
                    dims[20], dims[21], dims[22]);
                break;
                
            case 24:
                m_handle = gcnew array<T,24>(
                    dims[0], dims[1], dims[2], dims[3], dims[4],
                    dims[5], dims[6], dims[7], dims[8], dims[9],
                    dims[10], dims[11], dims[12], dims[13], dims[14],
                    dims[15], dims[16], dims[17], dims[18], dims[19],
                    dims[20], dims[21], dims[22], dims[23]);
                break;
                
            case 25:
                m_handle = gcnew array<T,25>(
                    dims[0], dims[1], dims[2], dims[3], dims[4],
                    dims[5], dims[6], dims[7], dims[8], dims[9],
                    dims[10], dims[11], dims[12], dims[13], dims[14],
                    dims[15], dims[16], dims[17], dims[18], dims[19],
                    dims[20], dims[21], dims[22], dims[23], dims[24]);
                break;
                
            case 26:
                m_handle = gcnew array<T,26>(
                    dims[0], dims[1], dims[2], dims[3], dims[4],
                    dims[5], dims[6], dims[7], dims[8], dims[9],
                    dims[10], dims[11], dims[12], dims[13], dims[14],
                    dims[15], dims[16], dims[17], dims[18], dims[19],
                    dims[20], dims[21], dims[22], dims[23], dims[24],
                    dims[25]);
                break;
                
            case 27:
                m_handle = gcnew array<T,27>(
                    dims[0], dims[1], dims[2], dims[3], dims[4],
                    dims[5], dims[6], dims[7], dims[8], dims[9],
                    dims[10], dims[11], dims[12], dims[13], dims[14],
                    dims[15], dims[16], dims[17], dims[18], dims[19],
                    dims[20], dims[21], dims[22], dims[23], dims[24],
                    dims[25], dims[26]);
                break;
                
            case 28:
                m_handle = gcnew array<T,28>(
                    dims[0], dims[1], dims[2], dims[3], dims[4],
                    dims[5], dims[6], dims[7], dims[8], dims[9],
                    dims[10], dims[11], dims[12], dims[13], dims[14],
                    dims[15], dims[16], dims[17], dims[18], dims[19],
                    dims[20], dims[21], dims[22], dims[23], dims[24],
                    dims[25], dims[26], dims[27]);
                break;
                
            case 29:
                m_handle = gcnew array<T,29>(
                    dims[0], dims[1], dims[2], dims[3], dims[4],
                    dims[5], dims[6], dims[7], dims[8], dims[9],
                    dims[10], dims[11], dims[12], dims[13], dims[14],
                    dims[15], dims[16], dims[17], dims[18], dims[19],
                    dims[20], dims[21], dims[22], dims[23], dims[24],
                    dims[25], dims[26], dims[27], dims[28]);
                break;
                
            case 30:
                m_handle = gcnew array<T,30>(
                    dims[0], dims[1], dims[2], dims[3], dims[4],
                    dims[5], dims[6], dims[7], dims[8], dims[9],
                    dims[10], dims[11], dims[12], dims[13], dims[14],
                    dims[15], dims[16], dims[17], dims[18], dims[19],
                    dims[20], dims[21], dims[22], dims[23], dims[24],
                    dims[25], dims[26], dims[27], dims[28], dims[29]);
                break;
                
            case 31:
                m_handle = gcnew array<T,31>(
                    dims[0], dims[1], dims[2], dims[3], dims[4],
                    dims[5], dims[6], dims[7], dims[8], dims[9],
                    dims[10], dims[11], dims[12], dims[13], dims[14],
                    dims[15], dims[16], dims[17], dims[18], dims[19],
                    dims[20], dims[21], dims[22], dims[23], dims[24],
                    dims[25], dims[26], dims[27], dims[28], dims[29],
                    dims[30]);
                break;
                
            case 32:
                m_handle = gcnew array<T,32>(
                    dims[0], dims[1], dims[2], dims[3], dims[4],
                    dims[5], dims[6], dims[7], dims[8], dims[9],
                    dims[10], dims[11], dims[12], dims[13], dims[14],
                    dims[15], dims[16], dims[17], dims[18], dims[19],
                    dims[20], dims[21], dims[22], dims[23], dims[24],
                    dims[25], dims[26], dims[27], dims[28], dims[29],
                    dims[30], dims[31]);
                break;

            default:
                m_handle = nullptr;
                throw gcnew PSH5XException("Invalid rank!");
                break;
            }

#pragma endregion

        }


    private:

        System::Array^ m_handle;

    };

}