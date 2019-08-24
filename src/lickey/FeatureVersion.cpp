#include "stdafx.h"
#include "FeatureVersion.h"
using namespace ETLicense;

FeatureVersion::FeatureVersion()
{}


FeatureVersion::FeatureVersion(const FeatureVersion& obj)
    : version(obj.version)
{}


FeatureVersion::~FeatureVersion()
{}


FeatureVersion& FeatureVersion::operator=(const FeatureVersion& obj)
{
    version = obj.version;
    return *this;
}


FeatureVersion& FeatureVersion::operator=(const std::string& v)
{
    version = v;
    return *this;
}