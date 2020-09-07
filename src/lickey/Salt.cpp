#include "stdafx.h"
#include "Salt.h"


namespace lickey
{
	Salt::Salt()
	{
	}


	Salt::Salt(const Salt& obj)
		: salt(obj.salt)
	{
	}


	Salt::~Salt()
	{
	}

	Salt& Salt::operator=(const Salt& obj)
	{
		salt = obj.salt;
		return *this;
	}


	Salt& Salt::operator=(const std::string& obj)
	{
		salt = obj;
		return *this;
	}
}
