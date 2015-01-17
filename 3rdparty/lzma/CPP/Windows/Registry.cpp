// Windows/Registry.cpp

#include "StdAfx.h"

#ifndef _UNICODE
#include "Common/StringConvert.h"
#endif
#include "Windows/Registry.h"

#ifndef _UNICODE
extern bool g_IsNT;
#endif

namespace NWindows {
namespace NRegistry {

#define MYASSERT(expr) // _ASSERTE(expr)

LONG CKey::Create(HKEY parentKey, LPCTSTR keyName,
    LPTSTR keyClass, DWORD options, REGSAM accessMask,
    LPSECURITY_ATTRIBUTES securityAttributes, LPDWORD disposition)
{
  MYASSERT(parentKey != NULL);
  DWORD dispositionReal;
  HKEY key = NULL;
  LONG res = RegCreateKeyEx(parentKey, keyName, 0, keyClass,
      options, accessMask, securityAttributes, &key, &dispositionReal);
  if (disposition != NULL)
    *disposition = dispositionReal;
  if (res == ERROR_SUCCESS)
  {
    res = Close();
    _object = key;
  }
  return res;
}

LONG CKey::Open(HKEY parentKey, LPCTSTR keyName, REGSAM accessMask)
{
  MYASSERT(parentKey != NULL);
  HKEY key = NULL;
  LONG res = RegOpenKeyEx(parentKey, keyName, 0, accessMask, &key);
  if (res == ERROR_SUCCESS)
  {
    res = Close();
    MYASSERT(res == ERROR_SUCCESS);
    _object = key;
  }
  return res;
}

LONG CKey::Close()
{
  LONG res = ERROR_SUCCESS;
  if (_object != NULL)
  {
    res = RegCloseKey(_object);
    _object = NULL;
  }
  return res;
}

// win95, win98: deletes sunkey and all its subkeys
// winNT to be deleted must not have subkeys
LONG CKey::DeleteSubKey(LPCTSTR subKeyName)
{
  MYASSERT(_object != NULL);
  return RegDeleteKey(_object, subKeyName);
}

LONG CKey::RecurseDeleteKey(LPCTSTR subKeyName)
{
  CKey key;
  LONG res = key.Open(_object, subKeyName, KEY_READ | KEY_WRITE);
  if (res != ERROR_SUCCESS)
    return res;
  FILETIME fileTime;
  const UInt32 kBufferSize = MAX_PATH + 1; // 256 in ATL
  DWORD size = kBufferSize;
  TCHAR buffer[kBufferSize];
  while (RegEnumKeyEx(key._object, 0, buffer, &size, NULL, NULL, NULL, &fileTime) == ERROR_SUCCESS)
  {
    res = key.RecurseDeleteKey(buffer);
    if (res != ERROR_SUCCESS)
      return res;
    size = kBufferSize;
  }
  key.Close();
  return DeleteSubKey(subKeyName);
}


/////////////////////////
// Value Functions

static inline UInt32 BoolToUINT32(bool value) {  return (value ? 1: 0); }
static inline bool UINT32ToBool(UInt32 value) {  return (value != 0); }


LONG CKey::DeleteValue(LPCTSTR name)
{
  MYASSERT(_object != NULL);
  return ::RegDeleteValue(_object, name);
}

#ifndef _UNICODE
LONG CKey::DeleteValue(LPCWSTR name)
{
  MYASSERT(_object != NULL);
  if (g_IsNT)
    return ::RegDeleteValueW(_object, name);
  return DeleteValue(name == 0 ? 0 : (LPCSTR)GetSystemString(name));
}
#endif

LONG CKey::SetValue(LPCTSTR name, UInt32 value)
{
  MYASSERT(_object != NULL);
  return RegSetValueEx(_object, name, NULL, REG_DWORD,
      (BYTE * const)&value, sizeof(UInt32));
}

LONG CKey::SetValue(LPCTSTR name, bool value)
{
  return SetValue(name, BoolToUINT32(value));
}

LONG CKey::SetValue(LPCTSTR name, LPCTSTR value)
{
  MYASSERT(value != NULL);
  MYASSERT(_object != NULL);
  return RegSetValueEx(_object, name, NULL, REG_SZ,
      (const BYTE * )value, (lstrlen(value) + 1) * sizeof(TCHAR));
}

/*
LONG CKey::SetValue(LPCTSTR name, const CSysString &value)
{
  MYASSERT(value != NULL);
  MYASSERT(_object != NULL);
  return RegSetValueEx(_object, name, NULL, REG_SZ,
      (const BYTE *)(const TCHAR *)value, (value.Length() + 1) * sizeof(TCHAR));
}
*/

#ifndef _UNICODE

LONG CKey::SetValue(LPCWSTR name, LPCWSTR value)
{
  MYASSERT(value != NULL);
  MYASSERT(_object != NULL);
  if (g_IsNT)
    return RegSetValueExW(_object, name, NULL, REG_SZ,
      (const BYTE * )value, (DWORD)((wcslen(value) + 1) * sizeof(wchar_t)));
  return SetValue(name == 0 ? 0 : (LPCSTR)GetSystemString(name),
    value == 0 ? 0 : (LPCSTR)GetSystemString(value));
}

#endif


LONG CKey::SetValue(LPCTSTR name, const void *value, UInt32 size)
{
  MYASSERT(value != NULL);
  MYASSERT(_object != NULL);
  return RegSetValueEx(_object, name, NULL, REG_BINARY,
      (const BYTE *)value, size);
}

LONG SetValue(HKEY parentKey, LPCTSTR keyName, LPCTSTR valueName, LPCTSTR value)
{
  MYASSERT(value != NULL);
  CKey key;
  LONG res = key.Create(parentKey, keyName);
  if (res == ERROR_SUCCESS)
    res = key.SetValue(valueName, value);
  return res;
}

LONG CKey::SetKeyValue(LPCTSTR keyName, LPCTSTR valueName, LPCTSTR value)
{
  MYASSERT(value != NULL);
  CKey key;
  LONG res = key.Create(_object, keyName);
  if (res == ERROR_SUCCESS)
    res = key.SetValue(valueName, value);
  return res;
}

LONG CKey::QueryValue(LPCTSTR name, UInt32 &value)
{
  DWORD type = NULL;
  DWORD count = sizeof(DWORD);
  LONG res = RegQueryValueEx(_object, (LPTSTR)name, NULL, &type,
    (LPBYTE)&value, &count);
  MYASSERT((res!=ERROR_SUCCESS) || (type == REG_DWORD));
  MYASSERT((res!=ERROR_SUCCESS) || (count == sizeof(UInt32)));
  return res;
}

LONG CKey::QueryValue(LPCTSTR name, bool &value)
{
  UInt32 uintValue = BoolToUINT32(value);
  LONG res = QueryValue(name, uintValue);
  value = UINT32ToBool(uintValue);
  return res;
}

LONG CKey::GetValue_IfOk(LPCTSTR name, UInt32 &value)
{
  UInt32 newVal;
  LONG res = QueryValue(name, newVal);
  if (res == ERROR_SUCCESS)
    value = newVal;
  return res;
}

LONG CKey::GetValue_IfOk(LPCTSTR name, bool &value)
{
  bool newVal;
  LONG res = QueryValue(name, newVal);
  if (res == ERROR_SUCCESS)
    value = newVal;
  return res;
}

LONG CKey::QueryValue(LPCTSTR name, LPTSTR value, UInt32 &count)
{
  MYASSERT(count != NULL);
  DWORD type = NULL;
  LONG res = RegQueryValueEx(_object, (LPTSTR)name, NULL, &type, (LPBYTE)value, (DWORD *)&count);
  MYASSERT((res!=ERROR_SUCCESS) || (type == REG_SZ) || (type == REG_MULTI_SZ) || (type == REG_EXPAND_SZ));
  return res;
}

LONG CKey::QueryValue(LPCTSTR name, CSysString &value)
{
  value.Empty();
  DWORD type = NULL;
  UInt32 currentSize = 0;
  LONG res = RegQueryValueEx(_object, (LPTSTR)name, NULL, &type, NULL, (DWORD *)&currentSize);
  if (res != ERROR_SUCCESS && res != ERROR_MORE_DATA)
    return res;
  res = QueryValue(name, value.GetBuffer(currentSize), currentSize);
  value.ReleaseBuffer();
  return res;
}

#ifndef _UNICODE
LONG CKey::QueryValue(LPCWSTR name, LPWSTR value, UInt32 &count)
{
  MYASSERT(count != NULL);
  DWORD type = NULL;
  LONG res = RegQueryValueExW(_object, name, NULL, &type, (LPBYTE)value, (DWORD *)&count);
  MYASSERT((res!=ERROR_SUCCESS) || (type == REG_SZ) || (type == REG_MULTI_SZ) || (type == REG_EXPAND_SZ));
  return res;
}
LONG CKey::QueryValue(LPCWSTR name, UString &value)
{
  value.Empty();
  DWORD type = NULL;
  UInt32 currentSize = 0;

  LONG res;
  if (g_IsNT)
  {
    res = RegQueryValueExW(_object, name, NULL, &type, NULL, (DWORD *)&currentSize);
    if (res != ERROR_SUCCESS && res != ERROR_MORE_DATA)
      return res;
    res = QueryValue(name, value.GetBuffer(currentSize), currentSize);
    value.ReleaseBuffer();
  }
  else
  {
    AString vTemp;
    res = QueryValue(name == 0 ? 0 : (LPCSTR)GetSystemString(name), vTemp);
    value = GetUnicodeString(vTemp);
  }
  return res;
}
#endif

LONG CKey::QueryValue(LPCTSTR name, void *value, UInt32 &count)
{
  DWORD type = NULL;
  LONG res = RegQueryValueEx(_object, (LPTSTR)name, NULL, &type, (LPBYTE)value, (DWORD *)&count);
  MYASSERT((res!=ERROR_SUCCESS) || (type == REG_BINARY));
  return res;
}


LONG CKey::QueryValue(LPCTSTR name, CByteBuffer &value, UInt32 &dataSize)
{
  DWORD type = NULL;
  dataSize = 0;
  LONG res = RegQueryValueEx(_object, (LPTSTR)name, NULL, &type, NULL, (DWORD *)&dataSize);
  if (res != ERROR_SUCCESS && res != ERROR_MORE_DATA)
    return res;
  value.SetCapacity(dataSize);
  return QueryValue(name, (BYTE *)value, dataSize);
}

LONG CKey::EnumKeys(CSysStringVector &keyNames)
{
  keyNames.Clear();
  CSysString keyName;
  for (UInt32 index = 0; ; index++)
  {
    const UInt32 kBufferSize = MAX_PATH + 1; // 256 in ATL
    FILETIME lastWriteTime;
    UInt32 nameSize = kBufferSize;
    LONG result = ::RegEnumKeyEx(_object, index, keyName.GetBuffer(kBufferSize),
        (DWORD *)&nameSize, NULL, NULL, NULL, &lastWriteTime);
    keyName.ReleaseBuffer();
    if (result == ERROR_NO_MORE_ITEMS)
      break;
    if (result != ERROR_SUCCESS)
      return result;
    keyNames.Add(keyName);
  }
  return ERROR_SUCCESS;
}

LONG CKey::SetValue_Strings(LPCTSTR valueName, const UStringVector &strings)
{
  UInt32 numChars = 0;
  int i;
  for (i = 0; i < strings.Size(); i++)
    numChars += strings[i].Length() + 1;
  CBuffer<wchar_t> buffer;
  buffer.SetCapacity(numChars);
  int pos = 0;
  for (i = 0; i < strings.Size(); i++)
  {
    const UString &s = strings[i];
    MyStringCopy((wchar_t *)buffer + pos, (const wchar_t *)s);
    pos += s.Length() + 1;
  }
  return SetValue(valueName, buffer, numChars * sizeof(wchar_t));
}

LONG CKey::GetValue_Strings(LPCTSTR valueName, UStringVector &strings)
{
  strings.Clear();
  CByteBuffer buffer;
  UInt32 dataSize;
  LONG res = QueryValue(valueName, buffer, dataSize);
  if (res != ERROR_SUCCESS)
    return res;
  if (dataSize % sizeof(wchar_t) != 0)
    return E_FAIL;
  const wchar_t *data = (const wchar_t *)(const Byte  *)buffer;
  int numChars = dataSize / sizeof(wchar_t);
  UString s;
  for (int i = 0; i < numChars; i++)
  {
    wchar_t c = data[i];
    if (c == 0)
    {
      strings.Add(s);
      s.Empty();
    }
    else
      s += c;
  }
  return res;
}

}}
