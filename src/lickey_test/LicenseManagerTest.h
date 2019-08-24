#pragma once
#define BOOST_TEST_MODULE my_mod
#include <LicenseManager.h>
#include <Date.h>
#include <Features.h>
#include <FeatureInfo.h>
#include <License.h>
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(LicenseManagerTest)

BOOST_AUTO_TEST_CASE(Constructor01)
{
    ETLicense::HardwareKeyGetter keyGetter;
    ETLicense::HardwareKeys keys = keyGetter();
    if (keys.empty())
    {
        return;
    }
    {
        ETLicense::LicenseManager licMgr("eandt", "mujo-cadread");
        ETLicense::License lic;
        licMgr.Load("test(CC-E1-D5-41-21-D6).txt", keys.front(), lic);

        BOOST_CHECK_EQUAL(false, lic.FeatureMap().IsExpired("base"));
        BOOST_CHECK_EQUAL(true, lic.FeatureMap().IsValid("base"));
    }
}



BOOST_AUTO_TEST_SUITE_END()
#pragma once
