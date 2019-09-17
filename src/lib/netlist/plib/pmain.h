// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * poptions.h
 *
 */

#pragma once

#ifndef PMAIN_H_
#define PMAIN_H_

#include "palloc.h"
#include "poptions.h"
#include "pstream.h"
#include "pstring.h"
#include "putil.h"

#include <memory>

#ifdef _WIN32
#include <cwchar>
#define PMAIN(appclass) \
extern "C" int wmain(int argc, wchar_t *argv[]) { return plib::app::mainrun<appclass, wchar_t>(argc, argv); }
#else
#define PMAIN(appclass) \
int main(int argc, char **argv) { return plib::app::mainrun<appclass, char>(argc, argv); }
#endif


namespace plib {
/***************************************************************************
    Application
***************************************************************************/

	class app : public options
	{
	public:
		app();

		COPYASSIGNMOVE(app, delete)

		virtual ~app() = default;

		virtual pstring usage() = 0;
		virtual int execute() = 0;

		plib::pstdout pout_strm;
		plib::pstderr perr_strm;

		plib::putf8_fmt_writer pout;
		plib::putf8_fmt_writer perr;

		template <class C, typename T>
		static int mainrun(int argc, T **argv)
		{
			auto a = plib::make_unique<C>();
			return a->main_utfX(argc, argv);
		}

	private:
		int main_utfX(int argc, char **argv);
#ifdef _WIN32
		int main_utfX(int argc, wchar_t *argv[]);
#endif

	};

} // namespace plib



#endif /* PMAIN_H_ */
