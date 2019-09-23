#pragma once

namespace lickey
{
    class FeatureVersion
    {
        friend class LicenseManager;

    public:
        FeatureVersion();
        FeatureVersion(const FeatureVersion& obj);
        virtual ~FeatureVersion();

        FeatureVersion& operator=(const FeatureVersion& obj);
        FeatureVersion& operator=(const std::string& v);

        const std::string& Value() const
        {
            return version;
        };

    private:
        std::string version;
    };
}
