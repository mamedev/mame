// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pmain.cpp
 *
 */

#include "pmain.h"

#ifdef _WIN32
#include <windows.h>
#include <string.h>
#include <tchar.h>
#endif

namespace plib {

	#ifdef _WIN32
	static pstring toutf8(const wchar_t *w)
	{
		auto wlen = wcslen(w);
		int dst_char_count = WideCharToMultiByte(CP_UTF8, 0, w, wlen, nullptr, 0, nullptr, nullptr);
		char *buf = new char[dst_char_count + 1];
		WideCharToMultiByte(CP_UTF8, 0, w, wlen, buf, dst_char_count, nullptr, nullptr);
		buf[dst_char_count] = 0;
		auto ret = pstring(buf);
		delete [] buf;
		return ret;
	}
	#endif

/***************************************************************************
    Application
***************************************************************************/

	app::app()
	: options()
	, pout_strm()
	, perr_strm()
	, pout(&pout_strm)
	, perr(&perr_strm)
	{

	}

	int app::main_utfX(int argc, char **argv)
	{
		auto r = this->parse(argc, argv);
		int ret = 0;

		if (r != argc)
		{
			this->perr("Error parsing {}\n", argv[r]);
			//FIXME: usage_short
			this->perr(this->usage());
			ret = 1;
		}
		else
			ret = this->execute();

		return ret;
	}

#ifdef _WIN32
	int app::main_utfX(int argc, wchar_t *argv[])
	{
		std::vector<pstring> argv_vectors(argc);
		std::vector<char *> utf8_argv(argc);

		// convert arguments to UTF-8
		for (int i = 0; i < argc; i++)
		{
			argv_vectors[i] = toutf8(argv[i]);
			utf8_argv[i] = const_cast<char *>(argv_vectors[i].c_str());
		}

		// run utf8_main
		return main_utfX(argc, utf8_argv.data());
	}
#endif

} // namespace plib
