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
	namespace container
	{
		template <class C>
		const bool contains(C &con, const typename C::value_type &elem)
		{
			return std::find(con.begin(), con.end(), elem) != con.end();
		}

		template <class C>
		int indexof(C &con, const typename C::value_type &elem)
		{
			auto it = std::find(con.begin(), con.end(), elem);
			if (it != con.end())
				return it - con.begin();
			return -1;
		}

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
