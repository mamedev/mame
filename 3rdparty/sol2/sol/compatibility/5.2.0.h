// The MIT License (MIT) 

// Copyright (c) 2013-2016 Rapptz, ThePhD and contributors

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef SOL_5_2_0_H
#define SOL_5_2_0_H
#include "version.hpp"

#if SOL_LUA_VERSION < 503

inline int lua_isinteger(lua_State* L, int idx) {
	if (lua_type(L, idx) != LUA_TNUMBER)
		return 0;
	// This is a very slipshod way to do the testing
	// but lua_totingerx doesn't play ball nicely
	// on older versions...
	lua_Number n = lua_tonumber(L, idx);
	lua_Integer i = lua_tointeger(L, idx);
	if (i != n)
		return 0;
	// it's DEFINITELY an integer
	return 1;
}

#endif // SOL_LUA_VERSION == 502
#endif // SOL_5_2_0_H
