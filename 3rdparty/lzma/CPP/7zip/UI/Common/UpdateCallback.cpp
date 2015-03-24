// UpdateCallback.cpp

#include "StdAfx.h"

#include "Common/ComTry.h"
#include "Common/Defs.h"
#include "Common/IntToString.h"
#include "Common/StringConvert.h"

#include "Windows/PropVariant.h"

#include "../../Common/FileStreams.h"

#include "UpdateCallback.h"

using namespace NWindows;

CArchiveUpdateCallback::CArchiveUpdateCallback():
  Callback(0),
  ShareForWrite(false),
  StdInMode(false),
  DirItems(0),
  ArcItems(0),
  UpdatePairs(0),
  NewNames(0),
  KeepOriginalItemNames(0)
  {}


STDMETHODIMP CArchiveUpdateCallback::SetTotal(UInt64 size)
{
  COM_TRY_BEGIN
  return Callback->SetTotal(size);
  COM_TRY_END
}

STDMETHODIMP CArchiveUpdateCallback::SetCompleted(const UInt64 *completeValue)
{
  COM_TRY_BEGIN
  return Callback->SetCompleted(completeValue);
  COM_TRY_END
}

STDMETHODIMP CArchiveUpdateCallback::SetRatioInfo(const UInt64 *inSize, const UInt64 *outSize)
{
  COM_TRY_BEGIN
  return Callback->SetRatioInfo(inSize, outSize);
  COM_TRY_END
}


/*
STATPROPSTG kProperties[] =
{
  { NULL, kpidPath, VT_BSTR},
  { NULL, kpidIsDir, VT_BOOL},
  { NULL, kpidSize, VT_UI8},
  { NULL, kpidCTime, VT_FILETIME},
  { NULL, kpidATime, VT_FILETIME},
  { NULL, kpidMTime, VT_FILETIME},
  { NULL, kpidAttrib, VT_UI4},
  { NULL, kpidIsAnti, VT_BOOL}
};

STDMETHODIMP CArchiveUpdateCallback::EnumProperties(IEnumSTATPROPSTG **)
{
  return CStatPropEnumerator::CreateEnumerator(kProperties, sizeof(kProperties) / sizeof(kProperties[0]), enumerator);
}
*/

STDMETHODIMP CArchiveUpdateCallback::GetUpdateItemInfo(UInt32 index,
      Int32 *newData, Int32 *newProps, UInt32 *indexInArchive)
{
  COM_TRY_BEGIN
  RINOK(Callback->CheckBreak());
  const CUpdatePair2 &up = (*UpdatePairs)[index];
  if (newData != NULL) *newData = BoolToInt(up.NewData);
  if (newProps != NULL) *newProps = BoolToInt(up.NewProps);
  if (indexInArchive != NULL)
  {
    *indexInArchive = (UInt32)-1;
    if (up.ExistInArchive())
      *indexInArchive = (ArcItems == 0) ? up.ArcIndex : (*ArcItems)[up.ArcIndex].IndexInServer;
  }
  return S_OK;
  COM_TRY_END
}

STDMETHODIMP CArchiveUpdateCallback::GetProperty(UInt32 index, PROPID propID, PROPVARIANT *value)
{
  COM_TRY_BEGIN
  const CUpdatePair2 &up = (*UpdatePairs)[index];
  NWindows::NCOM::CPropVariant prop;
  
  if (propID == kpidIsAnti)
  {
    prop = up.IsAnti;
    prop.Detach(value);
    return S_OK;
  }

  if (up.IsAnti)
  {
    switch(propID)
    {
      case kpidIsDir:
      case kpidPath:
        break;
      case kpidSize:
        prop = (UInt64)0;
        prop.Detach(value);
        return S_OK;
      default:
        prop.Detach(value);
        return S_OK;
    }
  }
  
  if (up.ExistOnDisk())
  {
    const CDirItem &di = DirItems->Items[up.DirIndex];
    switch(propID)
    {
      case kpidPath:
        {
          if (KeepOriginalItemNames)
          {
            if (up.ExistInArchive() && Archive)
            {
              UInt32 indexInArchive;
              if (ArcItems == 0)
                indexInArchive = up.ArcIndex;
              else
                indexInArchive = (*ArcItems)[up.ArcIndex].IndexInServer;
              return Archive->GetProperty(indexInArchive, propID, value);
            }
          }
          prop = DirItems->GetLogPath(up.DirIndex); break;
        }
      case kpidIsDir:  prop = di.IsDir(); break;
      case kpidSize:  prop = di.Size; break;
      case kpidAttrib:  prop = di.Attrib; break;
      case kpidCTime:  prop = di.CTime; break;
      case kpidATime:  prop = di.ATime; break;
      case kpidMTime:  prop = di.MTime; break;
    }
  }
  else
  {
    if (propID == kpidPath)
    {
      if (up.NewNameIndex >= 0)
      {
        prop = (*NewNames)[up.NewNameIndex];
        prop.Detach(value);
        return S_OK;
      }
    }
    if (up.ExistInArchive() && Archive)
    {
      UInt32 indexInArchive;
      if (ArcItems == 0)
        indexInArchive = up.ArcIndex;
      else
        indexInArchive = (*ArcItems)[up.ArcIndex].IndexInServer;
      return Archive->GetProperty(indexInArchive, propID, value);
    }
  }
  prop.Detach(value);
  return S_OK;
  COM_TRY_END
}

STDMETHODIMP CArchiveUpdateCallback::GetStream(UInt32 index, ISequentialInStream **inStream)
{
  COM_TRY_BEGIN
  const CUpdatePair2 &up = (*UpdatePairs)[index];
  if (!up.NewData)
    return E_FAIL;
  
  RINOK(Callback->CheckBreak());
  RINOK(Callback->Finilize());

  if (up.IsAnti)
  {
    return Callback->GetStream((*ArcItems)[up.ArcIndex].Name, true);
  }
  const CDirItem &di = DirItems->Items[up.DirIndex];
  RINOK(Callback->GetStream(DirItems->GetLogPath(up.DirIndex), false));
 
  if (di.IsDir())
    return S_OK;

  if (StdInMode)
  {
    CStdInFileStream *inStreamSpec = new CStdInFileStream;
    CMyComPtr<ISequentialInStream> inStreamLoc(inStreamSpec);
    *inStream = inStreamLoc.Detach();
  }
  else
  {
    CInFileStream *inStreamSpec = new CInFileStream;
    CMyComPtr<ISequentialInStream> inStreamLoc(inStreamSpec);
    const UString path = DirItems->GetPhyPath(up.DirIndex);
    if (!inStreamSpec->OpenShared(us2fs(path), ShareForWrite))
    {
      return Callback->OpenFileError(path, ::GetLastError());
    }
    *inStream = inStreamLoc.Detach();
  }
  return S_OK;
  COM_TRY_END
}

STDMETHODIMP CArchiveUpdateCallback::SetOperationResult(Int32 operationResult)
{
  COM_TRY_BEGIN
  return Callback->SetOperationResult(operationResult);
  COM_TRY_END
}

STDMETHODIMP CArchiveUpdateCallback::GetVolumeSize(UInt32 index, UInt64 *size)
{
  if (VolumesSizes.Size() == 0)
    return S_FALSE;
  if (index >= (UInt32)VolumesSizes.Size())
    index = VolumesSizes.Size() - 1;
  *size = VolumesSizes[index];
  return S_OK;
}

STDMETHODIMP CArchiveUpdateCallback::GetVolumeStream(UInt32 index, ISequentialOutStream **volumeStream)
{
  COM_TRY_BEGIN
  FChar temp[16];
  ConvertUInt32ToString(index + 1, temp);
  FString res = temp;
  while (res.Length() < 2)
    res = FString(FTEXT('0')) + res;
  FString fileName = VolName;
  fileName += L'.';
  fileName += res;
  fileName += VolExt;
  COutFileStream *streamSpec = new COutFileStream;
  CMyComPtr<ISequentialOutStream> streamLoc(streamSpec);
  if (!streamSpec->Create(fileName, false))
    return ::GetLastError();
  *volumeStream = streamLoc.Detach();
  return S_OK;
  COM_TRY_END
}

STDMETHODIMP CArchiveUpdateCallback::CryptoGetTextPassword2(Int32 *passwordIsDefined, BSTR *password)
{
  COM_TRY_BEGIN
  return Callback->CryptoGetTextPassword2(passwordIsDefined, password);
  COM_TRY_END
}

STDMETHODIMP CArchiveUpdateCallback::CryptoGetTextPassword(BSTR *password)
{
  COM_TRY_BEGIN
  return Callback->CryptoGetTextPassword(password);
  COM_TRY_END
}
