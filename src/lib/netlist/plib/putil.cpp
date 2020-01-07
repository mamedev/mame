// license:GPL-2.0+
// copyright-holders:Couriersud

#include "putil.h"
#include "plists.h"
#include "pstrutil.h"
#include "ptypes.h"

#include <algorithm>
#include <cstdlib> // needed for getenv ...
#include <initializer_list>

namespace plib
{
	namespace util
	{
		#ifdef _WIN32
		static constexpr const char PATH_SEP = '\\';
		#else
		static constexpr const char PATH_SEP = '/';
		#endif

		pstring basename(const pstring &filename)
		{
			auto p=find_last_of(filename, pstring(1, PATH_SEP));
			if (p == pstring::npos)
				return filename;
			else
				return filename.substr(p+1);
		}

		pstring path(const pstring &filename)
		{
			auto p=find_last_of(filename, pstring(1, PATH_SEP));
			if (p == pstring::npos)
				return "";
			else if (p == 0) // root case
				return filename.substr(0, 1);
			else
				return filename.substr(0, p);
		}

		pstring buildpath(std::initializer_list<pstring> list )
		{
			pstring ret = "";
			for( const auto &elem : list )
			{
				if (ret == "")
					ret = elem;
				else
					ret += (PATH_SEP + elem);
			}
			return ret;
		}

		pstring environment(const pstring &var, const pstring &default_val)
		{
			if (std::getenv(var.c_str()) == nullptr)
				return default_val;
			else
				return pstring(std::getenv(var.c_str()));
		}
	} // namespace util

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

	std::vector<std::string> psplit_r(const std::string &stri,
			const std::string &token,
			const std::size_t maxsplit)
	{
		std::string str(stri);
		std::vector<std::string> result;
		std::size_t splits = 0;

		while(str.size())
		{
			std::size_t index = str.rfind(token);
			bool found = index!=std::string::npos;
			if (found)
				splits++;
			if ((splits <= maxsplit || maxsplit == 0) && found)
			{
				result.push_back(str.substr(index+token.size()));
				str = str.substr(0, index);
				if (str.size()==0)
					result.push_back(str);
			}
			else
			{
				result.push_back(str);
				str = "";
			}
		}
		return result;
	}

	std::vector<pstring> psplit(const pstring &str, const std::vector<pstring> &onstrl)
	{
		pstring col = "";
		std::vector<pstring> ret;

		auto i = str.begin();
		while (i != str.end())
		{
			auto p = static_cast<std::size_t>(-1);
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
				pstring::value_type c = *i;
				col += c;
				i++;
			}
		}
		if (col != "")
			ret.push_back(col);

		return ret;
	}


	int penum_base::from_string_int(const pstring &str, const pstring &x)
	{
		int cnt = 0;
		for (auto &s : psplit(str, ",", false))
		{
			if (trim(s) == x)
				return cnt;
			cnt++;
		}
		return -1;
	}

	std::string penum_base::nthstr(int n, const pstring &str)
	{
		return psplit(str, ",", false)[static_cast<std::size_t>(n)];
	}
} // namespace plib
