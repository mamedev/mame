// license:GPL-2.0+
// copyright-holders:Couriersud

#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <initializer_list>

#include "plib/putil.h"
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
}
