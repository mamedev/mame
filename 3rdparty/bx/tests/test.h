/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#ifndef BX_TEST_H_HEADER_GUARD
#define BX_TEST_H_HEADER_GUARD

#include <bx/bx.h>

BX_PRAGMA_DIAGNOSTIC_PUSH();
BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4312); // warning C4312 : 'reinterpret_cast' : conversion from 'int' to 'const char *' of greater size
#include <catch/catch_amalgamated.hpp>
BX_PRAGMA_DIAGNOSTIC_POP();

#define TEST(_x) TEST_CASE(#_x, "")

#if BX_CONFIG_DEBUG
#	define REQUIRE_ASSERTS(_x) REQUIRE_THROWS(_x)
#else
#	define REQUIRE_ASSERTS(_x) BX_UNUSED(_x)
#endif // BX_CONFIG_DEBUG

#include "dbg.h"

#endif // BX_TEST_H_HEADER_GUARD
