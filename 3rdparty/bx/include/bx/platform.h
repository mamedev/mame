/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#ifndef BX_PLATFORM_H_HEADER_GUARD
#define BX_PLATFORM_H_HEADER_GUARD

// Architecture
#define BX_ARCH_32BIT 0
#define BX_ARCH_64BIT 0

// Compiler
#define BX_COMPILER_CLANG          0
#define BX_COMPILER_CLANG_ANALYZER 0
#define BX_COMPILER_GCC            0
#define BX_COMPILER_MSVC           0

// Endianness
#define BX_CPU_ENDIAN_BIG    0
#define BX_CPU_ENDIAN_LITTLE 0

// CPU
#define BX_CPU_ARM   0
#define BX_CPU_JIT   0
#define BX_CPU_MIPS  0
#define BX_CPU_PPC   0
#define BX_CPU_RISCV 0
#define BX_CPU_X86   0

// C Runtime
#define BX_CRT_BIONIC 0
#define BX_CRT_GLIBC  0
#define BX_CRT_LIBCXX 0
#define BX_CRT_MINGW  0
#define BX_CRT_MSVC   0
#define BX_CRT_NEWLIB 0

#ifndef BX_CRT_NONE
#	define BX_CRT_NONE 0
#endif // BX_CRT_NONE

// Language standard version
#define BX_LANGUAGE_CPP17 201703L
#define BX_LANGUAGE_CPP20 202002L
#define BX_LANGUAGE_CPP23 202207L

// Platform
#define BX_PLATFORM_ANDROID    0
#define BX_PLATFORM_BSD        0
#define BX_PLATFORM_EMSCRIPTEN 0
#define BX_PLATFORM_HAIKU      0
#define BX_PLATFORM_HURD       0
#define BX_PLATFORM_IOS        0
#define BX_PLATFORM_LINUX      0
#define BX_PLATFORM_NX         0
#define BX_PLATFORM_OSX        0
#define BX_PLATFORM_PS4        0
#define BX_PLATFORM_PS5        0
#define BX_PLATFORM_RPI        0
#define BX_PLATFORM_VISIONOS   0
#define BX_PLATFORM_WINDOWS    0
#define BX_PLATFORM_WINRT      0
#define BX_PLATFORM_XBOXONE    0

// http://sourceforge.net/apps/mediawiki/predef/index.php?title=Compilers
#if defined(__clang__)
// clang defines __GNUC__ or _MSC_VER
#	undef  BX_COMPILER_CLANG
#	define BX_COMPILER_CLANG (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#	if defined(__clang_analyzer__)
#		undef  BX_COMPILER_CLANG_ANALYZER
#		define BX_COMPILER_CLANG_ANALYZER 1
#	endif // defined(__clang_analyzer__)
#elif defined(_MSC_VER)
#	undef  BX_COMPILER_MSVC
#	define BX_COMPILER_MSVC _MSC_VER
#elif defined(__GNUC__)
#	undef  BX_COMPILER_GCC
#	define BX_COMPILER_GCC (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#else
#	error "BX_COMPILER_* is not defined!"
#endif //

// http://sourceforge.net/apps/mediawiki/predef/index.php?title=Architectures
#if defined(__arm__)     \
 || defined(__aarch64__) \
 || defined(_M_ARM)
#	undef  BX_CPU_ARM
#	define BX_CPU_ARM 1
#	define BX_CACHE_LINE_SIZE 64
#elif defined(__MIPSEL__)     \
 ||   defined(__mips_isa_rev) \
 ||   defined(__mips64)
#	undef  BX_CPU_MIPS
#	define BX_CPU_MIPS 1
#	define BX_CACHE_LINE_SIZE 64
#elif defined(_M_PPC)        \
 ||   defined(__powerpc__)   \
 ||   defined(__powerpc64__)
#	undef  BX_CPU_PPC
#	define BX_CPU_PPC 1
#	define BX_CACHE_LINE_SIZE 128
#elif defined(__riscv)   \
 ||   defined(__riscv__) \
 ||   defined(RISCVEL)
#	undef  BX_CPU_RISCV
#	define BX_CPU_RISCV 1
#	define BX_CACHE_LINE_SIZE 64
#elif defined(_M_IX86)    \
 ||   defined(_M_X64)     \
 ||   defined(__i386__)   \
 ||   defined(__x86_64__)
#	undef  BX_CPU_X86
#	define BX_CPU_X86 1
#	define BX_CACHE_LINE_SIZE 64
#else // PNaCl doesn't have CPU defined.
#	undef  BX_CPU_JIT
#	define BX_CPU_JIT 1
#	define BX_CACHE_LINE_SIZE 64
#endif //

#if defined(__x86_64__)    \
 || defined(_M_X64)        \
 || defined(__aarch64__)   \
 || defined(__64BIT__)     \
 || defined(__mips64)      \
 || defined(__powerpc64__) \
 || defined(__ppc64__)     \
 || defined(__LP64__)
#	undef  BX_ARCH_64BIT
#	define BX_ARCH_64BIT 64
#else
#	undef  BX_ARCH_32BIT
#	define BX_ARCH_32BIT 32
#endif //

#if BX_CPU_PPC
// __BIG_ENDIAN__ is gcc predefined macro
#	if defined(__BIG_ENDIAN__)
#		undef  BX_CPU_ENDIAN_BIG
#		define BX_CPU_ENDIAN_BIG 1
#	else
#		undef  BX_CPU_ENDIAN_LITTLE
#		define BX_CPU_ENDIAN_LITTLE 1
#	endif
#else
#	undef  BX_CPU_ENDIAN_LITTLE
#	define BX_CPU_ENDIAN_LITTLE 1
#endif // BX_CPU_PPC

// http://sourceforge.net/apps/mediawiki/predef/index.php?title=Operating_Systems
#if defined(_DURANGO) || defined(_XBOX_ONE)
#	undef  BX_PLATFORM_XBOXONE
#	define BX_PLATFORM_XBOXONE 1
#elif defined(_WIN32) || defined(_WIN64)
// http://msdn.microsoft.com/en-us/library/6sehtctf.aspx
#	ifndef NOMINMAX
#		define NOMINMAX
#	endif // NOMINMAX
//  If _USING_V110_SDK71_ is defined it means we are using the v110_xp or v120_xp toolset.
#	if defined(_MSC_VER) && (_MSC_VER >= 1700) && !defined(_USING_V110_SDK71_)
#		include <winapifamily.h>
#	endif // defined(_MSC_VER) && (_MSC_VER >= 1700) && (!_USING_V110_SDK71_)
#	if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
#		undef  BX_PLATFORM_WINDOWS
#		if !defined(WINVER) && !defined(_WIN32_WINNT)
#			if BX_ARCH_64BIT
//				When building 64-bit target Win7 and above.
#				define WINVER 0x0601
#				define _WIN32_WINNT 0x0601
#			else
//				Windows Server 2003 with SP1, Windows XP with SP2 and above
#				define WINVER 0x0502
#				define _WIN32_WINNT 0x0502
#			endif // BX_ARCH_64BIT
#		endif // !defined(WINVER) && !defined(_WIN32_WINNT)
#		define BX_PLATFORM_WINDOWS _WIN32_WINNT
#	else
#		undef  BX_PLATFORM_WINRT
#		define BX_PLATFORM_WINRT 1
#	endif
#elif defined(__ANDROID__)
// Android compiler defines __linux__
#	include <sys/cdefs.h> // Defines __BIONIC__ and includes android/api-level.h
#	undef  BX_PLATFORM_ANDROID
#	define BX_PLATFORM_ANDROID __ANDROID_API__
#elif defined(__VCCOREVER__)
// RaspberryPi compiler defines __linux__
#	undef  BX_PLATFORM_RPI
#	define BX_PLATFORM_RPI 1
#elif  defined(__linux__)
#	undef  BX_PLATFORM_LINUX
#	define BX_PLATFORM_LINUX 1
#elif  defined(__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__) \
	|| defined(__ENVIRONMENT_TV_OS_VERSION_MIN_REQUIRED__)
#	undef  BX_PLATFORM_IOS
#	define BX_PLATFORM_IOS 1
#elif defined(__has_builtin) && __has_builtin(__is_target_os) && __is_target_os(xros)
#	undef  BX_PLATFORM_VISIONOS
#	define BX_PLATFORM_VISIONOS 1
#elif defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__)
#	undef  BX_PLATFORM_OSX
#	define BX_PLATFORM_OSX __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
#elif defined(__EMSCRIPTEN__)
#	include <emscripten/version.h>
#	undef  BX_PLATFORM_EMSCRIPTEN
#	define BX_PLATFORM_EMSCRIPTEN (__EMSCRIPTEN_major__ * 10000 + __EMSCRIPTEN_minor__ * 100 + __EMSCRIPTEN_tiny__)
#elif defined(__ORBIS__)
#	undef  BX_PLATFORM_PS4
#	define BX_PLATFORM_PS4 1
#elif defined(__PROSPERO__)
#	undef  BX_PLATFORM_PS5
#	define BX_PLATFORM_PS5 1
#elif  defined(__FreeBSD__)        \
	|| defined(__FreeBSD_kernel__) \
	|| defined(__NetBSD__)         \
	|| defined(__OpenBSD__)        \
	|| defined(__DragonFly__)
#	undef  BX_PLATFORM_BSD
#	define BX_PLATFORM_BSD 1
#elif defined(__GNU__)
#	undef  BX_PLATFORM_HURD
#	define BX_PLATFORM_HURD 1
#elif defined(__NX__)
#	undef  BX_PLATFORM_NX
#	define BX_PLATFORM_NX 1
#elif defined(__HAIKU__)
#	undef  BX_PLATFORM_HAIKU
#	define BX_PLATFORM_HAIKU 1
#endif //

#if !BX_CRT_NONE
// https://sourceforge.net/p/predef/wiki/Libraries/
#	if defined(__BIONIC__)
#		undef  BX_CRT_BIONIC
#		define BX_CRT_BIONIC 1
#	elif defined(__GLIBC__)
#		undef  BX_CRT_GLIBC
#		define BX_CRT_GLIBC (__GLIBC__ * 10000 + __GLIBC_MINOR__ * 100)
#	elif defined(__MINGW32__) \
	||   defined(__MINGW64__)
#		undef  BX_CRT_MINGW
#		define BX_CRT_MINGW 1
#	elif defined(_MSC_VER)
#		undef  BX_CRT_MSVC
#		define BX_CRT_MSVC 1
#	elif defined(__apple_build_version__) \
	||   BX_PLATFORM_PS4                  \
	||   BX_PLATFORM_EMSCRIPTEN           \
	||   defined(__llvm__)
#		undef  BX_CRT_LIBCXX
#		define BX_CRT_LIBCXX 1
#	endif //
#endif // !BX_CRT_NONE

///
#define BX_PLATFORM_POSIX (0   \
	||  BX_PLATFORM_ANDROID    \
	||  BX_PLATFORM_BSD        \
	||  BX_PLATFORM_EMSCRIPTEN \
	||  BX_PLATFORM_HAIKU      \
	||  BX_PLATFORM_HURD       \
	||  BX_PLATFORM_IOS        \
	||  BX_PLATFORM_LINUX      \
	||  BX_PLATFORM_NX         \
	||  BX_PLATFORM_OSX        \
	||  BX_PLATFORM_PS4        \
	||  BX_PLATFORM_PS5        \
	||  BX_PLATFORM_RPI        \
	||  BX_PLATFORM_VISIONOS   \
	)

///
#define BX_PLATFORM_NONE !(0   \
	||  BX_PLATFORM_ANDROID    \
	||  BX_PLATFORM_BSD        \
	||  BX_PLATFORM_EMSCRIPTEN \
	||  BX_PLATFORM_HAIKU      \
	||  BX_PLATFORM_HURD       \
	||  BX_PLATFORM_IOS        \
	||  BX_PLATFORM_LINUX      \
	||  BX_PLATFORM_NX         \
	||  BX_PLATFORM_OSX        \
	||  BX_PLATFORM_PS4        \
	||  BX_PLATFORM_PS5        \
	||  BX_PLATFORM_RPI        \
	||  BX_PLATFORM_VISIONOS   \
	||  BX_PLATFORM_WINDOWS    \
	||  BX_PLATFORM_WINRT      \
	||  BX_PLATFORM_XBOXONE    \
	)

///
#define BX_PLATFORM_OS_CONSOLE  (0 \
	||  BX_PLATFORM_NX             \
	||  BX_PLATFORM_PS4            \
	||  BX_PLATFORM_PS5            \
	||  BX_PLATFORM_WINRT          \
	||  BX_PLATFORM_XBOXONE        \
	)

///
#define BX_PLATFORM_OS_DESKTOP  (0 \
	||  BX_PLATFORM_BSD            \
	||  BX_PLATFORM_HAIKU          \
	||  BX_PLATFORM_HURD           \
	||  BX_PLATFORM_LINUX          \
	||  BX_PLATFORM_OSX            \
	||  BX_PLATFORM_WINDOWS        \
	)

///
#define BX_PLATFORM_OS_EMBEDDED (0 \
	||  BX_PLATFORM_RPI            \
	)

///
#define BX_PLATFORM_OS_MOBILE   (0 \
	||  BX_PLATFORM_ANDROID        \
	||  BX_PLATFORM_IOS            \
	)

///
#define BX_PLATFORM_OS_WEB      (0 \
	||  BX_PLATFORM_EMSCRIPTEN     \
	)

///
#if BX_COMPILER_GCC
#	define BX_COMPILER_NAME "GCC "       \
		BX_STRINGIZE(__GNUC__) "."       \
		BX_STRINGIZE(__GNUC_MINOR__) "." \
		BX_STRINGIZE(__GNUC_PATCHLEVEL__)
#elif BX_COMPILER_CLANG
#	define BX_COMPILER_NAME "Clang "      \
		BX_STRINGIZE(__clang_major__) "." \
		BX_STRINGIZE(__clang_minor__) "." \
		BX_STRINGIZE(__clang_patchlevel__)
#elif BX_COMPILER_MSVC
#	if BX_COMPILER_MSVC >= 1930 // Visual Studio 2022
#		define BX_COMPILER_NAME "MSVC 17.0"
#	elif BX_COMPILER_MSVC >= 1920 // Visual Studio 2019
#		define BX_COMPILER_NAME "MSVC 16.0"
#	else
#		define BX_COMPILER_NAME "MSVC"
#	endif //
#endif // BX_COMPILER_

#if BX_PLATFORM_ANDROID
#	define BX_PLATFORM_NAME "Android " \
		BX_STRINGIZE(BX_PLATFORM_ANDROID)
#elif BX_PLATFORM_BSD
#	define BX_PLATFORM_NAME "BSD"
#elif BX_PLATFORM_EMSCRIPTEN
#	define BX_PLATFORM_NAME "Emscripten "      \
		BX_STRINGIZE(__EMSCRIPTEN_major__) "." \
		BX_STRINGIZE(__EMSCRIPTEN_minor__) "." \
		BX_STRINGIZE(__EMSCRIPTEN_tiny__)
#elif BX_PLATFORM_HAIKU
#	define BX_PLATFORM_NAME "Haiku"
#elif BX_PLATFORM_HURD
#	define BX_PLATFORM_NAME "Hurd"
#elif BX_PLATFORM_IOS
#	define BX_PLATFORM_NAME "iOS"
#elif BX_PLATFORM_LINUX
#	define BX_PLATFORM_NAME "Linux"
#elif BX_PLATFORM_NONE
#	define BX_PLATFORM_NAME "None"
#elif BX_PLATFORM_NX
#	define BX_PLATFORM_NAME "NX"
#elif BX_PLATFORM_OSX
#	define BX_PLATFORM_NAME "macOS"
#elif BX_PLATFORM_PS4
#	define BX_PLATFORM_NAME "PlayStation 4"
#elif BX_PLATFORM_PS5
#	define BX_PLATFORM_NAME "PlayStation 5"
#elif BX_PLATFORM_RPI
#	define BX_PLATFORM_NAME "RaspberryPi"
#elif BX_PLATFORM_VISIONOS
#	define BX_PLATFORM_NAME "visionOS"
#elif BX_PLATFORM_WINDOWS
#	define BX_PLATFORM_NAME "Windows"
#elif BX_PLATFORM_WINRT
#	define BX_PLATFORM_NAME "WinRT"
#elif BX_PLATFORM_XBOXONE
#	define BX_PLATFORM_NAME "Xbox One"
#else
#	error "Unknown BX_PLATFORM!"
#endif // BX_PLATFORM_

#if BX_CPU_ARM
#	define BX_CPU_NAME "ARM"
#elif BX_CPU_JIT
#	define BX_CPU_NAME "JIT-VM"
#elif BX_CPU_MIPS
#	define BX_CPU_NAME "MIPS"
#elif BX_CPU_PPC
#	define BX_CPU_NAME "PowerPC"
#elif BX_CPU_RISCV
#	define BX_CPU_NAME "RISC-V"
#elif BX_CPU_X86
#	define BX_CPU_NAME "x86"
#endif // BX_CPU_

#if BX_CRT_BIONIC
#	define BX_CRT_NAME "Bionic libc"
#elif BX_CRT_GLIBC
#	define BX_CRT_NAME "GNU C Library"
#elif BX_CRT_MSVC
#	define BX_CRT_NAME "MSVC C Runtime"
#elif BX_CRT_MINGW
#	define BX_CRT_NAME "MinGW C Runtime"
#elif BX_CRT_LIBCXX
#	define BX_CRT_NAME "Clang C Library"
#elif BX_CRT_NEWLIB
#	define BX_CRT_NAME "Newlib"
#elif BX_CRT_NONE
#	define BX_CRT_NAME "None"
#else
#	define BX_CRT_NAME "Unknown CRT"
#endif // BX_CRT_

#if BX_ARCH_32BIT
#	define BX_ARCH_NAME "32-bit"
#elif BX_ARCH_64BIT
#	define BX_ARCH_NAME "64-bit"
#endif // BX_ARCH_

#if defined(__cplusplus)
#	if BX_COMPILER_MSVC && defined(_MSVC_LANG) && _MSVC_LANG != __cplusplus
#		error "When using MSVC you must set /Zc:__cplusplus compiler option."
#	endif // BX_COMPILER_MSVC && defined(_MSVC_LANG) && _MSVC_LANG != __cplusplus

#	if   __cplusplus < BX_LANGUAGE_CPP17
#		define BX_CPP_NAME "C++Unsupported"
#	elif __cplusplus < BX_LANGUAGE_CPP20
#		define BX_CPP_NAME "C++17"
#	elif __cplusplus < BX_LANGUAGE_CPP23
#		define BX_CPP_NAME "C++20"
#	else
// See: https://gist.github.com/bkaradzic/2e39896bc7d8c34e042b#orthodox-c
#		define BX_CPP_NAME "C++WayTooModern"
#	endif // BX_CPP_NAME
#else
#	define BX_CPP_NAME "C++Unknown"
#endif // defined(__cplusplus)

#if BX_COMPILER_MSVC && (!defined(_MSVC_TRADITIONAL) || _MSVC_TRADITIONAL)
#	error "When using MSVC you must set /Zc:preprocessor compiler option."
#endif // BX_COMPILER_MSVC && (!defined(_MSVC_TRADITIONAL) || _MSVC_TRADITIONAL)

#if defined(__cplusplus)

static_assert(__cplusplus >= BX_LANGUAGE_CPP17, "\n\n"
	"\t** IMPORTANT! **\n\n"
	"\tC++17 standard support is required to build.\n"
	"\t\n");

// https://releases.llvm.org/
static_assert(!BX_COMPILER_CLANG || BX_COMPILER_CLANG >= 110000, "\n\n"
	"\t** IMPORTANT! **\n\n"
	"\tMinimum supported Clang version is 11.0 (October 12, 2020).\n"
	"\t\n");

// https://gcc.gnu.org/releases.html
static_assert(!BX_COMPILER_GCC || BX_COMPILER_GCC >= 80400, "\n\n"
	"\t** IMPORTANT! **\n\n"
	"\tMinimum supported GCC version is 8.4 (March 4, 2020).\n"
	"\t\n");

// https://learn.microsoft.com/en-us/visualstudio/releases/2019/history
static_assert(!BX_COMPILER_MSVC || BX_COMPILER_MSVC >= 1927, "\n\n"
	"\t** IMPORTANT! **\n\n"
	"\tMinimum supported MSVC 19.27 / Visual Studio 2019 version 16.7 (August 5, 2020).\n"
	"\t\n");

static_assert(!BX_CPU_ENDIAN_BIG, "\n\n"
	"\t** IMPORTANT! **\n\n"
	"\tThe code was not tested for big endian, and big endian CPU is considered unsupported.\n"
	"\t\n");

static_assert(!BX_PLATFORM_BSD || !BX_PLATFORM_HAIKU || !BX_PLATFORM_HURD, "\n\n"
	"\t** IMPORTANT! **\n\n"
	"\tYou're compiling for unsupported platform!\n"
	"\tIf you wish to support this platform, make your own fork, and modify code for _yourself_.\n"
	"\t\n"
	"\tDo not submit PR to main repo, it won't be considered, and it would code rot anyway. I have no ability\n"
	"\tto test on these platforms, and over years there wasn't any serious contributor who wanted to take\n"
	"\tburden of maintaining code for these platforms.\n"
	"\t\n");

#endif // defined(__cplusplus)

#endif // BX_PLATFORM_H_HEADER_GUARD
