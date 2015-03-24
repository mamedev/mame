// Common/MemoryLock.cpp

#include "StdAfx.h"

namespace NWindows {
namespace NSecurity {

#ifndef UNDER_CE

#ifndef _UNICODE
typedef BOOL (WINAPI * OpenProcessTokenP)(HANDLE ProcessHandle, DWORD DesiredAccess, PHANDLE TokenHandle);
typedef BOOL (WINAPI * LookupPrivilegeValueP)(LPCTSTR lpSystemName, LPCTSTR lpName, PLUID  lpLuid);
typedef BOOL (WINAPI * AdjustTokenPrivilegesP)(HANDLE TokenHandle, BOOL DisableAllPrivileges,
    PTOKEN_PRIVILEGES NewState, DWORD BufferLength, PTOKEN_PRIVILEGES PreviousState,PDWORD ReturnLength);
#endif

#ifdef _UNICODE
bool EnableLockMemoryPrivilege(
#else
static bool EnableLockMemoryPrivilege2(HMODULE hModule,
#endif
bool enable)
{
  #ifndef _UNICODE
  if (hModule == NULL)
    return false;
  OpenProcessTokenP openProcessToken = (OpenProcessTokenP)GetProcAddress(hModule, "OpenProcessToken");
  LookupPrivilegeValueP lookupPrivilegeValue = (LookupPrivilegeValueP)GetProcAddress(hModule, "LookupPrivilegeValueA" );
  AdjustTokenPrivilegesP adjustTokenPrivileges = (AdjustTokenPrivilegesP)GetProcAddress(hModule, "AdjustTokenPrivileges");
  if (openProcessToken == NULL || adjustTokenPrivileges == NULL || lookupPrivilegeValue == NULL)
    return false;
  #endif

  HANDLE token;
  if (!
    #ifdef _UNICODE
    ::OpenProcessToken
    #else
    openProcessToken
    #endif
    (::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token))
    return false;
  TOKEN_PRIVILEGES tp;
  bool res = false;
  if (
    #ifdef _UNICODE
    ::LookupPrivilegeValue
    #else
    lookupPrivilegeValue
    #endif
    (NULL, SE_LOCK_MEMORY_NAME, &(tp.Privileges[0].Luid)))
  {
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Attributes = enable ? SE_PRIVILEGE_ENABLED: 0;
    if (
      #ifdef _UNICODE
      ::AdjustTokenPrivileges
      #else
      adjustTokenPrivileges
      #endif
      (token, FALSE, &tp, 0, NULL, NULL))
      res = (GetLastError() == ERROR_SUCCESS);
  }
  ::CloseHandle(token);
  return res;
}

#ifndef _UNICODE
bool EnableLockMemoryPrivilege(bool enable)
{
  HMODULE hModule = LoadLibrary(TEXT("Advapi32.dll"));
  if (hModule == NULL)
    return false;
  bool res = EnableLockMemoryPrivilege2(hModule, enable);
  ::FreeLibrary(hModule);
  return res;
}
#endif

#endif

}}
