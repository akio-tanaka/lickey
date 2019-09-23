#include "stdafx.h"
#include "Features.h"


namespace lickey
{
    bool Features::IsValid(const std::string& featureName) const
    {
        const const_iterator cit = (*this).find(featureName);
        if(end() == cit)
        {
            LOG(error) << featureName << " to be checked validity not exist";
            return false;
        }
        return cit->second.IsValid();
    }


    bool Features::IsExpired(const std::string& featureName) const
    {
        const const_iterator cit = (*this).find(featureName);
        if(end() == cit)
        {
            LOG(error) << featureName << " to be checked expire date not exist";
            return false;
        }
        return cit->second.IsExpired();
    }


    bool Features::IsExist(const std::string& featureName) const
    {
        const const_iterator cit = (*this).find(featureName);
        return end() != cit;
    }
}