#pragma once

namespace PSH5X
{
    ref class ArrayUtils
    {
    public:

        static array<long long>^ GetIndex(array<long long>^ dims, long long l)
        {
			array<long long>^ result = gcnew array<long long>(dims->Length);
            for (int i = dims->Length-1; i >= 0; --i)
            {
                result[i] = l%dims[i];
                l = (l - result[i])/dims[i];
            }

            return result;
        }

    };
}
