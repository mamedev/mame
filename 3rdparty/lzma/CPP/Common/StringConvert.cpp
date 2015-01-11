// Common/StringConvert.cpp

#include "StdAfx.h"

#include "StringConvert.h"

#ifndef _WIN32
#include <stdlib.h>
#endif

#ifdef _WIN32
UString MultiByteToUnicodeString(const AString &srcString, UINT codePage)
{
  UString resultString;
  if (!srcString.IsEmpty())
  {
    int numChars = MultiByteToWideChar(codePage, 0, srcString,
      srcString.Length(), resultString.GetBuffer(srcString.Length()),
      srcString.Length() + 1);
    if (numChars == 0)
      throw 282228;
    resultString.ReleaseBuffer(numChars);
  }
  return resultString;
}

AString UnicodeStringToMultiByte(const UString &s, UINT codePage, char defaultChar, bool &defaultCharWasUsed)
{
  AString dest;
  defaultCharWasUsed = false;
  if (!s.IsEmpty())
  {
    int numRequiredBytes = s.Length() * 2;
    BOOL defUsed;
    int numChars = WideCharToMultiByte(codePage, 0, s, s.Length(),
        dest.GetBuffer(numRequiredBytes), numRequiredBytes + 1,
        &defaultChar, &defUsed);
    defaultCharWasUsed = (defUsed != FALSE);
    if (numChars == 0)
      throw 282229;
    dest.ReleaseBuffer(numChars);
  }
  return dest;
}

AString UnicodeStringToMultiByte(const UString &srcString, UINT codePage)
{
  bool defaultCharWasUsed;
  return UnicodeStringToMultiByte(srcString, codePage, '_', defaultCharWasUsed);
}

#ifndef UNDER_CE
AString SystemStringToOemString(const CSysString &srcString)
{
  AString result;
  CharToOem(srcString, result.GetBuffer(srcString.Length() * 2));
  result.ReleaseBuffer();
  return result;
}
#endif

#else

UString MultiByteToUnicodeString(const AString &srcString, UINT codePage)
{
  UString resultString;
  for (int i = 0; i < srcString.Length(); i++)
    resultString += wchar_t(srcString[i]);
  /*
  if (!srcString.IsEmpty())
  {
    int numChars = mbstowcs(resultString.GetBuffer(srcString.Length()), srcString, srcString.Length() + 1);
    if (numChars < 0) throw "Your environment does not support UNICODE";
    resultString.ReleaseBuffer(numChars);
  }
  */
  return resultString;
}

AString UnicodeStringToMultiByte(const UString &srcString, UINT codePage)
{
  AString resultString;
  for (int i = 0; i < srcString.Length(); i++)
    resultString += char(srcString[i]);
  /*
  if (!srcString.IsEmpty())
  {
    int numRequiredBytes = srcString.Length() * 6 + 1;
    int numChars = wcstombs(resultString.GetBuffer(numRequiredBytes), srcString, numRequiredBytes);
    if (numChars < 0) throw "Your environment does not support UNICODE";
    resultString.ReleaseBuffer(numChars);
  }
  */
  return resultString;
}

#endif
