#pragma once

namespace lickey
{
    class HardwareKey final
    {
        friend class HardwareKeyGetter;

    public:
        HardwareKey();
        HardwareKey(const HardwareKey& obj);
        explicit HardwareKey(const std::string& obj);
        ~HardwareKey();
        HardwareKey& operator=(const HardwareKey& obj);
        HardwareKey& operator=(const std::string& obj);

        std::string Value() const
        {
            return key;
        }

    private:
        std::string key;
    };

    typedef std::vector<HardwareKey> HardwareKeys;
}
