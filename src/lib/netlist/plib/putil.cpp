// license:GPL-2.0+
// copyright-holders:Couriersud

#include "putil.h"
#include "ptypes.h"
#include "plists.h"

#include <cstdlib>
#include <algorithm>
#include <initializer_list>
#include <cstring>

namespace plib
{
	namespace util
	{
		const pstring buildpath(std::initializer_list<pstring> list )
		{
			pstring ret = "";
			for( auto elem : list )
			{
				if (ret == "")
					ret = elem;
				else
					#ifdef _WIN32
					ret = ret + '\\' + elem;
					#else
					ret = ret + '/' + elem;
					#endif
			}
			return ret;
		}

		const pstring environment(const pstring &var, const pstring &default_val)
		{
			if (getenv(var.c_str()) == nullptr)
				return default_val;
			else
				return pstring(getenv(var.c_str()), pstring::UTF8);
		}
	}

	std::vector<pstring> psplit(const pstring &str, const pstring &onstr, bool ignore_empty)
	{
		std::vector<pstring> ret;

		pstring::size_type p = 0;
		pstring::size_type pn = str.find(onstr, p);

		while (pn != pstring::npos)
		{
			pstring t = str.substr(p, pn - p);
			if (!ignore_empty || t.length() != 0)
				ret.push_back(t);
			p = pn + onstr.length();
			pn = str.find(onstr, p);
		}
		if (p < str.length())
		{
			pstring t = str.substr(p);
			if (!ignore_empty || t.length() != 0)
				ret.push_back(t);
		}
		return ret;
	}

	std::vector<pstring> psplit(const pstring &str, const std::vector<pstring> &onstrl)
	{
		pstring col = "";
		std::vector<pstring> ret;

		auto i = str.begin();
		while (i != str.end())
		{
			std::size_t p = static_cast<std::size_t>(-1);
			for (std::size_t j=0; j < onstrl.size(); j++)
			{
				if (std::equal(onstrl[j].begin(), onstrl[j].end(), i))
				{
					p = j;
					break;
				}
			}
			if (p != static_cast<std::size_t>(-1))
			{
				if (col != "")
					ret.push_back(col);

				col = "";
				ret.push_back(onstrl[p]);
				i = std::next(i, static_cast<pstring::difference_type>(onstrl[p].length()));
			}
			else
			{
				pstring::code_t c = *i;
				col += c;
				i++;
			}
		}
		if (col != "")
			ret.push_back(col);

		return ret;
	}


	int penum_base::from_string_int(const char *str, const char *x)
	{
		int cnt = 0;
		const char *cur = str;
		std::size_t lx = strlen(x);
		while (*str)
		{
			if (*str == ',')
			{
				std::ptrdiff_t l = str-cur;
				if (static_cast<std::size_t>(l) == lx)
					if (strncmp(cur, x, lx) == 0)
						return cnt;
			}
			else if (*str == ' ')
			{
				cur = str + 1;
				cnt++;
			}
			str++;
		}
		std::ptrdiff_t l = str-cur;
		if (static_cast<std::size_t>(l) == lx)
			if (strncmp(cur, x, lx) == 0)
				return cnt;
		return -1;
	}
	pstring penum_base::nthstr(int n, const char *str)
	{
		char buf[64];
		char *bufp = buf;
		int cur = 0;
		while (*str)
		{
			if (cur == n)
			{
				if (*str == ',')
				{
					*bufp = 0;
					return pstring(buf, pstring::UTF8);
				}
				else if (*str != ' ')
					*bufp++ = *str;
			}
			else
			{
				if (*str == ',')
					cur++;
			}
			str++;
		}
		*bufp = 0;
		return pstring(buf, pstring::UTF8);
	}
} // namespace plib
