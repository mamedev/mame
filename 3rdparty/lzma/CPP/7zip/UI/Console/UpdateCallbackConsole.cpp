// UpdateCallbackConsole.cpp

#include "StdAfx.h"

#include "UpdateCallbackConsole.h"

#include "Windows/Error.h"
#ifndef _7ZIP_ST
#include "Windows/Synchronization.h"
#endif

#include "ConsoleClose.h"
#include "UserInputUtils.h"

using namespace NWindows;

#ifndef _7ZIP_ST
static NSynchronization::CCriticalSection g_CriticalSection;
#define MT_LOCK NSynchronization::CCriticalSectionLock lock(g_CriticalSection);
#else
#define MT_LOCK
#endif

static const wchar_t *kEmptyFileAlias = L"[Content]";

static const char *kCreatingArchiveMessage = "Creating archive ";
static const char *kUpdatingArchiveMessage = "Updating archive ";
static const char *kScanningMessage = "Scanning";


HRESULT CUpdateCallbackConsole::OpenResult(const wchar_t *name, HRESULT result)
{
  (*OutStream) << endl;
  if (result != S_OK)
    (*OutStream) << "Error: " << name << " is not supported archive" << endl;
  return S_OK;
}

HRESULT CUpdateCallbackConsole::StartScanning()
{
  (*OutStream) << kScanningMessage;
  return S_OK;
}

HRESULT CUpdateCallbackConsole::ScanProgress(UInt64 /* numFolders */, UInt64 /* numFiles */, const wchar_t * /* path */)
{
  return CheckBreak();
}

HRESULT CUpdateCallbackConsole::CanNotFindError(const wchar_t *name, DWORD systemError)
{
  CantFindFiles.Add(name);
  CantFindCodes.Add(systemError);
  // m_PercentPrinter.ClosePrint();
  if (!m_WarningsMode)
  {
    (*OutStream) << endl << endl;
    m_PercentPrinter.PrintNewLine();
    m_WarningsMode = true;
  }
  m_PercentPrinter.PrintString(name);
  m_PercentPrinter.PrintString(":  WARNING: ");
  m_PercentPrinter.PrintString(NError::MyFormatMessageW(systemError));
  m_PercentPrinter.PrintNewLine();
  return S_OK;
}

HRESULT CUpdateCallbackConsole::FinishScanning()
{
  (*OutStream) << endl << endl;
  return S_OK;
}

HRESULT CUpdateCallbackConsole::StartArchive(const wchar_t *name, bool updating)
{
  if(updating)
    (*OutStream) << kUpdatingArchiveMessage;
  else
    (*OutStream) << kCreatingArchiveMessage;
  if (name != 0)
    (*OutStream) << name;
  else
    (*OutStream) << "StdOut";
  (*OutStream) << endl << endl;
  return S_OK;
}

HRESULT CUpdateCallbackConsole::FinishArchive()
{
  (*OutStream) << endl;
  return S_OK;
}

HRESULT CUpdateCallbackConsole::CheckBreak()
{
  if (NConsoleClose::TestBreakSignal())
    return E_ABORT;
  return S_OK;
}

HRESULT CUpdateCallbackConsole::Finilize()
{
  MT_LOCK
  if (m_NeedBeClosed)
  {
    if (EnablePercents)
    {
      m_PercentPrinter.ClosePrint();
    }
    if (!StdOutMode && m_NeedNewLine)
    {
      m_PercentPrinter.PrintNewLine();
      m_NeedNewLine = false;
    }
    m_NeedBeClosed = false;
  }
  return S_OK;
}

HRESULT CUpdateCallbackConsole::SetNumFiles(UInt64 /* numFiles */)
{
  return S_OK;
}

HRESULT CUpdateCallbackConsole::SetTotal(UInt64 size)
{
  MT_LOCK
  if (EnablePercents)
    m_PercentPrinter.SetTotal(size);
  return S_OK;
}

HRESULT CUpdateCallbackConsole::SetCompleted(const UInt64 *completeValue)
{
  MT_LOCK
  if (completeValue != NULL)
  {
    if (EnablePercents)
    {
      m_PercentPrinter.SetRatio(*completeValue);
      m_PercentPrinter.PrintRatio();
      m_NeedBeClosed = true;
    }
  }
  if (NConsoleClose::TestBreakSignal())
    return E_ABORT;
  return S_OK;
}

HRESULT CUpdateCallbackConsole::SetRatioInfo(const UInt64 * /* inSize */, const UInt64 * /* outSize */)
{
  if (NConsoleClose::TestBreakSignal())
    return E_ABORT;
  return S_OK;
}

HRESULT CUpdateCallbackConsole::GetStream(const wchar_t *name, bool isAnti)
{
  MT_LOCK
  if (StdOutMode)
    return S_OK;
  if(isAnti)
    m_PercentPrinter.PrintString("Anti item    ");
  else
    m_PercentPrinter.PrintString("Compressing  ");
  if (name[0] == 0)
    name = kEmptyFileAlias;
  m_PercentPrinter.PrintString(name);
  if (EnablePercents)
    m_PercentPrinter.RePrintRatio();
  return S_OK;
}

HRESULT CUpdateCallbackConsole::OpenFileError(const wchar_t *name, DWORD systemError)
{
  MT_LOCK
  FailedCodes.Add(systemError);
  FailedFiles.Add(name);
  // if (systemError == ERROR_SHARING_VIOLATION)
  {
    m_PercentPrinter.ClosePrint();
    m_PercentPrinter.PrintNewLine();
    m_PercentPrinter.PrintString("WARNING: ");
    m_PercentPrinter.PrintString(NError::MyFormatMessageW(systemError));
    return S_FALSE;
  }
  // return systemError;
}

HRESULT CUpdateCallbackConsole::SetOperationResult(Int32 )
{
  m_NeedBeClosed = true;
  m_NeedNewLine = true;
  return S_OK;
}

HRESULT CUpdateCallbackConsole::CryptoGetTextPassword2(Int32 *passwordIsDefined, BSTR *password)
{
  *password = NULL;

  #ifdef _NO_CRYPTO

  *passwordIsDefined = false;
  return S_OK;
  
  #else
  
  if (!PasswordIsDefined)
  {
    if (AskPassword)
    {
      Password = GetPassword(OutStream);
      PasswordIsDefined = true;
    }
  }
  *passwordIsDefined = BoolToInt(PasswordIsDefined);
  return StringToBstr(Password, password);
  
  #endif
}

HRESULT CUpdateCallbackConsole::CryptoGetTextPassword(BSTR *password)
{
  *password = NULL;

  #ifdef _NO_CRYPTO

  return E_NOTIMPL;
  
  #else
  
  if (!PasswordIsDefined)
  {
    {
      Password = GetPassword(OutStream);
      PasswordIsDefined = true;
    }
  }
  return StringToBstr(Password, password);
  
  #endif
}

/*
HRESULT CUpdateCallbackConsole::ShowDeleteFile(const wchar_t *name)
{
  // MT_LOCK
  if (StdOutMode)
    return S_OK;
  RINOK(Finilize());
  m_PercentPrinter.PrintString("Deleting  ");
  if (name[0] == 0)
    name = kEmptyFileAlias;
  m_PercentPrinter.PrintString(name);
  if (EnablePercents)
    m_PercentPrinter.RePrintRatio();
  m_NeedBeClosed = true;
  m_NeedNewLine = true;
  return S_OK;
}
*/
