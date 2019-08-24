#pragma once
#include "FeatureVersion.h"
#include "Date.h"
#include "Hash.h"

namespace ETLicense
{
    class FeatureInfo
    {
        friend class LicenseManager;

    private:
        FeatureVersion version;
        unsigned int numLics;
        Date issueDate;
        Date expireDate;
        Hash sign;  ///< recoded
        Hash checkSum;  ///< calculated

    public:
        FeatureInfo();
        virtual ~FeatureInfo();
        bool IsValid() const;
        bool IsExpired() const;

    public:
        const FeatureVersion& Version() const { return version; };
        unsigned int NumLics() const { return numLics; };
        const Date& IssueDate() const { return issueDate; };
        const Date& ExpireDate() const { return expireDate; };
        const Hash& Sign() const { return sign; };
    };
}
