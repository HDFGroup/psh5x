#pragma once

#include "PSH5XException.h"

extern "C" {
#include "H5public.h"
}

namespace PSH5X
{
    ref class ArrayUtils
    {
    public:

        static array<long long>^ GetIndex(array<long long>^ dims, long long l)
        {
            array<long long>^ result = gcnew array<long long>(dims->Rank);
            for (int i = dims->Rank-1; i >= 0; --i)
            {
                result[i] = l%dims[i];
                l = (l - result[i])/dims[i];
            }

            return result;
        }

    };
}
