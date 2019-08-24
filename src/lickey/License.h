#pragma once
#include "Features.h"
#include "HardwareKey.h"
#include "Salt.h"

namespace ETLicense
{
    class License
    {
        friend class LicenseManager;

        unsigned int fileVersion;
        Features features;
        HardwareKey key;
        Salt explicitSalt;
        Salt implicitSalt;
        Date lastUsedDate;

    public:
        License();
        License(const License& obj);
        virtual ~License();

        License& operator=(const License& obj);

    public:
        Features& FeatureMap() { return features; };
    };

};
