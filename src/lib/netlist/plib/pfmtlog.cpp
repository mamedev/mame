// license:BSD-3-Clause
// copyright-holders:Couriersud

#include "pfmtlog.h"
#include "palloc.h"
#include "pstonum.h"
#include "pstrutil.h"

#include <algorithm>
#include <array>
#include <iomanip>
#include <iostream>

namespace plib {

#if 0
struct ptemporary_locale
{
	ptemporary_locale(std::locale tlocale)
	: new_locale(tlocale), old_clocale(std::setlocale(LC_ALL, nullptr))
	{
		if (old_locale != tlocale)
			std::locale::global(tlocale);
		if (old_clocale != tlocale.name().c_str())
			std::setlocale(LC_ALL, tlocale.name().c_str());
	}

	~ptemporary_locale()
	{
		if (old_clocale != new_locale.name().c_str())
			std::setlocale(LC_ALL, old_clocale.c_str());
		if (old_locale != new_locale)
			std::locale::global(old_locale);
	}
private:
	std::locale new_locale;
	std::locale old_locale;
	pstring old_clocale;
};
#endif

pfmt::rtype pfmt::set_format(std::stringstream &strm, char32_t cfmt_spec)
{
	pstring fmt;
	pstring search("{");
	search += plib::to_string(m_arg);

	rtype r;

	r.sl = search.length();
	r.p = m_str.find(search + ':');
	r.sl++; // ":"
	if (r.p == pstring::npos) // no further specifiers
	{
		r.p = m_str.find(search + '}');
		if (r.p == pstring::npos) // not found try default
		{
			r.sl = 2;
			r.p = m_str.find("{}");
		}
		else
			// found absolute positional place holder
			r.ret = 1;
		if (r.p == pstring::npos)
		{
			r.sl=2;
			r.p = m_str.find("{:");
			if (r.p != pstring:: npos)
			{
				auto p1 = m_str.find('}', r.p);
				if (p1 != pstring::npos)
				{
					r.sl = p1 - r.p + 1;
					fmt += m_str.substr(r.p+2, p1 - r.p - 2);
				}
			}
		}
	}
	else
	{
		// found absolute positional place holder
		auto p1 = m_str.find('}', r.p);
		if (p1 != pstring::npos)
		{
			r.sl = p1 - r.p + 1;
			fmt += ((m_arg>=10) ? m_str.substr(r.p+4, p1 - r.p - 4) : m_str.substr(r.p+3, p1 - r.p - 3));
			r.ret = 1;
		}
	}
	if (r.p != pstring::npos)
	{
		// a.b format here ...
		char32_t pend(0);
		int width(0);
		if (!fmt.empty() && pstring("duxofge").find(static_cast<pstring::value_type>(cfmt_spec)) != pstring::npos)
		{
			//pend = static_cast<char32_t>(fmt.at(fmt.size() - 1));
			pend = plib::right(fmt, 1).at(0);
			if (pstring("duxofge").find(static_cast<pstring::value_type>(pend)) == pstring::npos)
				pend = cfmt_spec;
			else
				fmt = plib::left(fmt, fmt.length() - 1);
		}
		else
			// FIXME: Error
			pend = cfmt_spec;

		auto pdot(fmt.find('.'));

		if (pdot==0)
			strm << std::setprecision(pstonum_ne_def<int>(fmt.substr(1), 6));
		else if (pdot != pstring::npos)
		{
			strm << std::setprecision(pstonum_ne_def<int>(fmt.substr(pdot + 1), 6));
			width = pstonum_ne_def<int>(left(fmt,pdot), 0);
		}
		else if (!fmt.empty())
			width = pstonum_ne_def<int>(fmt, 0);

		auto aw(plib::abs(width));

		strm << std::setw(aw);
		if (width < 0)
			strm << std::left;

		switch (pend)
		{
			case 'x':
				strm << std::hex;
				break;
			case 'o':
				strm << std::oct;
				break;
			case 'f':
				strm << std::fixed;
				break;
			case 'e':
				strm << std::scientific;
				break;
			default:
				break;
		}
	}
	else
		r.ret = -1;
	return r;

}

} // namespace plib
