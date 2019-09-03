#pragma once

namespace lickey
{
    class HardwareKey
    {
        friend class HardwareKeyGetter;

    private:
        std::string key;

    public:
        HardwareKey();
        HardwareKey(const HardwareKey& obj);
        HardwareKey(const std::string& obj);
        virtual ~HardwareKey();
        HardwareKey& operator=(const HardwareKey& obj);
        HardwareKey& operator=(const std::string& obj);

    public:
        std::string Value() const { return key; };
    };


    typedef std::vector<HardwareKey> HardwareKeys;
}
