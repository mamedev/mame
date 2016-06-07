// license:GPL-2.0+
// copyright-holders:Couriersud

#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <initializer_list>

#include "plib/putil.h"
#include "plib/ptypes.h"
#include "plib/plists.h"

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
					#ifdef WIN32
					ret = ret + '\\' + elem;
					#else
					ret = ret + '/' + elem;
					#endif
			}
			return ret;
		}

		const pstring environment(const pstring &var, const pstring &default_val)
		{
			if (getenv(var.cstr()) == nullptr)
				return default_val;
			else
				return pstring(getenv(var.cstr()));
		}
	}

	pstring_vector_t::pstring_vector_t(const pstring &str, const pstring &onstr, bool ignore_empty)
	: pvector_t<pstring>()
	{
		int p = 0;
		int pn;

		pn = str.find(onstr, p);
		while (pn>=0)
		{
			pstring t = str.substr(p, pn - p);
			if (!ignore_empty || t.len() != 0)
				this->push_back(t);
			p = pn + onstr.len();
			pn = str.find(onstr, p);
		}
		if (p < (int) str.len())
		{
			pstring t = str.substr(p);
			if (!ignore_empty || t.len() != 0)
				this->push_back(t);
		}
	}

	pstring_vector_t::pstring_vector_t(const pstring &str, const pstring_vector_t &onstrl)
	: pvector_t<pstring>()
	{
		pstring col = "";

		unsigned i = 0;
		while (i<str.blen())
		{
			int p = -1;
			for (std::size_t j=0; j < onstrl.size(); j++)
			{
				if (std::memcmp(onstrl[j].cstr(), &(str.cstr()[i]), onstrl[j].blen())==0)
				{
					p = j;
					break;
				}
			}
			if (p>=0)
			{
				if (col != "")
					this->push_back(col);

				col = "";
				this->push_back(onstrl[p]);
				i += onstrl[p].blen();
			}
			else
			{
				pstring::traits::code_t c = pstring::traits::code(str.cstr() + i);
				col += c;
				i+=pstring::traits::codelen(c);
			}
		}
		if (col != "")
			this->push_back(col);
	}


	int enum_base::from_string_int(const char *str, const char *x)
	{
		int cnt = 0;
		const char *cur = str;
		int lx = strlen(x);
		while (*str)
		{
			if (*str == ',')
			{
				int l = str-cur;
				if (l == lx)
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
		int l = str-cur;
		if (l == lx)
			if (strncmp(cur, x, lx) == 0)
				return cnt;
		return -1;
	}
	pstring enum_base::nthstr(int n, const char *str)
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
					return pstring(buf);
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
		return pstring(buf);
	}
} // namespace plib



