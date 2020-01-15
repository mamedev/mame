/*
 * Copyright 2010-2020 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
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

	template<typename MapType>
	bool mapRemove(MapType& _map, const typename MapType::value_type::first_type& _first)
	{
		typename MapType::const_iterator it = _map.find(_first);
		if (it != _map.end() )
		{
			_map.erase(it);
			return true;
		}

		return false;
	}

	template<typename MapType>
	bool mapRemove(MapType& _map, const typename MapType::value_type::second_type& _second)
	{
		for (typename MapType::const_iterator it = _map.begin(), itEnd = _map.end(); it != itEnd; ++it)
		{
			if (it->second == _second)
			{
				_map.erase(it);
				return true;
			}
		}

		return false;
	}

} // namespace bx

#endif // BX_MAPUTIL_H_HEADER_GUARD
