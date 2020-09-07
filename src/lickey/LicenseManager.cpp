#include "stdafx.h"
#include "LicenseManager.h"
#include "FileUtility.h"
#include "HardwareKeyGetter.h"
#include "CryptoUtility.h"
#include "Version.h"
#include <boost/algorithm/string.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include <boost/scoped_array.hpp>
#include <algorithm>
#include <fstream>

namespace
{
	using namespace lickey;

	const unsigned int BUF_SIZE = 65536;
	const std::string DATA_SECTION_DELIMITER = "***";

	typedef struct EncryptLicense
	{
		HardwareKey key;
		std::string vendorName;
		std::string appName;
		Hash firstFeatureSign;
		Salt explicitSalt;
		Salt implicitSalt;
		Date lastUsedDate;
	} EL;

	typedef struct DecryptLicense
	{
		HardwareKey key;
		std::string vendorName;
		std::string appName;
		Hash firstFeatureSign;
		Salt explicitSalt;
	} DL;

	struct UnsignedChar2Char
	{
		char operator()(unsigned char c) const
		{
			return static_cast<char>(c);
		};
	};

	auto IntoChar = [](unsigned char c)
	{
		return static_cast<char>(c);
	};

	int CalcBase64EncodedSize(int origDataSize)
	{
		const int numBlocks6 = (origDataSize * 8 + 5) / 6; // the number of blocks (6 bits per a block, rounding up)
		const int numBlocks4 = (numBlocks6 + 3) / 4; // the number of blocks (4 characters per a block, rounding up)
		const int numNetChars = numBlocks4 * 4; // the number of characters without carriage return
		return numNetChars + ((numNetChars / 76) * 2);
		// the number of encoded characters (76 characters per line, currently only for carriage return)
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


	void Split(const std::string& line, std::vector<std::string>& tokens, const std::string& delim = " ")
	{
		const auto trim = [](std::string& str)
		{
			boost::trim(str);
		};
		split(tokens, line, boost::is_any_of(delim));
		boost::for_each(tokens, trim);
	}


	void MakeFeatureTree(
		const std::vector<std::string>& tokens,
		FeatureTree& tree)
	{
		for (size_t i = 0; i < tokens.size(); ++i)
		{
			std::vector<std::string> subTokens;
			Split(tokens[i], subTokens, "=");

			if (2 > subTokens.size())
			{
				if (0 == subTokens.front().compare("feature"))
				{
					tree["feature"] = "feature";
				}

				continue;
			}

			if (0 == subTokens.front().compare("sign"))
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
		const auto isDataDelimiter = [](const std::string& line)
		{
			const std::string::size_type pos = line.find_first_of(DATA_SECTION_DELIMITER);

			if (std::string::npos == pos)
			{
				return false;
			}

			if (0 != pos)
			{
				return false;
			}

			return true;
		};
		bool doesDataExist = false;
		bool isInData = false;
		std::stringstream dataStream;

		for (size_t i = 0; i < lines.size(); ++i)
		{
			if (isDataDelimiter(lines[i]))
			{
				isInData = !isInData;

				if (!isInData)
				{
					break;
				}

				continue;
			}

			if (isInData)
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
		src << featureName << featureInfo.Version().Value() << ToString(featureInfo.IssueDate()) <<
			ToString(featureInfo.ExpireDate()) << featureInfo.NumLics() << implicitSalt.Value();
		unsigned char sha[32];
		SHA256(src.str().c_str(), src.str().size(), sha);
		std::string encodedSign;
		EncodeBase64(sha, 32, encodedSign);
		sign = encodedSign;
		return true;
	}


	bool DecryptData(
		const DL dl,
		Salt& implicitSalt,
		Date& lastUsedDate,
		unsigned char* data,
		size_t datalen
	)
	{
		unsigned char encryptionKey[16];

		if (!MakeEncryptionKey(dl.key, dl.vendorName, dl.appName, dl.firstFeatureSign, dl.explicitSalt, encryptionKey))
		{
			LOG(error) << "fail to get key";
			return false;
		}

		unsigned char encryptionIv[16];

		if (!MakeEncryptionIv(dl.key, dl.explicitSalt, encryptionKey, encryptionIv))
		{
			LOG(error) << "fail to get iv";
			return false;
		}

		unsigned char decryptedImpl[BUF_SIZE] = {'\0'};
		size_t decryptedImplSize = BUF_SIZE;
		Decrypt(data, datalen, encryptionKey, encryptionIv, decryptedImpl, decryptedImplSize);
		char* decryptedImplChar = static_cast<char*>(malloc(decryptedImplSize));
		boost::scoped_array<char> scopedDecryptedImplChar(decryptedImplChar);
		std::transform(decryptedImpl, decryptedImpl + decryptedImplSize, decryptedImplChar, UnsignedChar2Char());
		std::istringstream src(decryptedImplChar, std::ios::binary);
		const int validLen = CalcBase64EncodedSize(32) + 8;

		if (static_cast<size_t>(validLen) > decryptedImplSize)
		{
			LOG(error) << "invalid data section";
			return false;
		}

		char* saltImpl = static_cast<char*>(malloc(
			static_cast<size_t>(sizeof(char) * static_cast<size_t>(CalcBase64EncodedSize(32)) + 1)));

		if (saltImpl == nullptr)
		{
			assert(saltImpl);
		}
		else
		{
			boost::scoped_array<char> scopedSaltImpl(saltImpl);
			src.read(saltImpl, static_cast<int>(sizeof(char)) * CalcBase64EncodedSize(32));
			size_t it = static_cast<size_t>(CalcBase64EncodedSize(32)); //memsize
			saltImpl[it] = '\0';
			implicitSalt = saltImpl;
		}

		char* dateImpl = static_cast<char*>(malloc(sizeof(char) * 8 + 1));

		if (dateImpl == nullptr)
		{
			assert(dateImpl);
		}
		else
		{
			boost::scoped_array<char> scopedDateImpl(dateImpl);
			src.read(dateImpl, sizeof(char) * 8);
			dateImpl[8] = '\0';

			if (!Load(lastUsedDate, dateImpl))
			{
				LOG(error) << "fail to decrypt date because of invalid date";
				return false;
			}

			return true;
		}

		return false;
	}


	bool EncryptData(EL& el, std::string& encrypted)
	{
		unsigned char encryptionKey[16];

		if (!MakeEncryptionKey(el.key, el.vendorName, el.appName, el.firstFeatureSign, el.explicitSalt, encryptionKey))
		{
			LOG(error) << "fail to get key";
			return false;
		}

		unsigned char encryptionIv[16];

		if (!MakeEncryptionIv(el.key, el.explicitSalt, encryptionKey, encryptionIv))
		{
			LOG(error) << "fail to get iv";
			return false;
		}

		const std::string strDate = ToString(el.lastUsedDate);
		assert(8 == strDate.size());
		std::ostringstream dst(std::ios::binary);
		dst.write(el.implicitSalt.Value().c_str(), sizeof(char) * el.implicitSalt.Value().size());
		dst.write(strDate.c_str(), sizeof(char) * strDate.size());
		unsigned char ecryptedImpl[BUF_SIZE] = {'\0'};
		size_t ecryptedImplSize = BUF_SIZE;
		Encrypt(dst.str().c_str(), dst.str().size(), encryptionKey, encryptionIv, ecryptedImpl, ecryptedImplSize);
		EncodeBase64(ecryptedImpl, static_cast<int>(ecryptedImplSize), encrypted);
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
		  , isLicenseLoaded(false)
	{
		InitializeOpenSSL();
	}


	LicenseManager::~LicenseManager()
	{
		Update();
	}


	bool LicenseManager::isLicenseDecrypt(const HardwareKey& key, License& license, int decodedSize2,
	                                      unsigned char* decoded2)
	{
		DL decrypt_license;
		decrypt_license.key = key;
		decrypt_license.vendorName = vendorName;
		decrypt_license.appName = appName;
		decrypt_license.firstFeatureSign = license.features.begin()->second.sign;
		decrypt_license.explicitSalt = license.explicitSalt;

		if (DecryptData(decrypt_license, license.implicitSalt, license.lastUsedDate,
		                decoded2,
		                static_cast<const size_t>(decodedSize2)
		))
		{
			// validate each feature
			for (Features::iterator cit = license.features.begin(); cit != license.features.end(); ++cit)
			{
				Hash checkSum;
				MakeFeatureSign(cit->first, cit->second, license.implicitSalt, checkSum);
				cit->second.checkSum = checkSum;
			}

			loadedLicense = license;
			isLicenseLoaded = true;
			return true;
		}

		LOG(error) << "fail to decrypt";
		return false;
	}

	bool LicenseManager::isLicenseDataSectionRead(const HardwareKey& key, License& license,
	                                              std::vector<std::string> lines)
	{
		std::string data;

		if (FindDataSection(lines, data))
		{
			int decodedSize = 0;
			unsigned char* decoded = nullptr;
			DecodeBase64(data, decoded, decodedSize);
			boost::scoped_array<unsigned char> scopedDecoded(decoded);

			if (36 > decodedSize)
			{
				LOG(error) << "no information in data section";
				return false;
			}

			std::string dataBuffer(decodedSize, '\0');
			std::transform(decoded, decoded + static_cast<unsigned char>(decodedSize), dataBuffer.begin(), IntoChar);
			std::istringstream dataSection(dataBuffer, std::ios::binary);
			dataSection.read(static_cast<char*>(&license.fileVersion), sizeof(unsigned int));
			int saltLengthInBase64 = CalcBase64EncodedSize(32);
			char* saltImpl = static_cast<char*>(malloc(
				static_cast<size_t>(sizeof(char) * static_cast<size_t>(saltLengthInBase64) + 1)));

			if (saltImpl == nullptr)
			{
				assert(saltImpl);
			}
			else
			{
				boost::scoped_array<char> scopedSaltImpl(saltImpl);
				dataSection.read(saltImpl, static_cast<int>(sizeof(char)) * saltLengthInBase64);
				size_t it = static_cast<size_t>(saltLengthInBase64); //memsize
				saltImpl[it] = '\0';
				license.explicitSalt = saltImpl;
			}

			int remainLen = decodedSize - saltLengthInBase64 - static_cast<int>(sizeof(unsigned int));

			if (1 > remainLen)
			{
				LOG(error) << "no encrypted data in data section";
				return false;
			}

			char* base64Encrypted = static_cast<char*>(malloc(
				static_cast<size_t>(sizeof(char) * static_cast<size_t>(remainLen) + 1)));

			if (base64Encrypted == nullptr)
			{
				assert(base64Encrypted);
			}
			else
			{
				boost::scoped_array<char> scpdBase64Encrypted(base64Encrypted);
				dataSection.read(base64Encrypted, static_cast<int>(sizeof(char)) * remainLen);
				size_t it = static_cast<size_t>(remainLen); //memsize
				base64Encrypted[it] = '\0';
				int decodedSize2 = 0;
				unsigned char* decoded2 = nullptr;
				DecodeBase64(base64Encrypted, decoded2, decodedSize2);
				boost::scoped_array<unsigned char> scopedDecoded2(decoded2);
				return isLicenseDecrypt(key, license, decodedSize2, decoded2);
			}

			return false;
		}

		LOG(error) << "no data sections";
		return false;
	}

	bool LicenseManager::isLicenseRead(const std::string& filepath, const HardwareKey& key, License& license)
	{
		std::vector<std::string> lines;

		if (ReadLines(filepath, lines))
		{
			// load features section
			for (size_t i = 0; i < lines.size(); ++i)
			{
				std::string featureName;
				FeatureInfo featureInfo;

				if (ConvertFeature(lines[i], featureName, featureInfo))
				{
					license.features[featureName] = featureInfo;
				}
			}

			if (license.features.empty())
			{
				LOG(error) << "no feature";
				return false;
			}

			// load date section
			return isLicenseDataSectionRead(key, license, lines);
		}

		LOG(error) << "fail to open";
		return false;
	}

	bool LicenseManager::Load(const std::string& filepath, const HardwareKey& key, License& license)
	{
		licenseFilepath = filepath;
		isLicenseLoaded = false;
		license.key = key;
		LOG(info) << "start to load license file = " << filepath;
		return isLicenseRead(filepath, key, license);
	}

	bool LicenseManager::Update()
	{
		if (isLicenseLoaded)
		{
			if (loadedLicense.features.empty())
			{
				LOG(error) << "no feature to generate license file";
				return false;
			}

			MakeSalt(loadedLicense.explicitSalt);
			MakeSalt(loadedLicense.implicitSalt);

			for (Features::iterator it = loadedLicense.features.begin(); it != loadedLicense.features.end(); ++it)
			{
				Hash sign;
				MakeFeatureSign(it->first, it->second, loadedLicense.implicitSalt, sign);
				it->second.sign = sign;
			}

			return UpdateLicense();
		}

		LOG(error) << "license is not loaded";
		return false;
	}

	bool LicenseManager::UpdateLicense()
	{
		std::string encrypted;
		Date today;
		SetToday(today);
		EL encrypt_license;
		encrypt_license.key = loadedLicense.key;
		encrypt_license.vendorName = vendorName;
		encrypt_license.appName = appName;
		encrypt_license.firstFeatureSign = loadedLicense.features.begin()->second.sign;
		encrypt_license.explicitSalt = loadedLicense.explicitSalt;
		encrypt_license.implicitSalt = loadedLicense.implicitSalt;
		encrypt_license.lastUsedDate = today;

		if (EncryptData(encrypt_license, encrypted))
		{
			std::ostringstream dataSection(std::ios::binary);
			char fileVersion = VERSION();
			std::string explictSaltValue = loadedLicense.explicitSalt.Value();
			dataSection.write(static_cast<const char*>(&fileVersion), sizeof(unsigned int));
			dataSection.write(explictSaltValue.c_str(), sizeof(char) * explictSaltValue.size());
			dataSection.write(encrypted.c_str(), sizeof(char) * encrypted.size());
			EncodeBase64(dataSection.str(), encrypted);
			std::ofstream out(licenseFilepath.c_str());

			if (out)
			{
				for (Features::const_iterator cit = loadedLicense.features.begin(); cit != loadedLicense.features.
				     end();
				     ++
				     cit)
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

			LOG(error) << "fail to open = " << licenseFilepath;
			return false;
		}

		LOG(error) << "fail to make data section";
		return false;
	}

	bool LicenseManager::Save(const std::string& filepath, const HardwareKey& key, License& license)
	{
		licenseFilepath = filepath;
		loadedLicense = license;
		loadedLicense.key = key;
		isLicenseLoaded = true;
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

		if (tokens.empty())
		{
			return false;
		}

		FeatureTree featureTree;
		MakeFeatureTree(tokens, featureTree);
		FTItr it = featureTree.find("feature");

		if (featureTree.end() == it)
		{
			return false;
		}

		it = featureTree.find("name");

		if (featureTree.end() == it)
		{
			LOG(error) << "name not found in feature line (name = " << featureName << ")\n";
			return false;
		}

		featureName = it->second;
		it = featureTree.find("version");

		if (featureTree.end() == it)
		{
			LOG(error) << "version not found in feature line (name = " << featureName << ")\n";
			return false;
		}

		featureInfo.version.version = it->second;
		it = featureTree.find("issue");

		if (featureTree.end() == it)
		{
			LOG(error) << "issue not found in feature line (name = " << featureName << ")\n";
			return false;
		}

		if (!lickey::Load(featureInfo.issueDate, it->second))
		{
			LOG(error) << "invalid issue date = " << it->second << " (name = " << featureName << ")\n";
			return false;
		}

		it = featureTree.find("expire");

		if (featureTree.end() == it)
		{
			LOG(error) << "expire not found in feature line (name = " << featureName << ")\n";
			return false;
		}

		if (!lickey::Load(featureInfo.expireDate, it->second))
		{
			LOG(error) << "invalid expire date = " << it->second << " (name = " << featureName << ")\n";
			return false;
		}

		it = featureTree.find("num");

		if (featureTree.end() == it)
		{
			LOG(error) << "num not found in feature line (name = " << featureName << ")\n";
			return false;
		}

		featureInfo.numLics = boost::lexical_cast<int>(it->second);
		it = featureTree.find("sign");

		if (featureTree.end() == it)
		{
			LOG(error) << "sign not found in feature line (name = " << featureName << ")\n";
			return false;
		}

		featureInfo.sign = it->second;
		LOG(info) << "done to convert feature successfully (name = " << featureName << ")\n";
		return true;
	}
}
