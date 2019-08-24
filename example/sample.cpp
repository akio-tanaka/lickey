
static const std::string LICENSE_FILENAME = "license.txt";


std::string GetLicenseFilepath(const std::string& exeFilepath)
{
    // get folder path
    std::string::size_type pos = exeFilepath.find_last_of("\\");
    if(std::string::npos == pos)
    {
        return LICENSE_FILENAME;
    }
    std::string folder = exeFilepath.substr(0, pos) + "\\";
    return folder + LICENSE_FILENAME;
}


int main(int argc, char* argv[])
{
    ETLicense::HardwareKeyGetter keyGetter;
    ETLicense::HardwareKey keys = keyGetter();
    if(keys.empty())
    {
        MUJO_LOG(MUJO::ERR) << "no hardware information from this computer\n";
        return 1;
    }

    ETLicense::LicenseManager licMgr("eandt", "mujo-cadread");
    ETLicense::License lic;
    std::string licenseFilepath  = GetLicenseFilepath(argv[0]);
    if(!licMgr.Load(licenseFilepath, keys.front(), lic))
    {
        MUJO_LOG(MUJO::ERR) << "fail to load license file = " << licenseFilepath << "\n";
        return 2;
    }

    it(lic.FeatureMap().IsExpired("base"))
    {
        MUJO_LOG(MUJO::ERR) << "license expired = " << licenseFilepath << "\n";
        return 3;
    }
    if(!lic.FeatureMap().IsValid("base"))
    {
        MUJO_LOG(MUJO::ERR) << "invalid information in license = " << licenseFilepath << "\n";
        return 4;
    }
    MUJO_LOG(MUJO::INFO) << "license is valid\n";


    // ... your code ...

    return 0;
}