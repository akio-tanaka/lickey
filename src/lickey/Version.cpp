#include "stdafx.h"
#include "Version.h"


namespace lickey
{
    Version::Version()
        : version(1)    // first version
    {}


    Version& Version::GetInstance()
    {
        static Version obj;
        return obj;
    }


    Version::~Version()
    {}


    /*
    # warning
    * because this file internal algorithm, it must not be public.

    # fileformat
    feature name=NNN version=VVV issue=YYYYMMDD expire=YYYYMMDD num=N sign=SSSSSSSSSS
    ...(multiple features, 1 feature / line)

    data=XXXX

    # feature
    * "name is option name of application (string without space)
    * "version is option vaersion of application (integer)
    * "issue is date when this license file is isseud (8 digit integer)
    * "expire" is date when this license is expired(8 digit integer)
    * "num" is how many application can be launched(reserved for floating / does not work for nodelock)
    * "sign" is check sum whehther this feature is valid
    ** this sign is hash generated with this line(without "feature" and sign) and implicit salt by SHA - 256 as base64

    * "data" section stores as base64
    ** the first 4 byte version of license file format
    ** 32 byte before decrypt of data is explicit salt as base64
    ** the other data is encrypted data
    * encryption key = hardware key, explicit salt, vendorName, appName and sign at the first feature digested MD5
    * encryption iv = encryption key, hardware key and explicit salt digested MD5

    # decrypted "data" section
    * implicit salt(32 byte) as base64
    * date to use last(8 digit integer)

    # salt
    * both explicit and implicit salt are updated for each launching application
    * key to decrypt "data" section is generated with MD5("hadrware information" + explicit salt)
    * iv to decrypt "data" section is generated with MD5(key + "hardware information" + explicit salt)

    # dependency
    * boost V1.60.0
    * openssl-1.0.2l
    */
}