#pragma once

namespace ETLicense
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

    public:
        bool operator==(const Hash& other) const;
        bool operator!=(const Hash& other) const;

    public:
        std::string Value() const { return hash; };
    };
}