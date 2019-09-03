#include "stdafx.h"
#include "License.h"
#include "Version.h"
using namespace lickey;

License::License()
    : fileVersion(VERSION())
{}

License::License(const License& obj)
    : fileVersion(obj.fileVersion)
    , features(obj.features)
    , key(obj.key)
    , explicitSalt(obj.explicitSalt)
    , implicitSalt(obj.implicitSalt)
    , lastUsedDate(obj.lastUsedDate)
{}

License::~License()
{}

License& License::operator=(const License& obj)
{
    fileVersion = obj.fileVersion;
    features = obj.features;
    key = obj.key;
    explicitSalt = obj.explicitSalt;
    implicitSalt = obj.implicitSalt;
    lastUsedDate = obj.lastUsedDate;
    return *this;
}