// license:BSD-3-Clause
// copyright-holders:Brad Hughes, Miodrag Milanovic
//============================================================
//
//  uwpcompat.h - Universal Windows Platform compat forced includes
//
//============================================================

#include "uwpcompat.h"

#include <errno.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef interface

#include "emu.h"

extern "C" {
	
	BOOL WINAPI GetVersionEx(
		_Inout_ LPOSVERSIONINFO lpVersionInfo
	)
	{
		lpVersionInfo->dwMajorVersion = 10;
		return TRUE;
	}

	HANDLE
	WINAPI
	CreateFileW(
		_In_ LPCWSTR lpFileName,
		_In_ DWORD dwDesiredAccess,
		_In_ DWORD dwShareMode,
		_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		_In_ DWORD dwCreationDisposition,
		_In_ DWORD dwFlagsAndAttributes,
		_In_opt_ HANDLE hTemplateFile
		)
	{
		// TODO: Handle other arguments that go into last param (pCreateExParams)
		return CreateFile2((wchar_t*)lpFileName, dwDesiredAccess, dwShareMode, dwCreationDisposition, NULL);
	}

	HANDLE
	WINAPI
	CreateFileA(
		_In_ LPCSTR lpFileName,
		_In_ DWORD dwDesiredAccess,
		_In_ DWORD dwShareMode,
		_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		_In_ DWORD dwCreationDisposition,
		_In_ DWORD dwFlagsAndAttributes,
		_In_opt_ HANDLE hTemplateFile
		)
	{
		wchar_t filepath[MAX_PATH + 1];
		if (MultiByteToWideChar(CP_ACP, 0, lpFileName, strlen(lpFileName), filepath, MAX_PATH))
			return CreateFileW(filepath, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

		SetLastError(E_FAIL);
		return INVALID_HANDLE_VALUE;
	}

	DWORD WINAPI GetTickCount(void)
	{
		return osd_ticks();
	}

	HMODULE WINAPI LoadLibraryExA(
		_In_ LPCSTR lpLibFileName,
		_Reserved_ HANDLE hFile,
		_In_ DWORD dwFlags
		)
	{
		wchar_t libfile_wide[MAX_PATH + 1];
		if (MultiByteToWideChar(CP_ACP, 0, lpLibFileName, strlen(lpLibFileName), libfile_wide, MAX_PATH))
			return LoadPackagedLibrary(libfile_wide, 0);

		return nullptr;
	}

	HMODULE WINAPI LoadLibraryExW(
		_In_ LPCWSTR lpLibFileName,
		_Reserved_ HANDLE hFile,
		_In_ DWORD dwFlags
		)
	{
		return LoadPackagedLibrary(lpLibFileName, 0);
	}

	DWORD WINAPI GetFileSize(
		_In_ HANDLE hFile,
		_Out_opt_ LPDWORD lpFileSizeHigh
	)
	{
		return 0;
	}
}
