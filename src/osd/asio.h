// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    asio.hpp

    ASIO library implementation loader

***************************************************************************/

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#if defined(_WIN32) && !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x0603
#endif

#define ASIO_HEADER_ONLY
//#define ASIO_SEPARATE_COMPILATION

// only define this for testing - users may build with newer system ASIO
//#define ASIO_NO_DEPRECATED

#include <asio.hpp>

#ifdef __clang__
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
