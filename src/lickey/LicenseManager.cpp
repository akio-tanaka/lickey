#include "stdafx.h"
#include "LicenseManager.h"
#include "License.h"
#include "FeatureInfo.h"
#include "FileUtility.h"
#include "HardwareKeyGetter.h"
#include "CryptoUtility.h"
#include "Version.h"
#include <boost/algorithm/string.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/scoped_array.hpp>
#include <algorithm>
#include <fstream>
#include <locale>
// for C4996: http://eng-notebook.com/blog-entry-229/


namespace
{
    using namespace lickey;

    const unsigned int BUF_SIZE = 65536;
    const std::string DATA_SECTION_DELIMITER = "***";


    struct UnsignedChar2Char
    {
        char operator()(unsigned char c) const
        {
            return static_cast<char>(c);
        };
    };


    int CalcBase64EncodedSize(int origDataSize)
    {
        const int numBlocks6 = (origDataSize * 8 + 5) / 6;  // the number of blocks (6 bits per a block, rounding up)
        const int numBlocks4 = (numBlocks6 + 3) / 4;    // the number of blocks (4 characters per a block, rounding up)
        const int numNetChars = numBlocks4 * 4; // the number of characters without carriage return
        return numNetChars + ((numNetChars / 76) * 2);  // the number of encoded characters (76 characters per line, curretly only for carriage return)
    }


    std::string Convert(const std::string& featureName, const FeatureInfo& featureInfo)
    {
        std::stringstream converted;
        converted << "feature "
                  << "name=" << featureName << " "
                  << "version=" << featureInfo.Version().Value() << " "
                  << "issue=" << ToString(featureInfo.IssueDate()) << " "
                  << "expire=" << ToString(featureInfo.ExpireDate()) << " "
                  << "num=" << featureInfo.NumLics() << " "
                  << "sign=" << featureInfo.Sign().Value();
        return converted.str();
    }
}

namespace
{
    typedef std::map<std::string, std::string> FeatureTree;
    typedef FeatureTree::iterator FTItr;


    void Split(const std::string& line, std::vector<std::string>& tokens, const std::string delim = " ")
    {
        const auto trim = [](std::string & str)
        {
            boost::trim(str);
        };
        boost::algorithm::split(tokens, line, boost::is_any_of(delim));
        boost::for_each(tokens, trim);
    }


    void MakeFeatureTree(
        const std::vector<std::string>& tokens,
        FeatureTree& tree)
    {
        for(size_t i = 0; i < tokens.size(); ++i)
        {
            std::vector<std::string> subTokens;
            Split(tokens[i], subTokens, "=");
            if(2 > subTokens.size())
            {
                if(0 == subTokens.front().compare("feature"))
                {
                    tree["feature"] = "feature";
                }
                continue;
            }
            if(0 == subTokens.front().compare("sign"))
            {
                // because sign value can have "=",
                tree["sign"] = tokens[i].substr(5, tokens[i].size() - 5);
            }
            else
            {
                std::string key = subTokens.front();
                std::transform(key.begin(), key.end(), key.begin(), tolower);
                tree[key] = subTokens[1];
            }
        }
    }


    bool FindDataSection(
        const std::vector<std::string>& lines,
        std::string& data)
    {
        const auto isDataDelmiter = [](const std::string & line)
        {
            const std::string::size_type pos = line.find_first_of(DATA_SECTION_DELIMITER);
            if(std::string::npos == pos)
            {
                return false;
            }
            if(0 != pos)
            {
                return false;
            }
            return true;
        };

        bool doesDataExist = false;
        bool isInData = false;
        std::stringstream dataStream;
        for(size_t i = 0; i < lines.size(); ++i)
        {
            if(isDataDelmiter(lines[i]))
            {
                isInData = !isInData;
                if(!isInData)
                {
                    break;
                }
                continue;
            }
            if(isInData)
            {
                dataStream << lines[i];
                doesDataExist = true;
            }
        }
        data = dataStream.str();
        return doesDataExist;
    }


    bool MakeEncryptionKey(
        const HardwareKey& key,
        const std::string& vendorName,
        const std::string& appName,
        const Hash& firstFeatureSign,
        const Salt& explicitSalt,
        unsigned char encryptionKey[16])
    {
        std::stringstream src;
        src << key.Value() << explicitSalt.Value() << vendorName << appName << firstFeatureSign.Value();
        MD5(src.str().c_str(), src.str().size(), encryptionKey);
        return true;
    }


    bool MakeEncryptionIv(
        const HardwareKey& key,
        const Salt& explicitSalt,
        const unsigned char encryptionKey[16],
        unsigned char encryptionIv[16])
    {
        std::string encodedKey;
        EncodeBase64(encryptionKey, 16, encodedKey);

        std::stringstream src;
        src << encodedKey << key.Value() << explicitSalt.Value();
        MD5(src.str().c_str(), src.str().size(), encryptionIv);
        return true;
    }


    bool MakeFeatureSign(
        const std::string& featureName,
        const FeatureInfo& featureInfo,
        const Salt& implicitSalt,
        Hash& sign)
    {
        std::stringstream src;
        src << featureName << featureInfo.Version().Value() << ToString(featureInfo.IssueDate()) << ToString(featureInfo.ExpireDate()) << featureInfo.NumLics() << implicitSalt.Value();

        unsigned char sha[32];
        SHA256(src.str().c_str(), src.str().size(), sha);

        std::string encodedSign;
        EncodeBase64(sha, 32, encodedSign);
        sign = encodedSign;
        return true;
    }


    bool DecryptData(
        const HardwareKey& key,
        const std::string& vendorName,
        const std::string& appName,
        const Hash& firstFeatureSign,
        const Salt& explicitSalt,
        const unsigned char* data,
        const size_t datalen,
        Salt& implicitSalt,
        Date& lastUsedDate)
    {
        unsigned char encryptionKey[16];
        if(!MakeEncryptionKey(key, vendorName, appName, firstFeatureSign, explicitSalt, encryptionKey))
        {
            LOG(error) << "fail to get key";
            return false;
        }

        unsigned char encryptionIv[16];
        if(!MakeEncryptionIv(key, explicitSalt, encryptionKey, encryptionIv))
        {
            LOG(error) << "fail to get iv";
            return false;
        }

        unsigned char decryptedImpl[BUF_SIZE] = { '\0' };
        size_t decryptedImplSize = BUF_SIZE;
        Decrypt(data, datalen, encryptionKey, encryptionIv, decryptedImpl, decryptedImplSize);

        char* decryptedImplChar = (char*)malloc(decryptedImplSize);
        boost::scoped_array<char> scopedDecryptedImplChar(decryptedImplChar);
        std::transform(decryptedImpl, decryptedImpl + decryptedImplSize, decryptedImplChar, UnsignedChar2Char());
        std::istringstream src(decryptedImplChar, std::ios::binary);

        const int validLen = CalcBase64EncodedSize(32) + 8;
        if(static_cast<size_t>(validLen) > decryptedImplSize)
        {
            LOG(error) << "invalid data section";
            return false;
        }

        char* saltImpl = (char*)malloc(sizeof(char) * CalcBase64EncodedSize(32) + 1);
        boost::scoped_array<char> scopedSaltImpl(saltImpl);
        src.read(saltImpl, sizeof(char) * CalcBase64EncodedSize(32));
        saltImpl[CalcBase64EncodedSize(32)] = '\0';
        implicitSalt = saltImpl;

        char* dateImpl = (char*)malloc(sizeof(char) * 8 + 1);
        boost::scoped_array<char> scopedDateImpl(dateImpl);
        src.read(dateImpl, sizeof(char) * 8);
        dateImpl[8] = '\0';
        if(!Load(lastUsedDate, dateImpl))
        {
            LOG(error) << "fail to decrypt date because of invalid date";
            return false;
        }
        return true;
    }


    bool EncryptData(
        const HardwareKey& key,
        const std::string& vendorName,
        const std::string& appName,
        const Hash& firstFeatureSign,
        const Salt& explicitSalt,
        const Salt& implicitSalt,
        const Date& lastUsedDate,
        std::string& encrepted)
    {
        unsigned char encryptionKey[16];
        if(!MakeEncryptionKey(key, vendorName, appName, firstFeatureSign, explicitSalt, encryptionKey))
        {
            LOG(error) << "fail to get key";
            return false;
        }

        unsigned char encryptionIv[16];
        if(!MakeEncryptionIv(key, explicitSalt, encryptionKey, encryptionIv))
        {
            LOG(error) << "fail to get iv";
            return false;
        }

        const std::string strDate = ToString(lastUsedDate);
        assert(8 == strDate.size());

        std::ostringstream dst(std::ios::binary);
        dst.write(implicitSalt.Value().c_str(), sizeof(char) * implicitSalt.Value().size());
        dst.write(strDate.c_str(), sizeof(char) * strDate.size());

        unsigned char ecryptedImpl[BUF_SIZE] = { '\0' };
        size_t ecryptedImplSize = BUF_SIZE;
        Encrypt(dst.str().c_str(), dst.str().size(), encryptionKey, encryptionIv, ecryptedImpl, ecryptedImplSize);
        EncodeBase64(ecryptedImpl, static_cast<int>(ecryptedImplSize), encrepted);
        return true;
    }
}



namespace lickey
{
    LicenseManager::LicenseManager(
        const std::string& vn,
        const std::string& an)
        : vendorName(vn)
        , appName(an)
        , isLicenseLorded(false)
    {
        InitializeOpenSSL();
    }


    LicenseManager::~LicenseManager()
    {
        Update();
    }


    bool LicenseManager::Load(const std::string& filepath, const HardwareKey& key, License& license)
    {
        auto IntoChar = [](unsigned char c)
        {
            return static_cast<char>(c);
        };

        licenseFilepath = filepath;
        isLicenseLorded = false;
        license.key = key;

        LOG(info) << "start to load license file = " << filepath;
        std::vector<std::string> lines;
        if(!ReadLines(filepath, lines))
        {
            LOG(error) << "fail to open";
            return false;
        }

        // load features section
        for(size_t i = 0; i < lines.size(); ++i)
        {
            std::string featureName;
            FeatureInfo featureInfo;
            if(!ConvertFeature(lines[i], featureName, featureInfo))
            {
                continue;
            }
            license.features[featureName] = featureInfo;
        }
        if(license.features.empty())
        {
            LOG(error) << "no feature";
            return false;
        }

        // load date section
        std::string data;
        if(!FindDataSection(lines, data))
        {
            LOG(error) << "no data sections";
            return false;
        }

        int decodedSize = 0;
        unsigned char* decoded = NULL;
        DecodeBase64(data, decoded, decodedSize);
        boost::scoped_array<unsigned char> scopedDecoded(decoded);
        if(36 > decodedSize)
        {
            LOG(error) << "no information in data section";
            return false;
        }

        std::string dataBuffer(decodedSize, '\0');
        std::transform(decoded, decoded + decodedSize, dataBuffer.begin(), IntoChar);
        std::istringstream dataSection(dataBuffer, std::ios::binary);

        dataSection.read((char*)&license.fileVersion, sizeof(unsigned int));
        int saltLengthInBase64 = CalcBase64EncodedSize(32);
        char* saltImpl = (char*)malloc(sizeof(char) * saltLengthInBase64 + 1);
        boost::scoped_array<char> scopedSaltImpl(saltImpl);
        dataSection.read(saltImpl, sizeof(char) * saltLengthInBase64);
        saltImpl[saltLengthInBase64] = '\0';
        license.explicitSalt = saltImpl;

        int remainLen = decodedSize - saltLengthInBase64 - sizeof(unsigned int);
        if(1 > remainLen)
        {
            LOG(error) << "no encrypted data in data section";
            return false;
        }
        char* base64Encrypted = (char*)malloc(sizeof(char) * remainLen + 1);
        boost::scoped_array<char> scpdBase64Encrypted(base64Encrypted);
        dataSection.read(base64Encrypted, sizeof(char) * remainLen);
        base64Encrypted[remainLen] = '\0';

        int decodedSize2 = 0;
        unsigned char* decoded2 = NULL;
        DecodeBase64(base64Encrypted, decoded2, decodedSize2);
        boost::scoped_array<unsigned char> scopedDecoded2(decoded2);

        std::string decrypted;
        if(!::DecryptData(
                    key,
                    vendorName,
                    appName,
                    license.features.begin()->second.sign,
                    license.explicitSalt,
                    decoded2,
                    decodedSize2,
                    license.implicitSalt,
                    license.lastUsedDate))
        {
            LOG(error) << "fail to decrypt";
            return false;
        }

        // validate each feature
        for(Features::iterator cit = license.features.begin(); cit != license.features.end(); ++cit)
        {
            Hash checkSum;
            MakeFeatureSign(cit->first, cit->second, license.implicitSalt, checkSum);
            cit->second.checkSum = checkSum;
        }

        loadedLicense = license;
        isLicenseLorded = true;
        return true;
    }


    bool LicenseManager::Update(/*
        const std::string& filepath,
        const HardwareKey& key,
        License& license*/)
    {
        if(!isLicenseLorded)
        {
            LOG(error) << "license is not loaded";
            return false;
        }
        if(loadedLicense.features.empty())
        {
            LOG(error) << "no feature to generate license file";
            return false;
        }

        MakeSalt(loadedLicense.explicitSalt);
        MakeSalt(loadedLicense.implicitSalt);
        for(Features::iterator it = loadedLicense.features.begin(); it != loadedLicense.features.end(); ++it)
        {
            Hash sign;
            MakeFeatureSign(it->first, it->second, loadedLicense.implicitSalt, sign);
            it->second.sign = sign;
        }

        std::string encrypted;
        Date today;
        SetToday(today);
        if(!EncryptData(
                    loadedLicense.key,
                    vendorName,
                    appName,
                    loadedLicense.features.begin()->second.sign,
                    loadedLicense.explicitSalt,
                    loadedLicense.implicitSalt,
                    today, encrypted))
        {
            LOG(error) << "fail to make data section";
            return false;
        }

        std::ostringstream dataSection(std::ios::binary);
        unsigned int fileVersion = VERSION();
        std::string explictSaltValue = loadedLicense.explicitSalt.Value();
        dataSection.write((const char*)&fileVersion, sizeof(unsigned int));
        dataSection.write(explictSaltValue.c_str(), sizeof(char) * explictSaltValue.size());
        dataSection.write(encrypted.c_str(), sizeof(char) * encrypted.size());
        EncodeBase64(dataSection.str(), encrypted);

        std::ofstream out(licenseFilepath.c_str());
        if(!out)
        {
            LOG(error) << "fail to open = " << licenseFilepath;
            return false;
        }
        for(Features::const_iterator cit = loadedLicense.features.begin(); cit != loadedLicense.features.end(); ++cit)
        {
            out << Convert(cit->first, cit->second) << "\n";
        }
        out << "\n";
        out << DATA_SECTION_DELIMITER << "\n";
        out << encrypted << "\n";
        out << DATA_SECTION_DELIMITER << "\n";
        out.close();
        return true;
    }


    bool LicenseManager::Save(const std::string& filepath, const HardwareKey& key, License& license)
    {
        licenseFilepath = filepath;
        loadedLicense = license;
        loadedLicense.key = key;
        isLicenseLorded = true;
        return Update();
    }


    void LicenseManager::Add(
        const std::string& featureName,
        const FeatureVersion& featureVersion,
        const Date& issueDate,
        const Date& expireDate,
        unsigned int numLics,
        License& license)
    {
        FeatureInfo featureInfo;
        featureInfo.version = featureVersion;
        featureInfo.issueDate = issueDate;
        featureInfo.expireDate = expireDate;
        featureInfo.numLics = numLics;
        license.features[featureName] = featureInfo;
    }


    bool LicenseManager::ConvertFeature(
        const std::string& line,
        std::string& featureName,
        FeatureInfo& featureInfo)
    {
        std::vector<std::string> tokens;
        Split(line, tokens);
        if(tokens.empty())
        {
            return false;
        }

        FeatureTree featureTree;
        MakeFeatureTree(tokens, featureTree);
        FTItr it = featureTree.find("feature");
        if(featureTree.end() == it)
        {
            return false;
        }

        it = featureTree.find("name");
        if(featureTree.end() == it)
        {
            LOG(error) << "name not found in feature line (name = " << featureName << ")\n";
            return false;
        }
        featureName = it->second;


        it = featureTree.find("version");
        if(featureTree.end() == it)
        {
            LOG(error) << "version not found in feature line (name = " << featureName << ")\n";
            return false;
        }
        featureInfo.version.version = it->second;

        it = featureTree.find("issue");
        if(featureTree.end() == it)
        {
            LOG(error) << "issue not found in feature line (name = " << featureName << ")\n";
            return false;
        }
        if(!lickey::Load(featureInfo.issueDate, it->second))
        {
            LOG(error) << "invalid issue date = " << it->second << " (name = " << featureName << ")\n";
            return false;
        }

        it = featureTree.find("expire");
        if(featureTree.end() == it)
        {
            LOG(error) << "expire not found in feature line (name = " << featureName << ")\n";
            return false;
        }
        if(!lickey::Load(featureInfo.expireDate, it->second))
        {
            LOG(error) << "invalid expire date = " << it->second << " (name = " << featureName << ")\n";
            return false;
        }

        it = featureTree.find("num");
        if(featureTree.end() == it)
        {
            LOG(error) << "num not found in feature line (name = " << featureName << ")\n";
            return false;
        }
        featureInfo.numLics = boost::lexical_cast<int>(it->second);

        it = featureTree.find("sign");
        if(featureTree.end() == it)
        {
            LOG(error) << "sign not found in feature line (name = " << featureName << ")\n";
            return false;
        }
        featureInfo.sign = it->second;

        LOG(info) << "done to convert feature successfully (name = " << featureName << ")\n";
        return true;
    }
}