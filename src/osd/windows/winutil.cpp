// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  winutil.c - Win32 OSD core utility functions
//
//============================================================

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>

// MAME headers
#include "emu.h"

// MAMEOS headers
#include "winutil.h"
#include "strconv.h"


//============================================================
//  win_attributes_to_entry_type
//============================================================

osd_dir_entry_type win_attributes_to_entry_type(DWORD attributes)
{
	if (attributes == 0xFFFFFFFF)
		return ENTTYPE_NONE;
	else if (attributes & FILE_ATTRIBUTE_DIRECTORY)
		return ENTTYPE_DIR;
	else
		return ENTTYPE_FILE;
}



//============================================================
//  win_is_gui_application
//============================================================

BOOL win_is_gui_application(void)
{
	static BOOL is_gui_frontend;
	static BOOL is_first_time = TRUE;
	HMODULE module;
	BYTE *image_ptr;
	IMAGE_DOS_HEADER *dos_header;
	IMAGE_NT_HEADERS *nt_headers;
	IMAGE_OPTIONAL_HEADER *opt_header;

	// is this the first time we've been ran?
	if (is_first_time)
	{
		is_first_time = FALSE;

		// get the current module
		module = GetModuleHandleUni();
		if (!module)
			return FALSE;
		image_ptr = (BYTE*) module;

		// access the DOS header
		dos_header = (IMAGE_DOS_HEADER *) image_ptr;
		if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
			return FALSE;

		// access the NT headers
		nt_headers = (IMAGE_NT_HEADERS *) ((BYTE*)(dos_header) + (DWORD)(dos_header->e_lfanew));
		if (nt_headers->Signature != IMAGE_NT_SIGNATURE)
			return FALSE;

		// access the optional header
		opt_header = &nt_headers->OptionalHeader;
		switch (opt_header->Subsystem)
		{
			case IMAGE_SUBSYSTEM_WINDOWS_GUI:
				is_gui_frontend = TRUE;
				break;

			case IMAGE_SUBSYSTEM_WINDOWS_CUI:
				is_gui_frontend = FALSE;
				break;
		}
	}
	return is_gui_frontend;
}

//============================================================
//  osd_subst_env
//============================================================
void osd_subst_env(std::string &dst, const std::string &src)
{
	TCHAR buffer[MAX_PATH];

	TCHAR *t_src = tstring_from_utf8(src.c_str());
	ExpandEnvironmentStrings(t_src, buffer, ARRAY_LENGTH(buffer));
	osd_free(t_src);
	char *cnv = utf8_from_tstring(buffer);
	dst = cnv;
	osd_free(cnv);
}

//-------------------------------------------------
//  Universal way to get module handle
//-------------------------------------------------

HMODULE WINAPI GetModuleHandleUni()
{
	MEMORY_BASIC_INFORMATION mbi;
	VirtualQuery((LPCVOID)GetModuleHandleUni, &mbi, sizeof(mbi));
	return (HMODULE)mbi.AllocationBase;
}

//-----------------------------------------------------------
//  Lazy loaded function using LoadLibrary / GetProcAddress
//-----------------------------------------------------------

lazy_loaded_function::lazy_loaded_function(const char * name, const wchar_t* dll_name)
	: lazy_loaded_function(name, &dll_name, 1)
{
}

lazy_loaded_function::lazy_loaded_function(const char * name, const wchar_t** dll_names, int dll_count)
	: m_name(name), m_module(NULL), m_initialized(false), m_pfn(nullptr)
{
	for (int i = 0; i < dll_count; i++)
		m_dll_names.push_back(std::wstring(dll_names[i]));
}

lazy_loaded_function::~lazy_loaded_function()
{
	if (m_module != nullptr)
	{
		FreeLibrary(m_module);
		m_module = nullptr;
	}
}

int lazy_loaded_function::initialize()
{
	if (m_module == nullptr)
	{
		for (int i = 0; i < m_dll_names.size(); i++)
		{
			m_module = LoadLibraryW(m_dll_names[i].c_str());
			if (m_module != NULL)
				break;
		}

		if (m_module == NULL)
		{
			osd_printf_verbose("Could not find DLL to dynamically link function %s.\n", m_name.c_str());
			return ERROR_DLL_NOT_FOUND;
		}
	}

	if (m_pfn == nullptr)
	{
		m_pfn = GetProcAddress(m_module, m_name.c_str());
		if (m_pfn == nullptr)
		{
			osd_printf_verbose("Could not find function address to dynamically link function %s.\n", m_name.c_str());
			return ERROR_NOT_FOUND;
		}
	}

	m_initialized = true;

	return 0;
}

void lazy_loaded_function::check_init()
{
	if (!m_initialized)
		fatalerror("Attempt to use function pointer for function %s prior to init!", name());
}
