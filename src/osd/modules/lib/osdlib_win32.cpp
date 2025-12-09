// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  sdlos_*.c - OS specific low level code
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

// MAME headers
#include "osdlib.h"
#include "osdcomm.h"
#include "osdcore.h"
#include "strconv.h"

#include "winutf8.h"
#include "winutil.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>

#include <windows.h>
#include <memoryapi.h>

#ifndef _MSC_VER
#include <unistd.h>
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
//  osd_get_cache_line_size
//============================================================

std::pair<std::error_condition, unsigned> osd_get_cache_line_size() noexcept
{
	DWORD resultsize = 0;
	if (GetLogicalProcessorInformation(nullptr, &resultsize) || (ERROR_INSUFFICIENT_BUFFER != GetLastError()) || !resultsize)
		return std::make_pair(std::errc::operation_not_permitted, 0U);

	auto const result = reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION *>(std::malloc(resultsize));
	if (!result)
		return std::make_pair(std::errc::not_enough_memory, 0U);

	if (!GetLogicalProcessorInformation(result, &resultsize))
	{
		std::free(result);
		return std::make_pair(std::errc::operation_not_permitted, 0U);
	}

	for (unsigned i = 0; i < (resultsize / sizeof(result[0])); ++i)
	{
		if ((RelationCache == result[i].Relationship) && (1 == result[i].Cache.Level))
		{
			unsigned const linesize = result[i].Cache.LineSize;
			std::free(result);
			return std::make_pair(std::error_condition(), linesize);
		}
	}

	std::free(result);
	return std::make_pair(std::errc::operation_not_permitted, 0U);
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

std::string osd_get_clipboard_text() noexcept
{
	std::string result;

	// TODO: better error handling
	try
	{
		// try to access unicode text
		if (!get_clipboard_text_by_format(result, CF_UNICODETEXT, convert_wide))
		{
			// try to access ANSI text
			get_clipboard_text_by_format(result, CF_TEXT, convert_ansi);
		}
	}
	catch (...)
	{
	}

	return result;
}

//============================================================
//  osd_set_clipboard_text
//============================================================

std::error_condition osd_set_clipboard_text(std::string_view text) noexcept
{
	try
	{
		// convert the text to a wide char string and create a moveable global block
		std::wstring const wtext = osd::text::to_wstring(text);
		HGLOBAL const clip = GlobalAlloc(GMEM_MOVEABLE, (text.length() + 1) * sizeof(wchar_t));
		if (!clip)
			return win_error_to_error_condition(GetLastError());
		LPWSTR const lock = reinterpret_cast<LPWSTR>(GlobalLock(clip));
		if (!lock)
		{
			DWORD const err(GetLastError());
			GlobalFree(clip);
			return win_error_to_error_condition(err);
		}

		// clear current clipboard contents
		if (!OpenClipboard(nullptr))
		{
			DWORD const err(GetLastError());
			GlobalUnlock(clip);
			GlobalFree(clip);
			return win_error_to_error_condition(err);
		}
		if (!EmptyClipboard())
		{
			DWORD const err(GetLastError());
			CloseClipboard();
			GlobalUnlock(clip);
			GlobalFree(clip);
			return win_error_to_error_condition(err);
		}

		// copy the text (plus NUL terminator) to the moveable block and put it on the clipboard
		std::copy_n(wtext.c_str(), wtext.length() + 1, lock);
		GlobalUnlock(clip);
		if (!SetClipboardData(CF_UNICODETEXT, clip))
		{
			DWORD const err(GetLastError());
			CloseClipboard();
			GlobalFree(clip);
			return win_error_to_error_condition(err);
		}

		// clean up
		if (!CloseClipboard())
			return win_error_to_error_condition(GetLastError());
		return std::error_condition();
	}
	catch (std::bad_alloc const &)
	{
		return std::errc::not_enough_memory;
	}
}

//============================================================
//  osd_getpid
//============================================================

int osd_getpid() noexcept
{
	return GetCurrentProcessId();
}

//============================================================
//  osd_dynamic_bind
//============================================================

// for classic desktop applications
#define load_library(filename) LoadLibrary(filename)

namespace osd {

namespace {

class dynamic_module_win32_impl : public dynamic_module
{
public:
	dynamic_module_win32_impl(std::vector<std::string> &&libraries) : m_libraries(std::move(libraries))
	{
	}

	virtual ~dynamic_module_win32_impl() override
	{
		if (m_module)
			FreeLibrary(m_module);
	}

protected:
	virtual generic_fptr_t get_symbol_address(char const *symbol) override
	{
		/*
		 * given a list of libraries, if a first symbol is successfully loaded from
		 * one of them, all additional symbols will be loaded from the same library
		 */
		if (m_module)
			return reinterpret_cast<generic_fptr_t>(GetProcAddress(m_module, symbol));

		for (auto const &library : m_libraries)
		{
			osd::text::tstring const tempstr = osd::text::to_tstring(library);
			HMODULE const module = load_library(tempstr.c_str());

			if (module)
			{
				auto const function = reinterpret_cast<generic_fptr_t>(GetProcAddress(module, symbol));

				if (function)
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
	HMODULE                  m_module = nullptr;
};

} // anonymous namespace


bool invalidate_instruction_cache(void const *start, std::size_t size) noexcept
{
	return FlushInstructionCache(GetCurrentProcess(), start, size) != 0;
}


void *virtual_memory_allocation::do_alloc(std::initializer_list<std::size_t> blocks, unsigned intent, std::size_t &size, std::size_t &page_size) noexcept
{
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	SIZE_T s(0);
	for (std::size_t b : blocks)
		s += (b + info.dwPageSize - 1) / info.dwPageSize;
	s *= info.dwPageSize;
	if (!s)
		return nullptr;
	LPVOID const result(VirtualAlloc(nullptr, s, MEM_COMMIT, PAGE_NOACCESS));
	if (result)
	{
		size = s;
		page_size = info.dwPageSize;
	}
	return result;
}

void virtual_memory_allocation::do_free(void *start, std::size_t size) noexcept
{
	VirtualFree(start, 0, MEM_RELEASE);
}

bool virtual_memory_allocation::do_set_access(void *start, std::size_t size, unsigned access) noexcept
{
	DWORD p;
	if (access & EXECUTE)
		p = (access & WRITE) ? PAGE_EXECUTE_READWRITE : (access & READ) ? PAGE_EXECUTE_READ : PAGE_EXECUTE;
	else
		p = (access & WRITE) ? PAGE_READWRITE : (access & READ) ? PAGE_READONLY : PAGE_NOACCESS;
	return VirtualAlloc(start, size, MEM_COMMIT, p) != nullptr;
}


dynamic_module::ptr dynamic_module::open(std::vector<std::string> &&names)
{
	return std::make_unique<dynamic_module_win32_impl>(std::move(names));
}

} // namespace osd
