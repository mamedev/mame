// license:BSD-3-Clause
// copyright-holders:Couriersud

#include "pmain.h"

namespace plib {

	app::app()
	: std_out(&std::cout)
	, std_err(&std::cerr)
	{

	}

	int app::main_utfX(const std::vector<putf8string> &argv)
	{
		auto r = this->parse(argv);

		if (r != argv.size())
		{
			this->std_err("Error parsing {}\n", argv[r]);
			this->std_err(this->usage_short());
			return 1;
		}

		return this->execute();
	}

	int app::main_utfX(int argc, char *argv[])
	{
		std::vector<putf8string> arg;
		for (std::size_t i = 0; i < narrow_cast<std::size_t>(argc); i++)
			arg.emplace_back(putf8string(argv[i]));

		return main_utfX(arg);
	}

	int app::main_utfX(int argc, wchar_t *argv[])
	{
		std::vector<putf8string> arg;
		for (std::size_t i = 0; i < narrow_cast<std::size_t>(argc); i++)
			arg.emplace_back(putf8string(pwstring(argv[i])));

		return main_utfX(arg);
	}

} // namespace plib
