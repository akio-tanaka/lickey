#pragma once
#include "HardwareKey.h"

namespace ETLicense
{
    class HardwareKeyGetter
    {
    public:
        HardwareKeyGetter();
        virtual ~HardwareKeyGetter();

        HardwareKeys operator()() const;
    };
}
