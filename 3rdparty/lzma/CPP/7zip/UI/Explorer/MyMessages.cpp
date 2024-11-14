// MyMessages.cpp

#include "StdAfx.h"

#include "MyMessages.h"

#include "../../../Windows/ErrorMsg.h"
#include "../../../Windows/ResourceString.h"

#include "../FileManager/LangUtils.h"

using namespace NWindows;

void ShowErrorMessage(HWND window, LPCWSTR message)
{
  ::MessageBoxW(window, message, L"7-Zip", MB_OK | MB_ICONSTOP);
}

void ShowErrorMessageHwndRes(HWND window, UINT resID)
{
  UString s = LangString(resID);
  if (s.IsEmpty())
    s.Add_UInt32(resID);
  ShowErrorMessage(window, s);
}

void ShowErrorMessageRes(UINT resID)
{
  ShowErrorMessageHwndRes(NULL, resID);
}

static void ShowErrorMessageDWORD(HWND window, DWORD errorCode)
{
  ShowErrorMessage(window, NError::MyFormatMessage(errorCode));
}

void ShowLastErrorMessage(HWND window)
{
  ShowErrorMessageDWORD(window, ::GetLastError());
}
