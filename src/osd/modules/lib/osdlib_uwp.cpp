// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, Brad Hughes
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

#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>

#include <windows.h>
#include <memoryapi.h>

#include <wrl\client.h>


using namespace Platform;
using namespace Windows::ApplicationModel::DataTransfer;
using namespace Windows::Foundation;


//============================================================
//  GLOBAL VARIABLES
//============================================================

std::map<const char *, std::unique_ptr<char>> g_runtime_environment;

//============================================================
//  osd_getenv
//============================================================

const char *osd_getenv(const char *name)
{
	for (auto iter = g_runtime_environment.begin(); iter != g_runtime_environment.end(); iter++)
	{
		if (stricmp(iter->first, name) == 0)
		{
			osd_printf_debug("ENVIRONMENT: Get %s = value: '%s'", name, iter->second.get());
			return iter->second.get();
		}
	}

	return nullptr;
}


//============================================================
//  osd_setenv
//============================================================

int osd_setenv(const char *name, const char *value, int overwrite)
{
	if (!overwrite)
	{
		if (osd_getenv(name) != nullptr)
			return 0;
	}

	auto buf = std::make_unique<char>(strlen(name) + strlen(value) + 2);
	sprintf(buf.get(), "%s=%s", name, value);

	g_runtime_environment[name] = std::move(buf);
	osd_printf_debug("ENVIRONMENT: Set %s to value: '%s'", name, buf.get());

	return 0;
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
	if (IsDebuggerPresent())
	{
		OutputDebugStringA(message);
		__debugbreak();
	}
}

//============================================================
//  get_clipboard_text_by_format
//============================================================

static bool get_clipboard_text_by_format(std::string &result_text, UINT format, std::string (*convert)(LPCVOID data))
{
	DataPackageView^ dataPackageView;
	IAsyncOperation<String^>^ getTextOp;
	String^ clipboardText;

	dataPackageView = Clipboard::GetContent();
	getTextOp = dataPackageView->GetTextAsync();
	clipboardText = getTextOp->GetResults();

	result_text = convert(clipboardText->Data());
	return !result_text.empty();
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

std::string osd_get_clipboard_text(void)
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

int osd_getpid(void)
{
	return GetCurrentProcessId();
}


namespace osd {

bool invalidate_instruction_cache(void const *start, std::size_t size)
{
	return FlushInstructionCache(GetCurrentProcess(), start, size) != 0;
}


void *virtual_memory_allocation::do_alloc(std::initializer_list<std::size_t> blocks, std::size_t &size, std::size_t &page_size)
{
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	SIZE_T s(0);
	for (std::size_t b : blocks)
		s += (b + info.dwPageSize - 1) / info.dwPageSize;
	s *= info.dwPageSize;
	if (!s)
		return nullptr;
	LPVOID const result(VirtualAllocFromApp(nullptr, s, MEM_COMMIT, PAGE_NOACCESS));
	if (result)
	{
		size = s;
		page_size = info.dwPageSize;
	}
	return result;
}

void virtual_memory_allocation::do_free(void *start, std::size_t size)
{
	VirtualFree(start, 0, MEM_RELEASE);
}

bool virtual_memory_allocation::do_set_access(void *start, std::size_t size, unsigned access)
{
	ULONG p, o;
	if (access & EXECUTE)
		p = (access & WRITE) ? PAGE_EXECUTE_READWRITE : (access & READ) ? PAGE_EXECUTE_READ : PAGE_EXECUTE;
	else
		p = (access & WRITE) ? PAGE_READWRITE : (access & READ) ? PAGE_READONLY : PAGE_NOACCESS;
	return VirtualProtectFromApp(start, size, p, &o) != 0;
}

} // namespace osd
