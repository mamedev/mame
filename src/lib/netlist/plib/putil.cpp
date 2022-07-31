// license:BSD-3-Clause
// copyright-holders:Couriersud

#include "putil.h"
#include "penum.h"
#include "pstream.h"
#include "pstrutil.h"
#include "ptypes.h"

#include <algorithm>
// needed for getenv ...
#include <initializer_list>

namespace plib
{
	namespace util
	{
		static constexpr const char PATH_SEP = compile_info::win32::value ? '\\' : '/';
		static constexpr const char *PATH_SEPS = compile_info::win32::value ? "\\/" :"/";

		pstring basename(const pstring &filename, const pstring &suffix)
		{
			auto p=find_last_of(filename, pstring(PATH_SEPS));
			pstring ret = (p == pstring::npos) ? filename : filename.substr(p+1);
			if (!suffix.empty() && endsWith(ret, suffix))
				return ret.substr(0, ret.length() - suffix.length());
			return ret;
		}

		pstring path(const pstring &filename)
		{
			auto p=find_last_of(filename, pstring(1, PATH_SEP));
			if (p == pstring::npos)
				return "";
			if (p == 0) // root case
				return filename.substr(0, 1);

			return filename.substr(0, p);
		}

		bool exists (const pstring &filename)
		{
			plib::ifstream f(filename);
			return f.good();
		}

		pstring build_path(std::initializer_list<pstring> list )
		{
			pstring ret = "";
			for( const auto &elem : list )
			{
				if (ret.empty())
					ret = elem;
				else
					ret += (PATH_SEP + elem);
			}
			return ret;
		}

		pstring environment(const pstring &var, const pstring &default_val)
		{
			putf8string varu8(var);
			return (std::getenv(varu8.c_str()) == nullptr) ? default_val
				: pstring(std::getenv(varu8.c_str()));
		}
	} // namespace util

	int penum_base::from_string_int(const pstring &str, const pstring &x)
	{
		int cnt = 0;
		for (auto &s : psplit(str, ',', false))
		{
			if (trim(s) == x)
				return cnt;
			cnt++;
		}
		return -1;
	}

	pstring penum_base::nthstr(std::size_t n, const pstring &str)
	{
		return psplit(str, ',', false)[n];
	}
} // namespace plib
