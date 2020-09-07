#pragma once
#include "HardwareKey.h"

namespace lickey
{
	class HardwareKeyGetter
	{
	public:
		HardwareKeyGetter();
		virtual ~HardwareKeyGetter();

		HardwareKeys operator()() const;
	};
}
