// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  winutil.cpp - Win32 OSD core utility functions
//
//============================================================

// standard windows headers
#include <windows.h>
#include <direct.h>

// MAME headers
#include "emu.h"

// MAMEOS headers
#include "winutil.h"
#include "strconv.h"
#include "timeconv.h"


//============================================================
//  win_attributes_to_entry_type
//============================================================

osd::directory::entry::entry_type win_attributes_to_entry_type(DWORD attributes)
{
	if (attributes == 0xFFFFFFFF)
		return osd::directory::entry::entry_type::NONE;
	else if (attributes & FILE_ATTRIBUTE_DIRECTORY)
		return osd::directory::entry::entry_type::DIR;
	else
		return osd::directory::entry::entry_type::FILE;
}



//============================================================
//  win_time_point_from_filetime
//============================================================

std::chrono::system_clock::time_point win_time_point_from_filetime(LPFILETIME file_time)
{
	auto converted_file_time = util::ntfs_duration_from_filetime(file_time->dwHighDateTime, file_time->dwLowDateTime);
	return util::system_clock_time_point_from_ntfs_duration(converted_file_time);
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

	osd::text::tstring t_src = osd::text::to_tstring(src);
#if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
	ExpandEnvironmentStrings(t_src.c_str(), buffer, ARRAY_LENGTH(buffer));
#else
	wcsncpy(buffer, t_src.c_str(), ARRAY_LENGTH(buffer));
#endif
	osd::text::from_tstring(dst, buffer);
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
