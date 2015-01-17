// Common/StdInStream.cpp

#include "StdAfx.h"

#include <tchar.h>

#include "StdInStream.h"
#include "StringConvert.h"
#include "UTFConvert.h"

#ifdef _MSC_VER
// "was declared deprecated" disabling
#pragma warning(disable : 4996 )
#endif

static const char kIllegalChar = '\0';
static const char kNewLineChar = '\n';

static const char *kEOFMessage = "Unexpected end of input stream";
static const char *kReadErrorMessage  ="Error reading input stream";
static const char *kIllegalCharMessage = "Illegal character in input stream";

static LPCTSTR kFileOpenMode = TEXT("r");

extern int g_CodePage;

CStdInStream g_StdIn(stdin);

bool CStdInStream::Open(LPCTSTR fileName)
{
  Close();
  _stream = _tfopen(fileName, kFileOpenMode);
  _streamIsOpen = (_stream != 0);
  return _streamIsOpen;
}

bool CStdInStream::Close()
{
  if (!_streamIsOpen)
    return true;
  _streamIsOpen = (fclose(_stream) != 0);
  return !_streamIsOpen;
}

CStdInStream::~CStdInStream()
{
  Close();
}

AString CStdInStream::ScanStringUntilNewLine(bool allowEOF)
{
  AString s;
  for (;;)
  {
    int intChar = GetChar();
    if (intChar == EOF)
    {
      if (allowEOF)
        break;
      throw kEOFMessage;
    }
    char c = char(intChar);
    if (c == kIllegalChar)
      throw kIllegalCharMessage;
    if (c == kNewLineChar)
      break;
    s += c;
  }
  return s;
}

UString CStdInStream::ScanUStringUntilNewLine()
{
  AString s = ScanStringUntilNewLine(true);
  int codePage = g_CodePage;
  if (codePage == -1)
    codePage = CP_OEMCP;
  UString dest;
  if (codePage == CP_UTF8)
    ConvertUTF8ToUnicode(s, dest);
  else
    dest = MultiByteToUnicodeString(s, (UINT)codePage);
  return dest;
}

void CStdInStream::ReadToString(AString &resultString)
{
  resultString.Empty();
  int c;
  while ((c = GetChar()) != EOF)
    resultString += char(c);
}

bool CStdInStream::Eof()
{
  return (feof(_stream) != 0);
}

int CStdInStream::GetChar()
{
  int c = fgetc(_stream); // getc() doesn't work in BeOS?
  if (c == EOF && !Eof())
    throw kReadErrorMessage;
  return c;
}


