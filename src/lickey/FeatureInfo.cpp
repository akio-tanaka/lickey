#include "stdafx.h"
#include "FeatureInfo.h"
#include "Date.h"


namespace lickey
{
    FeatureInfo::FeatureInfo(): numLics(0)
    {
    }


    FeatureInfo::~FeatureInfo()
    {}


    bool FeatureInfo::IsValid() const
    {
        if(1 > numLics)
        {
            LOG(error) << "the number of license is zero";
            return false;
        }
        if(checkSum != sign)
        {
            LOG(error) << "invalid sign";
            return false;
        }
        return true;
    }


    bool FeatureInfo::IsExpired() const
    {
        Date today;
        SetToday(today);
        return today > expireDate;
    }
}