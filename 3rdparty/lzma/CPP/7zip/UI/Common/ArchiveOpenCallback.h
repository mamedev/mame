// ArchiveOpenCallback.h

#ifndef __ARCHIVE_OPEN_CALLBACK_H
#define __ARCHIVE_OPEN_CALLBACK_H

#include "Common/MyCom.h"
#include "Common/MyString.h"

#include "Windows/FileFind.h"

#ifndef _NO_CRYPTO
#include "../../IPassword.h"
#endif
#include "../../Archive/IArchive.h"

#ifdef _NO_CRYPTO

#define INTERFACE_IOpenCallbackUI_Crypto(x)

#else

#define INTERFACE_IOpenCallbackUI_Crypto(x) \
  virtual HRESULT Open_CryptoGetTextPassword(BSTR *password) x; \
  virtual HRESULT Open_GetPasswordIfAny(UString &password) x; \
  virtual bool Open_WasPasswordAsked() x; \
  virtual void Open_ClearPasswordWasAskedFlag() x; \
  
#endif

#define INTERFACE_IOpenCallbackUI(x) \
  virtual HRESULT Open_CheckBreak() x; \
  virtual HRESULT Open_SetTotal(const UInt64 *files, const UInt64 *bytes) x; \
  virtual HRESULT Open_SetCompleted(const UInt64 *files, const UInt64 *bytes) x; \
  INTERFACE_IOpenCallbackUI_Crypto(x)

struct IOpenCallbackUI
{
  INTERFACE_IOpenCallbackUI(=0)
};

class COpenCallbackImp:
  public IArchiveOpenCallback,
  public IArchiveOpenVolumeCallback,
  public IArchiveOpenSetSubArchiveName,
  #ifndef _NO_CRYPTO
  public ICryptoGetTextPassword,
  #endif
  public CMyUnknownImp
{
public:
  #ifndef _NO_CRYPTO
  MY_UNKNOWN_IMP3(
      IArchiveOpenVolumeCallback,
      ICryptoGetTextPassword,
      IArchiveOpenSetSubArchiveName
      )
  #else
  MY_UNKNOWN_IMP2(
      IArchiveOpenVolumeCallback,
      IArchiveOpenSetSubArchiveName
      )
  #endif

  INTERFACE_IArchiveOpenCallback(;)
  INTERFACE_IArchiveOpenVolumeCallback(;)

  #ifndef _NO_CRYPTO
  STDMETHOD(CryptoGetTextPassword)(BSTR *password);
  #endif

  STDMETHOD(SetSubArchiveName(const wchar_t *name))
  {
    _subArchiveMode = true;
    _subArchiveName = name;
    TotalSize = 0;
    return  S_OK;
  }

private:
  FString _folderPrefix;
  NWindows::NFile::NFind::CFileInfo _fileInfo;
  bool _subArchiveMode;
  UString _subArchiveName;
public:
  UStringVector FileNames;
  IOpenCallbackUI *Callback;
  CMyComPtr<IArchiveOpenCallback> ReOpenCallback;
  UInt64 TotalSize;

  COpenCallbackImp(): Callback(NULL) {}
  void Init(const FString &folderPrefix, const FString &fileName)
  {
    _folderPrefix = folderPrefix;
    if (!_fileInfo.Find(_folderPrefix + fileName))
      throw 1;
    FileNames.Clear();
    _subArchiveMode = false;
    TotalSize = 0;
  }
  int FindName(const UString &name);
};

#endif
