#pragma once

namespace lickey
{
	class Version
	{
		char version;
	public:
		static Version& GetInstance();
		virtual ~Version();

		unsigned int Value() const
		{
			return version;
		};

	private:
		Version();
	};


#define VERSION() Version::GetInstance().Value()
}
