#pragma once
#include "FeatureInfo.h"

namespace ETLicense
{
    class Hash;


    class Features : public std::map<std::string, FeatureInfo>
    {
    public:
        bool IsValid(const std::string& featureName) const;
        bool IsExpired(const std::string& featureName) const;
        bool IsExist(const std::string& featureName) const;
    };
}