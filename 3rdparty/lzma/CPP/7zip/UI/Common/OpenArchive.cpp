// OpenArchive.cpp

#include "StdAfx.h"

#include "Common/Wildcard.h"

#include "Windows/FileDir.h"
#include "Windows/PropVariant.h"

#include "../../Common/FileStreams.h"
#include "../../Common/StreamUtils.h"

#include "DefaultName.h"
#include "OpenArchive.h"

using namespace NWindows;

// Static-SFX (for Linux) can be big.
const UInt64 kMaxCheckStartPosition = 1 << 22;

HRESULT GetArchiveItemBoolProp(IInArchive *archive, UInt32 index, PROPID propID, bool &result)
{
  NCOM::CPropVariant prop;
  result = false;
  RINOK(archive->GetProperty(index, propID, &prop));
  if (prop.vt == VT_BOOL)
    result = VARIANT_BOOLToBool(prop.boolVal);
  else if (prop.vt != VT_EMPTY)
    return E_FAIL;
  return S_OK;
}

HRESULT IsArchiveItemFolder(IInArchive *archive, UInt32 index, bool &result)
{
  return GetArchiveItemBoolProp(archive, index, kpidIsDir, result);
}

HRESULT CArc::GetItemPath(UInt32 index, UString &result) const
{
  {
    NCOM::CPropVariant prop;
    RINOK(Archive->GetProperty(index, kpidPath, &prop));
    if (prop.vt == VT_BSTR)
      result = prop.bstrVal;
    else if (prop.vt == VT_EMPTY)
      result.Empty();
    else
      return E_FAIL;
  }
  if (result.IsEmpty())
  {
    result = DefaultName;
    NCOM::CPropVariant prop;
    RINOK(Archive->GetProperty(index, kpidExtension, &prop));
    if (prop.vt == VT_BSTR)
    {
      result += L'.';
      result += prop.bstrVal;
    }
    else if (prop.vt != VT_EMPTY)
      return E_FAIL;
  }
  return S_OK;
}

HRESULT CArc::GetItemSize(UInt32 index, UInt64 &size, bool &defined) const
{
  NCOM::CPropVariant prop;
  defined = false;
  size = 0;
  RINOK(Archive->GetProperty(index, kpidSize, &prop));
  switch (prop.vt)
  {
    case VT_UI1: size = prop.bVal; break;
    case VT_UI2: size = prop.uiVal; break;
    case VT_UI4: size = prop.ulVal; break;
    case VT_UI8: size = (UInt64)prop.uhVal.QuadPart; break;
    case VT_EMPTY: return S_OK;
    default: return E_FAIL;
  }
  defined = true;
  return S_OK;
}

HRESULT CArc::GetItemMTime(UInt32 index, FILETIME &ft, bool &defined) const
{
  NCOM::CPropVariant prop;
  defined = false;
  ft.dwHighDateTime = ft.dwLowDateTime = 0;
  RINOK(Archive->GetProperty(index, kpidMTime, &prop));
  if (prop.vt == VT_FILETIME)
  {
    ft = prop.filetime;
    defined = true;
  }
  else if (prop.vt != VT_EMPTY)
    return E_FAIL;
  else if (MTimeDefined)
  {
    ft = MTime;
    defined = true;
  }
  return S_OK;
}

#ifndef _SFX
static inline bool TestSignature(const Byte *p1, const Byte *p2, size_t size)
{
  for (size_t i = 0; i < size; i++)
    if (p1[i] != p2[i])
      return false;
  return true;
}
#endif

#ifdef UNDER_CE
static const int kNumHashBytes = 1;
#define HASH_VAL(buf, pos) ((buf)[pos])
#else
static const int kNumHashBytes = 2;
#define HASH_VAL(buf, pos) ((buf)[pos] | ((UInt32)(buf)[pos + 1] << 8))
#endif


HRESULT CArc::OpenStream(
    CCodecs *codecs,
    int formatIndex,
    IInStream *stream,
    ISequentialInStream *seqStream,
    IArchiveOpenCallback *callback)
{
  Archive.Release();
  ErrorMessage.Empty();
  const UString fileName = ExtractFileNameFromPath(Path);
  UString extension;
  {
    int dotPos = fileName.ReverseFind(L'.');
    if (dotPos >= 0)
      extension = fileName.Mid(dotPos + 1);
  }
  CIntVector orderIndices;
  if (formatIndex >= 0)
    orderIndices.Add(formatIndex);
  else
  {

  int i;
  int numFinded = 0;
  for (i = 0; i < codecs->Formats.Size(); i++)
    if (codecs->Formats[i].FindExtension(extension) >= 0)
      orderIndices.Insert(numFinded++, i);
    else
      orderIndices.Add(i);
  
  if (!stream)
  {
    if (numFinded != 1)
      return E_NOTIMPL;
    orderIndices.DeleteFrom(1);
  }

  #ifndef _SFX
  if (orderIndices.Size() >= 2 && (numFinded == 0 || extension.CompareNoCase(L"exe") == 0))
  {
    CIntVector orderIndices2;
    CByteBuffer byteBuffer;
    const size_t kBufferSize = (1 << 21);
    byteBuffer.SetCapacity(kBufferSize);
    RINOK(stream->Seek(0, STREAM_SEEK_SET, NULL));
    size_t processedSize = kBufferSize;
    RINOK(ReadStream(stream, byteBuffer, &processedSize));
    if (processedSize == 0)
      return S_FALSE;

    const Byte *buf = byteBuffer;
    CByteBuffer hashBuffer;
    const UInt32 kNumVals = 1 << (kNumHashBytes * 8);
    hashBuffer.SetCapacity(kNumVals);
    Byte *hash = hashBuffer;
    memset(hash, 0xFF, kNumVals);
    Byte prevs[256];
    if (orderIndices.Size() >= 256)
      return S_FALSE;
    int i;
    for (i = 0; i < orderIndices.Size(); i++)
    {
      const CArcInfoEx &ai = codecs->Formats[orderIndices[i]];
      const CByteBuffer &sig = ai.StartSignature;
      if (sig.GetCapacity() < kNumHashBytes)
        continue;
      UInt32 v = HASH_VAL(sig, 0);
      prevs[i] = hash[v];
      hash[v] = (Byte)i;
    }

    processedSize -= (kNumHashBytes - 1);
    for (UInt32 pos = 0; pos < processedSize; pos++)
    {
      for (; pos < processedSize && hash[HASH_VAL(buf, pos)] == 0xFF; pos++);
      if (pos == processedSize)
        break;
      UInt32 v = HASH_VAL(buf, pos);
      Byte *ptr = &hash[v];
      int i = *ptr;
      do
      {
        int index = orderIndices[i];
        const CArcInfoEx &ai = codecs->Formats[index];
        const CByteBuffer &sig = ai.StartSignature;
        if (sig.GetCapacity() != 0 && pos + sig.GetCapacity() <= processedSize + (kNumHashBytes - 1) &&
            TestSignature(buf + pos, sig, sig.GetCapacity()))
        {
          orderIndices2.Add(index);
          orderIndices[i] = 0xFF;
          *ptr = prevs[i];
        }
        else
          ptr = &prevs[i];
        i = *ptr;
      }
      while (i != 0xFF);
    }
    
    for (i = 0; i < orderIndices.Size(); i++)
    {
      int val = orderIndices[i];
      if (val != 0xFF)
        orderIndices2.Add(val);
    }
    orderIndices = orderIndices2;
  }
  else if (extension == L"000" || extension == L"001")
  {
    CByteBuffer byteBuffer;
    const size_t kBufferSize = (1 << 10);
    byteBuffer.SetCapacity(kBufferSize);
    Byte *buffer = byteBuffer;
    RINOK(stream->Seek(0, STREAM_SEEK_SET, NULL));
    size_t processedSize = kBufferSize;
    RINOK(ReadStream(stream, buffer, &processedSize));
    if (processedSize >= 16)
    {
      Byte kRarHeader[] = {0x52 , 0x61, 0x72, 0x21, 0x1a, 0x07, 0x00};
      if (TestSignature(buffer, kRarHeader, 7) && buffer[9] == 0x73 && (buffer[10] & 1) != 0)
      {
        for (int i = 0; i < orderIndices.Size(); i++)
        {
          int index = orderIndices[i];
          const CArcInfoEx &ai = codecs->Formats[index];
          if (ai.Name.CompareNoCase(L"rar") != 0)
            continue;
          orderIndices.Delete(i--);
          orderIndices.Insert(0, index);
          break;
        }
      }
    }
  }
  if (orderIndices.Size() >= 2)
  {
    int isoIndex = codecs->FindFormatForArchiveType(L"iso");
    int udfIndex = codecs->FindFormatForArchiveType(L"udf");
    int iIso = -1;
    int iUdf = -1;
    for (int i = 0; i < orderIndices.Size(); i++)
    {
      if (orderIndices[i] == isoIndex) iIso = i;
      if (orderIndices[i] == udfIndex) iUdf = i;
    }
    if (iUdf > iIso && iIso >= 0)
    {
      orderIndices[iUdf] = isoIndex;
      orderIndices[iIso] = udfIndex;
    }
  }

  #endif
  }

  for (int i = 0; i < orderIndices.Size(); i++)
  {
    if (stream)
    {
      RINOK(stream->Seek(0, STREAM_SEEK_SET, NULL));
    }
    CMyComPtr<IInArchive> archive;

    FormatIndex = orderIndices[i];
    RINOK(codecs->CreateInArchive(FormatIndex, archive));
    if (!archive)
      continue;

    #ifdef EXTERNAL_CODECS
    {
      CMyComPtr<ISetCompressCodecsInfo> setCompressCodecsInfo;
      archive.QueryInterface(IID_ISetCompressCodecsInfo, (void **)&setCompressCodecsInfo);
      if (setCompressCodecsInfo)
      {
        RINOK(setCompressCodecsInfo->SetCompressCodecsInfo(codecs));
      }
    }
    #endif

    // OutputDebugStringW(codecs->Formats[FormatIndex].Name);

    HRESULT result;
    if (stream)
      result = archive->Open(stream, &kMaxCheckStartPosition, callback);
    else
    {
      CMyComPtr<IArchiveOpenSeq> openSeq;
      archive.QueryInterface(IID_IArchiveOpenSeq, (void **)&openSeq);
      if (!openSeq)
        return E_NOTIMPL;
      result = openSeq->OpenSeq(seqStream);
    }

    if (result == S_FALSE)
      continue;
    RINOK(result);

    {
      NCOM::CPropVariant prop;
      archive->GetArchiveProperty(kpidError, &prop);
      if (prop.vt != VT_EMPTY)
        ErrorMessage = (prop.vt == VT_BSTR) ? prop.bstrVal : L"Unknown error";
    }
    
    Archive = archive;
    const CArcInfoEx &format = codecs->Formats[FormatIndex];
    if (format.Exts.Size() == 0)
      DefaultName = GetDefaultName2(fileName, L"", L"");
    else
    {
      int subExtIndex = format.FindExtension(extension);
      if (subExtIndex < 0)
        subExtIndex = 0;
      const CArcExtInfo &extInfo = format.Exts[subExtIndex];
      DefaultName = GetDefaultName2(fileName, extInfo.Ext, extInfo.AddExt);
    }
    return S_OK;
  }
  return S_FALSE;
}

HRESULT CArc::OpenStreamOrFile(
    CCodecs *codecs,
    int formatIndex,
    bool stdInMode,
    IInStream *stream,
    IArchiveOpenCallback *callback)
{
  CMyComPtr<IInStream> fileStream;
  CMyComPtr<ISequentialInStream> seqStream;
  if (stdInMode)
    seqStream = new CStdInFileStream;
  else if (!stream)
  {
    CInFileStream *fileStreamSpec = new CInFileStream;
    fileStream = fileStreamSpec;
    if (!fileStreamSpec->Open(us2fs(Path)))
      return GetLastError();
    stream = fileStream;
  }

  /*
  if (callback)
  {
    UInt64 fileSize;
    RINOK(stream->Seek(0, STREAM_SEEK_END, &fileSize));
    RINOK(callback->SetTotal(NULL, &fileSize))
  }
  */

  return OpenStream(codecs, formatIndex, stream, seqStream, callback);
}

HRESULT CArchiveLink::Close()
{
  for (int i = Arcs.Size() - 1;  i >= 0; i--)
  {
    RINOK(Arcs[i].Archive->Close());
  }
  IsOpen = false;
  return S_OK;
}

void CArchiveLink::Release()
{
  while (!Arcs.IsEmpty())
    Arcs.DeleteBack();
}

HRESULT CArchiveLink::Open(
    CCodecs *codecs,
    const CIntVector &formatIndices,
    bool stdInMode,
    IInStream *stream,
    const UString &filePath,
    IArchiveOpenCallback *callback)
{
  Release();
  if (formatIndices.Size() >= 32)
    return E_NOTIMPL;
  
  HRESULT resSpec;

  for (;;)
  {
    resSpec = S_OK;
    int formatIndex = -1;
    if (formatIndices.Size() >= 1)
    {
      if (Arcs.Size() >= formatIndices.Size())
        break;
      formatIndex = formatIndices[formatIndices.Size() - Arcs.Size() - 1];
    }
    else if (Arcs.Size() >= 32)
      break;

    if (Arcs.IsEmpty())
    {
      CArc arc;
      arc.Path = filePath;
      arc.SubfileIndex = (UInt32)(Int32)-1;
      RINOK(arc.OpenStreamOrFile(codecs, formatIndex, stdInMode, stream, callback));
      Arcs.Add(arc);
      continue;
    }
    
    const CArc &arc = Arcs.Back();
    
    resSpec = (formatIndices.Size() == 0 ? S_OK : E_NOTIMPL);
    
    UInt32 mainSubfile;
    {
      NCOM::CPropVariant prop;
      RINOK(arc.Archive->GetArchiveProperty(kpidMainSubfile, &prop));
      if (prop.vt == VT_UI4)
        mainSubfile = prop.ulVal;
      else
        break;
      UInt32 numItems;
      RINOK(arc.Archive->GetNumberOfItems(&numItems));
      if (mainSubfile >= numItems)
        break;
    }

  
    CMyComPtr<IInArchiveGetStream> getStream;
    if (arc.Archive->QueryInterface(IID_IInArchiveGetStream, (void **)&getStream) != S_OK || !getStream)
      break;
    
    CMyComPtr<ISequentialInStream> subSeqStream;
    if (getStream->GetStream(mainSubfile, &subSeqStream) != S_OK || !subSeqStream)
      break;
    
    CMyComPtr<IInStream> subStream;
    if (subSeqStream.QueryInterface(IID_IInStream, &subStream) != S_OK || !subStream)
      break;
    
    CArc arc2;
    RINOK(arc.GetItemPath(mainSubfile, arc2.Path));
    
    CMyComPtr<IArchiveOpenSetSubArchiveName> setSubArchiveName;
    callback->QueryInterface(IID_IArchiveOpenSetSubArchiveName, (void **)&setSubArchiveName);
    if (setSubArchiveName)
      setSubArchiveName->SetSubArchiveName(arc2.Path);
    
    arc2.SubfileIndex = mainSubfile;
    HRESULT result = arc2.OpenStream(codecs, formatIndex, subStream, NULL, callback);
    resSpec = (formatIndices.Size() == 0 ? S_OK : S_FALSE);
    if (result == S_FALSE)
      break;
    RINOK(result);
    RINOK(arc.GetItemMTime(mainSubfile, arc2.MTime, arc2.MTimeDefined));
    Arcs.Add(arc2);
  }
  IsOpen = !Arcs.IsEmpty();
  return S_OK;
}

static void SetCallback(const FString &filePath,
    IOpenCallbackUI *callbackUI,
    IArchiveOpenCallback *reOpenCallback,
    CMyComPtr<IArchiveOpenCallback> &callback)
{
  COpenCallbackImp *openCallbackSpec = new COpenCallbackImp;
  callback = openCallbackSpec;
  openCallbackSpec->Callback = callbackUI;
  openCallbackSpec->ReOpenCallback = reOpenCallback;

  FString dirPrefix, fileName;
  NFile::NDirectory::GetFullPathAndSplit(filePath, dirPrefix, fileName);
  openCallbackSpec->Init(dirPrefix, fileName);
}

HRESULT CArchiveLink::Open2(CCodecs *codecs,
    const CIntVector &formatIndices,
    bool stdInMode,
    IInStream *stream,
    const UString &filePath,
    IOpenCallbackUI *callbackUI)
{
  VolumesSize = 0;
  COpenCallbackImp *openCallbackSpec = new COpenCallbackImp;
  CMyComPtr<IArchiveOpenCallback> callback = openCallbackSpec;
  openCallbackSpec->Callback = callbackUI;

  FString prefix, name;
  if (!stream && !stdInMode)
  {
    NFile::NDirectory::GetFullPathAndSplit(us2fs(filePath), prefix, name);
    openCallbackSpec->Init(prefix, name);
  }
  else
  {
    openCallbackSpec->SetSubArchiveName(filePath);
  }

  RINOK(Open(codecs, formatIndices, stdInMode, stream, filePath, callback));
  VolumePaths.Add(fs2us(prefix + name));
  for (int i = 0; i < openCallbackSpec->FileNames.Size(); i++)
    VolumePaths.Add(fs2us(prefix) + openCallbackSpec->FileNames[i]);
  VolumesSize = openCallbackSpec->TotalSize;
  return S_OK;
}

HRESULT CArchiveLink::ReOpen(CCodecs *codecs, const UString &filePath,
    IArchiveOpenCallback *callback)
{
  if (Arcs.Size() > 1)
    return E_NOTIMPL;

  if (Arcs.Size() == 0)
    return Open2(codecs, CIntVector(), false, NULL, filePath, 0);

  CMyComPtr<IArchiveOpenCallback> openCallbackNew;
  SetCallback(us2fs(filePath), NULL, callback, openCallbackNew);

  CInFileStream *fileStreamSpec = new CInFileStream;
  CMyComPtr<IInStream> stream(fileStreamSpec);
  if (!fileStreamSpec->Open(us2fs(filePath)))
    return GetLastError();
  HRESULT res = GetArchive()->Open(stream, &kMaxCheckStartPosition, callback);
  IsOpen = (res == S_OK);
  return res;
}
