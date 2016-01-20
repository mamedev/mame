/*
 * Copyright 2010-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef __TEST_H__
#define __TEST_H__

#include <bx/bx.h>
#include <UnitTest++.h>
#include "dbg.h"

#if !BX_COMPILER_MSVC
#	define _strdup strdup
#endif // !BX_COMPILER_MSVC

#endif // __TEST_H__
