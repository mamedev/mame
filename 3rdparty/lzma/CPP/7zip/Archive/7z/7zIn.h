// 7zIn.h

#ifndef __7Z_IN_H
#define __7Z_IN_H

#include "../../../Common/MyCom.h"

#include "../../IPassword.h"
#include "../../IStream.h"

#include "../../Common/CreateCoder.h"
#include "../../Common/InBuffer.h"

#include "7zItem.h"
 
namespace NArchive {
namespace N7z {
  
struct CInArchiveInfo
{
  CArchiveVersion Version;
  UInt64 StartPosition;
  UInt64 StartPositionAfterHeader;
  UInt64 DataStartPosition;
  UInt64 DataStartPosition2;
  CRecordVector<UInt64> FileInfoPopIDs;
  void Clear()
  {
    FileInfoPopIDs.Clear();
  }
};

struct CArchiveDatabaseEx: public CArchiveDatabase
{
  CInArchiveInfo ArchiveInfo;
  CRecordVector<UInt64> PackStreamStartPositions;
  CRecordVector<CNum> FolderStartPackStreamIndex;
  CRecordVector<CNum> FolderStartFileIndex;
  CRecordVector<CNum> FileIndexToFolderIndexMap;

  UInt64 HeadersSize;
  UInt64 PhySize;

  void Clear()
  {
    CArchiveDatabase::Clear();
    ArchiveInfo.Clear();
    PackStreamStartPositions.Clear();
    FolderStartPackStreamIndex.Clear();
    FolderStartFileIndex.Clear();
    FileIndexToFolderIndexMap.Clear();

    HeadersSize = 0;
    PhySize = 0;
  }

  void FillFolderStartPackStream();
  void FillStartPos();
  void FillFolderStartFileIndex();

  void Fill()
  {
    FillFolderStartPackStream();
    FillStartPos();
    FillFolderStartFileIndex();
  }
  
  UInt64 GetFolderStreamPos(int folderIndex, int indexInFolder) const
  {
    return ArchiveInfo.DataStartPosition +
        PackStreamStartPositions[FolderStartPackStreamIndex[folderIndex] + indexInFolder];
  }
  
  UInt64 GetFolderFullPackSize(int folderIndex) const
  {
    CNum packStreamIndex = FolderStartPackStreamIndex[folderIndex];
    const CFolder &folder = Folders[folderIndex];
    UInt64 size = 0;
    for (int i = 0; i < folder.PackStreams.Size(); i++)
      size += PackSizes[packStreamIndex + i];
    return size;
  }
  
  UInt64 GetFolderPackStreamSize(int folderIndex, int streamIndex) const
  {
    return PackSizes[FolderStartPackStreamIndex[folderIndex] + streamIndex];
  }

  UInt64 GetFilePackSize(CNum fileIndex) const
  {
    CNum folderIndex = FileIndexToFolderIndexMap[fileIndex];
    if (folderIndex != kNumNoIndex)
      if (FolderStartFileIndex[folderIndex] == fileIndex)
        return GetFolderFullPackSize(folderIndex);
    return 0;
  }
};

class CInByte2
{
  const Byte *_buffer;
  size_t _size;
public:
  size_t _pos;
  void Init(const Byte *buffer, size_t size)
  {
    _buffer = buffer;
    _size = size;
    _pos = 0;
  }
  Byte ReadByte();
  void ReadBytes(Byte *data, size_t size);
  void SkipData(UInt64 size);
  void SkipData();
  UInt64 ReadNumber();
  CNum ReadNum();
  UInt32 ReadUInt32();
  UInt64 ReadUInt64();
  void ReadString(UString &s);
};

class CStreamSwitch;

const UInt32 kHeaderSize = 32;

class CInArchive
{
  friend class CStreamSwitch;

  CMyComPtr<IInStream> _stream;

  CObjectVector<CInByte2> _inByteVector;
  CInByte2 *_inByteBack;
 
  UInt64 _arhiveBeginStreamPosition;
  UInt64 _fileEndPosition;

  Byte _header[kHeaderSize];

  UInt64 HeadersSize;

  void AddByteStream(const Byte *buffer, size_t size)
  {
    _inByteVector.Add(CInByte2());
    _inByteBack = &_inByteVector.Back();
    _inByteBack->Init(buffer, size);
  }
  
  void DeleteByteStream()
  {
    _inByteVector.DeleteBack();
    if (!_inByteVector.IsEmpty())
      _inByteBack = &_inByteVector.Back();
  }

private:
  HRESULT FindAndReadSignature(IInStream *stream, const UInt64 *searchHeaderSizeLimit);
  
  void ReadBytes(Byte *data, size_t size) { _inByteBack->ReadBytes(data, size); }
  Byte ReadByte() { return _inByteBack->ReadByte(); }
  UInt64 ReadNumber() { return _inByteBack->ReadNumber(); }
  CNum ReadNum() { return _inByteBack->ReadNum(); }
  UInt64 ReadID() { return _inByteBack->ReadNumber(); }
  UInt32 ReadUInt32() { return _inByteBack->ReadUInt32(); }
  UInt64 ReadUInt64() { return _inByteBack->ReadUInt64(); }
  void SkipData(UInt64 size) { _inByteBack->SkipData(size); }
  void SkipData() { _inByteBack->SkipData(); }
  void WaitAttribute(UInt64 attribute);

  void ReadArchiveProperties(CInArchiveInfo &archiveInfo);
  void GetNextFolderItem(CFolder &itemInfo);
  void ReadHashDigests(int numItems,
      CBoolVector &digestsDefined, CRecordVector<UInt32> &digests);
  
  void ReadPackInfo(
      UInt64 &dataOffset,
      CRecordVector<UInt64> &packSizes,
      CBoolVector &packCRCsDefined,
      CRecordVector<UInt32> &packCRCs);
  
  void ReadUnpackInfo(
      const CObjectVector<CByteBuffer> *dataVector,
      CObjectVector<CFolder> &folders);
  
  void ReadSubStreamsInfo(
      const CObjectVector<CFolder> &folders,
      CRecordVector<CNum> &numUnpackStreamsInFolders,
      CRecordVector<UInt64> &unpackSizes,
      CBoolVector &digestsDefined,
      CRecordVector<UInt32> &digests);

  void ReadStreamsInfo(
      const CObjectVector<CByteBuffer> *dataVector,
      UInt64 &dataOffset,
      CRecordVector<UInt64> &packSizes,
      CBoolVector &packCRCsDefined,
      CRecordVector<UInt32> &packCRCs,
      CObjectVector<CFolder> &folders,
      CRecordVector<CNum> &numUnpackStreamsInFolders,
      CRecordVector<UInt64> &unpackSizes,
      CBoolVector &digestsDefined,
      CRecordVector<UInt32> &digests);


  void ReadBoolVector(int numItems, CBoolVector &v);
  void ReadBoolVector2(int numItems, CBoolVector &v);
  void ReadUInt64DefVector(const CObjectVector<CByteBuffer> &dataVector,
      CUInt64DefVector &v, int numFiles);
  HRESULT ReadAndDecodePackedStreams(
      DECL_EXTERNAL_CODECS_LOC_VARS
      UInt64 baseOffset, UInt64 &dataOffset,
      CObjectVector<CByteBuffer> &dataVector
      #ifndef _NO_CRYPTO
      , ICryptoGetTextPassword *getTextPassword, bool &passwordIsDefined
      #endif
      );
  HRESULT ReadHeader(
      DECL_EXTERNAL_CODECS_LOC_VARS
      CArchiveDatabaseEx &db
      #ifndef _NO_CRYPTO
      ,ICryptoGetTextPassword *getTextPassword, bool &passwordIsDefined
      #endif
      );
  HRESULT ReadDatabase2(
      DECL_EXTERNAL_CODECS_LOC_VARS
      CArchiveDatabaseEx &db
      #ifndef _NO_CRYPTO
      ,ICryptoGetTextPassword *getTextPassword, bool &passwordIsDefined
      #endif
      );
public:
  HRESULT Open(IInStream *stream, const UInt64 *searchHeaderSizeLimit); // S_FALSE means is not archive
  void Close();

  HRESULT ReadDatabase(
      DECL_EXTERNAL_CODECS_LOC_VARS
      CArchiveDatabaseEx &db
      #ifndef _NO_CRYPTO
      ,ICryptoGetTextPassword *getTextPassword, bool &passwordIsDefined
      #endif
      );
};
  
}}
  
#endif
