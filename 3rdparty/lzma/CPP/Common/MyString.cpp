// Common/MyString.cpp

#include "StdAfx.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <ctype.h>
#endif

#ifndef _UNICODE
#include "StringConvert.h"
#endif

#include "MyString.h"

const char* MyStringGetNextCharPointer(const char *p)
{
  #if defined(_WIN32) && !defined(UNDER_CE)
  return CharNextA(p);
  #else
  return p + 1;
  #endif
}

int FindCharPosInString(const char *s, char c)
{
  for (const char *p = s;;)
  {
    if (*p == c)
      return (int)(p - s);
    if (*p == 0)
      return -1;
    p = MyStringGetNextCharPointer(p);
  }
}

int FindCharPosInString(const wchar_t *s, wchar_t c)
{
  for (const wchar_t *p = s;; p++)
  {
    if (*p == c)
      return (int)(p - s);
    if (*p == 0)
      return -1;
  }
}

#ifdef _WIN32

#ifdef _UNICODE

wchar_t MyCharUpper(wchar_t c)
{
  return (wchar_t)(unsigned int)(UINT_PTR)CharUpperW((LPWSTR)(UINT_PTR)(unsigned int)c);
}

/*
wchar_t MyCharLower(wchar_t c)
{
  return (wchar_t)(unsigned int)(UINT_PTR)CharLowerW((LPWSTR)(UINT_PTR)(unsigned int)c);
}

char MyCharLower(char c)
#ifdef UNDER_CE
  { return (char)MyCharLower((wchar_t)c); }
#else
  { return (char)(unsigned int)(UINT_PTR)CharLowerA((LPSTR)(UINT_PTR)(unsigned int)(unsigned char)c); }
#endif
*/

wchar_t * MyStringUpper(wchar_t *s) { return CharUpperW(s); }
wchar_t * MyStringLower(wchar_t *s) { return CharLowerW(s); }

// for WinCE - FString - char
const char *MyStringGetPrevCharPointer(const char * /* base */, const char *p)
{
  return p - 1;
}

#else

const char * MyStringGetPrevCharPointer(const char *base, const char *p) { return CharPrevA(base, p); }
char * MyStringUpper(char *s) { return CharUpperA(s); }
char * MyStringLower(char *s) { return CharLowerA(s); }

wchar_t MyCharUpper(wchar_t c)
{
  if (c == 0)
    return 0;
  wchar_t *res = CharUpperW((LPWSTR)(UINT_PTR)(unsigned int)c);
  if (res != 0 || ::GetLastError() != ERROR_CALL_NOT_IMPLEMENTED)
    return (wchar_t)(unsigned int)(UINT_PTR)res;
  const int kBufferSize = 4;
  char s[kBufferSize + 1];
  int numChars = ::WideCharToMultiByte(CP_ACP, 0, &c, 1, s, kBufferSize, 0, 0);
  if (numChars == 0 || numChars > kBufferSize)
    return c;
  s[numChars] = 0;
  ::CharUpperA(s);
  ::MultiByteToWideChar(CP_ACP, 0, s, numChars, &c, 1);
  return c;
}

wchar_t MyCharLower(wchar_t c)
{
  if (c == 0)
    return 0;
  wchar_t *res = CharLowerW((LPWSTR)(UINT_PTR)(unsigned int)c);
  if (res != 0 || ::GetLastError() != ERROR_CALL_NOT_IMPLEMENTED)
    return (wchar_t)(unsigned int)(UINT_PTR)res;
  const int kBufferSize = 4;
  char s[kBufferSize + 1];
  int numChars = ::WideCharToMultiByte(CP_ACP, 0, &c, 1, s, kBufferSize, 0, 0);
  if (numChars == 0 || numChars > kBufferSize)
    return c;
  s[numChars] = 0;
  ::CharLowerA(s);
  ::MultiByteToWideChar(CP_ACP, 0, s, numChars, &c, 1);
  return c;
}

wchar_t * MyStringUpper(wchar_t *s)
{
  if (s == 0)
    return 0;
  wchar_t *res = CharUpperW(s);
  if (res != 0 || ::GetLastError() != ERROR_CALL_NOT_IMPLEMENTED)
    return res;
  AString a = UnicodeStringToMultiByte(s);
  a.MakeUpper();
  MyStringCopy(s, (const wchar_t *)MultiByteToUnicodeString(a));
  return s;
}

wchar_t * MyStringLower(wchar_t *s)
{
  if (s == 0)
    return 0;
  wchar_t *res = CharLowerW(s);
  if (res != 0 || ::GetLastError() != ERROR_CALL_NOT_IMPLEMENTED)
    return res;
  AString a = UnicodeStringToMultiByte(s);
  a.MakeLower();
  MyStringCopy(s, (const wchar_t *)MultiByteToUnicodeString(a));
  return s;
}

#endif

#else

wchar_t MyCharUpper(wchar_t c)
{
  return toupper(c);
}

wchar_t * MyStringUpper(wchar_t *s)
{
  if (s == 0)
    return 0;
  for (wchar_t *p = s; *p != 0; p++)
    *p = MyCharUpper(*p);
  return s;
}

/*
int MyStringCollateNoCase(const wchar_t *s1, const wchar_t *s2)
{
  for (;;)
  {
    wchar_t c1 = *s1++;
    wchar_t c2 = *s2++;
    wchar_t u1 = MyCharUpper(c1);
    wchar_t u2 = MyCharUpper(c2);

    if (u1 < u2) return -1;
    if (u1 > u2) return 1;
    if (u1 == 0) return 0;
  }
}
*/

#endif

int MyStringCompare(const char *s1, const char *s2)
{
  for (;;)
  {
    unsigned char c1 = (unsigned char)*s1++;
    unsigned char c2 = (unsigned char)*s2++;
    if (c1 < c2) return -1;
    if (c1 > c2) return 1;
    if (c1 == 0) return 0;
  }
}

int MyStringCompare(const wchar_t *s1, const wchar_t *s2)
{
  for (;;)
  {
    wchar_t c1 = *s1++;
    wchar_t c2 = *s2++;
    if (c1 < c2) return -1;
    if (c1 > c2) return 1;
    if (c1 == 0) return 0;
  }
}

int MyStringCompareNoCase(const wchar_t *s1, const wchar_t *s2)
{
  for (;;)
  {
    wchar_t c1 = *s1++;
    wchar_t c2 = *s2++;
    if (c1 != c2)
    {
      wchar_t u1 = MyCharUpper(c1);
      wchar_t u2 = MyCharUpper(c2);
      if (u1 < u2) return -1;
      if (u1 > u2) return 1;
    }
    if (c1 == 0) return 0;
  }
}

UString MultiByteToUnicodeString(const AString &srcString, UINT codePage);
AString UnicodeStringToMultiByte(const UString &srcString, UINT codePage);

int MyStringCompareNoCase(const char *s1, const char *s2)
{
  return MyStringCompareNoCase(MultiByteToUnicodeString(s1, CP_ACP), MultiByteToUnicodeString(s2, CP_ACP));
}

static inline UINT GetCurrentCodePage()
{
  #if defined(UNDER_CE) || !defined(defined)
  return CP_ACP;
  #else
  return ::AreFileApisANSI() ? CP_ACP : CP_OEMCP;
  #endif
}

#ifdef USE_UNICODE_FSTRING

AString fs2fas(CFSTR s)
{
  return UnicodeStringToMultiByte(s, GetCurrentCodePage());
}

FString fas2fs(const AString &s)
{
  return MultiByteToUnicodeString(s, GetCurrentCodePage());
}

#else

UString fs2us(const FString &s)
{
  return MultiByteToUnicodeString((AString)s, GetCurrentCodePage());
}

FString us2fs(const wchar_t *s)
{
  return UnicodeStringToMultiByte(s, GetCurrentCodePage());
}

#endif
