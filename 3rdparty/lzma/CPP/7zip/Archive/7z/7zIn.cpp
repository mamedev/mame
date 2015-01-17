// 7zIn.cpp

#include "StdAfx.h"

#include "../../../../C/7zCrc.h"
#include "../../../../C/CpuArch.h"

#include "../../Common/StreamObjects.h"
#include "../../Common/StreamUtils.h"

#include "7zDecode.h"
#include "7zIn.h"

#define Get16(p) GetUi16(p)
#define Get32(p) GetUi32(p)
#define Get64(p) GetUi64(p)

// define FORMAT_7Z_RECOVERY if you want to recover multivolume archives with empty StartHeader
#ifndef _SFX
#define FORMAT_7Z_RECOVERY
#endif

namespace NArchive {
namespace N7z {

static void BoolVector_Fill_False(CBoolVector &v, int size)
{
  v.Clear();
  v.Reserve(size);
  for (int i = 0; i < size; i++)
    v.Add(false);
}

static bool BoolVector_GetAndSet(CBoolVector &v, UInt32 index)
{
  if (index >= (UInt32)v.Size())
    return true;
  bool res = v[index];
  v[index] = true;
  return res;
}

bool CFolder::CheckStructure() const
{
  const int kNumCodersMax = sizeof(UInt32) * 8; // don't change it
  const int kMaskSize = sizeof(UInt32) * 8; // it must be >= kNumCodersMax
  const int kNumBindsMax = 32;

  if (Coders.Size() > kNumCodersMax || BindPairs.Size() > kNumBindsMax)
    return false;

  {
    CBoolVector v;
    BoolVector_Fill_False(v, BindPairs.Size() + PackStreams.Size());
    
    int i;
    for (i = 0; i < BindPairs.Size(); i++)
      if (BoolVector_GetAndSet(v, BindPairs[i].InIndex))
        return false;
    for (i = 0; i < PackStreams.Size(); i++)
      if (BoolVector_GetAndSet(v, PackStreams[i]))
        return false;
    
    BoolVector_Fill_False(v, UnpackSizes.Size());
    for (i = 0; i < BindPairs.Size(); i++)
      if (BoolVector_GetAndSet(v, BindPairs[i].OutIndex))
        return false;
  }
  
  UInt32 mask[kMaskSize];
  int i;
  for (i = 0; i < kMaskSize; i++)
    mask[i] = 0;

  {
    CIntVector inStreamToCoder, outStreamToCoder;
    for (i = 0; i < Coders.Size(); i++)
    {
      CNum j;
      const CCoderInfo &coder = Coders[i];
      for (j = 0; j < coder.NumInStreams; j++)
        inStreamToCoder.Add(i);
      for (j = 0; j < coder.NumOutStreams; j++)
        outStreamToCoder.Add(i);
    }
    
    for (i = 0; i < BindPairs.Size(); i++)
    {
      const CBindPair &bp = BindPairs[i];
      mask[inStreamToCoder[bp.InIndex]] |= (1 << outStreamToCoder[bp.OutIndex]);
    }
  }
  
  for (i = 0; i < kMaskSize; i++)
    for (int j = 0; j < kMaskSize; j++)
      if (((1 << j) & mask[i]) != 0)
        mask[i] |= mask[j];

  for (i = 0; i < kMaskSize; i++)
    if (((1 << i) & mask[i]) != 0)
      return false;

  return true;
}

class CInArchiveException {};

static void ThrowException() { throw CInArchiveException(); }
static inline void ThrowEndOfData()   { ThrowException(); }
static inline void ThrowUnsupported() { ThrowException(); }
static inline void ThrowIncorrect()   { ThrowException(); }
static inline void ThrowUnsupportedVersion() { ThrowException(); }

/*
class CInArchiveException
{
public:
  enum CCauseType
  {
    kUnsupportedVersion = 0,
    kUnsupported,
    kIncorrect,
    kEndOfData
  } Cause;
  CInArchiveException(CCauseType cause): Cause(cause) {};
};

static void ThrowException(CInArchiveException::CCauseType c) { throw CInArchiveException(c); }
static void ThrowEndOfData()   { ThrowException(CInArchiveException::kEndOfData); }
static void ThrowUnsupported() { ThrowException(CInArchiveException::kUnsupported); }
static void ThrowIncorrect()   { ThrowException(CInArchiveException::kIncorrect); }
static void ThrowUnsupportedVersion() { ThrowException(CInArchiveException::kUnsupportedVersion); }
*/

class CStreamSwitch
{
  CInArchive *_archive;
  bool _needRemove;
public:
  CStreamSwitch(): _needRemove(false) {}
  ~CStreamSwitch() { Remove(); }
  void Remove();
  void Set(CInArchive *archive, const Byte *data, size_t size);
  void Set(CInArchive *archive, const CByteBuffer &byteBuffer);
  void Set(CInArchive *archive, const CObjectVector<CByteBuffer> *dataVector);
};

void CStreamSwitch::Remove()
{
  if (_needRemove)
  {
    _archive->DeleteByteStream();
    _needRemove = false;
  }
}

void CStreamSwitch::Set(CInArchive *archive, const Byte *data, size_t size)
{
  Remove();
  _archive = archive;
  _archive->AddByteStream(data, size);
  _needRemove = true;
}

void CStreamSwitch::Set(CInArchive *archive, const CByteBuffer &byteBuffer)
{
  Set(archive, byteBuffer, byteBuffer.GetCapacity());
}

void CStreamSwitch::Set(CInArchive *archive, const CObjectVector<CByteBuffer> *dataVector)
{
  Remove();
  Byte external = archive->ReadByte();
  if (external != 0)
  {
    int dataIndex = (int)archive->ReadNum();
    if (dataIndex < 0 || dataIndex >= dataVector->Size())
      ThrowIncorrect();
    Set(archive, (*dataVector)[dataIndex]);
  }
}

Byte CInByte2::ReadByte()
{
  if (_pos >= _size)
    ThrowEndOfData();
  return _buffer[_pos++];
}

void CInByte2::ReadBytes(Byte *data, size_t size)
{
  if (size > _size - _pos)
    ThrowEndOfData();
  for (size_t i = 0; i < size; i++)
    data[i] = _buffer[_pos++];
}

void CInByte2::SkipData(UInt64 size)
{
  if (size > _size - _pos)
    ThrowEndOfData();
  _pos += (size_t)size;
}

void CInByte2::SkipData()
{
  SkipData(ReadNumber());
}

UInt64 CInByte2::ReadNumber()
{
  if (_pos >= _size)
    ThrowEndOfData();
  Byte firstByte = _buffer[_pos++];
  Byte mask = 0x80;
  UInt64 value = 0;
  for (int i = 0; i < 8; i++)
  {
    if ((firstByte & mask) == 0)
    {
      UInt64 highPart = firstByte & (mask - 1);
      value += (highPart << (i * 8));
      return value;
    }
    if (_pos >= _size)
      ThrowEndOfData();
    value |= ((UInt64)_buffer[_pos++] << (8 * i));
    mask >>= 1;
  }
  return value;
}

CNum CInByte2::ReadNum()
{
  UInt64 value = ReadNumber();
  if (value > kNumMax)
    ThrowUnsupported();
  return (CNum)value;
}

UInt32 CInByte2::ReadUInt32()
{
  if (_pos + 4 > _size)
    ThrowEndOfData();
  UInt32 res = Get32(_buffer + _pos);
  _pos += 4;
  return res;
}

UInt64 CInByte2::ReadUInt64()
{
  if (_pos + 8 > _size)
    ThrowEndOfData();
  UInt64 res = Get64(_buffer + _pos);
  _pos += 8;
  return res;
}

void CInByte2::ReadString(UString &s)
{
  const Byte *buf = _buffer + _pos;
  size_t rem = (_size - _pos) / 2 * 2;
  {
    size_t i;
    for (i = 0; i < rem; i += 2)
      if (buf[i] == 0 && buf[i + 1] == 0)
        break;
    if (i == rem)
      ThrowEndOfData();
    rem = i;
  }
  int len = (int)(rem / 2);
  if (len < 0 || (size_t)len * 2 != rem)
    ThrowUnsupported();
  wchar_t *p = s.GetBuffer(len);
  int i;
  for (i = 0; i < len; i++, buf += 2)
    p[i] = (wchar_t)Get16(buf);
  s.ReleaseBuffer(len);
  _pos += rem + 2;
}

static inline bool TestSignature(const Byte *p)
{
  for (int i = 0; i < kSignatureSize; i++)
    if (p[i] != kSignature[i])
      return false;
  return CrcCalc(p + 12, 20) == GetUi32(p + 8);
}

#ifdef FORMAT_7Z_RECOVERY
static inline bool TestSignature2(const Byte *p)
{
  int i;
  for (i = 0; i < kSignatureSize; i++)
    if (p[i] != kSignature[i])
      return false;
  if (CrcCalc(p + 12, 20) == GetUi32(p + 8))
    return true;
  for (i = 8; i < kHeaderSize; i++)
    if (p[i] != 0)
      return false;
  return (p[6] != 0 || p[7] != 0);
}
#else
#define TestSignature2(p) TestSignature(p)
#endif

HRESULT CInArchive::FindAndReadSignature(IInStream *stream, const UInt64 *searchHeaderSizeLimit)
{
  RINOK(ReadStream_FALSE(stream, _header, kHeaderSize));

  if (TestSignature2(_header))
    return S_OK;

  CByteBuffer byteBuffer;
  const UInt32 kBufferSize = (1 << 16);
  byteBuffer.SetCapacity(kBufferSize);
  Byte *buffer = byteBuffer;
  memcpy(buffer, _header, kHeaderSize);
  UInt64 curTestPos = _arhiveBeginStreamPosition;
  for (;;)
  {
    if (searchHeaderSizeLimit != NULL)
      if (curTestPos - _arhiveBeginStreamPosition > *searchHeaderSizeLimit)
        break;
    UInt32 processedSize;
    RINOK(stream->Read(buffer + kHeaderSize, kBufferSize - kHeaderSize, &processedSize));
    if (processedSize == 0)
      return S_FALSE;
    for (UInt32 pos = 1; pos <= processedSize; pos++)
    {
      for (; buffer[pos] != '7' && pos <= processedSize; pos++);
      if (pos > processedSize)
        break;
      if (TestSignature(buffer + pos))
      {
        memcpy(_header, buffer + pos, kHeaderSize);
        curTestPos += pos;
        _arhiveBeginStreamPosition = curTestPos;
        return stream->Seek(curTestPos + kHeaderSize, STREAM_SEEK_SET, NULL);
      }
    }
    curTestPos += processedSize;
    memmove(buffer, buffer + processedSize, kHeaderSize);
  }
  return S_FALSE;
}

// S_FALSE means that file is not archive
HRESULT CInArchive::Open(IInStream *stream, const UInt64 *searchHeaderSizeLimit)
{
  HeadersSize = 0;
  Close();
  RINOK(stream->Seek(0, STREAM_SEEK_CUR, &_arhiveBeginStreamPosition))
  RINOK(stream->Seek(0, STREAM_SEEK_END, &_fileEndPosition))
  RINOK(stream->Seek(_arhiveBeginStreamPosition, STREAM_SEEK_SET, NULL))
  RINOK(FindAndReadSignature(stream, searchHeaderSizeLimit));
  _stream = stream;
  return S_OK;
}
  
void CInArchive::Close()
{
  _stream.Release();
}

void CInArchive::ReadArchiveProperties(CInArchiveInfo & /* archiveInfo */)
{
  for (;;)
  {
    if (ReadID() == NID::kEnd)
      break;
    SkipData();
  }
}

void CInArchive::GetNextFolderItem(CFolder &folder)
{
  CNum numCoders = ReadNum();

  folder.Coders.Clear();
  folder.Coders.Reserve((int)numCoders);
  CNum numInStreams = 0;
  CNum numOutStreams = 0;
  CNum i;
  for (i = 0; i < numCoders; i++)
  {
    folder.Coders.Add(CCoderInfo());
    CCoderInfo &coder = folder.Coders.Back();

    {
      Byte mainByte = ReadByte();
      int idSize = (mainByte & 0xF);
      Byte longID[15];
      ReadBytes(longID, idSize);
      if (idSize > 8)
        ThrowUnsupported();
      UInt64 id = 0;
      for (int j = 0; j < idSize; j++)
        id |= (UInt64)longID[idSize - 1 - j] << (8 * j);
      coder.MethodID = id;

      if ((mainByte & 0x10) != 0)
      {
        coder.NumInStreams = ReadNum();
        coder.NumOutStreams = ReadNum();
      }
      else
      {
        coder.NumInStreams = 1;
        coder.NumOutStreams = 1;
      }
      if ((mainByte & 0x20) != 0)
      {
        CNum propsSize = ReadNum();
        coder.Props.SetCapacity((size_t)propsSize);
        ReadBytes((Byte *)coder.Props, (size_t)propsSize);
      }
      if ((mainByte & 0x80) != 0)
        ThrowUnsupported();
    }
    numInStreams += coder.NumInStreams;
    numOutStreams += coder.NumOutStreams;
  }

  CNum numBindPairs = numOutStreams - 1;
  folder.BindPairs.Clear();
  folder.BindPairs.Reserve(numBindPairs);
  for (i = 0; i < numBindPairs; i++)
  {
    CBindPair bp;
    bp.InIndex = ReadNum();
    bp.OutIndex = ReadNum();
    folder.BindPairs.Add(bp);
  }

  if (numInStreams < numBindPairs)
    ThrowUnsupported();
  CNum numPackStreams = numInStreams - numBindPairs;
  folder.PackStreams.Reserve(numPackStreams);
  if (numPackStreams == 1)
  {
    for (i = 0; i < numInStreams; i++)
      if (folder.FindBindPairForInStream(i) < 0)
      {
        folder.PackStreams.Add(i);
        break;
      }
    if (folder.PackStreams.Size() != 1)
      ThrowUnsupported();
  }
  else
    for (i = 0; i < numPackStreams; i++)
      folder.PackStreams.Add(ReadNum());
}

void CInArchive::WaitAttribute(UInt64 attribute)
{
  for (;;)
  {
    UInt64 type = ReadID();
    if (type == attribute)
      return;
    if (type == NID::kEnd)
      ThrowIncorrect();
    SkipData();
  }
}

void CInArchive::ReadHashDigests(int numItems,
    CBoolVector &digestsDefined,
    CRecordVector<UInt32> &digests)
{
  ReadBoolVector2(numItems, digestsDefined);
  digests.Clear();
  digests.Reserve(numItems);
  for (int i = 0; i < numItems; i++)
  {
    UInt32 crc = 0;
    if (digestsDefined[i])
      crc = ReadUInt32();
    digests.Add(crc);
  }
}

void CInArchive::ReadPackInfo(
    UInt64 &dataOffset,
    CRecordVector<UInt64> &packSizes,
    CBoolVector &packCRCsDefined,
    CRecordVector<UInt32> &packCRCs)
{
  dataOffset = ReadNumber();
  CNum numPackStreams = ReadNum();

  WaitAttribute(NID::kSize);
  packSizes.Clear();
  packSizes.Reserve(numPackStreams);
  for (CNum i = 0; i < numPackStreams; i++)
    packSizes.Add(ReadNumber());

  UInt64 type;
  for (;;)
  {
    type = ReadID();
    if (type == NID::kEnd)
      break;
    if (type == NID::kCRC)
    {
      ReadHashDigests(numPackStreams, packCRCsDefined, packCRCs);
      continue;
    }
    SkipData();
  }
  if (packCRCsDefined.IsEmpty())
  {
    BoolVector_Fill_False(packCRCsDefined, numPackStreams);
    packCRCs.Reserve(numPackStreams);
    packCRCs.Clear();
    for (CNum i = 0; i < numPackStreams; i++)
      packCRCs.Add(0);
  }
}

void CInArchive::ReadUnpackInfo(
    const CObjectVector<CByteBuffer> *dataVector,
    CObjectVector<CFolder> &folders)
{
  WaitAttribute(NID::kFolder);
  CNum numFolders = ReadNum();

  {
    CStreamSwitch streamSwitch;
    streamSwitch.Set(this, dataVector);
    folders.Clear();
    folders.Reserve(numFolders);
    for (CNum i = 0; i < numFolders; i++)
    {
      folders.Add(CFolder());
      GetNextFolderItem(folders.Back());
    }
  }

  WaitAttribute(NID::kCodersUnpackSize);

  CNum i;
  for (i = 0; i < numFolders; i++)
  {
    CFolder &folder = folders[i];
    CNum numOutStreams = folder.GetNumOutStreams();
    folder.UnpackSizes.Reserve(numOutStreams);
    for (CNum j = 0; j < numOutStreams; j++)
      folder.UnpackSizes.Add(ReadNumber());
  }

  for (;;)
  {
    UInt64 type = ReadID();
    if (type == NID::kEnd)
      return;
    if (type == NID::kCRC)
    {
      CBoolVector crcsDefined;
      CRecordVector<UInt32> crcs;
      ReadHashDigests(numFolders, crcsDefined, crcs);
      for (i = 0; i < numFolders; i++)
      {
        CFolder &folder = folders[i];
        folder.UnpackCRCDefined = crcsDefined[i];
        folder.UnpackCRC = crcs[i];
      }
      continue;
    }
    SkipData();
  }
}

void CInArchive::ReadSubStreamsInfo(
    const CObjectVector<CFolder> &folders,
    CRecordVector<CNum> &numUnpackStreamsInFolders,
    CRecordVector<UInt64> &unpackSizes,
    CBoolVector &digestsDefined,
    CRecordVector<UInt32> &digests)
{
  numUnpackStreamsInFolders.Clear();
  numUnpackStreamsInFolders.Reserve(folders.Size());
  UInt64 type;
  for (;;)
  {
    type = ReadID();
    if (type == NID::kNumUnpackStream)
    {
      for (int i = 0; i < folders.Size(); i++)
        numUnpackStreamsInFolders.Add(ReadNum());
      continue;
    }
    if (type == NID::kCRC || type == NID::kSize)
      break;
    if (type == NID::kEnd)
      break;
    SkipData();
  }

  if (numUnpackStreamsInFolders.IsEmpty())
    for (int i = 0; i < folders.Size(); i++)
      numUnpackStreamsInFolders.Add(1);

  int i;
  for (i = 0; i < numUnpackStreamsInFolders.Size(); i++)
  {
    // v3.13 incorrectly worked with empty folders
    // v4.07: we check that folder is empty
    CNum numSubstreams = numUnpackStreamsInFolders[i];
    if (numSubstreams == 0)
      continue;
    UInt64 sum = 0;
    for (CNum j = 1; j < numSubstreams; j++)
      if (type == NID::kSize)
      {
        UInt64 size = ReadNumber();
        unpackSizes.Add(size);
        sum += size;
      }
    unpackSizes.Add(folders[i].GetUnpackSize() - sum);
  }
  if (type == NID::kSize)
    type = ReadID();

  int numDigests = 0;
  int numDigestsTotal = 0;
  for (i = 0; i < folders.Size(); i++)
  {
    CNum numSubstreams = numUnpackStreamsInFolders[i];
    if (numSubstreams != 1 || !folders[i].UnpackCRCDefined)
      numDigests += numSubstreams;
    numDigestsTotal += numSubstreams;
  }

  for (;;)
  {
    if (type == NID::kCRC)
    {
      CBoolVector digestsDefined2;
      CRecordVector<UInt32> digests2;
      ReadHashDigests(numDigests, digestsDefined2, digests2);
      int digestIndex = 0;
      for (i = 0; i < folders.Size(); i++)
      {
        CNum numSubstreams = numUnpackStreamsInFolders[i];
        const CFolder &folder = folders[i];
        if (numSubstreams == 1 && folder.UnpackCRCDefined)
        {
          digestsDefined.Add(true);
          digests.Add(folder.UnpackCRC);
        }
        else
          for (CNum j = 0; j < numSubstreams; j++, digestIndex++)
          {
            digestsDefined.Add(digestsDefined2[digestIndex]);
            digests.Add(digests2[digestIndex]);
          }
      }
    }
    else if (type == NID::kEnd)
    {
      if (digestsDefined.IsEmpty())
      {
        BoolVector_Fill_False(digestsDefined, numDigestsTotal);
        digests.Clear();
        for (int i = 0; i < numDigestsTotal; i++)
          digests.Add(0);
      }
      return;
    }
    else
      SkipData();
    type = ReadID();
  }
}

void CInArchive::ReadStreamsInfo(
    const CObjectVector<CByteBuffer> *dataVector,
    UInt64 &dataOffset,
    CRecordVector<UInt64> &packSizes,
    CBoolVector &packCRCsDefined,
    CRecordVector<UInt32> &packCRCs,
    CObjectVector<CFolder> &folders,
    CRecordVector<CNum> &numUnpackStreamsInFolders,
    CRecordVector<UInt64> &unpackSizes,
    CBoolVector &digestsDefined,
    CRecordVector<UInt32> &digests)
{
  for (;;)
  {
    UInt64 type = ReadID();
    if (type > ((UInt32)1 << 30))
      ThrowIncorrect();
    switch((UInt32)type)
    {
      case NID::kEnd:
        return;
      case NID::kPackInfo:
      {
        ReadPackInfo(dataOffset, packSizes, packCRCsDefined, packCRCs);
        break;
      }
      case NID::kUnpackInfo:
      {
        ReadUnpackInfo(dataVector, folders);
        break;
      }
      case NID::kSubStreamsInfo:
      {
        ReadSubStreamsInfo(folders, numUnpackStreamsInFolders,
            unpackSizes, digestsDefined, digests);
        break;
      }
      default:
        ThrowIncorrect();
    }
  }
}

void CInArchive::ReadBoolVector(int numItems, CBoolVector &v)
{
  v.Clear();
  v.Reserve(numItems);
  Byte b = 0;
  Byte mask = 0;
  for (int i = 0; i < numItems; i++)
  {
    if (mask == 0)
    {
      b = ReadByte();
      mask = 0x80;
    }
    v.Add((b & mask) != 0);
    mask >>= 1;
  }
}

void CInArchive::ReadBoolVector2(int numItems, CBoolVector &v)
{
  Byte allAreDefined = ReadByte();
  if (allAreDefined == 0)
  {
    ReadBoolVector(numItems, v);
    return;
  }
  v.Clear();
  v.Reserve(numItems);
  for (int i = 0; i < numItems; i++)
    v.Add(true);
}

void CInArchive::ReadUInt64DefVector(const CObjectVector<CByteBuffer> &dataVector,
    CUInt64DefVector &v, int numFiles)
{
  ReadBoolVector2(numFiles, v.Defined);

  CStreamSwitch streamSwitch;
  streamSwitch.Set(this, &dataVector);
  v.Values.Reserve(numFiles);

  for (int i = 0; i < numFiles; i++)
  {
    UInt64 t = 0;
    if (v.Defined[i])
      t = ReadUInt64();
    v.Values.Add(t);
  }
}

HRESULT CInArchive::ReadAndDecodePackedStreams(
    DECL_EXTERNAL_CODECS_LOC_VARS
    UInt64 baseOffset,
    UInt64 &dataOffset, CObjectVector<CByteBuffer> &dataVector
    #ifndef _NO_CRYPTO
    , ICryptoGetTextPassword *getTextPassword, bool &passwordIsDefined
    #endif
    )
{
  CRecordVector<UInt64> packSizes;
  CBoolVector packCRCsDefined;
  CRecordVector<UInt32> packCRCs;
  CObjectVector<CFolder> folders;
  
  CRecordVector<CNum> numUnpackStreamsInFolders;
  CRecordVector<UInt64> unpackSizes;
  CBoolVector digestsDefined;
  CRecordVector<UInt32> digests;
  
  ReadStreamsInfo(NULL,
    dataOffset,
    packSizes,
    packCRCsDefined,
    packCRCs,
    folders,
    numUnpackStreamsInFolders,
    unpackSizes,
    digestsDefined,
    digests);
  
  // db.ArchiveInfo.DataStartPosition2 += db.ArchiveInfo.StartPositionAfterHeader;
  
  CNum packIndex = 0;
  CDecoder decoder(
    #ifdef _ST_MODE
    false
    #else
    true
    #endif
    );
  UInt64 dataStartPos = baseOffset + dataOffset;
  for (int i = 0; i < folders.Size(); i++)
  {
    const CFolder &folder = folders[i];
    dataVector.Add(CByteBuffer());
    CByteBuffer &data = dataVector.Back();
    UInt64 unpackSize64 = folder.GetUnpackSize();
    size_t unpackSize = (size_t)unpackSize64;
    if (unpackSize != unpackSize64)
      ThrowUnsupported();
    data.SetCapacity(unpackSize);
    
    CBufPtrSeqOutStream *outStreamSpec = new CBufPtrSeqOutStream;
    CMyComPtr<ISequentialOutStream> outStream = outStreamSpec;
    outStreamSpec->Init(data, unpackSize);
    
    HRESULT result = decoder.Decode(
      EXTERNAL_CODECS_LOC_VARS
      _stream, dataStartPos,
      &packSizes[packIndex], folder, outStream, NULL
      #ifndef _NO_CRYPTO
      , getTextPassword, passwordIsDefined
      #endif
      #if !defined(_7ZIP_ST) && !defined(_SFX)
      , false, 1
      #endif
      );
    RINOK(result);
    
    if (folder.UnpackCRCDefined)
      if (CrcCalc(data, unpackSize) != folder.UnpackCRC)
        ThrowIncorrect();
    for (int j = 0; j < folder.PackStreams.Size(); j++)
    {
      UInt64 packSize = packSizes[packIndex++];
      dataStartPos += packSize;
      HeadersSize += packSize;
    }
  }
  return S_OK;
}

HRESULT CInArchive::ReadHeader(
    DECL_EXTERNAL_CODECS_LOC_VARS
    CArchiveDatabaseEx &db
    #ifndef _NO_CRYPTO
    , ICryptoGetTextPassword *getTextPassword, bool &passwordIsDefined
    #endif
    )
{
  UInt64 type = ReadID();

  if (type == NID::kArchiveProperties)
  {
    ReadArchiveProperties(db.ArchiveInfo);
    type = ReadID();
  }
 
  CObjectVector<CByteBuffer> dataVector;
  
  if (type == NID::kAdditionalStreamsInfo)
  {
    HRESULT result = ReadAndDecodePackedStreams(
        EXTERNAL_CODECS_LOC_VARS
        db.ArchiveInfo.StartPositionAfterHeader,
        db.ArchiveInfo.DataStartPosition2,
        dataVector
        #ifndef _NO_CRYPTO
        , getTextPassword, passwordIsDefined
        #endif
        );
    RINOK(result);
    db.ArchiveInfo.DataStartPosition2 += db.ArchiveInfo.StartPositionAfterHeader;
    type = ReadID();
  }

  CRecordVector<UInt64> unpackSizes;
  CBoolVector digestsDefined;
  CRecordVector<UInt32> digests;
  
  if (type == NID::kMainStreamsInfo)
  {
    ReadStreamsInfo(&dataVector,
        db.ArchiveInfo.DataStartPosition,
        db.PackSizes,
        db.PackCRCsDefined,
        db.PackCRCs,
        db.Folders,
        db.NumUnpackStreamsVector,
        unpackSizes,
        digestsDefined,
        digests);
    db.ArchiveInfo.DataStartPosition += db.ArchiveInfo.StartPositionAfterHeader;
    type = ReadID();
  }
  else
  {
    for (int i = 0; i < db.Folders.Size(); i++)
    {
      db.NumUnpackStreamsVector.Add(1);
      CFolder &folder = db.Folders[i];
      unpackSizes.Add(folder.GetUnpackSize());
      digestsDefined.Add(folder.UnpackCRCDefined);
      digests.Add(folder.UnpackCRC);
    }
  }

  db.Files.Clear();

  if (type == NID::kEnd)
    return S_OK;
  if (type != NID::kFilesInfo)
    ThrowIncorrect();
  
  CNum numFiles = ReadNum();
  db.Files.Reserve(numFiles);
  CNum i;
  for (i = 0; i < numFiles; i++)
    db.Files.Add(CFileItem());

  db.ArchiveInfo.FileInfoPopIDs.Add(NID::kSize);
  if (!db.PackSizes.IsEmpty())
    db.ArchiveInfo.FileInfoPopIDs.Add(NID::kPackInfo);
  if (numFiles > 0  && !digests.IsEmpty())
    db.ArchiveInfo.FileInfoPopIDs.Add(NID::kCRC);

  CBoolVector emptyStreamVector;
  BoolVector_Fill_False(emptyStreamVector, (int)numFiles);
  CBoolVector emptyFileVector;
  CBoolVector antiFileVector;
  CNum numEmptyStreams = 0;

  for (;;)
  {
    UInt64 type = ReadID();
    if (type == NID::kEnd)
      break;
    UInt64 size = ReadNumber();
    size_t ppp = _inByteBack->_pos;
    bool addPropIdToList = true;
    bool isKnownType = true;
    if (type > ((UInt32)1 << 30))
      isKnownType = false;
    else switch((UInt32)type)
    {
      case NID::kName:
      {
        CStreamSwitch streamSwitch;
        streamSwitch.Set(this, &dataVector);
        for (int i = 0; i < db.Files.Size(); i++)
          _inByteBack->ReadString(db.Files[i].Name);
        break;
      }
      case NID::kWinAttributes:
      {
        CBoolVector boolVector;
        ReadBoolVector2(db.Files.Size(), boolVector);
        CStreamSwitch streamSwitch;
        streamSwitch.Set(this, &dataVector);
        for (i = 0; i < numFiles; i++)
        {
          CFileItem &file = db.Files[i];
          file.AttribDefined = boolVector[i];
          if (file.AttribDefined)
            file.Attrib = ReadUInt32();
        }
        break;
      }
      case NID::kEmptyStream:
      {
        ReadBoolVector(numFiles, emptyStreamVector);
        for (i = 0; i < (CNum)emptyStreamVector.Size(); i++)
          if (emptyStreamVector[i])
            numEmptyStreams++;

        BoolVector_Fill_False(emptyFileVector, numEmptyStreams);
        BoolVector_Fill_False(antiFileVector, numEmptyStreams);

        break;
      }
      case NID::kEmptyFile:  ReadBoolVector(numEmptyStreams, emptyFileVector); break;
      case NID::kAnti:  ReadBoolVector(numEmptyStreams, antiFileVector); break;
      case NID::kStartPos:  ReadUInt64DefVector(dataVector, db.StartPos, (int)numFiles); break;
      case NID::kCTime:  ReadUInt64DefVector(dataVector, db.CTime, (int)numFiles); break;
      case NID::kATime:  ReadUInt64DefVector(dataVector, db.ATime, (int)numFiles); break;
      case NID::kMTime:  ReadUInt64DefVector(dataVector, db.MTime, (int)numFiles); break;
      case NID::kDummy:
      {
        for (UInt64 j = 0; j < size; j++)
          if (ReadByte() != 0)
            ThrowIncorrect();
        addPropIdToList = false;
        break;
      }
      default:
        addPropIdToList = isKnownType = false;
    }
    if (isKnownType)
    {
      if(addPropIdToList)
        db.ArchiveInfo.FileInfoPopIDs.Add(type);
    }
    else
      SkipData(size);
    bool checkRecordsSize = (db.ArchiveInfo.Version.Major > 0 ||
        db.ArchiveInfo.Version.Minor > 2);
    if (checkRecordsSize && _inByteBack->_pos - ppp != size)
      ThrowIncorrect();
  }

  CNum emptyFileIndex = 0;
  CNum sizeIndex = 0;

  CNum numAntiItems = 0;
  for (i = 0; i < numEmptyStreams; i++)
    if (antiFileVector[i])
      numAntiItems++;
    
  for (i = 0; i < numFiles; i++)
  {
    CFileItem &file = db.Files[i];
    bool isAnti;
    file.HasStream = !emptyStreamVector[i];
    if (file.HasStream)
    {
      file.IsDir = false;
      isAnti = false;
      file.Size = unpackSizes[sizeIndex];
      file.Crc = digests[sizeIndex];
      file.CrcDefined = digestsDefined[sizeIndex];
      sizeIndex++;
    }
    else
    {
      file.IsDir = !emptyFileVector[emptyFileIndex];
      isAnti = antiFileVector[emptyFileIndex];
      emptyFileIndex++;
      file.Size = 0;
      file.CrcDefined = false;
    }
    if (numAntiItems != 0)
      db.IsAnti.Add(isAnti);
  }
  return S_OK;
}


void CArchiveDatabaseEx::FillFolderStartPackStream()
{
  FolderStartPackStreamIndex.Clear();
  FolderStartPackStreamIndex.Reserve(Folders.Size());
  CNum startPos = 0;
  for (int i = 0; i < Folders.Size(); i++)
  {
    FolderStartPackStreamIndex.Add(startPos);
    startPos += (CNum)Folders[i].PackStreams.Size();
  }
}

void CArchiveDatabaseEx::FillStartPos()
{
  PackStreamStartPositions.Clear();
  PackStreamStartPositions.Reserve(PackSizes.Size());
  UInt64 startPos = 0;
  for (int i = 0; i < PackSizes.Size(); i++)
  {
    PackStreamStartPositions.Add(startPos);
    startPos += PackSizes[i];
  }
}

void CArchiveDatabaseEx::FillFolderStartFileIndex()
{
  FolderStartFileIndex.Clear();
  FolderStartFileIndex.Reserve(Folders.Size());
  FileIndexToFolderIndexMap.Clear();
  FileIndexToFolderIndexMap.Reserve(Files.Size());
  
  int folderIndex = 0;
  CNum indexInFolder = 0;
  for (int i = 0; i < Files.Size(); i++)
  {
    const CFileItem &file = Files[i];
    bool emptyStream = !file.HasStream;
    if (emptyStream && indexInFolder == 0)
    {
      FileIndexToFolderIndexMap.Add(kNumNoIndex);
      continue;
    }
    if (indexInFolder == 0)
    {
      // v3.13 incorrectly worked with empty folders
      // v4.07: Loop for skipping empty folders
      for (;;)
      {
        if (folderIndex >= Folders.Size())
          ThrowIncorrect();
        FolderStartFileIndex.Add(i); // check it
        if (NumUnpackStreamsVector[folderIndex] != 0)
          break;
        folderIndex++;
      }
    }
    FileIndexToFolderIndexMap.Add(folderIndex);
    if (emptyStream)
      continue;
    indexInFolder++;
    if (indexInFolder >= NumUnpackStreamsVector[folderIndex])
    {
      folderIndex++;
      indexInFolder = 0;
    }
  }
}

HRESULT CInArchive::ReadDatabase2(
    DECL_EXTERNAL_CODECS_LOC_VARS
    CArchiveDatabaseEx &db
    #ifndef _NO_CRYPTO
    , ICryptoGetTextPassword *getTextPassword, bool &passwordIsDefined
    #endif
    )
{
  db.Clear();
  db.ArchiveInfo.StartPosition = _arhiveBeginStreamPosition;

  db.ArchiveInfo.Version.Major = _header[6];
  db.ArchiveInfo.Version.Minor = _header[7];

  if (db.ArchiveInfo.Version.Major != kMajorVersion)
    ThrowUnsupportedVersion();

  UInt32 crcFromArchive = Get32(_header + 8);
  UInt64 nextHeaderOffset = Get64(_header + 0xC);
  UInt64 nextHeaderSize = Get64(_header + 0x14);
  UInt32 nextHeaderCRC = Get32(_header + 0x1C);
  UInt32 crc = CrcCalc(_header + 0xC, 20);

  #ifdef FORMAT_7Z_RECOVERY
  if (crcFromArchive == 0 && nextHeaderOffset == 0 && nextHeaderSize == 0 && nextHeaderCRC == 0)
  {
    UInt64 cur, cur2;
    RINOK(_stream->Seek(0, STREAM_SEEK_CUR, &cur));
    const int kCheckSize = 500;
    Byte buf[kCheckSize];
    RINOK(_stream->Seek(0, STREAM_SEEK_END, &cur2));
    int checkSize = kCheckSize;
    if (cur2 - cur < kCheckSize)
      checkSize = (int)(cur2 - cur);
    RINOK(_stream->Seek(-checkSize, STREAM_SEEK_END, &cur2));
    
    RINOK(ReadStream_FALSE(_stream, buf, (size_t)checkSize));

    int i;
    for (i = (int)checkSize - 2; i >= 0; i--)
      if (buf[i] == 0x17 && buf[i + 1] == 0x6 || buf[i] == 0x01 && buf[i + 1] == 0x04)
        break;
    if (i < 0)
      return S_FALSE;
    nextHeaderSize = checkSize - i;
    nextHeaderOffset = cur2 - cur + i;
    nextHeaderCRC = CrcCalc(buf + i, (size_t)nextHeaderSize);
    RINOK(_stream->Seek(cur, STREAM_SEEK_SET, NULL));
  }
  else
  #endif
  {
    if (crc != crcFromArchive)
      ThrowIncorrect();
  }

  db.ArchiveInfo.StartPositionAfterHeader = _arhiveBeginStreamPosition + kHeaderSize;

  if (nextHeaderSize == 0)
    return S_OK;

  if (nextHeaderSize > (UInt64)(UInt32)0xFFFFFFFF)
    return S_FALSE;

  if ((Int64)nextHeaderOffset < 0)
    return S_FALSE;

  if (db.ArchiveInfo.StartPositionAfterHeader + nextHeaderOffset > _fileEndPosition)
    return S_FALSE;
  RINOK(_stream->Seek(nextHeaderOffset, STREAM_SEEK_CUR, NULL));

  CByteBuffer buffer2;
  buffer2.SetCapacity((size_t)nextHeaderSize);

  RINOK(ReadStream_FALSE(_stream, buffer2, (size_t)nextHeaderSize));
  HeadersSize += kHeaderSize + nextHeaderSize;
  db.PhySize = kHeaderSize + nextHeaderOffset + nextHeaderSize;

  if (CrcCalc(buffer2, (UInt32)nextHeaderSize) != nextHeaderCRC)
    ThrowIncorrect();
  
  CStreamSwitch streamSwitch;
  streamSwitch.Set(this, buffer2);
  
  CObjectVector<CByteBuffer> dataVector;
  
  UInt64 type = ReadID();
  if (type != NID::kHeader)
  {
    if (type != NID::kEncodedHeader)
      ThrowIncorrect();
    HRESULT result = ReadAndDecodePackedStreams(
        EXTERNAL_CODECS_LOC_VARS
        db.ArchiveInfo.StartPositionAfterHeader,
        db.ArchiveInfo.DataStartPosition2,
        dataVector
        #ifndef _NO_CRYPTO
        , getTextPassword, passwordIsDefined
        #endif
        );
    RINOK(result);
    if (dataVector.Size() == 0)
      return S_OK;
    if (dataVector.Size() > 1)
      ThrowIncorrect();
    streamSwitch.Remove();
    streamSwitch.Set(this, dataVector.Front());
    if (ReadID() != NID::kHeader)
      ThrowIncorrect();
  }

  db.HeadersSize = HeadersSize;

  return ReadHeader(
    EXTERNAL_CODECS_LOC_VARS
    db
    #ifndef _NO_CRYPTO
    , getTextPassword, passwordIsDefined
    #endif
    );
}

HRESULT CInArchive::ReadDatabase(
    DECL_EXTERNAL_CODECS_LOC_VARS
    CArchiveDatabaseEx &db
    #ifndef _NO_CRYPTO
    , ICryptoGetTextPassword *getTextPassword, bool &passwordIsDefined
    #endif
    )
{
  try
  {
    return ReadDatabase2(
      EXTERNAL_CODECS_LOC_VARS db
      #ifndef _NO_CRYPTO
      , getTextPassword, passwordIsDefined
      #endif
      );
  }
  catch(CInArchiveException &) { return S_FALSE; }
}

}}
