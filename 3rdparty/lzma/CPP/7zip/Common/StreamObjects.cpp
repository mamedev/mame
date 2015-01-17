// StreamObjects.cpp

#include "StdAfx.h"

#include <stdlib.h>

#include "../../../C/Alloc.h"

#include "StreamObjects.h"

STDMETHODIMP CBufInStream::Read(void *data, UInt32 size, UInt32 *processedSize)
{
  if (processedSize)
    *processedSize = 0;
  if (size == 0)
    return S_OK;
  if (_pos > _size)
    return E_FAIL;
  size_t rem = _size - (size_t)_pos;
  if (rem > size)
    rem = (size_t)size;
  memcpy(data, _data + (size_t)_pos, rem);
  _pos += rem;
  if (processedSize)
    *processedSize = (UInt32)rem;
  return S_OK;
}

STDMETHODIMP CBufInStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition)
{
  switch(seekOrigin)
  {
    case STREAM_SEEK_SET: _pos = offset; break;
    case STREAM_SEEK_CUR: _pos += offset; break;
    case STREAM_SEEK_END: _pos = _size + offset; break;
    default: return STG_E_INVALIDFUNCTION;
  }
  if (newPosition)
    *newPosition = _pos;
  return S_OK;
}

void CByteDynBuffer::Free()
{
  free(_buf);
  _buf = 0;
  _capacity = 0;
}

bool CByteDynBuffer::EnsureCapacity(size_t cap)
{
  if (cap <= _capacity)
    return true;
  size_t delta;
  if (_capacity > 64)
    delta = _capacity / 4;
  else if (_capacity > 8)
    delta = 16;
  else
    delta = 4;
  cap = MyMax(_capacity + delta, cap);
  Byte *buf = (Byte *)realloc(_buf, cap);
  if (!buf)
    return false;
  _buf = buf;
  _capacity = cap;
  return true;
}

Byte *CDynBufSeqOutStream::GetBufPtrForWriting(size_t addSize)
{
  addSize += _size;
  if (addSize < _size)
    return NULL;
  if (!_buffer.EnsureCapacity(addSize))
    return NULL;
  return (Byte *)_buffer + _size;
}

void CDynBufSeqOutStream::CopyToBuffer(CByteBuffer &dest) const
{
  dest.SetCapacity(_size);
  memcpy(dest, (const Byte *)_buffer, _size);
}

STDMETHODIMP CDynBufSeqOutStream::Write(const void *data, UInt32 size, UInt32 *processedSize)
{
  if (processedSize)
    *processedSize = 0;
  if (size == 0)
    return S_OK;
  Byte *buf = GetBufPtrForWriting(size);
  if (!buf)
    return E_OUTOFMEMORY;
  memcpy(buf, data, size);
  UpdateSize(size);
  if (processedSize)
    *processedSize = size;
  return S_OK;
}

STDMETHODIMP CBufPtrSeqOutStream::Write(const void *data, UInt32 size, UInt32 *processedSize)
{
  size_t rem = _size - _pos;
  if (rem > size)
    rem = (size_t)size;
  memcpy(_buffer + _pos, data, rem);
  _pos += rem;
  if (processedSize)
    *processedSize = (UInt32)rem;
  return (rem != 0 || size == 0) ? S_OK : E_FAIL;
}

STDMETHODIMP CSequentialOutStreamSizeCount::Write(const void *data, UInt32 size, UInt32 *processedSize)
{
  UInt32 realProcessedSize;
  HRESULT result = _stream->Write(data, size, &realProcessedSize);
  _size += realProcessedSize;
  if (processedSize)
    *processedSize = realProcessedSize;
  return result;
}

static const UInt64 kEmptyTag = (UInt64)(Int64)-1;

void CCachedInStream::Free()
{
  MyFree(_tags);
  _tags = 0;
  MidFree(_data);
  _data = 0;
}

bool CCachedInStream::Alloc(unsigned blockSizeLog, unsigned numBlocksLog)
{
  unsigned sizeLog = blockSizeLog + numBlocksLog;
  if (sizeLog >= sizeof(size_t) * 8)
    return false;
  size_t dataSize = (size_t)1 << sizeLog;
  if (_data == 0 || dataSize != _dataSize)
  {
    MidFree(_data);
    _data = (Byte *)MidAlloc(dataSize);
    if (_data == 0)
      return false;
    _dataSize = dataSize;
  }
  if (_tags == 0 || numBlocksLog != _numBlocksLog)
  {
    MyFree(_tags);
    _tags = (UInt64 *)MyAlloc(sizeof(UInt64) << numBlocksLog);
    if (_tags == 0)
      return false;
    _numBlocksLog = numBlocksLog;
  }
  _blockSizeLog = blockSizeLog;
  return true;
}

void CCachedInStream::Init(UInt64 size)
{
  _size = size;
  _pos = 0;
  size_t numBlocks = (size_t)1 << _numBlocksLog;
  for (size_t i = 0; i < numBlocks; i++)
    _tags[i] = kEmptyTag;
}

STDMETHODIMP CCachedInStream::Read(void *data, UInt32 size, UInt32 *processedSize)
{
  if (processedSize)
    *processedSize = 0;
  if (size == 0)
    return S_OK;
  if (_pos > _size)
    return E_FAIL;

  {
    UInt64 rem = _size - _pos;
    if (size > rem)
      size = (UInt32)rem;
  }

  while (size != 0)
  {
    UInt64 cacheTag = _pos >> _blockSizeLog;
    size_t cacheIndex = (size_t)cacheTag & (((size_t)1 << _numBlocksLog) - 1);
    Byte *p = _data + (cacheIndex << _blockSizeLog);
    if (_tags[cacheIndex] != cacheTag)
    {
      UInt64 remInBlock = _size - (cacheTag << _blockSizeLog);
      size_t blockSize = (size_t)1 << _blockSizeLog;
      if (blockSize > remInBlock)
        blockSize = (size_t)remInBlock;
      RINOK(ReadBlock(cacheTag, p, blockSize));
      _tags[cacheIndex] = cacheTag;
    }
    size_t offset = (size_t)_pos & (((size_t)1 << _blockSizeLog) - 1);
    UInt32 cur = (UInt32)MyMin(((size_t)1 << _blockSizeLog) - offset, (size_t)size);
    memcpy(data, p + offset, cur);
    if (processedSize)
      *processedSize += cur;
    data = (void *)((const Byte *)data + cur);
    _pos += cur;
    size -= cur;
  }

  return S_OK;
}
 
STDMETHODIMP CCachedInStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition)
{
  switch(seekOrigin)
  {
    case STREAM_SEEK_SET: _pos = offset; break;
    case STREAM_SEEK_CUR: _pos = _pos + offset; break;
    case STREAM_SEEK_END: _pos = _size + offset; break;
    default: return STG_E_INVALIDFUNCTION;
  }
  if (newPosition != 0)
    *newPosition = _pos;
  return S_OK;
}
