// 7zOut.h

#ifndef __7Z_OUT_H
#define __7Z_OUT_H

#include "7zCompressionMode.h"
#include "7zEncode.h"
#include "7zHeader.h"
#include "7zItem.h"

#include "../../Common/OutBuffer.h"

namespace NArchive {
namespace N7z {

class CWriteBufferLoc
{
  Byte *_data;
  size_t _size;
  size_t _pos;
public:
  CWriteBufferLoc(): _size(0), _pos(0) {}
  void Init(Byte *data, size_t size)
  {
    _data = data;
    _size = size;
    _pos = 0;
  }
  void WriteBytes(const void *data, size_t size)
  {
    if (size > _size - _pos)
      throw 1;
    memcpy(_data + _pos, data, size);
    _pos += size;
  }
  void WriteByte(Byte b)
  {
    if (_size == _pos)
      throw 1;
    _data[_pos++] = b;
  }
  size_t GetPos() const { return _pos; }
};

struct CHeaderOptions
{
  bool CompressMainHeader;
  bool WriteCTime;
  bool WriteATime;
  bool WriteMTime;

  CHeaderOptions():
      CompressMainHeader(true),
      WriteCTime(false),
      WriteATime(false),
      WriteMTime(true)
      {}
};

class COutArchive
{
  UInt64 _prefixHeaderPos;

  HRESULT WriteDirect(const void *data, UInt32 size);
  
  UInt64 GetPos() const;
  void WriteBytes(const void *data, size_t size);
  void WriteBytes(const CByteBuffer &data) { WriteBytes(data, data.GetCapacity()); }
  void WriteByte(Byte b);
  void WriteUInt32(UInt32 value);
  void WriteUInt64(UInt64 value);
  void WriteNumber(UInt64 value);
  void WriteID(UInt64 value) { WriteNumber(value); }

  void WriteFolder(const CFolder &folder);
  HRESULT WriteFileHeader(const CFileItem &itemInfo);
  void WriteBoolVector(const CBoolVector &boolVector);
  void WriteHashDigests(
      const CRecordVector<bool> &digestsDefined,
      const CRecordVector<UInt32> &hashDigests);

  void WritePackInfo(
      UInt64 dataOffset,
      const CRecordVector<UInt64> &packSizes,
      const CRecordVector<bool> &packCRCsDefined,
      const CRecordVector<UInt32> &packCRCs);

  void WriteUnpackInfo(const CObjectVector<CFolder> &folders);

  void WriteSubStreamsInfo(
      const CObjectVector<CFolder> &folders,
      const CRecordVector<CNum> &numUnpackStreamsInFolders,
      const CRecordVector<UInt64> &unpackSizes,
      const CRecordVector<bool> &digestsDefined,
      const CRecordVector<UInt32> &hashDigests);

  void SkipAlign(unsigned pos, unsigned alignSize);
  void WriteAlignedBoolHeader(const CBoolVector &v, int numDefined, Byte type, unsigned itemSize);
  void WriteUInt64DefVector(const CUInt64DefVector &v, Byte type);

  HRESULT EncodeStream(
      DECL_EXTERNAL_CODECS_LOC_VARS
      CEncoder &encoder, const CByteBuffer &data,
      CRecordVector<UInt64> &packSizes, CObjectVector<CFolder> &folders);
  void WriteHeader(
      const CArchiveDatabase &db,
      const CHeaderOptions &headerOptions,
      UInt64 &headerOffset);
  
  bool _countMode;
  bool _writeToStream;
  size_t _countSize;
  UInt32 _crc;
  COutBuffer _outByte;
  CWriteBufferLoc _outByte2;

  #ifdef _7Z_VOL
  bool _endMarker;
  #endif

  HRESULT WriteSignature();
  #ifdef _7Z_VOL
  HRESULT WriteFinishSignature();
  #endif
  HRESULT WriteStartHeader(const CStartHeader &h);
  #ifdef _7Z_VOL
  HRESULT WriteFinishHeader(const CFinishHeader &h);
  #endif
  CMyComPtr<IOutStream> Stream;
public:

  COutArchive() { _outByte.Create(1 << 16); }
  CMyComPtr<ISequentialOutStream> SeqStream;
  HRESULT Create(ISequentialOutStream *stream, bool endMarker);
  void Close();
  HRESULT SkipPrefixArchiveHeader();
  HRESULT WriteDatabase(
      DECL_EXTERNAL_CODECS_LOC_VARS
      const CArchiveDatabase &db,
      const CCompressionMethodMode *options,
      const CHeaderOptions &headerOptions);

  #ifdef _7Z_VOL
  static UInt32 GetVolHeadersSize(UInt64 dataSize, int nameLength = 0, bool props = false);
  static UInt64 GetVolPureSize(UInt64 volSize, int nameLength = 0, bool props = false);
  #endif

};

}}

#endif
