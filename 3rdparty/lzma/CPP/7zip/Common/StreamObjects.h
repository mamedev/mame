// StreamObjects.h

#ifndef __STREAM_OBJECTS_H
#define __STREAM_OBJECTS_H

#include "../../Common/Buffer.h"
#include "../../Common/MyCom.h"
#include "../IStream.h"

struct CReferenceBuf:
  public IUnknown,
  public CMyUnknownImp
{
  CByteBuffer Buf;
  MY_UNKNOWN_IMP
};

class CBufInStream:
  public IInStream,
  public CMyUnknownImp
{
  const Byte *_data;
  UInt64 _pos;
  size_t _size;
  CMyComPtr<IUnknown> _ref;
public:
  void Init(const Byte *data, size_t size, IUnknown *ref = 0)
  {
    _data = data;
    _size = size;
    _pos = 0;
    _ref = ref;
  }
  void Init(CReferenceBuf *ref) { Init(ref->Buf, ref->Buf.GetCapacity(), ref); }

  MY_UNKNOWN_IMP1(IInStream)
  STDMETHOD(Read)(void *data, UInt32 size, UInt32 *processedSize);
  STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition);
};

class CByteDynBuffer
{
  size_t _capacity;
  Byte *_buf;
public:
  CByteDynBuffer(): _capacity(0), _buf(0) {};
  // there is no copy constructor. So don't copy this object.
  ~CByteDynBuffer() { Free(); }
  void Free();
  size_t GetCapacity() const { return  _capacity; }
  operator Byte*() const { return _buf; };
  operator const Byte*() const { return _buf; };
  bool EnsureCapacity(size_t capacity);
};

class CDynBufSeqOutStream:
  public ISequentialOutStream,
  public CMyUnknownImp
{
  CByteDynBuffer _buffer;
  size_t _size;
public:
  CDynBufSeqOutStream(): _size(0) {}
  void Init() { _size = 0;  }
  size_t GetSize() const { return _size; }
  const Byte *GetBuffer() const { return _buffer; }
  void CopyToBuffer(CByteBuffer &dest) const;
  Byte *GetBufPtrForWriting(size_t addSize);
  void UpdateSize(size_t addSize) { _size += addSize; }

  MY_UNKNOWN_IMP
  STDMETHOD(Write)(const void *data, UInt32 size, UInt32 *processedSize);
};

class CBufPtrSeqOutStream:
  public ISequentialOutStream,
  public CMyUnknownImp
{
  Byte *_buffer;
  size_t _size;
  size_t _pos;
public:
  void Init(Byte *buffer, size_t size)
  {
    _buffer = buffer;
    _pos = 0;
    _size = size;
  }
  size_t GetPos() const { return _pos; }

  MY_UNKNOWN_IMP
  STDMETHOD(Write)(const void *data, UInt32 size, UInt32 *processedSize);
};

class CSequentialOutStreamSizeCount:
  public ISequentialOutStream,
  public CMyUnknownImp
{
  CMyComPtr<ISequentialOutStream> _stream;
  UInt64 _size;
public:
  void SetStream(ISequentialOutStream *stream) { _stream = stream; }
  void Init() { _size = 0; }
  UInt64 GetSize() const { return _size; }

  MY_UNKNOWN_IMP
  STDMETHOD(Write)(const void *data, UInt32 size, UInt32 *processedSize);
};

class CCachedInStream:
  public IInStream,
  public CMyUnknownImp
{
  UInt64 *_tags;
  Byte *_data;
  size_t _dataSize;
  unsigned _blockSizeLog;
  unsigned _numBlocksLog;
  UInt64 _size;
  UInt64 _pos;
protected:
  virtual HRESULT ReadBlock(UInt64 blockIndex, Byte *dest, size_t blockSize) = 0;
public:
  CCachedInStream(): _tags(0), _data(0) {}
  virtual ~CCachedInStream() { Free(); } // the destructor must be virtual (release calls it) !!!
  void Free();
  bool Alloc(unsigned blockSizeLog, unsigned numBlocksLog);
  void Init(UInt64 size);

  MY_UNKNOWN_IMP2(ISequentialInStream, IInStream)
  STDMETHOD(Read)(void *data, UInt32 size, UInt32 *processedSize);
  STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition);
};

#endif
