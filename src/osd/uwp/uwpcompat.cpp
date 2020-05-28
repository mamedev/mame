// license:BSD-3-Clause
// copyright-holders:Brad Hughes, Miodrag Milanovic
//============================================================
//
//  uwpcompat.h - Universal Windows Platform compat forced includes
//
//============================================================

#include "uwpcompat.h"

#include <windows.h>
#include <cerrno>

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

	// This is only in here so callers get an error
	HMODULE WINAPI LoadLibraryExA(
		_In_ LPCSTR lpLibFileName,
		_Reserved_ HANDLE hFile,
		_In_ DWORD dwFlags
	)
	{
		SetLastError(ERROR_FILE_NOT_FOUND);
		return nullptr;
	}

	HMODULE WINAPI LoadLibraryExW(
		_In_ LPCWSTR lpLibFileName,
		_Reserved_ HANDLE hFile,
		_In_ DWORD dwFlags
	)
	{
		SetLastError(ERROR_FILE_NOT_FOUND);
		return nullptr;
	}

	DWORD WINAPI GetFileSize(
		_In_ HANDLE hFile,
		_Out_opt_ LPDWORD lpFileSizeHigh
	)
	{
		FILE_STANDARD_INFO file_info;
		GetFileInformationByHandleEx(hFile, FileStandardInfo, &file_info, sizeof(file_info));
		if(lpFileSizeHigh!=nullptr)
		{
			*lpFileSizeHigh = file_info.EndOfFile.HighPart;
		}
		return file_info.EndOfFile.LowPart;
	}
}
