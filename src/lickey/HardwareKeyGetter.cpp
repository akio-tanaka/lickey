#include "stdafx.h"
#include "HardwareKeyGetter.h"
#include <boost/scoped_ptr.hpp>
#include <winsock2.h>
#include <iphlpapi.h>
#include <iomanip>
#pragma comment(lib, "IPHLPAPI.lib")
// https://msdn.microsoft.com/en-us/library/windows/desktop/aa365917(v=vs.85).aspx for mac address


namespace lickey
{
    HardwareKeyGetter::HardwareKeyGetter()
    {}


    HardwareKeyGetter::~HardwareKeyGetter()
    {}


    HardwareKeys HardwareKeyGetter::operator()() const
    {
        HardwareKeys keys;
        ULONG outbufferLength = sizeof(IP_ADAPTER_INFO);
        PIP_ADAPTER_INFO adapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));

        if (ERROR_BUFFER_OVERFLOW == GetAdaptersInfo(adapterInfo, &outbufferLength))
        {
            free(adapterInfo);
            adapterInfo = (IP_ADAPTER_INFO *)malloc(outbufferLength);
            if (!adapterInfo)
            {
                return keys;
            }
        }

        DWORD dwRetVal;
        if (NO_ERROR != (dwRetVal = GetAdaptersInfo(adapterInfo, &outbufferLength)))
        {
            return keys;
        }
        boost::scoped_ptr<IP_ADAPTER_INFO> sptrAdapterInfo(adapterInfo);

        PIP_ADAPTER_INFO pAdapter = adapterInfo;
        int indexCount = 0;
        while (pAdapter)
        {
            std::stringstream physicalAddress;
            physicalAddress << std::hex << std::uppercase;
            for (UINT i = 0; i < pAdapter->AddressLength; i++)
            {
                physicalAddress << std::setw(2) << std::setfill('0');   // every loop needs this statement for all tokens
                if (i == (pAdapter->AddressLength - 1))
                {
                    physicalAddress << (int)pAdapter->Address[i];
                }
                else
                {
                    physicalAddress << (int)pAdapter->Address[i] << "-";
                }
            }
            HardwareKey key;
            key.key = physicalAddress.str();
            keys.push_back(key);

            pAdapter = pAdapter->Next;
            ++indexCount;
        }
        return keys;
    }
}