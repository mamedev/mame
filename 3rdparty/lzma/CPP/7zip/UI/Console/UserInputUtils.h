// UserInputUtils.h

#ifndef __USERINPUTUTILS_H
#define __USERINPUTUTILS_H

#include "Common/StdOutStream.h"

namespace NUserAnswerMode {

enum EEnum
{
  kYes,
  kNo,
  kYesAll,
  kNoAll,
  kAutoRenameAll,
  kQuit
};
}

NUserAnswerMode::EEnum ScanUserYesNoAllQuit(CStdOutStream *outStream);
UString GetPassword(CStdOutStream *outStream);

#endif
