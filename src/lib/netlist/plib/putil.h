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

	// ----------------------------------------------------------------------------------------
	// string list
	// ----------------------------------------------------------------------------------------

	class pstring_vector_t : public pvector_t<pstring>
	{
	public:
		pstring_vector_t() : pvector_t<pstring>() { }
		pstring_vector_t(const pstring &str, const pstring &onstr, bool ignore_empty = false);
		pstring_vector_t(const pstring &str, const pstring_vector_t &onstrl);
	};

}

#endif /* P_UTIL_H_ */
