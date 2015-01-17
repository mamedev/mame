// 7zExtract.cpp

#include "StdAfx.h"

#include "../../../Common/ComTry.h"

#include "../../Common/ProgressUtils.h"

#include "7zDecode.h"
// #include "7z1Decode.h"
#include "7zFolderOutStream.h"
#include "7zHandler.h"

namespace NArchive {
namespace N7z {

struct CExtractFolderInfo
{
  #ifdef _7Z_VOL
  int VolumeIndex;
  #endif
  CNum FileIndex;
  CNum FolderIndex;
  CBoolVector ExtractStatuses;
  UInt64 UnpackSize;
  CExtractFolderInfo(
    #ifdef _7Z_VOL
    int volumeIndex,
    #endif
    CNum fileIndex, CNum folderIndex):
    #ifdef _7Z_VOL
    VolumeIndex(volumeIndex),
    #endif
    FileIndex(fileIndex),
    FolderIndex(folderIndex),
    UnpackSize(0)
  {
    if (fileIndex != kNumNoIndex)
    {
      ExtractStatuses.Reserve(1);
      ExtractStatuses.Add(true);
    }
  };
};

STDMETHODIMP CHandler::Extract(const UInt32 *indices, UInt32 numItems,
    Int32 testModeSpec, IArchiveExtractCallback *extractCallbackSpec)
{
  COM_TRY_BEGIN
  bool testMode = (testModeSpec != 0);
  CMyComPtr<IArchiveExtractCallback> extractCallback = extractCallbackSpec;
  UInt64 importantTotalUnpacked = 0;

  bool allFilesMode = (numItems == (UInt32)-1);
  if (allFilesMode)
    numItems =
    #ifdef _7Z_VOL
    _refs.Size();
    #else
    _db.Files.Size();
    #endif

  if(numItems == 0)
    return S_OK;

  /*
  if(_volumes.Size() != 1)
    return E_FAIL;
  const CVolume &volume = _volumes.Front();
  const CArchiveDatabaseEx &_db = volume.Database;
  IInStream *_inStream = volume.Stream;
  */
  
  CObjectVector<CExtractFolderInfo> extractFolderInfoVector;
  for (UInt32 ii = 0; ii < numItems; ii++)
  {
    // UInt32 fileIndex = allFilesMode ? indexIndex : indices[indexIndex];
    UInt32 ref2Index = allFilesMode ? ii : indices[ii];
    // const CRef2 &ref2 = _refs[ref2Index];

    // for (UInt32 ri = 0; ri < ref2.Refs.Size(); ri++)
    {
      #ifdef _7Z_VOL
      // const CRef &ref = ref2.Refs[ri];
      const CRef &ref = _refs[ref2Index];

      int volumeIndex = ref.VolumeIndex;
      const CVolume &volume = _volumes[volumeIndex];
      const CArchiveDatabaseEx &db = volume.Database;
      UInt32 fileIndex = ref.ItemIndex;
      #else
      const CArchiveDatabaseEx &db = _db;
      UInt32 fileIndex = ref2Index;
      #endif

      CNum folderIndex = db.FileIndexToFolderIndexMap[fileIndex];
      if (folderIndex == kNumNoIndex)
      {
        extractFolderInfoVector.Add(CExtractFolderInfo(
            #ifdef _7Z_VOL
            volumeIndex,
            #endif
            fileIndex, kNumNoIndex));
        continue;
      }
      if (extractFolderInfoVector.IsEmpty() ||
        folderIndex != extractFolderInfoVector.Back().FolderIndex
        #ifdef _7Z_VOL
        || volumeIndex != extractFolderInfoVector.Back().VolumeIndex
        #endif
        )
      {
        extractFolderInfoVector.Add(CExtractFolderInfo(
            #ifdef _7Z_VOL
            volumeIndex,
            #endif
            kNumNoIndex, folderIndex));
        const CFolder &folderInfo = db.Folders[folderIndex];
        UInt64 unpackSize = folderInfo.GetUnpackSize();
        importantTotalUnpacked += unpackSize;
        extractFolderInfoVector.Back().UnpackSize = unpackSize;
      }
      
      CExtractFolderInfo &efi = extractFolderInfoVector.Back();
      
      // const CFolderInfo &folderInfo = m_dam_Folders[folderIndex];
      CNum startIndex = db.FolderStartFileIndex[folderIndex];
      for (CNum index = efi.ExtractStatuses.Size();
          index <= fileIndex - startIndex; index++)
      {
        // UInt64 unpackSize = _db.Files[startIndex + index].UnpackSize;
        // Count partial_folder_size
        // efi.UnpackSize += unpackSize;
        // importantTotalUnpacked += unpackSize;
        efi.ExtractStatuses.Add(index == fileIndex - startIndex);
      }
    }
  }

  RINOK(extractCallback->SetTotal(importantTotalUnpacked));

  CDecoder decoder(
    #ifdef _ST_MODE
    false
    #else
    true
    #endif
    );
  // CDecoder1 decoder;

  UInt64 totalPacked = 0;
  UInt64 totalUnpacked = 0;
  UInt64 curPacked, curUnpacked;

  CLocalProgress *lps = new CLocalProgress;
  CMyComPtr<ICompressProgressInfo> progress = lps;
  lps->Init(extractCallback, false);

  for (int i = 0;; i++, totalUnpacked += curUnpacked, totalPacked += curPacked)
  {
    lps->OutSize = totalUnpacked;
    lps->InSize = totalPacked;
    RINOK(lps->SetCur());

    if (i >= extractFolderInfoVector.Size())
      break;
    
    const CExtractFolderInfo &efi = extractFolderInfoVector[i];
    curUnpacked = efi.UnpackSize;
    curPacked = 0;

    CFolderOutStream *folderOutStream = new CFolderOutStream;
    CMyComPtr<ISequentialOutStream> outStream(folderOutStream);

    #ifdef _7Z_VOL
    const CVolume &volume = _volumes[efi.VolumeIndex];
    const CArchiveDatabaseEx &db = volume.Database;
    #else
    const CArchiveDatabaseEx &db = _db;
    #endif

    CNum startIndex;
    if (efi.FileIndex != kNumNoIndex)
      startIndex = efi.FileIndex;
    else
      startIndex = db.FolderStartFileIndex[efi.FolderIndex];

    HRESULT result = folderOutStream->Init(&db,
        #ifdef _7Z_VOL
        volume.StartRef2Index,
        #else
        0,
        #endif
        startIndex,
        &efi.ExtractStatuses, extractCallback, testMode, _crcSize != 0);

    RINOK(result);

    if (efi.FileIndex != kNumNoIndex)
      continue;

    CNum folderIndex = efi.FolderIndex;
    const CFolder &folderInfo = db.Folders[folderIndex];

    curPacked = _db.GetFolderFullPackSize(folderIndex);

    CNum packStreamIndex = db.FolderStartPackStreamIndex[folderIndex];
    UInt64 folderStartPackPos = db.GetFolderStreamPos(folderIndex, 0);

    #ifndef _NO_CRYPTO
    CMyComPtr<ICryptoGetTextPassword> getTextPassword;
    if (extractCallback)
      extractCallback.QueryInterface(IID_ICryptoGetTextPassword, &getTextPassword);
    #endif

    try
    {
      #ifndef _NO_CRYPTO
      bool passwordIsDefined;
      #endif

      HRESULT result = decoder.Decode(
          EXTERNAL_CODECS_VARS
          #ifdef _7Z_VOL
          volume.Stream,
          #else
          _inStream,
          #endif
          folderStartPackPos,
          &db.PackSizes[packStreamIndex],
          folderInfo,
          outStream,
          progress
          #ifndef _NO_CRYPTO
          , getTextPassword, passwordIsDefined
          #endif
          #if !defined(_7ZIP_ST) && !defined(_SFX)
          , true, _numThreads
          #endif
          );

      if (result == S_FALSE)
      {
        RINOK(folderOutStream->FlushCorrupted(NExtract::NOperationResult::kDataError));
        continue;
      }
      if (result == E_NOTIMPL)
      {
        RINOK(folderOutStream->FlushCorrupted(NExtract::NOperationResult::kUnSupportedMethod));
        continue;
      }
      if (result != S_OK)
        return result;
      if (folderOutStream->WasWritingFinished() != S_OK)
      {
        RINOK(folderOutStream->FlushCorrupted(NExtract::NOperationResult::kDataError));
        continue;
      }
    }
    catch(...)
    {
      RINOK(folderOutStream->FlushCorrupted(NExtract::NOperationResult::kDataError));
      continue;
    }
  }
  return S_OK;
  COM_TRY_END
}

}}
