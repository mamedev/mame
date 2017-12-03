// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#if defined(MAME_EMU_LOGMACRO_H) || !defined(__EMU_H__)
#error This file should only be included once per compilation unit after all other headers
#endif
#define MAME_EMU_LOGMACRO_H

#ifndef VERBOSE
#define VERBOSE 0
#endif

#ifndef LOG_OUTPUT_FUNC
#ifdef LOG_OUTPUT_STREAM
#define LOG_OUTPUT_FUNC [] (auto &&... args) { util::stream_format((LOG_OUTPUT_STREAM), std::forward<decltype(args)>(args)...); }
#else
#define LOG_OUTPUT_FUNC logerror
#endif
#endif

#ifndef LOG_GENERAL
#define LOG_GENERAL (1U << 0)
#endif

#define LOGMASKED(mask, ...) do { if (VERBOSE & (mask)) (LOG_OUTPUT_FUNC)(__VA_ARGS__); } while (false)

#define LOG(...) LOGMASKED(LOG_GENERAL, __VA_ARGS__)
