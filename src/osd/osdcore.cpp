// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#include "osdcore.h"
#include <thread>
#include <chrono>

#if defined(SDLMAME_ANDROID)
#include <android/log.h>
#endif

#ifdef _WIN32
#include <windows.h>
#include <cstdio>
#include <shellapi.h>
#include "strconv.h"
#endif

static const int MAXSTACK = 10;
static osd_output *m_stack[MAXSTACK];
static int m_ptr = -1;

/*-------------------------------------------------
    osd_output
-------------------------------------------------*/

void osd_output::push(osd_output *delegate)
{
	if (m_ptr < MAXSTACK)
	{
		delegate->m_chain = (m_ptr >= 0 ? m_stack[m_ptr] : nullptr);
		m_ptr++;
		m_stack[m_ptr] = delegate;
	}
}

void osd_output::pop(osd_output *delegate)
{
	int f = -1;
	for (int i=0; i<=m_ptr; i++)
		if (m_stack[i] == delegate)
		{
			f = i;
			break;
		}
	if (f >= 0)
	{
		if (f < m_ptr)
			m_stack[f+1]->m_chain = m_stack[f]->m_chain;
		m_ptr--;
		for (int i = f; i <= m_ptr; i++)
			m_stack[i] = m_stack[i+1];
	}
}


/***************************************************************************
    OUTPUT MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    osd_vprintf_error - output an error to the
    appropriate callback
-------------------------------------------------*/

void osd_vprintf_error(util::format_argument_pack<std::ostream> const &args)
{
#if defined(SDLMAME_ANDROID)
	__android_log_write(ANDROID_LOG_ERROR, "%s", util::string_format(args).c_str());
#else
	if (m_ptr >= 0) m_stack[m_ptr]->output_callback(OSD_OUTPUT_CHANNEL_ERROR, args);
#endif
}


/*-------------------------------------------------
    osd_vprintf_warning - output a warning to the
    appropriate callback
-------------------------------------------------*/

void osd_vprintf_warning(util::format_argument_pack<std::ostream> const &args)
{
#if defined(SDLMAME_ANDROID)
	__android_log_write(ANDROID_LOG_WARN, "%s", util::string_format(args).c_str());
#else
	if (m_ptr >= 0) m_stack[m_ptr]->output_callback(OSD_OUTPUT_CHANNEL_WARNING, args);
#endif
}


/*-------------------------------------------------
    osd_vprintf_info - output info text to the
    appropriate callback
-------------------------------------------------*/

void osd_vprintf_info(util::format_argument_pack<std::ostream> const &args)
{
#if defined(SDLMAME_ANDROID)
	__android_log_write(ANDROID_LOG_INFO, "%s", util::string_format(args).c_str());
#else
	if (m_ptr >= 0) m_stack[m_ptr]->output_callback(OSD_OUTPUT_CHANNEL_INFO, args);
#endif
}


/*-------------------------------------------------
    osd_vprintf_verbose - output verbose text to
    the appropriate callback
-------------------------------------------------*/

void osd_vprintf_verbose(util::format_argument_pack<std::ostream> const &args)
{
#if defined(SDLMAME_ANDROID)
	__android_log_write( ANDROID_LOG_VERBOSE, "%s", util::string_format(args).c_str());
#else
	if (m_ptr >= 0) m_stack[m_ptr]->output_callback(OSD_OUTPUT_CHANNEL_VERBOSE, args);
#endif
}


/*-------------------------------------------------
    osd_vprintf_debug - output debug text to the
    appropriate callback
-------------------------------------------------*/

void osd_vprintf_debug(util::format_argument_pack<std::ostream> const &args)
{
#if defined(SDLMAME_ANDROID)
	__android_log_write(ANDROID_LOG_DEBUG, "%s", util::string_format(args).c_str());
#else
	if (m_ptr >= 0) m_stack[m_ptr]->output_callback(OSD_OUTPUT_CHANNEL_DEBUG, args);
#endif
}


//============================================================
//  osd_ticks
//============================================================

osd_ticks_t osd_ticks()
{
#ifdef WIN32
	LARGE_INTEGER val;
	QueryPerformanceCounter(&val);
	return val.QuadPart;
#else
	return std::chrono::high_resolution_clock::now().time_since_epoch().count();
#endif
}


//============================================================
//  osd_ticks_per_second
//============================================================

osd_ticks_t osd_ticks_per_second()
{
#ifdef WIN32
	LARGE_INTEGER val;
	QueryPerformanceFrequency(&val);
	return val.QuadPart;
#else
	return std::chrono::high_resolution_clock::period::den / std::chrono::high_resolution_clock::period::num;
#endif
}

//============================================================
//  osd_sleep
//============================================================

void osd_sleep(osd_ticks_t duration)
{
#ifdef WIN32
// sleep_for appears to oversleep on Windows with gcc 8
	Sleep(duration / (osd_ticks_per_second() / 1000));
#else
	std::this_thread::sleep_for(std::chrono::high_resolution_clock::duration(duration));
#endif
}


//============================================================
//  osd_get_command_line - returns command line arguments
//  in an std::vector<std::string> in UTF-8
//
//  The real purpose of this call is to hide details necessary
//  on Windows (provided that one wants to avoid using wmain)
//============================================================

std::vector<std::string> osd_get_command_line(int argc, char *argv[])
{
	std::vector<std::string> results;
#ifdef WIN32
	{
		// Get the command line from Windows
		int count;
		LPWSTR *wide_args = CommandLineToArgvW(GetCommandLineW(), &count);

		// Convert the returned command line arguments to UTF8 std::vector<std::string>
		results.reserve(count);
		for (int i = 0; i < count; i++)
		{
			std::string arg = osd::text::from_wstring(wide_args[i]);
			results.push_back(std::move(arg));
		}

		LocalFree(wide_args);
	}
#else // !WIN32
	{
		// for non Windows platforms, we are assuming that arguments are
		// already UTF-8; we just need to convert to std::vector<std::string>
		results.reserve(argc);
		for (int i = 0; i < argc; i++)
			results.emplace_back(argv[i]);
	}
#endif // WIN32
	return results;
}
