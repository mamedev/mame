// ExtractCallbackConsole.h

#include "StdAfx.h"

#include "ExtractCallbackConsole.h"
#include "UserInputUtils.h"
#include "ConsoleClose.h"

#include "Common/Wildcard.h"

#include "Windows/FileDir.h"
#include "Windows/FileFind.h"
#include "Windows/Time.h"
#include "Windows/Defs.h"
#include "Windows/PropVariant.h"
#include "Windows/Error.h"
#include "Windows/PropVariantConversions.h"

#include "../../Common/FilePathAutoRename.h"

#include "../Common/ExtractingFilePath.h"

using namespace NWindows;
using namespace NFile;
using namespace NDirectory;

static const char *kTestString    =  "Testing     ";
static const char *kExtractString =  "Extracting  ";
static const char *kSkipString   =  "Skipping    ";

// static const char *kCantAutoRename = "can not create file with auto name\n";
// static const char *kCantRenameFile = "can not rename existing file\n";
// static const char *kCantDeleteOutputFile = "can not delete output file ";
static const char *kError = "ERROR: ";
static const char *kMemoryExceptionMessage = "Can't allocate required memory!";

static const char *kProcessing = "Processing archive: ";
static const char *kEverythingIsOk = "Everything is Ok";
static const char *kNoFiles = "No files to process";

static const char *kUnsupportedMethod = "Unsupported Method";
static const char *kCrcFailed = "CRC Failed";
static const char *kCrcFailedEncrypted = "CRC Failed in encrypted file. Wrong password?";
static const char *kDataError = "Data Error";
static const char *kDataErrorEncrypted = "Data Error in encrypted file. Wrong password?";
static const char *kUnknownError = "Unknown Error";

STDMETHODIMP CExtractCallbackConsole::SetTotal(UInt64)
{
  if (NConsoleClose::TestBreakSignal())
    return E_ABORT;
  return S_OK;
}

STDMETHODIMP CExtractCallbackConsole::SetCompleted(const UInt64 *)
{
  if (NConsoleClose::TestBreakSignal())
    return E_ABORT;
  return S_OK;
}

STDMETHODIMP CExtractCallbackConsole::AskOverwrite(
    const wchar_t *existName, const FILETIME *, const UInt64 *,
    const wchar_t *newName, const FILETIME *, const UInt64 *,
    Int32 *answer)
{
  (*OutStream) << "file " << existName <<
    "\nalready exists. Overwrite with " << endl;
  (*OutStream) << newName;
  
  NUserAnswerMode::EEnum overwriteAnswer = ScanUserYesNoAllQuit(OutStream);
  
  switch(overwriteAnswer)
  {
    case NUserAnswerMode::kQuit:  return E_ABORT;
    case NUserAnswerMode::kNo:     *answer = NOverwriteAnswer::kNo; break;
    case NUserAnswerMode::kNoAll:  *answer = NOverwriteAnswer::kNoToAll; break;
    case NUserAnswerMode::kYesAll: *answer = NOverwriteAnswer::kYesToAll; break;
    case NUserAnswerMode::kYes:    *answer = NOverwriteAnswer::kYes; break;
    case NUserAnswerMode::kAutoRenameAll: *answer = NOverwriteAnswer::kAutoRename; break;
    default: return E_FAIL;
  }
  return S_OK;
}

STDMETHODIMP CExtractCallbackConsole::PrepareOperation(const wchar_t *name, bool /* isFolder */, Int32 askExtractMode, const UInt64 *position)
{
  switch (askExtractMode)
  {
    case NArchive::NExtract::NAskMode::kExtract: (*OutStream) << kExtractString; break;
    case NArchive::NExtract::NAskMode::kTest:    (*OutStream) << kTestString; break;
    case NArchive::NExtract::NAskMode::kSkip:    (*OutStream) << kSkipString; break;
  };
  (*OutStream) << name;
  if (position != 0)
    (*OutStream) << " <" << *position << ">";
  return S_OK;
}

STDMETHODIMP CExtractCallbackConsole::MessageError(const wchar_t *message)
{
  (*OutStream) << message << endl;
  NumFileErrorsInCurrentArchive++;
  NumFileErrors++;
  return S_OK;
}

STDMETHODIMP CExtractCallbackConsole::SetOperationResult(Int32 operationResult, bool encrypted)
{
  switch(operationResult)
  {
    case NArchive::NExtract::NOperationResult::kOK:
      break;
    default:
    {
      NumFileErrorsInCurrentArchive++;
      NumFileErrors++;
      (*OutStream) << "     ";
      switch(operationResult)
      {
        case NArchive::NExtract::NOperationResult::kUnSupportedMethod:
          (*OutStream) << kUnsupportedMethod;
          break;
        case NArchive::NExtract::NOperationResult::kCRCError:
          (*OutStream) << (encrypted ? kCrcFailedEncrypted: kCrcFailed);
          break;
        case NArchive::NExtract::NOperationResult::kDataError:
          (*OutStream) << (encrypted ? kDataErrorEncrypted : kDataError);
          break;
        default:
          (*OutStream) << kUnknownError;
      }
    }
  }
  (*OutStream) << endl;
  return S_OK;
}

#ifndef _NO_CRYPTO

HRESULT CExtractCallbackConsole::SetPassword(const UString &password)
{
  PasswordIsDefined = true;
  Password = password;
  return S_OK;
}

STDMETHODIMP CExtractCallbackConsole::CryptoGetTextPassword(BSTR *password)
{
  if (!PasswordIsDefined)
  {
    Password = GetPassword(OutStream);
    PasswordIsDefined = true;
  }
  return StringToBstr(Password, password);
}

#endif

HRESULT CExtractCallbackConsole::BeforeOpen(const wchar_t *name)
{
  NumArchives++;
  NumFileErrorsInCurrentArchive = 0;
  (*OutStream) << endl << kProcessing << name << endl;
  return S_OK;
}

HRESULT CExtractCallbackConsole::OpenResult(const wchar_t * /* name */, HRESULT result, bool encrypted)
{
  (*OutStream) << endl;
  if (result != S_OK)
  {
    (*OutStream) << "Error: ";
    if (result == S_FALSE)
    {
      (*OutStream) << (encrypted ?
        "Can not open encrypted archive. Wrong password?" :
        "Can not open file as archive");
    }
    else
    {
      if (result == E_OUTOFMEMORY)
        (*OutStream) << "Can't allocate required memory";
      else
        (*OutStream) << NError::MyFormatMessageW(result);
    }
    (*OutStream) << endl;
    NumArchiveErrors++;
  }
  return S_OK;
}
  
HRESULT CExtractCallbackConsole::ThereAreNoFiles()
{
  (*OutStream) << endl << kNoFiles << endl;
  return S_OK;
}

HRESULT CExtractCallbackConsole::ExtractResult(HRESULT result)
{
  if (result == S_OK)
  {
    (*OutStream) << endl;
    if (NumFileErrorsInCurrentArchive == 0)
      (*OutStream) << kEverythingIsOk << endl;
    else
    {
      NumArchiveErrors++;
      (*OutStream) << "Sub items Errors: " << NumFileErrorsInCurrentArchive << endl;
    }
  }
  if (result == S_OK)
    return result;
  NumArchiveErrors++;
  if (result == E_ABORT || result == ERROR_DISK_FULL)
    return result;
  (*OutStream) << endl << kError;
  if (result == E_OUTOFMEMORY)
    (*OutStream) << kMemoryExceptionMessage;
  else
    (*OutStream) << NError::MyFormatMessageW(result);
  (*OutStream) << endl;
  return S_OK;
}
