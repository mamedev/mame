// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * putil.h
 *
 */

#ifndef P_UTIL_H_
#define P_UTIL_H_

#include <initializer_list>

#include "plib/pstring.h"
#include "plib/plists.h"

namespace plib
{
	namespace util
	{
		const pstring buildpath(std::initializer_list<pstring> list );
		const pstring environment(const pstring &var, const pstring &default_val = "");
	}
}

#endif /* P_UTIL_H_ */
