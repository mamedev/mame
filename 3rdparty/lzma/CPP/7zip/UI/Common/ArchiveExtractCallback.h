// ArchiveExtractCallback.h

#ifndef __ARCHIVE_EXTRACT_CALLBACK_H
#define __ARCHIVE_EXTRACT_CALLBACK_H

#include "Common/MyCom.h"
#include "Common/Wildcard.h"

#include "../../IPassword.h"

#include "../../Common/FileStreams.h"
#include "../../Common/ProgressUtils.h"

#include "../../Archive/IArchive.h"

#include "../../Archive/Common/OutStreamWithCRC.h"

#include "ExtractMode.h"
#include "IFileExtractCallback.h"
#include "OpenArchive.h"

class CArchiveExtractCallback:
  public IArchiveExtractCallback,
  // public IArchiveVolumeExtractCallback,
  public ICryptoGetTextPassword,
  public ICompressProgressInfo,
  public CMyUnknownImp
{
  const CArc *_arc;
  const NWildcard::CCensorNode *_wildcardCensor;
  CMyComPtr<IFolderArchiveExtractCallback> _extractCallback2;
  CMyComPtr<ICompressProgressInfo> _compressProgress;
  CMyComPtr<ICryptoGetTextPassword> _cryptoGetTextPassword;
  FString _directoryPath;
  NExtract::NPathMode::EEnum _pathMode;
  NExtract::NOverwriteMode::EEnum _overwriteMode;

  FString _diskFilePath;
  UString _filePath;
  UInt64 _position;
  bool _isSplit;

  bool _extractMode;

  bool WriteCTime;
  bool WriteATime;
  bool WriteMTime;

  bool _encrypted;

  struct CProcessedFileInfo
  {
    FILETIME CTime;
    FILETIME ATime;
    FILETIME MTime;
    UInt32 Attrib;
  
    bool CTimeDefined;
    bool ATimeDefined;
    bool MTimeDefined;
    bool AttribDefined;

    bool IsDir;
  } _fi;

  UInt32 _index;
  UInt64 _curSize;
  bool _curSizeDefined;
  COutFileStream *_outFileStreamSpec;
  CMyComPtr<ISequentialOutStream> _outFileStream;

  COutStreamWithCRC *_crcStreamSpec;
  CMyComPtr<ISequentialOutStream> _crcStream;

  UStringVector _removePathParts;

  bool _stdOutMode;
  bool _testMode;
  bool _crcMode;
  bool _multiArchives;

  CMyComPtr<ICompressProgressInfo> _localProgress;
  UInt64 _packTotal;
  UInt64 _unpTotal;

  void CreateComplexDirectory(const UStringVector &dirPathParts, FString &fullPath);
  HRESULT GetTime(int index, PROPID propID, FILETIME &filetime, bool &filetimeIsDefined);
  HRESULT GetUnpackSize();

  HRESULT SendMessageError(const char *message, const FString &path);

public:

  CLocalProgress *LocalProgressSpec;

  UInt64 NumFolders;
  UInt64 NumFiles;
  UInt64 UnpackSize;
  UInt32 CrcSum;
  
  MY_UNKNOWN_IMP2(ICryptoGetTextPassword, ICompressProgressInfo)
  // COM_INTERFACE_ENTRY(IArchiveVolumeExtractCallback)

  INTERFACE_IArchiveExtractCallback(;)

  STDMETHOD(SetRatioInfo)(const UInt64 *inSize, const UInt64 *outSize);

  // IArchiveVolumeExtractCallback
  // STDMETHOD(GetInStream)(const wchar_t *name, ISequentialInStream **inStream);

  STDMETHOD(CryptoGetTextPassword)(BSTR *password);

  CArchiveExtractCallback():
      WriteCTime(true),
      WriteATime(true),
      WriteMTime(true),
      _multiArchives(false)
  {
    LocalProgressSpec = new CLocalProgress();
    _localProgress = LocalProgressSpec;
  }

  void InitForMulti(bool multiArchives,
      NExtract::NPathMode::EEnum pathMode,
      NExtract::NOverwriteMode::EEnum overwriteMode)
  {
    _multiArchives = multiArchives;
    _pathMode = pathMode;
    _overwriteMode = overwriteMode;
    NumFolders = NumFiles = UnpackSize = 0;
    CrcSum = 0;
  }

  void Init(
      const NWildcard::CCensorNode *wildcardCensor,
      const CArc *arc,
      IFolderArchiveExtractCallback *extractCallback2,
      bool stdOutMode, bool testMode, bool crcMode,
      const FString &directoryPath,
      const UStringVector &removePathParts,
      UInt64 packSize);

};

#endif
