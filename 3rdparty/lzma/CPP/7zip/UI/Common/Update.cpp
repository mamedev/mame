// Update.cpp

#include "StdAfx.h"

#include "Update.h"

#include "Common/IntToString.h"
#include "Common/StringConvert.h"

#include "Windows/DLL.h"
#include "Windows/FileDir.h"
#include "Windows/FileFind.h"
#include "Windows/FileName.h"
#include "Windows/PropVariant.h"
#include "Windows/PropVariantConversions.h"
#include "Windows/Time.h"

#include "../../Common/FileStreams.h"

#include "../../Compress/CopyCoder.h"

#include "../Common/DirItem.h"
#include "../Common/EnumDirItems.h"
#include "../Common/OpenArchive.h"
#include "../Common/UpdateProduce.h"

#include "EnumDirItems.h"
#include "SetProperties.h"
#include "TempFiles.h"
#include "UpdateCallback.h"

static const char *kUpdateIsNotSupoorted =
  "update operations are not supported for this archive";

using namespace NWindows;
using namespace NCOM;
using namespace NFile;
using namespace NName;

static CFSTR kTempFolderPrefix = FTEXT("7zE");

using namespace NUpdateArchive;

class COutMultiVolStream:
  public IOutStream,
  public CMyUnknownImp
{
  int _streamIndex; // required stream
  UInt64 _offsetPos; // offset from start of _streamIndex index
  UInt64 _absPos;
  UInt64 _length;

  struct CSubStreamInfo
  {
    COutFileStream *StreamSpec;
    CMyComPtr<IOutStream> Stream;
    FString Name;
    UInt64 Pos;
    UInt64 RealSize;
  };
  CObjectVector<CSubStreamInfo> Streams;
public:
  // CMyComPtr<IArchiveUpdateCallback2> VolumeCallback;
  CRecordVector<UInt64> Sizes;
  FString Prefix;
  CTempFiles *TempFiles;

  void Init()
  {
    _streamIndex = 0;
    _offsetPos = 0;
    _absPos = 0;
    _length = 0;
  }

  HRESULT Close();

  MY_UNKNOWN_IMP1(IOutStream)

  STDMETHOD(Write)(const void *data, UInt32 size, UInt32 *processedSize);
  STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition);
  STDMETHOD(SetSize)(UInt64 newSize);
};

// static NSynchronization::CCriticalSection g_TempPathsCS;

HRESULT COutMultiVolStream::Close()
{
  HRESULT res = S_OK;
  for (int i = 0; i < Streams.Size(); i++)
  {
    CSubStreamInfo &s = Streams[i];
    if (s.StreamSpec)
    {
      HRESULT res2 = s.StreamSpec->Close();
      if (res2 != S_OK)
        res = res2;
    }
  }
  return res;
}

STDMETHODIMP COutMultiVolStream::Write(const void *data, UInt32 size, UInt32 *processedSize)
{
  if (processedSize != NULL)
    *processedSize = 0;
  while (size > 0)
  {
    if (_streamIndex >= Streams.Size())
    {
      CSubStreamInfo subStream;

      FChar temp[16];
      ConvertUInt32ToString(_streamIndex + 1, temp);
      FString res = temp;
      while (res.Length() < 3)
        res = FString(FTEXT('0')) + res;
      FString name = Prefix + res;
      subStream.StreamSpec = new COutFileStream;
      subStream.Stream = subStream.StreamSpec;
      if (!subStream.StreamSpec->Create(name, false))
        return ::GetLastError();
      {
        // NSynchronization::CCriticalSectionLock lock(g_TempPathsCS);
        TempFiles->Paths.Add(name);
      }

      subStream.Pos = 0;
      subStream.RealSize = 0;
      subStream.Name = name;
      Streams.Add(subStream);
      continue;
    }
    CSubStreamInfo &subStream = Streams[_streamIndex];

    int index = _streamIndex;
    if (index >= Sizes.Size())
      index = Sizes.Size() - 1;
    UInt64 volSize = Sizes[index];

    if (_offsetPos >= volSize)
    {
      _offsetPos -= volSize;
      _streamIndex++;
      continue;
    }
    if (_offsetPos != subStream.Pos)
    {
      // CMyComPtr<IOutStream> outStream;
      // RINOK(subStream.Stream.QueryInterface(IID_IOutStream, &outStream));
      RINOK(subStream.Stream->Seek(_offsetPos, STREAM_SEEK_SET, NULL));
      subStream.Pos = _offsetPos;
    }

    UInt32 curSize = (UInt32)MyMin((UInt64)size, volSize - subStream.Pos);
    UInt32 realProcessed;
    RINOK(subStream.Stream->Write(data, curSize, &realProcessed));
    data = (void *)((Byte *)data + realProcessed);
    size -= realProcessed;
    subStream.Pos += realProcessed;
    _offsetPos += realProcessed;
    _absPos += realProcessed;
    if (_absPos > _length)
      _length = _absPos;
    if (_offsetPos > subStream.RealSize)
      subStream.RealSize = _offsetPos;
    if (processedSize != NULL)
      *processedSize += realProcessed;
    if (subStream.Pos == volSize)
    {
      _streamIndex++;
      _offsetPos = 0;
    }
    if (realProcessed == 0 && curSize != 0)
      return E_FAIL;
    break;
  }
  return S_OK;
}

STDMETHODIMP COutMultiVolStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition)
{
  if (seekOrigin >= 3)
    return STG_E_INVALIDFUNCTION;
  switch(seekOrigin)
  {
    case STREAM_SEEK_SET:
      _absPos = offset;
      break;
    case STREAM_SEEK_CUR:
      _absPos += offset;
      break;
    case STREAM_SEEK_END:
      _absPos = _length + offset;
      break;
  }
  _offsetPos = _absPos;
  if (newPosition != NULL)
    *newPosition = _absPos;
  _streamIndex = 0;
  return S_OK;
}

STDMETHODIMP COutMultiVolStream::SetSize(UInt64 newSize)
{
  if (newSize < 0)
    return E_INVALIDARG;
  int i = 0;
  while (i < Streams.Size())
  {
    CSubStreamInfo &subStream = Streams[i++];
    if ((UInt64)newSize < subStream.RealSize)
    {
      RINOK(subStream.Stream->SetSize(newSize));
      subStream.RealSize = newSize;
      break;
    }
    newSize -= subStream.RealSize;
  }
  while (i < Streams.Size())
  {
    {
      CSubStreamInfo &subStream = Streams.Back();
      subStream.Stream.Release();
      NDirectory::DeleteFileAlways(subStream.Name);
    }
    Streams.DeleteBack();
  }
  _offsetPos = _absPos;
  _streamIndex = 0;
  _length = newSize;
  return S_OK;
}

static const wchar_t *kDefaultArchiveType = L"7z";
static const wchar_t *kSFXExtension =
  #ifdef _WIN32
    L"exe";
  #else
    L"";
  #endif

bool CUpdateOptions::Init(const CCodecs *codecs, const CIntVector &formatIndices, const UString &arcPath)
{
  if (formatIndices.Size() > 1)
    return false;
  int arcTypeIndex = -1;
  if (formatIndices.Size() != 0)
    arcTypeIndex = formatIndices[0];
  if (arcTypeIndex >= 0)
    MethodMode.FormatIndex = arcTypeIndex;
  else
  {
    MethodMode.FormatIndex = codecs->FindFormatForArchiveName(arcPath);
    // It works incorrectly for update command if archive has some non-default extension!
    if (MethodMode.FormatIndex < 0)
      MethodMode.FormatIndex = codecs->FindFormatForArchiveType(kDefaultArchiveType);
  }
  if (MethodMode.FormatIndex < 0)
    return false;
  const CArcInfoEx &arcInfo = codecs->Formats[MethodMode.FormatIndex];
  if (!arcInfo.UpdateEnabled)
    return false;
  UString typeExt = arcInfo.GetMainExt();
  UString ext = typeExt;
  if (SfxMode)
    ext = kSFXExtension;
  ArchivePath.BaseExtension = ext;
  ArchivePath.VolExtension = typeExt;
  ArchivePath.ParseFromPath(arcPath);
  for (int i = 0; i < Commands.Size(); i++)
  {
    CUpdateArchiveCommand &uc = Commands[i];
    uc.ArchivePath.BaseExtension = ext;
    uc.ArchivePath.VolExtension = typeExt;
    uc.ArchivePath.ParseFromPath(uc.UserArchivePath);
  }
  return true;
}

/*
struct CUpdateProduceCallbackImp: public IUpdateProduceCallback
{
  const CObjectVector<CArcItem> *_arcItems;
  IUpdateCallbackUI *_callback;
  
  CUpdateProduceCallbackImp(const CObjectVector<CArcItem> *a,
      IUpdateCallbackUI *callback): _arcItems(a), _callback(callback) {}
  virtual HRESULT ShowDeleteFile(int arcIndex);
};

HRESULT CUpdateProduceCallbackImp::ShowDeleteFile(int arcIndex)
{
  return _callback->ShowDeleteFile((*_arcItems)[arcIndex].Name);
}
*/

static HRESULT Compress(
    CCodecs *codecs,
    const CActionSet &actionSet,
    IInArchive *archive,
    const CCompressionMethodMode &compressionMethod,
    CArchivePath &archivePath,
    const CObjectVector<CArcItem> &arcItems,
    bool shareForWrite,
    bool stdInMode,
    /* const UString & stdInFileName, */
    bool stdOutMode,
    const CDirItems &dirItems,
    bool sfxMode,
    const FString &sfxModule,
    const CRecordVector<UInt64> &volumesSizes,
    CTempFiles &tempFiles,
    CUpdateErrorInfo &errorInfo,
    IUpdateCallbackUI *callback)
{
  CMyComPtr<IOutArchive> outArchive;
  if (archive != NULL)
  {
    CMyComPtr<IInArchive> archive2 = archive;
    HRESULT result = archive2.QueryInterface(IID_IOutArchive, &outArchive);
    if (result != S_OK)
      throw kUpdateIsNotSupoorted;
  }
  else
  {
    RINOK(codecs->CreateOutArchive(compressionMethod.FormatIndex, outArchive));

    #ifdef EXTERNAL_CODECS
    {
      CMyComPtr<ISetCompressCodecsInfo> setCompressCodecsInfo;
      outArchive.QueryInterface(IID_ISetCompressCodecsInfo, (void **)&setCompressCodecsInfo);
      if (setCompressCodecsInfo)
      {
        RINOK(setCompressCodecsInfo->SetCompressCodecsInfo(codecs));
      }
    }
    #endif
  }
  if (outArchive == 0)
    throw kUpdateIsNotSupoorted;
  
  NFileTimeType::EEnum fileTimeType;
  UInt32 value;
  RINOK(outArchive->GetFileTimeType(&value));

  switch(value)
  {
    case NFileTimeType::kWindows:
    case NFileTimeType::kUnix:
    case NFileTimeType::kDOS:
      fileTimeType = (NFileTimeType::EEnum)value;
      break;
    default:
      return E_FAIL;
  }

  CRecordVector<CUpdatePair2> updatePairs2;

  {
    CRecordVector<CUpdatePair> updatePairs;
    GetUpdatePairInfoList(dirItems, arcItems, fileTimeType, updatePairs); // must be done only once!!!
    // CUpdateProduceCallbackImp upCallback(&arcItems, callback);
    UpdateProduce(updatePairs, actionSet, updatePairs2, NULL /* &upCallback */);
  }

  UInt32 numFiles = 0;
  for (int i = 0; i < updatePairs2.Size(); i++)
    if (updatePairs2[i].NewData)
      numFiles++;
  
  RINOK(callback->SetNumFiles(numFiles));

  
  CArchiveUpdateCallback *updateCallbackSpec = new CArchiveUpdateCallback;
  CMyComPtr<IArchiveUpdateCallback> updateCallback(updateCallbackSpec);
  
  updateCallbackSpec->ShareForWrite = shareForWrite;
  updateCallbackSpec->StdInMode = stdInMode;
  updateCallbackSpec->Callback = callback;
  updateCallbackSpec->DirItems = &dirItems;
  updateCallbackSpec->ArcItems = &arcItems;
  updateCallbackSpec->UpdatePairs = &updatePairs2;

  CMyComPtr<ISequentialOutStream> outStream;

  if (!stdOutMode)
  {
    FString dirPrefix;
    if (!NFile::NDirectory::GetOnlyDirPrefix(us2fs(archivePath.GetFinalPath()), dirPrefix))
      throw 1417161;
    NFile::NDirectory::CreateComplexDirectory(dirPrefix);
  }

  COutFileStream *outStreamSpec = NULL;
  COutMultiVolStream *volStreamSpec = NULL;

  if (volumesSizes.Size() == 0)
  {
    if (stdOutMode)
      outStream = new CStdOutFileStream;
    else
    {
      outStreamSpec = new COutFileStream;
      outStream = outStreamSpec;
      bool isOK = false;
      FString realPath;
      for (int i = 0; i < (1 << 16); i++)
      {
        if (archivePath.Temp)
        {
          if (i > 0)
          {
            FChar s[16];
            ConvertUInt32ToString(i, s);
            archivePath.TempPostfix = s;
          }
          realPath = archivePath.GetTempPath();
        }
        else
          realPath = us2fs(archivePath.GetFinalPath());
        if (outStreamSpec->Create(realPath, false))
        {
          tempFiles.Paths.Add(realPath);
          isOK = true;
          break;
        }
        if (::GetLastError() != ERROR_FILE_EXISTS)
          break;
        if (!archivePath.Temp)
          break;
      }
      if (!isOK)
      {
        errorInfo.SystemError = ::GetLastError();
        errorInfo.FileName = realPath;
        errorInfo.Message = L"7-Zip cannot open file";
        return E_FAIL;
      }
    }
  }
  else
  {
    if (stdOutMode)
      return E_FAIL;
    volStreamSpec = new COutMultiVolStream;
    outStream = volStreamSpec;
    volStreamSpec->Sizes = volumesSizes;
    volStreamSpec->Prefix = us2fs(archivePath.GetFinalPath() + L".");
    volStreamSpec->TempFiles = &tempFiles;
    volStreamSpec->Init();

    /*
    updateCallbackSpec->VolumesSizes = volumesSizes;
    updateCallbackSpec->VolName = archivePath.Prefix + archivePath.Name;
    if (!archivePath.VolExtension.IsEmpty())
      updateCallbackSpec->VolExt = UString(L'.') + archivePath.VolExtension;
    */
  }

  RINOK(SetProperties(outArchive, compressionMethod.Properties));

  if (sfxMode)
  {
    CInFileStream *sfxStreamSpec = new CInFileStream;
    CMyComPtr<IInStream> sfxStream(sfxStreamSpec);
    if (!sfxStreamSpec->Open(sfxModule))
    {
      errorInfo.SystemError = ::GetLastError();
      errorInfo.Message = L"7-Zip cannot open SFX module";
      errorInfo.FileName = sfxModule;
      return E_FAIL;
    }

    CMyComPtr<ISequentialOutStream> sfxOutStream;
    COutFileStream *outStreamSpec = NULL;
    if (volumesSizes.Size() == 0)
      sfxOutStream = outStream;
    else
    {
      outStreamSpec = new COutFileStream;
      sfxOutStream = outStreamSpec;
      FString realPath = us2fs(archivePath.GetFinalPath());
      if (!outStreamSpec->Create(realPath, false))
      {
        errorInfo.SystemError = ::GetLastError();
        errorInfo.FileName = realPath;
        errorInfo.Message = L"7-Zip cannot open file";
        return E_FAIL;
      }
    }
    RINOK(NCompress::CopyStream(sfxStream, sfxOutStream, NULL));
    if (outStreamSpec)
    {
      RINOK(outStreamSpec->Close());
    }
  }

  HRESULT result = outArchive->UpdateItems(outStream, updatePairs2.Size(), updateCallback);
  callback->Finilize();
  RINOK(result);
  if (outStreamSpec)
    result = outStreamSpec->Close();
  else if (volStreamSpec)
    result = volStreamSpec->Close();
  return result;
}

HRESULT EnumerateInArchiveItems(const NWildcard::CCensor &censor,
    const CArc &arc,
    CObjectVector<CArcItem> &arcItems)
{
  arcItems.Clear();
  UInt32 numItems;
  IInArchive *archive = arc.Archive;
  RINOK(archive->GetNumberOfItems(&numItems));
  arcItems.Reserve(numItems);
  for (UInt32 i = 0; i < numItems; i++)
  {
    CArcItem ai;

    RINOK(arc.GetItemPath(i, ai.Name));
    RINOK(IsArchiveItemFolder(archive, i, ai.IsDir));
    ai.Censored = censor.CheckPath(ai.Name, !ai.IsDir);
    RINOK(arc.GetItemMTime(i, ai.MTime, ai.MTimeDefined));
    RINOK(arc.GetItemSize(i, ai.Size, ai.SizeDefined));

    {
      CPropVariant prop;
      RINOK(archive->GetProperty(i, kpidTimeType, &prop));
      if (prop.vt == VT_UI4)
      {
        ai.TimeType = (int)(NFileTimeType::EEnum)prop.ulVal;
        switch(ai.TimeType)
        {
          case NFileTimeType::kWindows:
          case NFileTimeType::kUnix:
          case NFileTimeType::kDOS:
            break;
          default:
            return E_FAIL;
        }
      }
    }

    ai.IndexInServer = i;
    arcItems.Add(ai);
  }
  return S_OK;
}


static HRESULT UpdateWithItemLists(
    CCodecs *codecs,
    CUpdateOptions &options,
    IInArchive *archive,
    const CObjectVector<CArcItem> &arcItems,
    CDirItems &dirItems,
    CTempFiles &tempFiles,
    CUpdateErrorInfo &errorInfo,
    IUpdateCallbackUI2 *callback)
{
  for(int i = 0; i < options.Commands.Size(); i++)
  {
    CUpdateArchiveCommand &command = options.Commands[i];
    if (options.StdOutMode)
    {
      RINOK(callback->StartArchive(L"stdout", archive != 0));
    }
    else
    {
      RINOK(callback->StartArchive(command.ArchivePath.GetFinalPath(),
          i == 0 && options.UpdateArchiveItself && archive != 0));
    }

    RINOK(Compress(
        codecs,
        command.ActionSet, archive,
        options.MethodMode,
        command.ArchivePath,
        arcItems,
        options.OpenShareForWrite,
        options.StdInMode,
        /* options.StdInFileName, */
        options.StdOutMode,
        dirItems,
        options.SfxMode, options.SfxModule,
        options.VolumesSizes,
        tempFiles,
        errorInfo, callback));

    RINOK(callback->FinishArchive());
  }
  return S_OK;
}

#if defined(_WIN32) && !defined(UNDER_CE)
class CCurrentDirRestorer
{
  FString _path;
public:
  CCurrentDirRestorer() { NFile::NDirectory::MyGetCurrentDirectory(_path); }
  ~CCurrentDirRestorer() { RestoreDirectory();}
  bool RestoreDirectory() const { return BOOLToBool(NFile::NDirectory::MySetCurrentDirectory(_path)); }
};
#endif

struct CEnumDirItemUpdateCallback: public IEnumDirItemCallback
{
  IUpdateCallbackUI2 *Callback;
  HRESULT ScanProgress(UInt64 numFolders, UInt64 numFiles, const wchar_t *path)
  {
    return Callback->ScanProgress(numFolders, numFiles, path);
  }
};

#ifdef _WIN32
typedef ULONG (FAR PASCAL MY_MAPISENDDOCUMENTS)(
  ULONG_PTR ulUIParam,
  LPSTR lpszDelimChar,
  LPSTR lpszFilePaths,
  LPSTR lpszFileNames,
  ULONG ulReserved
);
typedef MY_MAPISENDDOCUMENTS FAR *MY_LPMAPISENDDOCUMENTS;
#endif

HRESULT UpdateArchive(
    CCodecs *codecs,
    const NWildcard::CCensor &censor,
    CUpdateOptions &options,
    CUpdateErrorInfo &errorInfo,
    IOpenCallbackUI *openCallback,
    IUpdateCallbackUI2 *callback)
{
  if (options.StdOutMode && options.EMailMode)
    return E_FAIL;

  if (options.VolumesSizes.Size() > 0 && (options.EMailMode || options.SfxMode))
    return E_NOTIMPL;

  if (options.SfxMode)
  {
    CProperty property;
    property.Name = L"rsfx";
    property.Value = L"on";
    options.MethodMode.Properties.Add(property);
    if (options.SfxModule.IsEmpty())
    {
      errorInfo.Message = L"SFX file is not specified";
      return E_FAIL;
    }
    bool found = false;
    if (options.SfxModule.Find(FCHAR_PATH_SEPARATOR) < 0)
    {
      const FString fullName = NDLL::GetModuleDirPrefix() + options.SfxModule;
      if (NFind::DoesFileExist(fullName))
      {
        options.SfxModule = fullName;
        found = true;
      }
    }
    if (!found)
    {
      if (!NFind::DoesFileExist(options.SfxModule))
      {
        errorInfo.SystemError = ::GetLastError();
        errorInfo.Message = L"7-Zip cannot find specified SFX module";
        errorInfo.FileName = options.SfxModule;
        return E_FAIL;
      }
    }
  }


  CArchiveLink arcLink;
  const UString arcPath = options.ArchivePath.GetFinalPath();

  if (!options.ArchivePath.OriginalPath.IsEmpty())
  {
    NFind::CFileInfo fi;
    if (fi.Find(us2fs(arcPath)))
    {
      if (fi.IsDir())
        throw "there is no such archive";
      if (options.VolumesSizes.Size() > 0)
        return E_NOTIMPL;
      CIntVector formatIndices;
      if (options.MethodMode.FormatIndex >= 0)
        formatIndices.Add(options.MethodMode.FormatIndex);
      HRESULT result = arcLink.Open2(codecs, formatIndices, false, NULL, arcPath, openCallback);
      if (result == E_ABORT)
        return result;
      RINOK(callback->OpenResult(arcPath, result));
      RINOK(result);
      if (arcLink.VolumePaths.Size() > 1)
      {
        errorInfo.SystemError = (DWORD)E_NOTIMPL;
        errorInfo.Message = L"Updating for multivolume archives is not implemented";
        return E_NOTIMPL;
      }
      
      CArc &arc = arcLink.Arcs.Back();
      arc.MTimeDefined = !fi.IsDevice;
      arc.MTime = fi.MTime;
    }
  }
  else
  {
    /*
    if (archiveType.IsEmpty())
      throw "type of archive is not specified";
    */
  }

  CDirItems dirItems;
  if (options.StdInMode)
  {
    CDirItem di;
    di.Name = options.StdInFileName;
    di.Size = (UInt64)(Int64)-1;
    di.Attrib = 0;
    NTime::GetCurUtcFileTime(di.MTime);
    di.CTime = di.ATime = di.MTime;
    dirItems.Items.Add(di);
  }
  else
  {
    bool needScanning = false;
    for(int i = 0; i < options.Commands.Size(); i++)
      if (options.Commands[i].ActionSet.NeedScanning())
        needScanning = true;
    if (needScanning)
    {
      CEnumDirItemUpdateCallback enumCallback;
      enumCallback.Callback = callback;
      RINOK(callback->StartScanning());
      FStringVector errorPaths;
      CRecordVector<DWORD> errorCodes;
      HRESULT res = EnumerateItems(censor, dirItems, &enumCallback, errorPaths, errorCodes);
      for (int i = 0; i < errorPaths.Size(); i++)
      {
        RINOK(callback->CanNotFindError(fs2us(errorPaths[i]), errorCodes[i]));
      }
      if (res != S_OK)
      {
        if (res != E_ABORT)
          errorInfo.Message = L"Scanning error";
        return res;
      }
      RINOK(callback->FinishScanning());
    }
  }

  FString tempDirPrefix;
  bool usesTempDir = false;
  
  #ifdef _WIN32
  NDirectory::CTempDir tempDirectory;
  if (options.EMailMode && options.EMailRemoveAfter)
  {
    tempDirectory.Create(kTempFolderPrefix);
    tempDirPrefix = tempDirectory.GetPath();
    NormalizeDirPathPrefix(tempDirPrefix);
    usesTempDir = true;
  }
  #endif

  CTempFiles tempFiles;

  bool createTempFile = false;

  bool thereIsInArchive = arcLink.IsOpen;

  if (!options.StdOutMode && options.UpdateArchiveItself)
  {
    CArchivePath &ap = options.Commands[0].ArchivePath;
    ap = options.ArchivePath;
    // if ((archive != 0 && !usesTempDir) || !options.WorkingDir.IsEmpty())
    if ((thereIsInArchive || !options.WorkingDir.IsEmpty()) && !usesTempDir && options.VolumesSizes.Size() == 0)
    {
      createTempFile = true;
      ap.Temp = true;
      if (!options.WorkingDir.IsEmpty())
      {
        ap.TempPrefix = options.WorkingDir;
        NormalizeDirPathPrefix(ap.TempPrefix);
      }
    }
  }

  for (int i = 0; i < options.Commands.Size(); i++)
  {
    CArchivePath &ap = options.Commands[i].ArchivePath;
    if (usesTempDir)
    {
      // Check it
      ap.Prefix = fs2us(tempDirPrefix);
      // ap.Temp = true;
      // ap.TempPrefix = tempDirPrefix;
    }
    if (!options.StdOutMode &&
        (i > 0 || !createTempFile))
    {
      const FString path = us2fs(ap.GetFinalPath());
      if (NFind::DoesFileOrDirExist(path))
      {
        errorInfo.SystemError = 0;
        errorInfo.Message = L"The file already exists";
        errorInfo.FileName = path;
        return E_FAIL;
      }
    }
  }

  CObjectVector<CArcItem> arcItems;
  if (thereIsInArchive)
  {
    RINOK(EnumerateInArchiveItems(censor, arcLink.Arcs.Back(), arcItems));
  }

  RINOK(UpdateWithItemLists(codecs, options,
      thereIsInArchive ? arcLink.GetArchive() : 0,
      arcItems, dirItems,
      tempFiles, errorInfo, callback));

  if (thereIsInArchive)
  {
    RINOK(arcLink.Close());
    arcLink.Release();
  }

  tempFiles.Paths.Clear();
  if (createTempFile)
  {
    try
    {
      CArchivePath &ap = options.Commands[0].ArchivePath;
      const FString &tempPath = ap.GetTempPath();
      if (thereIsInArchive)
        if (!NDirectory::DeleteFileAlways(us2fs(arcPath)))
        {
          errorInfo.SystemError = ::GetLastError();
          errorInfo.Message = L"7-Zip cannot delete the file";
          errorInfo.FileName = us2fs(arcPath);
          return E_FAIL;
        }
      if (!NDirectory::MyMoveFile(tempPath, us2fs(arcPath)))
      {
        errorInfo.SystemError = ::GetLastError();
        errorInfo.Message = L"7-Zip cannot move the file";
        errorInfo.FileName = tempPath;
        errorInfo.FileName2 = us2fs(arcPath);
        return E_FAIL;
      }
    }
    catch(...)
    {
      throw;
    }
  }

  #if defined(_WIN32) && !defined(UNDER_CE)
  if (options.EMailMode)
  {
    NDLL::CLibrary mapiLib;
    if (!mapiLib.Load(FTEXT("Mapi32.dll")))
    {
      errorInfo.SystemError = ::GetLastError();
      errorInfo.Message = L"7-Zip cannot load Mapi32.dll";
      return E_FAIL;
    }
    MY_LPMAPISENDDOCUMENTS fnSend = (MY_LPMAPISENDDOCUMENTS)mapiLib.GetProc("MAPISendDocuments");
    if (fnSend == 0)
    {
      errorInfo.SystemError = ::GetLastError();
      errorInfo.Message = L"7-Zip cannot find MAPISendDocuments function";
      return E_FAIL;
    }
    FStringVector fullPaths;
    int i;
    for (i = 0; i < options.Commands.Size(); i++)
    {
      CArchivePath &ap = options.Commands[i].ArchivePath;
      FString arcPath;
      if (!NFile::NDirectory::MyGetFullPathName(us2fs(ap.GetFinalPath()), arcPath))
      {
        errorInfo.SystemError = ::GetLastError();
        errorInfo.Message = L"GetFullPathName error";
        return E_FAIL;
      }
      fullPaths.Add(arcPath);
    }
    CCurrentDirRestorer curDirRestorer;
    for (i = 0; i < fullPaths.Size(); i++)
    {
      UString arcPath = fs2us(fullPaths[i]);
      UString fileName = ExtractFileNameFromPath(arcPath);
      AString path = GetAnsiString(arcPath);
      AString name = GetAnsiString(fileName);
      // Warning!!! MAPISendDocuments function changes Current directory
      fnSend(0, ";", (LPSTR)(LPCSTR)path, (LPSTR)(LPCSTR)name, 0);
    }
  }
  #endif
  return S_OK;
}
