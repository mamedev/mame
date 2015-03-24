// UpdateCallbackConsole.h

#ifndef __UPDATE_CALLBACK_CONSOLE_H
#define __UPDATE_CALLBACK_CONSOLE_H

#include "Common/StdOutStream.h"

#include "../Common/Update.h"

#include "PercentPrinter.h"

class CUpdateCallbackConsole: public IUpdateCallbackUI2
{
  CPercentPrinter m_PercentPrinter;
  bool m_NeedBeClosed;
  bool m_NeedNewLine;

  bool m_WarningsMode;

  CStdOutStream *OutStream;
public:
  bool EnablePercents;
  bool StdOutMode;

  #ifndef _NO_CRYPTO
  bool PasswordIsDefined;
  UString Password;
  bool AskPassword;
  #endif

  CUpdateCallbackConsole():
      m_PercentPrinter(1 << 16),
      #ifndef _NO_CRYPTO
      PasswordIsDefined(false),
      AskPassword(false),
      #endif
      StdOutMode(false),
      EnablePercents(true),
      m_WarningsMode(false)
      {}
  
  ~CUpdateCallbackConsole() { Finilize(); }
  void Init(CStdOutStream *outStream)
  {
    m_NeedBeClosed = false;
    m_NeedNewLine = false;
    FailedFiles.Clear();
    FailedCodes.Clear();
    OutStream = outStream;
    m_PercentPrinter.OutStream = outStream;
  }

  INTERFACE_IUpdateCallbackUI2(;)

  UStringVector FailedFiles;
  CRecordVector<HRESULT> FailedCodes;

  UStringVector CantFindFiles;
  CRecordVector<HRESULT> CantFindCodes;
};

#endif
