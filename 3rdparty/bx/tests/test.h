/*
 * Copyright 2010-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef __TEST_H__
#define __TEST_H__

#include <bx/bx.h>
#include <catch/catch.hpp>
#define TEST(_x) TEST_CASE(#_x, "")
#define CHECK_EQUAL(_x, _y) REQUIRE(_x == _y)

#include "dbg.h"

#if !BX_COMPILER_MSVC
#	define _strdup strdup
#endif // !BX_COMPILER_MSVC

#endif // __TEST_H__
