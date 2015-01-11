// PercentPrinter.h

#ifndef __PERCENT_PRINTER_H
#define __PERCENT_PRINTER_H

#include "Common/StdOutStream.h"

class CPercentPrinter
{
  UInt64 m_MinStepSize;
  UInt64 m_PrevValue;
  UInt64 m_CurValue;
  UInt64 m_Total;
  unsigned m_NumExtraChars;
public:
  CStdOutStream *OutStream;

  CPercentPrinter(UInt64 minStepSize = 1): m_MinStepSize(minStepSize),
      m_PrevValue(0), m_CurValue(0), m_Total((UInt64)(Int64)-1), m_NumExtraChars(0) {}
  void SetTotal(UInt64 total) { m_Total = total; m_PrevValue = 0; }
  void SetRatio(UInt64 doneValue) { m_CurValue = doneValue; }
  void PrintString(const char *s);
  void PrintString(const wchar_t *s);
  void PrintNewLine();
  void ClosePrint();
  void RePrintRatio();
  void PrintRatio();
};

#endif
