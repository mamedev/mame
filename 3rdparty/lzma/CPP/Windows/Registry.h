// Windows/Registry.h

#ifndef __WINDOWS_REGISTRY_H
#define __WINDOWS_REGISTRY_H

#include "Common/Buffer.h"
#include "Common/MyString.h"
#include "Common/Types.h"

namespace NWindows {
namespace NRegistry {

LONG SetValue(HKEY parentKey, LPCTSTR keyName, LPCTSTR valueName, LPCTSTR value);

class CKey
{
  HKEY _object;
public:
  CKey(): _object(NULL) {}
  ~CKey() { Close(); }

  operator HKEY() const { return _object; }
  void Attach(HKEY key) { _object = key; }
  HKEY Detach()
  {
    HKEY key = _object;
    _object = NULL;
    return key;
  }

  LONG Create(HKEY parentKey, LPCTSTR keyName,
      LPTSTR keyClass = REG_NONE, DWORD options = REG_OPTION_NON_VOLATILE,
      REGSAM accessMask = KEY_ALL_ACCESS,
      LPSECURITY_ATTRIBUTES securityAttributes = NULL,
      LPDWORD disposition = NULL);
  LONG Open(HKEY parentKey, LPCTSTR keyName, REGSAM accessMask = KEY_ALL_ACCESS);

  LONG Close();

  LONG DeleteSubKey(LPCTSTR subKeyName);
  LONG RecurseDeleteKey(LPCTSTR subKeyName);

  LONG DeleteValue(LPCTSTR name);
  #ifndef _UNICODE
  LONG DeleteValue(LPCWSTR name);
  #endif

  LONG SetValue(LPCTSTR valueName, UInt32 value);
  LONG SetValue(LPCTSTR valueName, bool value);
  LONG SetValue(LPCTSTR valueName, LPCTSTR value);
  // LONG SetValue(LPCTSTR valueName, const CSysString &value);
  #ifndef _UNICODE
  LONG SetValue(LPCWSTR name, LPCWSTR value);
  // LONG SetValue(LPCWSTR name, const UString &value);
  #endif

  LONG SetValue(LPCTSTR name, const void *value, UInt32 size);

  LONG SetValue_Strings(LPCTSTR valueName, const UStringVector &strings);
  LONG GetValue_Strings(LPCTSTR valueName, UStringVector &strings);

  LONG SetKeyValue(LPCTSTR keyName, LPCTSTR valueName, LPCTSTR value);

  LONG QueryValue(LPCTSTR name, UInt32 &value);
  LONG QueryValue(LPCTSTR name, bool &value);
  LONG QueryValue(LPCTSTR name, LPTSTR value, UInt32 &dataSize);
  LONG QueryValue(LPCTSTR name, CSysString &value);

  LONG GetValue_IfOk(LPCTSTR name, UInt32 &value);
  LONG GetValue_IfOk(LPCTSTR name, bool &value);

  #ifndef _UNICODE
  LONG QueryValue(LPCWSTR name, LPWSTR value, UInt32 &dataSize);
  LONG QueryValue(LPCWSTR name, UString &value);
  #endif

  LONG QueryValue(LPCTSTR name, void *value, UInt32 &dataSize);
  LONG QueryValue(LPCTSTR name, CByteBuffer &value, UInt32 &dataSize);

  LONG EnumKeys(CSysStringVector &keyNames);
};

}}

#endif
