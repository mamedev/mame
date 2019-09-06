// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#include "osdcore.h"
#include <thread>
#include <chrono>

#if defined(SDLMAME_ANDROID)
#include <SDL2/SDL.h>
#endif

#ifdef _WIN32
#include <windows.h>
#include <stdio.h>
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
    osd_printf_error - output an error to the
    appropriate callback
-------------------------------------------------*/

void CLIB_DECL osd_printf_error(const char *format, ...)
{
	va_list argptr;

	/* do the output */
	va_start(argptr, format);
#if defined(SDLMAME_ANDROID)
	SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, format, argptr);
#else
	if (m_ptr >= 0) m_stack[m_ptr]->output_callback(OSD_OUTPUT_CHANNEL_ERROR, format, argptr);
#endif
	va_end(argptr);
}


/*-------------------------------------------------
    osd_printf_warning - output a warning to the
    appropriate callback
-------------------------------------------------*/

void CLIB_DECL osd_printf_warning(const char *format, ...)
{
	va_list argptr;

	/* do the output */
	va_start(argptr, format);
#if defined(SDLMAME_ANDROID)
	SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, format, argptr);
#else
	if (m_ptr >= 0) m_stack[m_ptr]->output_callback(OSD_OUTPUT_CHANNEL_WARNING, format, argptr);
#endif
	va_end(argptr);
}


/*-------------------------------------------------
    osd_printf_info - output info text to the
    appropriate callback
-------------------------------------------------*/

void CLIB_DECL osd_printf_info(const char *format, ...)
{
	va_list argptr;

	/* do the output */
	va_start(argptr, format);
#if defined(SDLMAME_ANDROID)
	SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, format, argptr);
#else
	if (m_ptr >= 0) m_stack[m_ptr]->output_callback(OSD_OUTPUT_CHANNEL_INFO, format, argptr);
#endif
	va_end(argptr);
}


/*-------------------------------------------------
    osd_printf_verbose - output verbose text to
    the appropriate callback
-------------------------------------------------*/

void CLIB_DECL osd_printf_verbose(const char *format, ...)
{
	va_list argptr;

	/* do the output */
	va_start(argptr, format);
#if defined(SDLMAME_ANDROID)
	SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE, format, argptr);
#else
	if (m_ptr >= 0) m_stack[m_ptr]->output_callback(OSD_OUTPUT_CHANNEL_VERBOSE, format, argptr);
#endif
	va_end(argptr);
}


/*-------------------------------------------------
    osd_printf_debug - output debug text to the
    appropriate callback
-------------------------------------------------*/

void CLIB_DECL osd_printf_debug(const char *format, ...)
{
	va_list argptr;

	/* do the output */
	va_start(argptr, format);
#if defined(SDLMAME_ANDROID)
	SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG, format, argptr);
#else
	if (m_ptr >= 0) m_stack[m_ptr]->output_callback(OSD_OUTPUT_CHANNEL_DEBUG, format, argptr);
#endif
	va_end(argptr);
}


/*-------------------------------------------------
    osd_printf_log - output log text to the
    appropriate callback
-------------------------------------------------*/

#ifdef UNUSED_FUNCTION
void CLIB_DECL osd_printf_log(const char *format, ...)
{
	va_list argptr;

	/* do the output */
	va_start(argptr, format);
	output_cb[OSD_OUTPUT_CHANNEL_LOG])(format, argptr);
	va_end(argptr);
}
#endif

//============================================================
//  osd_ticks
//============================================================

osd_ticks_t osd_ticks(void)
{
	return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}


//============================================================
//  osd_ticks_per_second
//============================================================

osd_ticks_t osd_ticks_per_second(void)
{
	return std::chrono::high_resolution_clock::period::den / std::chrono::high_resolution_clock::period::num;
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
