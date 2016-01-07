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

// MAMEOS headers
#include "winutil.h"
#include "strconv.h"

//============================================================
//  win_error_to_file_error
//============================================================

file_error win_error_to_file_error(DWORD error)
{
	file_error filerr;

	// convert a Windows error to a file_error
	switch (error)
	{
		case ERROR_SUCCESS:
			filerr = FILERR_NONE;
			break;

		case ERROR_OUTOFMEMORY:
			filerr = FILERR_OUT_OF_MEMORY;
			break;

		case ERROR_FILE_NOT_FOUND:
		case ERROR_FILENAME_EXCED_RANGE:
		case ERROR_PATH_NOT_FOUND:
			filerr = FILERR_NOT_FOUND;
			break;

		case ERROR_ACCESS_DENIED:
			filerr = FILERR_ACCESS_DENIED;
			break;

		case ERROR_SHARING_VIOLATION:
			filerr = FILERR_ALREADY_OPEN;
			break;

		default:
			filerr = FILERR_FAILURE;
			break;
	}
	return filerr;
}



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
void osd_subst_env(char **dst, const char *src)
{
	TCHAR buffer[MAX_PATH];

	TCHAR *t_src = tstring_from_utf8(src);
	ExpandEnvironmentStrings(t_src, buffer, ARRAY_LENGTH(buffer));
	*dst = utf8_from_tstring(buffer);
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
