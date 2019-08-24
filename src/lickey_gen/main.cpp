#include <algorithm>
#include <string>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <FeatureInfo.h>
#include <FileUtility.h>
#include <Date.h>
#include <Log.h>
#include <License.h>
#include <LicenseManager.h>
#include <Version.h>
using namespace ETLicense;

namespace
{
    void ToLower(std::string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(), tolower);
    }


    void ToLowerAndTrim(std::string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(), tolower);
        boost::trim(str);
    }


    bool AddFeature(
        const std::string& featureName,
        const std::string& featureVersion,
        const Date& issue,
        const std::string& expireDate,
        const unsigned int numLics,
        License& lic,
        LicenseManager& licMgr)
    {
        Date expire;
        if (!::Load(expire, expireDate))
        {
            std::cout << "invalid expire date = " << expireDate << "\n";
            return false;
        }

        FeatureVersion version;
        version = featureVersion;
        licMgr.Add(featureName, version, issue, expire, numLics, lic);
        std::cout << "done to add feature = " << featureName << "\n";
        std::cout << "  version = " << featureVersion << "\n";
        std::cout << "  issue date = " << ToString(issue) << "\n";
        std::cout << "  expire date = " << expireDate << "\n";
        std::cout << "  num licenses = " << numLics << "\n";
        return true;
    }
}



int main(int argc, char* argv[])
{
    std::cout << "License generator V" << VERSION() << "\n";
    std::cout << "(half-width characters only / without space and tabspace)\n";
    std::cout << "\n";

    std::string venderName;
    std::cout << "vender name:";
    std::cin >> venderName;
    boost::trim(venderName);

    std::string appName;
    std::cout << "application name:";
    std::cin >> appName;
    boost::trim(appName);

    std::string hardwareKey;
    std::cout << "hardware key(11-22-33-AA-BB-CC format):";
    std::cin >> hardwareKey;
    boost::trim(hardwareKey);

    LicenseManager licMgr(venderName, appName);
    License lic;
    do
    {
        std::cout << "feature name (\"quit\" to quit this operation):";
        std::string feature;
        std::cin >> feature;
        ToLowerAndTrim(feature);
        if (0 == feature.compare("quit"))
        {
            break;
        }

        std::string featureVersion;
        std::cout << "feature version(positive integer):";
        std::cin >> featureVersion;

        Date issue;
        SetToday(issue);

        std::string expireDate;
        do
        {
            std::cout << "expire date(YYYYMMDD format):";
            std::cin >> expireDate;
            ToLowerAndTrim(expireDate);
            if (0 == expireDate.compare("quit"))
            {
                break;
            }
            Date tmp;
            if (!Load(tmp, expireDate))
            {
                std::cout << "invalid date format\n";
                continue;
            }
            break;
        } while (true);

        unsigned int numLics = 0;
        do
        {
            std::cout << "num licenses(position integer):";
            std::cin >> numLics;
            if (0 == numLics)
            {
                std::cout << "num licenses must be more than 0\n";
                continue;
            }
            break;
        } while (true);

        if (!AddFeature(feature, featureVersion, issue, expireDate, numLics, lic, licMgr))
        {
            std::cout << "fail to add new feature\n";
        }
    } while (true);

    if (lic.FeatureMap().empty())
    {
        std::cout << "no feature defined\n";
        std::string buf;
        std::cin >> buf;
        return 0;
    }
    
    do
    {
        std::string filepath;
        std::cout << "license file name:";
        std::cin >> filepath;
        ToLowerAndTrim(filepath);
        if (0 == filepath.compare("quit"))
        {
            std::cout << "done without saving license file\n";
            break;
        }

        std::string baseFilepath = GetBaseFilePath(filepath);
        std::string extension = GetExtension(filepath);
        std::stringstream filepathImpl;
        filepathImpl << baseFilepath << "(" << hardwareKey << ")" << extension;

        if (!licMgr.Save(filepathImpl.str(), HardwareKey(hardwareKey), lic))
        {
            std::cout << "fail to save into = " << filepathImpl.str() << "\n";
        }
        else
        {
            std::cout << "done to save into = " << filepathImpl.str() << "\n";
            break;
        }
    } while (true);

    std::cout << "please press any key\n";
    std::string buf;
    std::cin >> buf;
    return 0;
}

