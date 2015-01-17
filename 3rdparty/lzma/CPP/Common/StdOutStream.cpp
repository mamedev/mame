// Common/StdOutStream.cpp

#include "StdAfx.h"

#include <tchar.h>

#include "IntToString.h"
#include "StdOutStream.h"
#include "StringConvert.h"
#include "UTFConvert.h"

#ifdef _MSC_VER
// "was declared deprecated" disabling
#pragma warning(disable : 4996 )
#endif

static const char kNewLineChar =  '\n';

static const char *kFileOpenMode = "wt";

extern int g_CodePage;

CStdOutStream  g_StdOut(stdout);
CStdOutStream  g_StdErr(stderr);

bool CStdOutStream::Open(const char *fileName)
{
  Close();
  _stream = fopen(fileName, kFileOpenMode);
  _streamIsOpen = (_stream != 0);
  return _streamIsOpen;
}

bool CStdOutStream::Close()
{
  if (!_streamIsOpen)
    return true;
  if (fclose(_stream) != 0)
    return false;
  _stream = 0;
  _streamIsOpen = false;
  return true;
}

bool CStdOutStream::Flush()
{
  return (fflush(_stream) == 0);
}

CStdOutStream::~CStdOutStream ()
{
  Close();
}

CStdOutStream & CStdOutStream::operator<<(CStdOutStream & (*aFunction)(CStdOutStream  &))
{
  (*aFunction)(*this);
  return *this;
}

CStdOutStream & endl(CStdOutStream & outStream)
{
  return outStream << kNewLineChar;
}

CStdOutStream & CStdOutStream::operator<<(const char *s)
{
  fputs(s, _stream);
  return *this;
}

CStdOutStream & CStdOutStream::operator<<(const wchar_t *s)
{
  int codePage = g_CodePage;
  if (codePage == -1)
    codePage = CP_OEMCP;
  AString dest;
  if (codePage == CP_UTF8)
    ConvertUnicodeToUTF8(s, dest);
  else
    dest = UnicodeStringToMultiByte(s, (UINT)codePage);
  *this << (const char *)dest;
  return *this;
}

CStdOutStream & CStdOutStream::operator<<(char c)
{
  fputc(c, _stream);
  return *this;
}

CStdOutStream & CStdOutStream::operator<<(int number)
{
  char textString[32];
  ConvertInt64ToString(number, textString);
  return operator<<(textString);
}

CStdOutStream & CStdOutStream::operator<<(UInt64 number)
{
  char textString[32];
  ConvertUInt64ToString(number, textString);
  return operator<<(textString);
}
