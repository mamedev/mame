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

namespace emu::detail
{
	template <typename Self, typename Format, typename... Args>
	inline void dispatch_logerror(Self &&self, Format &&fmt, Args &&... args)
	{
		if constexpr (requires { self.logerror(std::forward<Format>(fmt), std::forward<Args>(args)...); })
			self.logerror(std::forward<Format>(fmt), std::forward<Args>(args)...);
		else if constexpr (requires { self.device().logerror(std::forward<Format>(fmt), std::forward<Args>(args)...); })
			self.device().logerror(std::forward<Format>(fmt), std::forward<Args>(args)...);
		else
			static_assert(std::is_void_v<Self>, "dispatch_logerror requires logerror() or device().logerror()");
	}
}

#define LOG_OUTPUT_FUNC [this] (auto &&... args) { emu::detail::dispatch_logerror(*this, std::forward<decltype(args)>(args)...); }
#endif
#endif

#ifndef LOG_GENERAL
#define LOG_GENERAL (1U << 0)
#endif

#define LOGMASKED(mask, ...) do { if (VERBOSE & (mask)) (LOG_OUTPUT_FUNC)(__VA_ARGS__); } while (false)

#define LOG(...) LOGMASKED(LOG_GENERAL, __VA_ARGS__)
