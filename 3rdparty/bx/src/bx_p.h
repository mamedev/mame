/*
 * Copyright 2010-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_P_H_HEADER_GUARD
#define BX_P_H_HEADER_GUARD

#ifndef BX_CONFIG_DEBUG
#	define BX_CONFIG_DEBUG 0
#endif // BX_CONFIG_DEBUG

#if BX_CONFIG_DEBUG
#	define BX_TRACE _BX_TRACE
#	define BX_WARN  _BX_WARN
#	define BX_CHECK _BX_CHECK
#	define BX_CONFIG_ALLOCATOR_DEBUG 1
#endif // BX_CONFIG_DEBUG

#define _BX_TRACE(_format, ...)                                                                       \
				BX_MACRO_BLOCK_BEGIN                                                                  \
					bx::debugPrintf(__FILE__ "(" BX_STRINGIZE(__LINE__) "): BX " _format "\n", ##__VA_ARGS__); \
				BX_MACRO_BLOCK_END

#define _BX_WARN(_condition, _format, ...)                        \
				BX_MACRO_BLOCK_BEGIN                              \
					if (!BX_IGNORE_C4127(_condition) )            \
					{                                             \
						BX_TRACE("WARN " _format, ##__VA_ARGS__); \
					}                                             \
				BX_MACRO_BLOCK_END

#define _BX_CHECK(_condition, _format, ...)                        \
				BX_MACRO_BLOCK_BEGIN                               \
					if (!BX_IGNORE_C4127(_condition) )             \
					{                                              \
						BX_TRACE("CHECK " _format, ##__VA_ARGS__); \
						bx::debugBreak();                          \
					}                                              \
				BX_MACRO_BLOCK_END

#include <bx/bx.h>
#include <bx/debug.h>

#endif // BX_P_H_HEADER_GUARD
