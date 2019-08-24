#pragma once

namespace ETLicense
{
    class Salt
    {
        std::string salt;

    public:
        Salt();
        Salt(const Salt& obj);
        virtual ~Salt();
        Salt& operator=(const Salt& obj);
        Salt& operator=(const std::string& obj);

    public:
        std::string Value() const { return salt; };
    };
}
