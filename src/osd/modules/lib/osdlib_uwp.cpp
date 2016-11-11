// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, Brad Hughes
//============================================================
//
//  sdlos_*.c - OS specific low level code
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#include <stdlib.h>

#include <cstdio>
#include <memory>

// MAME headers
#include "osdlib.h"
#include "osdcomm.h"
#include "osdcore.h"
#include "strconv.h"

#include <windows.h>
#include <wrl\client.h>

#include "strconv.h"

using namespace Platform;
using namespace Windows::ApplicationModel::DataTransfer;
using namespace Windows::Foundation;

#include <map>

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
//  osd_alloc_executable
//
//  allocates "size" bytes of executable memory.  this must take
//  things like NX support into account.
//============================================================

void *osd_alloc_executable(size_t size)
{
	return nullptr;
}


//============================================================
//  osd_free_executable
//
//  frees memory allocated with osd_alloc_executable
//============================================================

void osd_free_executable(void *ptr, size_t size)
{
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

static char *get_clipboard_text_by_format(UINT format, std::string (*convert)(LPCVOID data))
{
	DataPackageView^ dataPackageView;
	IAsyncOperation<String^>^ getTextOp;
	String^ clipboardText;

	dataPackageView = Clipboard::GetContent();
	getTextOp = dataPackageView->GetTextAsync();
	clipboardText = getTextOp->GetResults();

	return (char *)convert(clipboardText->Data()).c_str();
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

char *osd_get_clipboard_text(void)
{
	// try to access unicode text
	char *result = get_clipboard_text_by_format(CF_UNICODETEXT, convert_wide);

	// try to access ANSI text
	if (result == nullptr)
		result = get_clipboard_text_by_format(CF_TEXT, convert_ansi);

	return result;
}

//============================================================
//  osd_dynamic_bind
//============================================================
// for Windows Store universal applications
// This needs to change ASAP as it won't be allowed in the store
typedef HMODULE __stdcall t_LLA(const char *);
typedef FARPROC __stdcall t_GPA(HMODULE H, const char *);

PIMAGE_NT_HEADERS WINAPI ImageNtHeader(PVOID Base)
{
	return (PIMAGE_NT_HEADERS)
		((LPBYTE)Base + ((PIMAGE_DOS_HEADER)Base)->e_lfanew);
}

PIMAGE_SECTION_HEADER WINAPI RtlImageRvaToSection(const IMAGE_NT_HEADERS *nt,
	HMODULE module, DWORD_PTR rva)
{
	int i;
	const IMAGE_SECTION_HEADER *sec;

	sec = (const IMAGE_SECTION_HEADER*)((const char*)&nt->OptionalHeader +
		nt->FileHeader.SizeOfOptionalHeader);
	for (i = 0; i < nt->FileHeader.NumberOfSections; i++, sec++)
	{
		if ((sec->VirtualAddress <= rva) && (sec->VirtualAddress + sec->SizeOfRawData > rva))
			return (PIMAGE_SECTION_HEADER)sec;
	}
	return NULL;
}

PVOID WINAPI RtlImageRvaToVa(const IMAGE_NT_HEADERS *nt, HMODULE module,
	DWORD_PTR rva, IMAGE_SECTION_HEADER **section)
{
	IMAGE_SECTION_HEADER *sec;

	if (section && *section)  /* try this section first */
	{
		sec = *section;
		if ((sec->VirtualAddress <= rva) && (sec->VirtualAddress + sec->SizeOfRawData > rva))
			goto found;
	}
	if (!(sec = RtlImageRvaToSection(nt, module, rva))) return NULL;
found:
	if (section) *section = sec;
	return (char *)module + sec->PointerToRawData + (rva - sec->VirtualAddress);
}

PVOID WINAPI ImageDirectoryEntryToDataEx(PVOID base, BOOLEAN image, USHORT dir, PULONG size, PIMAGE_SECTION_HEADER *section)
{
	const IMAGE_NT_HEADERS *nt;
	DWORD_PTR addr;

	*size = 0;
	if (section) *section = NULL;

	if (!(nt = ImageNtHeader(base))) return NULL;
	if (dir >= nt->OptionalHeader.NumberOfRvaAndSizes) return NULL;
	if (!(addr = nt->OptionalHeader.DataDirectory[dir].VirtualAddress)) return NULL;

	*size = nt->OptionalHeader.DataDirectory[dir].Size;
	if (image || addr < nt->OptionalHeader.SizeOfHeaders) return (char *)base + addr;

	return RtlImageRvaToVa(nt, (HMODULE)base, addr, section);
}

// == Windows API GetProcAddress
void *PeGetProcAddressA(void *Base, LPCSTR Name)
{
	DWORD Tmp;

	IMAGE_NT_HEADERS *NT = ImageNtHeader(Base);
	IMAGE_EXPORT_DIRECTORY *Exp = (IMAGE_EXPORT_DIRECTORY*)ImageDirectoryEntryToDataEx(Base, TRUE, IMAGE_DIRECTORY_ENTRY_EXPORT, &Tmp, 0);
	if (Exp == 0 || Exp->NumberOfFunctions == 0)
	{
		SetLastError(ERROR_NOT_FOUND);
		return 0;
	}

	DWORD *Names = (DWORD*)(Exp->AddressOfNames + (DWORD_PTR)Base);
	WORD *Ordinals = (WORD*)(Exp->AddressOfNameOrdinals + (DWORD_PTR)Base);
	DWORD *Functions = (DWORD*)(Exp->AddressOfFunctions + (DWORD_PTR)Base);

	FARPROC Ret = 0;

	if ((DWORD_PTR)Name<65536)
	{
		if ((DWORD_PTR)Name - Exp->Base<Exp->NumberOfFunctions)
			Ret = (FARPROC)(Functions[(DWORD_PTR)Name - Exp->Base] + (DWORD_PTR)Base);
	}
	else
	{
		for (DWORD i = 0; i<Exp->NumberOfNames && Ret == 0; i++)
		{
			char *Func = (char*)(Names[i] + std::uintptr_t(Base));
			if (Func && strcmp(Func, Name) == 0)
				Ret = (FARPROC)(Functions[Ordinals[i]] + std::uintptr_t(Base));
		}
	}

	if (Ret)
	{
		std::uintptr_t ExpStart = NT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress + std::uintptr_t(Base);
		std::uintptr_t ExpSize = NT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
		if (std::uintptr_t(Ret) >= ExpStart && std::uintptr_t(Ret) <= ExpStart + ExpSize)
		{
			// Forwarder
			return 0;
		}
		return Ret;
	}

	return 0;
}

t_LLA* g_LoadLibraryA = 0;
t_GPA* g_GetProcAddressA = 0;

void find_load_exports()
{
	char *Tmp = (char*)GetTickCount64;
	Tmp = (char*)((~0xFFF)&(DWORD_PTR)Tmp);

	while (Tmp)
	{
		__try
		{
			if (Tmp[0] == 'M' && Tmp[1] == 'Z')
				break;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}
		Tmp -= 0x1000;
	}

	if (Tmp == 0)
		return;

	g_LoadLibraryA = (t_LLA*)PeGetProcAddressA(Tmp, "LoadLibraryA");
	g_GetProcAddressA = (t_GPA*)PeGetProcAddressA(Tmp, "GetProcAddress");
}

#define load_library(filename) g_LoadLibraryA(osd::text::from_wstring(filename).c_str())
#define get_proc_address(mod, proc) g_GetProcAddressA(mod, proc)



namespace osd {
class dynamic_module_win32_impl : public dynamic_module
{
public:
	dynamic_module_win32_impl(std::vector<std::string> &libraries)
		: m_module(nullptr)
	{
		m_libraries = libraries;
		find_load_exports();
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
				generic_fptr_t function = reinterpret_cast<generic_fptr_t>(GetProcAddress(module, symbol));

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
