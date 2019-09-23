#pragma once

namespace lickey
{
    class Hash
    {
        std::string hash;

    public:
        Hash();
        Hash(const Hash& obj);
        virtual ~Hash();
        Hash& operator=(const Hash& obj);
        Hash& operator=(const std::string& other);

        bool operator==(const Hash& other) const;
        bool operator!=(const Hash& other) const;

        std::string Value() const
        {
            return hash;
        };
    };
}