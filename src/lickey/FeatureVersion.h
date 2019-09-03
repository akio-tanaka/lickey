#pragma once

namespace lickey
{
    class FeatureVersion
    {
        friend class LicenseManager;

    private:
        std::string version;

    public:
        FeatureVersion();
        FeatureVersion(const FeatureVersion& obj);
        virtual ~FeatureVersion();

        FeatureVersion& operator=(const FeatureVersion& obj);
        FeatureVersion& operator=(const std::string& v);
    public:
        const std::string& Value() const { return version; };
    };
}
