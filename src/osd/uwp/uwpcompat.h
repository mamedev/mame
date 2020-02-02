// license:BSD-3-Clause
// copyright-holders:Brad Hughes, Miodrag Milanovic
//============================================================
//
//  uwpcompat.h - Universal Windows Platform compat forced includes
//
//============================================================

#ifndef __UWPCOMPAT_H__
#define __UWPCOMPAT_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NOGDI
#define NOGDI
#endif
#include <windows.h>
#include <cstdio>

int system(const char *command);

const char *getenv(const char *varname);

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
	);

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
	);

#ifdef UNICODE
#define CreateFile  CreateFileW
#else
#define CreateFile  CreateFileA
#endif // !UNICODE

BOOL WINAPI GetVersionEx(
	_Inout_ LPOSVERSIONINFO lpVersionInfo
);

DWORD WINAPI GetTickCount(void);

FILE *_popen(
	const char *command,
	const char *mode
	);

int _pclose(
	FILE *stream);

_Ret_maybenull_
HMODULE WINAPI LoadLibraryA(
	_In_ LPCSTR lpFileName
);

_Ret_maybenull_
HMODULE WINAPI LoadLibraryW(
	_In_ LPCWSTR lpFileName
);

#ifdef UNICODE
#define LoadLibrary  LoadLibraryW
#else
#define LoadLibrary  LoadLibraryA
#endif // !UNICODE

_Ret_maybenull_
HMODULE WINAPI GetModuleHandleA(
	_In_ LPCSTR lpModuleName
);

_Ret_maybenull_
HMODULE WINAPI GetModuleHandleW(
	_In_ LPCWSTR lpModuleName
);

#ifdef UNICODE
#define GetModuleHandle  GetModuleHandleW
#else
#define GetModuleHandle  GetModuleHandleA
#endif // !UNICODE

_Ret_maybenull_
HMODULE
WINAPI
LoadLibraryExA(
	_In_ LPCSTR lpLibFileName,
	_Reserved_ HANDLE hFile,
	_In_ DWORD dwFlags
);

_Ret_maybenull_
HMODULE
WINAPI
LoadLibraryExW(
	_In_ LPCWSTR lpLibFileName,
	_Reserved_ HANDLE hFile,
	_In_ DWORD dwFlags
);

#ifdef UNICODE
#define LoadLibraryEx  LoadLibraryExW
#else
#define LoadLibraryEx  LoadLibraryExA
#endif // !UNICODE

DWORD
WINAPI
GetFileSize(
	_In_ HANDLE hFile,
	_Out_opt_ LPDWORD lpFileSizeHigh
);

#ifdef __cplusplus
}
#endif

#endif // __UWPCOMPAT_H__


