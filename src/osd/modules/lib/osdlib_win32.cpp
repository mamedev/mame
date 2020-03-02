// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  sdlos_*.c - OS specific low level code
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#include <windows.h>
#include <mmsystem.h>

#include <cstdlib>
#ifndef _MSC_VER
#include <unistd.h>
#endif

#include <cstdio>
#include <memory>

// MAME headers
#include "osdlib.h"
#include "osdcomm.h"
#include "osdcore.h"
#include "strconv.h"

#ifdef OSD_WINDOWS
#include "winutf8.h"
#endif

//============================================================
//  GLOBAL VARIABLES
//============================================================

#ifdef OSD_WINDOWS
void (*s_debugger_stack_crawler)() = nullptr;
#endif


//============================================================
//  osd_getenv
//============================================================

const char *osd_getenv(const char *name)
{
	return getenv(name);
}


//============================================================
//  osd_setenv
//============================================================

int osd_setenv(const char *name, const char *value, int overwrite)
{
	char *buf;
	int result;

	if (!overwrite)
	{
		if (osd_getenv(name) != nullptr)
			return 0;
	}
	buf = (char *) malloc(strlen(name)+strlen(value)+2);
	sprintf(buf, "%s=%s", name, value);
	result = putenv(buf);

	/* will be referenced by environment
	 * Therefore it is not freed here
	 */

	return result;
}

//============================================================
//  osd_process_kill
//============================================================

void osd_process_kill()
{
	TerminateProcess(GetCurrentProcess(), -1);
}

//============================================================
//  osd_alloc_executable
//
//  allocates "size" bytes of executable memory.  this must take
//  things like NX support into account.
//============================================================

void *osd_alloc_executable(size_t size)
{
	return VirtualAlloc(nullptr, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
}


//============================================================
//  osd_free_executable
//
//  frees memory allocated with osd_alloc_executable
//============================================================

void osd_free_executable(void *ptr, size_t size)
{
	VirtualFree(ptr, 0, MEM_RELEASE);
}


//============================================================
//  osd_break_into_debugger
//============================================================

void osd_break_into_debugger(const char *message)
{
#ifdef OSD_WINDOWS
	if (IsDebuggerPresent())
	{
		win_output_debug_string_utf8(message);
		DebugBreak();
	}
	else if (s_debugger_stack_crawler != nullptr)
		(*s_debugger_stack_crawler)();
#else
	if (IsDebuggerPresent())
	{
		OutputDebugStringA(message);
		DebugBreak();
	}
#endif
}

//============================================================
//  get_clipboard_text_by_format
//============================================================

static bool get_clipboard_text_by_format(std::string &result_text, UINT format, std::string (*convert)(LPCVOID data))
{
	bool result = false;
	HANDLE data_handle;
	LPVOID data;

	// check to see if this format is available
	if (IsClipboardFormatAvailable(format))
	{
		// open the clipboard
		if (OpenClipboard(nullptr))
		{
			// try to access clipboard data
			data_handle = GetClipboardData(format);
			if (data_handle != nullptr)
			{
				// lock the data
				data = GlobalLock(data_handle);
				if (data != nullptr)
				{
					// invoke the convert
					result_text = (*convert)(data);
					result = true;

					// unlock the data
					GlobalUnlock(data_handle);
				}
			}

			// close out the clipboard
			CloseClipboard();
		}
	}
	return result;
}

//============================================================
//  convert_wide
//============================================================

static std::string convert_wide(LPCVOID data)
{
	return osd::text::from_wstring((LPCWSTR) data);
}

//============================================================
//  convert_ansi
//============================================================

static std::string convert_ansi(LPCVOID data)
{
	return osd::text::from_astring((LPCSTR) data);
}

//============================================================
//  osd_get_clipboard_text
//============================================================

std::string osd_get_clipboard_text()
{
	std::string result;

	// try to access unicode text
	if (!get_clipboard_text_by_format(result, CF_UNICODETEXT, convert_wide))
	{
		// try to access ANSI text
		get_clipboard_text_by_format(result, CF_TEXT, convert_ansi);
	}

	return result;
}

//============================================================
//  osd_getpid
//============================================================

int osd_getpid()
{
	return GetCurrentProcessId();
}

//============================================================
//  osd_dynamic_bind
//============================================================

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
// for classic desktop applications
#define load_library(filename) LoadLibrary(filename)
#else
// for Windows Store universal applications
#define load_library(filename) LoadPackagedLibrary(filename, 0)
#endif

namespace osd {
class dynamic_module_win32_impl : public dynamic_module
{
public:
	dynamic_module_win32_impl(std::vector<std::string> &libraries)
		: m_module(nullptr)
	{
		m_libraries = libraries;
	}

	virtual ~dynamic_module_win32_impl() override
	{
		if (m_module != nullptr)
			FreeLibrary(m_module);
	};

protected:
	virtual generic_fptr_t get_symbol_address(char const *symbol) override
	{
		/*
		 * given a list of libraries, if a first symbol is successfully loaded from
		 * one of them, all additional symbols will be loaded from the same library
		 */
		if (m_module)
		{
			return reinterpret_cast<generic_fptr_t>(GetProcAddress(m_module, symbol));
		}

		for (auto const &library : m_libraries)
		{
			osd::text::tstring tempstr = osd::text::to_tstring(library);
			HMODULE module = load_library(tempstr.c_str());

			if (module != nullptr)
			{
				auto function = reinterpret_cast<generic_fptr_t>(GetProcAddress(module, symbol));

				if (function != nullptr)
				{
					m_module = module;
					return function;
				}
				else
				{
					FreeLibrary(module);
				}
			}
		}

		return nullptr;
	}

private:
	std::vector<std::string> m_libraries;
	HMODULE                  m_module;
};

dynamic_module::ptr dynamic_module::open(std::vector<std::string> &&names)
{
	return std::make_unique<dynamic_module_win32_impl>(names);
}

} // namespace osd
