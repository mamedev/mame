// UserInputUtils.cpp

#include "StdAfx.h"

#include "Common/StdInStream.h"
#include "Common/StringConvert.h"

#include "UserInputUtils.h"

static const char kYes = 'Y';
static const char kNo = 'N';
static const char kYesAll = 'A';
static const char kNoAll = 'S';
static const char kAutoRenameAll = 'U';
static const char kQuit = 'Q';

static const char *kFirstQuestionMessage = "?\n";
static const char *kHelpQuestionMessage =
  "(Y)es / (N)o / (A)lways / (S)kip all / A(u)to rename all / (Q)uit? ";

// return true if pressed Quite;

NUserAnswerMode::EEnum ScanUserYesNoAllQuit(CStdOutStream *outStream)
{
  (*outStream) << kFirstQuestionMessage;
  for (;;)
  {
    (*outStream) << kHelpQuestionMessage;
    outStream->Flush();
    AString scannedString = g_StdIn.ScanStringUntilNewLine();
    scannedString.Trim();
    if (!scannedString.IsEmpty())
      switch(
        ::MyCharUpper(
        #ifdef UNDER_CE
        (wchar_t)
        #endif
        scannedString[0]))
      {
        case kYes:
          return NUserAnswerMode::kYes;
        case kNo:
          return NUserAnswerMode::kNo;
        case kYesAll:
          return NUserAnswerMode::kYesAll;
        case kNoAll:
          return NUserAnswerMode::kNoAll;
        case kAutoRenameAll:
          return NUserAnswerMode::kAutoRenameAll;
        case kQuit:
          return NUserAnswerMode::kQuit;
      }
  }
}

#ifdef _WIN32
#ifndef UNDER_CE
#define MY_DISABLE_ECHO
#endif
#endif

UString GetPassword(CStdOutStream *outStream)
{
  (*outStream) << "\nEnter password"
      #ifdef MY_DISABLE_ECHO
      " (will not be echoed)"
      #endif
      ":";
  outStream->Flush();

  #ifdef MY_DISABLE_ECHO
  HANDLE console = GetStdHandle(STD_INPUT_HANDLE);
  bool wasChanged = false;
  DWORD mode = 0;
  if (console != INVALID_HANDLE_VALUE && console != 0)
    if (GetConsoleMode(console, &mode))
      wasChanged = (SetConsoleMode(console, mode & ~ENABLE_ECHO_INPUT) != 0);
  UString res = g_StdIn.ScanUStringUntilNewLine();
  if (wasChanged)
    SetConsoleMode(console, mode);
  (*outStream) << "\n";
  outStream->Flush();
  return res;
  #else
  return g_StdIn.ScanUStringUntilNewLine();
  #endif
}
