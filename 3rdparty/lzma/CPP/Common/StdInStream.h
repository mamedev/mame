// Common/StdInStream.h

#ifndef __COMMON_STDINSTREAM_H
#define __COMMON_STDINSTREAM_H

#include <stdio.h>

#include "MyString.h"
#include "Types.h"

class CStdInStream
{
  bool _streamIsOpen;
  FILE *_stream;
public:
  CStdInStream(): _streamIsOpen(false) {};
  CStdInStream(FILE *stream): _streamIsOpen(false), _stream(stream) {};
  ~CStdInStream();
  bool Open(LPCTSTR fileName);
  bool Close();

  AString ScanStringUntilNewLine(bool allowEOF = false);
  void ReadToString(AString &resultString);
  UString ScanUStringUntilNewLine();

  bool Eof();
  int GetChar();
};

extern CStdInStream g_StdIn;

#endif
