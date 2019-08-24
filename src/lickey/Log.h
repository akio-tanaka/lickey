#pragma once
#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>

namespace ETLicense
{
#define LOG(tag) BOOST_LOG_TRIVIAL(tag)
}