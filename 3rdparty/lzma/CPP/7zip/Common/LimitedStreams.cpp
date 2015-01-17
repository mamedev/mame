// LimitedStreams.cpp

#include "StdAfx.h"

#include "LimitedStreams.h"
#include "../../Common/Defs.h"

STDMETHODIMP CLimitedSequentialInStream::Read(void *data, UInt32 size, UInt32 *processedSize)
{
  UInt32 realProcessedSize = 0;
  UInt32 sizeToRead = (UInt32)MyMin((_size - _pos), (UInt64)size);
  HRESULT result = S_OK;
  if (sizeToRead > 0)
  {
    result = _stream->Read(data, sizeToRead, &realProcessedSize);
    _pos += realProcessedSize;
    if (realProcessedSize == 0)
      _wasFinished = true;
  }
  if (processedSize != NULL)
    *processedSize = realProcessedSize;
  return result;
}

STDMETHODIMP CLimitedInStream::Read(void *data, UInt32 size, UInt32 *processedSize)
{
  if (processedSize != NULL)
    *processedSize = 0;
  if (_virtPos >= _size)
    return (_virtPos == _size) ? S_OK: E_FAIL;
  UInt64 rem = _size - _virtPos;
  if (rem < size)
    size = (UInt32)rem;
  UInt64 newPos = _startOffset + _virtPos;
  if (newPos != _physPos)
  {
    _physPos = newPos;
    RINOK(SeekToPhys());
  }
  HRESULT res = _stream->Read(data, size, &size);
  if (processedSize != NULL)
    *processedSize = size;
  _physPos += size;
  _virtPos += size;
  return res;
}

STDMETHODIMP CLimitedInStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition)
{
  switch(seekOrigin)
  {
    case STREAM_SEEK_SET: _virtPos = offset; break;
    case STREAM_SEEK_CUR: _virtPos += offset; break;
    case STREAM_SEEK_END: _virtPos = _size + offset; break;
    default: return STG_E_INVALIDFUNCTION;
  }
  if (newPosition)
    *newPosition = _virtPos;
  return S_OK;
}

STDMETHODIMP CClusterInStream::Read(void *data, UInt32 size, UInt32 *processedSize)
{
  if (processedSize != NULL)
    *processedSize = 0;
  if (_virtPos >= Size)
    return (_virtPos == Size) ? S_OK: E_FAIL;

  if (_curRem == 0)
  {
    UInt32 blockSize = (UInt32)1 << BlockSizeLog;
    UInt32 virtBlock = (UInt32)(_virtPos >> BlockSizeLog);
    UInt32 offsetInBlock = (UInt32)_virtPos & (blockSize - 1);
    UInt32 phyBlock = Vector[virtBlock];
    UInt64 newPos = StartOffset + ((UInt64)phyBlock << BlockSizeLog) + offsetInBlock;
    if (newPos != _physPos)
    {
      _physPos = newPos;
      RINOK(SeekToPhys());
    }
    _curRem = blockSize - offsetInBlock;
    for (int i = 1; i < 64 && (virtBlock + i) < (UInt32)Vector.Size() && phyBlock + i == Vector[virtBlock + i]; i++)
      _curRem += (UInt32)1 << BlockSizeLog;
    UInt64 rem = Size - _virtPos;
    if (_curRem > rem)
      _curRem = (UInt32)rem;
  }
  if (size > _curRem)
    size = _curRem;
  HRESULT res = Stream->Read(data, size, &size);
  if (processedSize != NULL)
    *processedSize = size;
  _physPos += size;
  _virtPos += size;
  _curRem -= size;
  return res;
}
 
STDMETHODIMP CClusterInStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition)
{
  UInt64 newVirtPos = offset;
  switch(seekOrigin)
  {
    case STREAM_SEEK_SET: break;
    case STREAM_SEEK_CUR: newVirtPos += _virtPos; break;
    case STREAM_SEEK_END: newVirtPos += Size; break;
    default: return STG_E_INVALIDFUNCTION;
  }
  if (_virtPos != newVirtPos)
    _curRem = 0;
  _virtPos = newVirtPos;
  if (newPosition)
    *newPosition = newVirtPos;
  return S_OK;
}


HRESULT CreateLimitedInStream(IInStream *inStream, UInt64 pos, UInt64 size, ISequentialInStream **resStream)
{
  *resStream = 0;
  CLimitedInStream *streamSpec = new CLimitedInStream;
  CMyComPtr<ISequentialInStream> streamTemp = streamSpec;
  streamSpec->SetStream(inStream);
  RINOK(streamSpec->InitAndSeek(pos, size));
  streamSpec->SeekToStart();
  *resStream = streamTemp.Detach();
  return S_OK;
}

STDMETHODIMP CLimitedSequentialOutStream::Write(const void *data, UInt32 size, UInt32 *processedSize)
{
  HRESULT result = S_OK;
  if (processedSize != NULL)
    *processedSize = 0;
  if (size > _size)
  {
    if (_size == 0)
    {
      _overflow = true;
      if (!_overflowIsAllowed)
        return E_FAIL;
      if (processedSize != NULL)
        *processedSize = size;
      return S_OK;
    }
    size = (UInt32)_size;
  }
  if (_stream)
    result = _stream->Write(data, size, &size);
  _size -= size;
  if (processedSize != NULL)
    *processedSize = size;
  return result;
}
