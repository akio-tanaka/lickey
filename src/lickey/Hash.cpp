#include "stdafx.h"
#include "Hash.h"


namespace lickey
{
    Hash::Hash()
        : hash("")
    {}


    Hash::Hash(const Hash& obj)
        : hash(obj.hash)
    {}


    Hash::~Hash()
    {}


    Hash& Hash::operator=(const Hash& obj)
    {
        hash = obj.hash;
        return *this;
    }


    Hash& Hash::operator=(const std::string& other)
    {
        hash = other;
        return *this;
    }


    bool Hash::operator==(const Hash& other) const
    {
        return 0 == hash.compare(other.hash);
    }


    bool Hash::operator!=(const Hash& other) const
    {
        return !(*this == other);
    }
}