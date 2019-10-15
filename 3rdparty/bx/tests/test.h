/*
 * Copyright 2010-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef __TEST_H__
#define __TEST_H__

#include <bx/bx.h>

BX_PRAGMA_DIAGNOSTIC_PUSH();
BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4312); // warning C4312 : 'reinterpret_cast' : conversion from 'int' to 'const char *' of greater size
#include <catch/catch.hpp>
BX_PRAGMA_DIAGNOSTIC_POP();

#define TEST(_x) TEST_CASE(#_x, "")
#define CHECK_EQUAL(_x, _y) REQUIRE(_x == _y)

#include "dbg.h"

#endif // __TEST_H__
