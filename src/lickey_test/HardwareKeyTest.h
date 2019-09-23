#pragma once
#define BOOST_TEST_MODULE my_mod
#include <HardwareKey.h>
#include <HardwareKeyGetter.h>
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(HardwareKey)

BOOST_AUTO_TEST_CASE(Constructor01)
{
    lickey::HardwareKeyGetter keyGetter;
    lickey::HardwareKeys keys = keyGetter();

    for(size_t i = 0; i < keys.size(); ++i)
    {
        BOOST_TEST_MESSAGE(keys[i].Value());
    }

    BOOST_CHECK_EQUAL(5, keys.size());
    if(5 == keys.size())
    {
        BOOST_CHECK_EQUAL("CC-E1-D5-41-21-D6", keys[0].Value());
        BOOST_CHECK_EQUAL("80-FA-5B-11-34-19", keys[1].Value());
        BOOST_CHECK_EQUAL("D0-53-49-83-7B-4C", keys[2].Value());
        BOOST_CHECK_EQUAL("D0-53-49-83-8C-7C", keys[3].Value());
        BOOST_CHECK_EQUAL("0A-00-27-00-00-15", keys[4].Value());
    }
}

BOOST_AUTO_TEST_SUITE_END()
