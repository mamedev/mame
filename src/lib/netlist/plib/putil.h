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
<<<<<<< HEAD
		bool contains(C &con, const typename C::value_type &elem)
=======
		const bool contains(C &con, const typename C::value_type &elem)
>>>>>>> branch 'netlist_dev' of https://github.com/mamedev/mame.git
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

		template <class C>
		void insert_at(C &con, const std::size_t index, const typename C::value_type &elem)
		{
			con.insert(con.begin() + index, elem);
		}

	}

	template <class C>
	struct indexed_compare
	{
	    indexed_compare(const C& target): m_target(target) {}

	    bool operator()(int a, int b) const { return m_target[a] < m_target[b]; }

	    const C& m_target;
	};

	// ----------------------------------------------------------------------------------------
	// string list
	// ----------------------------------------------------------------------------------------

	class pstring_vector_t : public std::vector<pstring>
	{
	public:
		pstring_vector_t() : std::vector<pstring>() { }
		pstring_vector_t(const pstring &str, const pstring &onstr, bool ignore_empty = false);
		pstring_vector_t(const pstring &str, const pstring_vector_t &onstrl);
	};

}

#endif /* P_UTIL_H_ */
