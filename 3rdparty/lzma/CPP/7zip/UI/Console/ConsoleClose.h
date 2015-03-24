// ConsoleCloseUtils.h

#ifndef __CONSOLECLOSEUTILS_H
#define __CONSOLECLOSEUTILS_H

namespace NConsoleClose {

bool TestBreakSignal();

class CCtrlHandlerSetter
{
public:
  CCtrlHandlerSetter();
  virtual ~CCtrlHandlerSetter();
};

class CCtrlBreakException
{};

void CheckCtrlBreak();

}

#endif
