#include "stdafx.h"
#include "HardwareKey.h"

namespace lickey
{
	HardwareKey::HardwareKey()
	{
	}


	HardwareKey::HardwareKey(const HardwareKey& obj)
		: key(obj.key)
	{
	}


	HardwareKey::HardwareKey(const std::string& obj)
		: key(obj)
	{
	}


	HardwareKey::~HardwareKey()
	{
	}


	HardwareKey& HardwareKey::operator=(const HardwareKey& obj)
	{
		key = obj.key;
		return *this;
	}


	HardwareKey& HardwareKey::operator=(const std::string& obj)
	{
		key = obj;
		return *this;
	}
}
