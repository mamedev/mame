// IFileExtractCallback.h

#ifndef __IFILEEXTRACTCALLBACK_H
#define __IFILEEXTRACTCALLBACK_H

#include "Common/MyString.h"
#include "../../IDecl.h"

namespace NOverwriteAnswer
{
  enum EEnum
  {
    kYes,
    kYesToAll,
    kNo,
    kNoToAll,
    kAutoRename,
    kCancel
  };
}

DECL_INTERFACE_SUB(IFolderArchiveExtractCallback, IProgress, 0x01, 0x07)
{
public:
  STDMETHOD(AskOverwrite)(
      const wchar_t *existName, const FILETIME *existTime, const UInt64 *existSize,
      const wchar_t *newName, const FILETIME *newTime, const UInt64 *newSize,
      Int32 *answer) PURE;
  STDMETHOD(PrepareOperation)(const wchar_t *name, bool isFolder, Int32 askExtractMode, const UInt64 *position) PURE;
  STDMETHOD(MessageError)(const wchar_t *message) PURE;
  STDMETHOD(SetOperationResult)(Int32 operationResult, bool encrypted) PURE;
};

struct IExtractCallbackUI: IFolderArchiveExtractCallback
{
  virtual HRESULT BeforeOpen(const wchar_t *name) = 0;
  virtual HRESULT OpenResult(const wchar_t *name, HRESULT result, bool encrypted) = 0;
  virtual HRESULT ThereAreNoFiles() = 0;
  virtual HRESULT ExtractResult(HRESULT result) = 0;

  #ifndef _NO_CRYPTO
  virtual HRESULT SetPassword(const UString &password) = 0;
  #endif
};

#endif
