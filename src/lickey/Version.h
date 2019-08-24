#pragma once

namespace ETLicense
{
    class Version
    {
        unsigned int version;

    private:
        Version();

    public:
        static Version& GetInstance();
        virtual ~Version();

    public:
        unsigned int Value() const { return version; };
    };


#define VERSION() Version::GetInstance().Value()
}