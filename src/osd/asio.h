// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    asio.hpp

    ASIO library implementation loader

***************************************************************************/

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#if defined(_WIN32) && !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x0600
#endif

#define ASIO_HEADER_ONLY
#define ASIO_STANDALONE
#define ASIO_SEPARATE_COMPILATION

#if defined(__sun) || defined(__sun__)
#define ASIO_DISABLE_DEV_POLL
#define ASIO_HAS_EPOLL
#endif

#include <asio.hpp>
#undef interface

#ifdef __clang__
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
