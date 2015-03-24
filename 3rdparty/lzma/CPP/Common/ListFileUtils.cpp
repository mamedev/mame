// Common/ListFileUtils.cpp

#include "StdAfx.h"

#include "MyWindows.h"
#include "../Windows/FileIO.h"

#include "ListFileUtils.h"
#include "StringConvert.h"
#include "UTFConvert.h"

static const char kQuoteChar = '\"';

static void RemoveQuote(UString &s)
{
  if (s.Length() >= 2)
    if (s[0] == kQuoteChar && s.Back() == kQuoteChar)
      s = s.Mid(1, s.Length() - 2);
}

bool ReadNamesFromListFile(CFSTR fileName, UStringVector &resultStrings, UINT codePage)
{
  NWindows::NFile::NIO::CInFile file;
  if (!file.Open(fileName))
    return false;
  UInt64 length;
  if (!file.GetLength(length))
    return false;
  if (length > ((UInt32)1 << 31))
    return false;
  AString s;
  char *p = s.GetBuffer((int)length + 1);
  UInt32 processed;
  if (!file.Read(p, (UInt32)length, processed))
    return false;
  p[(UInt32)length] = 0;
  s.ReleaseBuffer();
  file.Close();

  UString u;
  #ifdef CP_UTF8
  if (codePage == CP_UTF8)
  {
    if (!ConvertUTF8ToUnicode(s, u))
      return false;
  }
  else
  #endif
    u = MultiByteToUnicodeString(s, codePage);
  if (!u.IsEmpty())
  {
    if (u[0] == 0xFEFF)
      u.Delete(0);
  }

  UString t;
  for (int i = 0; i < u.Length(); i++)
  {
    wchar_t c = u[i];
    if (c == L'\n' || c == 0xD)
    {
      t.Trim();
      RemoveQuote(t);
      if (!t.IsEmpty())
        resultStrings.Add(t);
      t.Empty();
    }
    else
      t += c;
  }
  t.Trim();
  RemoveQuote(t);
  if (!t.IsEmpty())
    resultStrings.Add(t);
  return true;
}
