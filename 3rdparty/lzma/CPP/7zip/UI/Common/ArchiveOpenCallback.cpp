// ArchiveOpenCallback.cpp

#include "StdAfx.h"

#include "Common/ComTry.h"

#include "Windows/PropVariant.h"

#include "../../Common/FileStreams.h"

#include "ArchiveOpenCallback.h"

using namespace NWindows;

STDMETHODIMP COpenCallbackImp::SetTotal(const UInt64 *files, const UInt64 *bytes)
{
  COM_TRY_BEGIN
  if (ReOpenCallback)
    return ReOpenCallback->SetTotal(files, bytes);
  if (!Callback)
    return S_OK;
  return Callback->Open_SetTotal(files, bytes);
  COM_TRY_END
}

STDMETHODIMP COpenCallbackImp::SetCompleted(const UInt64 *files, const UInt64 *bytes)
{
  COM_TRY_BEGIN
  if (ReOpenCallback)
    return ReOpenCallback->SetCompleted(files, bytes);
  if (!Callback)
    return S_OK;
  return Callback->Open_SetCompleted(files, bytes);
  COM_TRY_END
}
  
STDMETHODIMP COpenCallbackImp::GetProperty(PROPID propID, PROPVARIANT *value)
{
  COM_TRY_BEGIN
  NCOM::CPropVariant prop;
  if (_subArchiveMode)
    switch(propID)
    {
      case kpidName: prop = _subArchiveName; break;
    }
  else
    switch(propID)
    {
      case kpidName:  prop = _fileInfo.Name; break;
      case kpidIsDir:  prop = _fileInfo.IsDir(); break;
      case kpidSize:  prop = _fileInfo.Size; break;
      case kpidAttrib:  prop = (UInt32)_fileInfo.Attrib; break;
      case kpidCTime:  prop = _fileInfo.CTime; break;
      case kpidATime:  prop = _fileInfo.ATime; break;
      case kpidMTime:  prop = _fileInfo.MTime; break;
    }
  prop.Detach(value);
  return S_OK;
  COM_TRY_END
}

int COpenCallbackImp::FindName(const UString &name)
{
  for (int i = 0; i < FileNames.Size(); i++)
    if (name.CompareNoCase(FileNames[i]) == 0)
      return i;
  return -1;
}

struct CInFileStreamVol: public CInFileStream
{
  UString Name;
  COpenCallbackImp *OpenCallbackImp;
  CMyComPtr<IArchiveOpenCallback> OpenCallbackRef;
  ~CInFileStreamVol()
  {
    if (OpenCallbackRef)
    {
      int index = OpenCallbackImp->FindName(Name);
      if (index >= 0)
        OpenCallbackImp->FileNames.Delete(index);
    }
  }
};

STDMETHODIMP COpenCallbackImp::GetStream(const wchar_t *name, IInStream **inStream)
{
  COM_TRY_BEGIN
  if (_subArchiveMode)
    return S_FALSE;
  if (Callback)
  {
    RINOK(Callback->Open_CheckBreak());
  }
  *inStream = NULL;
  FString fullPath = _folderPrefix + us2fs(name);
  if (!_fileInfo.Find(fullPath))
    return S_FALSE;
  if (_fileInfo.IsDir())
    return S_FALSE;
  CInFileStreamVol *inFile = new CInFileStreamVol;
  CMyComPtr<IInStream> inStreamTemp = inFile;
  if (!inFile->Open(fullPath))
    return ::GetLastError();
  *inStream = inStreamTemp.Detach();
  inFile->Name = name;
  inFile->OpenCallbackImp = this;
  inFile->OpenCallbackRef = this;
  FileNames.Add(name);
  TotalSize += _fileInfo.Size;
  return S_OK;
  COM_TRY_END
}

#ifndef _NO_CRYPTO
STDMETHODIMP COpenCallbackImp::CryptoGetTextPassword(BSTR *password)
{
  COM_TRY_BEGIN
  if (ReOpenCallback)
  {
    CMyComPtr<ICryptoGetTextPassword> getTextPassword;
    ReOpenCallback.QueryInterface(IID_ICryptoGetTextPassword, &getTextPassword);
    if (getTextPassword)
      return getTextPassword->CryptoGetTextPassword(password);
  }
  if (!Callback)
    return E_NOTIMPL;
  return Callback->Open_CryptoGetTextPassword(password);
  COM_TRY_END
}
#endif
