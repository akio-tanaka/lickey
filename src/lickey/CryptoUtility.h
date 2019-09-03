#pragma once
#include "Salt.h"

namespace lickey
{
    void InitializeOpenSSL();


    bool Encrypt(
        const char* data,
        const size_t datalen,
        const unsigned char* key,
        const unsigned char* iv,
        unsigned char* dest,
        // const size_t destlen);
        size_t& destlen);


    bool Decrypt(
        const unsigned char* data,
        const size_t datalen,
        const unsigned char* key,
        const unsigned char* iv,
        unsigned char* dest,
        // const size_t destlen);
        size_t& destlen);


    bool MD5(
        const char* data,
        const size_t datalen,
        unsigned char hash[16]);


    bool SHA256(
        const char* data,
        const size_t datalen,
        unsigned char hash[32]);


    void EncodeBase64(
        const unsigned char* data,
        const int datalen,
        std::string& str);


    void EncodeBase64(
        const std::string& data,
        std::string& str);


    void DecodeBase64(
        const std::string& str,
        unsigned char*& data,
        int& datalen);


    bool MakeSalt(Salt& salt);
}