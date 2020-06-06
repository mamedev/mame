// license:GPL-2.0+
// copyright-holders:Couriersud

#include "pmain.h"

namespace plib {

	app::app()
	: pout(&std::cout)
	, perr(&std::cerr)
	{

	}

	int app::main_utfX(const std::vector<putf8string> &argv)
	{
		auto r = this->parse(argv);

		if (r != argv.size())
		{
			this->perr("Error parsing {}\n", argv[r]);
			this->perr(this->usage_short());
			return 1;
		}

		return this->execute();
	}

	int app::main_utfX(int argc, char *argv[])
	{
		std::vector<putf8string> arg;
		for (std::size_t i = 0; i < static_cast<std::size_t>(argc); i++)
			arg.push_back(putf8string(argv[i]));

		return main_utfX(arg);
	}

	int app::main_utfX(int argc, wchar_t *argv[])
	{
		std::vector<putf8string> arg;
		for (std::size_t i = 0; i < static_cast<std::size_t>(argc); i++)
			arg.push_back(putf8string(pwstring(argv[i])));

		return main_utfX(arg);
	}

} // namespace plib
