// OpenCallbackConsole.h

#ifndef __OPENCALLBACKCONSOLE_H
#define __OPENCALLBACKCONSOLE_H

#include "Common/StdOutStream.h"
#include "../Common/ArchiveOpenCallback.h"

class COpenCallbackConsole: public IOpenCallbackUI
{
public:
  INTERFACE_IOpenCallbackUI(;)
  
  CStdOutStream *OutStream;

  #ifndef _NO_CRYPTO
  bool PasswordIsDefined;
  bool PasswordWasAsked;
  UString Password;
  COpenCallbackConsole(): PasswordIsDefined(false), PasswordWasAsked(false) {}
  #endif
};

#endif
