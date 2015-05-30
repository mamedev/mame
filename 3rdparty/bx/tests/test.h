/*
 * Copyright 2010-2015 Branimir Karadzic. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
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
