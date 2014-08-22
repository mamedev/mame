/*
 * Copyright 2010-2013 Branimir Karadzic. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
 */

#ifndef BX_MAPUTIL_H_HEADER_GUARD
#define BX_MAPUTIL_H_HEADER_GUARD

#include "bx.h"

namespace bx
{
	template<typename MapType>
	typename MapType::iterator mapInsertOrUpdate(MapType& _map, const typename MapType::key_type& _key, const typename MapType::mapped_type& _value)
	{
		typename MapType::iterator it = _map.lower_bound(_key);
		if (it != _map.end()
		&&  !_map.key_comp()(_key, it->first) )
		{
			it->second = _value;
			return it;
		}

		typename MapType::value_type pair(_key, _value);
		return _map.insert(it, pair);
	}
} // namespace bx

#endif // BX_MAPUTIL_H_HEADER_GUARD
