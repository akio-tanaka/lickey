#include "stdafx.h"
#include "Date.h"
#include <ctime>
#include <iomanip>
#include <boost/lexical_cast.hpp>

bool ETLicense::Load(Date& date, const std::string& str)
{
    try
    {
        date = boost::gregorian::from_undelimited_string(str);
        return true;
    }
    catch (...)
    {
        LOG(error) << "invalid date = " << str;
        return false;
    }
}


void ETLicense::SetToday(Date& date)
{
    date = boost::gregorian::day_clock::local_day();
}


std::string ETLicense::ToString(const Date& date)
{
    return boost::gregorian::to_iso_string(date);
}