#pragma once
#include <boost/date_time/gregorian/gregorian.hpp>

namespace ETLicense
{
    typedef boost::gregorian::date Date;

    bool Load(Date& date, const std::string& str);
    void SetToday(Date& date);
    std::string ToString(const Date& date);
}
