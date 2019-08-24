#pragma once
#define BOOST_TEST_MODULE my_mod
#include <Date.h>
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(Date)

BOOST_AUTO_TEST_CASE(Constructor01)
{
    ETLicense::Date date(2017, 2, 28);

    BOOST_CHECK_EQUAL(2017, date.year());
    BOOST_CHECK_EQUAL(2, date.month());
    BOOST_CHECK_EQUAL(28, date.day());
}

BOOST_AUTO_TEST_CASE(Constructor02)
{
    ETLicense::Date date;
    ETLicense::Load(date, "20170228");
    BOOST_CHECK_EQUAL("20170228", ETLicense::ToString(date));
}

BOOST_AUTO_TEST_CASE(EQ01)
{
    ETLicense::Date date1(2017, 2, 28);
    ETLicense::Date date2;
    ETLicense::Load(date2, "20170228");
    BOOST_CHECK_EQUAL(true, date1 == date2);
}

BOOST_AUTO_TEST_SUITE_END()
