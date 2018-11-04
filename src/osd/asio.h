// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    asio.hpp

    ASIO library implementation loader

***************************************************************************/

#ifdef __clang__
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#if defined(WIN32) && !defined(_WIN32_WINNT)
#if defined(OSD_WINDOWS)
#define _WIN32_WINNT 0x0501
#else
#define _WIN32_WINNT 0x0603
#endif
#endif
#define ASIO_HEADER_ONLY
#define ASIO_STANDALONE
#define ASIO_SEPARATE_COMPILATION
#define ASIO_NOEXCEPT noexcept(true)
#define ASIO_NOEXCEPT_OR_NOTHROW noexcept(true)
#define ASIO_ERROR_CATEGORY_NOEXCEPT noexcept(true)

#include <asio.hpp>
#undef interface
