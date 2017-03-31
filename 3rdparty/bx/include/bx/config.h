/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_CONFIG_H_HEADER_GUARD
#define BX_CONFIG_H_HEADER_GUARD

#include "bx.h"

#ifndef BX_CONFIG_ALLOCATOR_DEBUG
#	define BX_CONFIG_ALLOCATOR_DEBUG 0
#endif // BX_CONFIG_DEBUG_ALLOC

#ifndef BX_CONFIG_ALLOCATOR_CRT
#	define BX_CONFIG_ALLOCATOR_CRT 1
#endif // BX_CONFIG_ALLOCATOR_CRT

#ifndef BX_CONFIG_CRT_FILE_READER_WRITER
#	define BX_CONFIG_CRT_FILE_READER_WRITER !(0 \
			|| BX_PLATFORM_NACL                 \
			|| BX_CRT_NONE                      \
			)
#endif // BX_CONFIG_CRT_FILE_READER_WRITER

#ifndef BX_CONFIG_CRT_PROCESS
#	define BX_CONFIG_CRT_PROCESS !(0  \
			|| BX_CRT_NONE            \
			|| BX_PLATFORM_EMSCRIPTEN \
			|| BX_PLATFORM_NACL       \
			|| BX_PLATFORM_PS4        \
			|| BX_PLATFORM_WINRT      \
			|| BX_PLATFORM_XBOXONE    \
			)
#endif // BX_CONFIG_CRT_PROCESS

#ifndef BX_CONFIG_SUPPORTS_THREADING
#	define BX_CONFIG_SUPPORTS_THREADING !(0 \
			|| BX_PLATFORM_EMSCRIPTEN       \
			|| BX_CRT_NONE                  \
			)
#endif // BX_CONFIG_SUPPORTS_THREADING

#endif // BX_CONFIG_H_HEADER_GUARD
