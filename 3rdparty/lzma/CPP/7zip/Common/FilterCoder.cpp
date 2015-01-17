// FilterCoder.cpp

#include "StdAfx.h"

#include "../../../C/Alloc.h"

#include "../../Common/Defs.h"

#include "FilterCoder.h"
#include "StreamUtils.h"

static const UInt32 kBufferSize = 1 << 17;

CFilterCoder::CFilterCoder()
{
  _buffer = (Byte *)::MidAlloc(kBufferSize);
  if (_buffer == 0)
    throw 1;
}

CFilterCoder::~CFilterCoder()
{
  ::MidFree(_buffer);
}

HRESULT CFilterCoder::WriteWithLimit(ISequentialOutStream *outStream, UInt32 size)
{
  if (_outSizeIsDefined)
  {
    UInt64 remSize = _outSize - _nowPos64;
    if (size > remSize)
      size = (UInt32)remSize;
  }
  RINOK(WriteStream(outStream, _buffer, size));
  _nowPos64 += size;
  return S_OK;
}

STDMETHODIMP CFilterCoder::Code(ISequentialInStream *inStream, ISequentialOutStream *outStream,
    const UInt64 * /* inSize */, const UInt64 *outSize, ICompressProgressInfo *progress)
{
  RINOK(Init());
  UInt32 bufferPos = 0;
  _outSizeIsDefined = (outSize != 0);
  if (_outSizeIsDefined)
    _outSize = *outSize;

  while (!_outSizeIsDefined || _nowPos64 < _outSize)
  {
    size_t processedSize = kBufferSize - bufferPos;
    
    // Change it: It can be optimized using ReadPart
    RINOK(ReadStream(inStream, _buffer + bufferPos, &processedSize));
    
    UInt32 endPos = bufferPos + (UInt32)processedSize;

    bufferPos = Filter->Filter(_buffer, endPos);
    if (bufferPos > endPos)
    {
      for (; endPos < bufferPos; endPos++)
        _buffer[endPos] = 0;
      bufferPos = Filter->Filter(_buffer, endPos);
    }

    if (bufferPos == 0)
    {
      if (endPos == 0)
        return S_OK;
      return WriteWithLimit(outStream, endPos);
    }
    RINOK(WriteWithLimit(outStream, bufferPos));
    if (progress != NULL)
    {
      RINOK(progress->SetRatioInfo(&_nowPos64, &_nowPos64));
    }
    UInt32 i = 0;
    while (bufferPos < endPos)
      _buffer[i++] = _buffer[bufferPos++];
    bufferPos = i;
  }
  return S_OK;
}

STDMETHODIMP CFilterCoder::SetOutStream(ISequentialOutStream *outStream)
{
  _bufferPos = 0;
  _outStream = outStream;
  return Init();
}

STDMETHODIMP CFilterCoder::ReleaseOutStream()
{
  _outStream.Release();
  return S_OK;
}


STDMETHODIMP CFilterCoder::Write(const void *data, UInt32 size, UInt32 *processedSize)
{
  if (processedSize != NULL)
    *processedSize = 0;
  while (size > 0)
  {
    UInt32 sizeTemp = MyMin(size, kBufferSize - _bufferPos);
    memcpy(_buffer + _bufferPos, data, sizeTemp);
    size -= sizeTemp;
    if (processedSize != NULL)
      *processedSize += sizeTemp;
    data = (const Byte *)data + sizeTemp;
    UInt32 endPos = _bufferPos + sizeTemp;
    _bufferPos = Filter->Filter(_buffer, endPos);
    if (_bufferPos == 0)
    {
      _bufferPos = endPos;
      break;
    }
    if (_bufferPos > endPos)
    {
      if (size != 0)
        return E_FAIL;
      break;
    }
    RINOK(WriteWithLimit(_outStream, _bufferPos));
    UInt32 i = 0;
    while (_bufferPos < endPos)
      _buffer[i++] = _buffer[_bufferPos++];
    _bufferPos = i;
  }
  return S_OK;
}

STDMETHODIMP CFilterCoder::Flush()
{
  if (_bufferPos != 0)
  {
    // _buffer contains only data refused by previous Filter->Filter call.
    UInt32 endPos = Filter->Filter(_buffer, _bufferPos);
    if (endPos > _bufferPos)
    {
      for (; _bufferPos < endPos; _bufferPos++)
        _buffer[_bufferPos] = 0;
      if (Filter->Filter(_buffer, endPos) != endPos)
        return E_FAIL;
    }
    RINOK(WriteWithLimit(_outStream, _bufferPos));
    _bufferPos = 0;
  }
  CMyComPtr<IOutStreamFlush> flush;
  _outStream.QueryInterface(IID_IOutStreamFlush, &flush);
  if (flush)
    return flush->Flush();
  return S_OK;
}


STDMETHODIMP CFilterCoder::SetInStream(ISequentialInStream *inStream)
{
  _convertedPosBegin = _convertedPosEnd = _bufferPos = 0;
  _inStream = inStream;
  return Init();
}

STDMETHODIMP CFilterCoder::ReleaseInStream()
{
  _inStream.Release();
  return S_OK;
}

STDMETHODIMP CFilterCoder::Read(void *data, UInt32 size, UInt32 *processedSize)
{
  if (processedSize != NULL)
    *processedSize = 0;
  while (size > 0)
  {
    if (_convertedPosBegin != _convertedPosEnd)
    {
      UInt32 sizeTemp = MyMin(size, _convertedPosEnd - _convertedPosBegin);
      memcpy(data, _buffer + _convertedPosBegin, sizeTemp);
      _convertedPosBegin += sizeTemp;
      data = (void *)((Byte *)data + sizeTemp);
      size -= sizeTemp;
      if (processedSize != NULL)
        *processedSize += sizeTemp;
      break;
    }
    UInt32 i;
    for (i = 0; _convertedPosEnd + i < _bufferPos; i++)
      _buffer[i] = _buffer[_convertedPosEnd + i];
    _bufferPos = i;
    _convertedPosBegin = _convertedPosEnd = 0;
    size_t processedSizeTemp = kBufferSize - _bufferPos;
    RINOK(ReadStream(_inStream, _buffer + _bufferPos, &processedSizeTemp));
    _bufferPos += (UInt32)processedSizeTemp;
    _convertedPosEnd = Filter->Filter(_buffer, _bufferPos);
    if (_convertedPosEnd == 0)
    {
      if (_bufferPos == 0)
        break;
      _convertedPosEnd = _bufferPos; // check it
      continue;
    }
    if (_convertedPosEnd > _bufferPos)
    {
      for (; _bufferPos < _convertedPosEnd; _bufferPos++)
        _buffer[_bufferPos] = 0;
      _convertedPosEnd = Filter->Filter(_buffer, _bufferPos);
    }
  }
  return S_OK;
}

#ifndef _NO_CRYPTO
STDMETHODIMP CFilterCoder::CryptoSetPassword(const Byte *data, UInt32 size)
{
  return _setPassword->CryptoSetPassword(data, size);
}
#endif

#ifndef EXTRACT_ONLY
STDMETHODIMP CFilterCoder::SetCoderProperties(const PROPID *propIDs,
      const PROPVARIANT *properties, UInt32 numProperties)
{
  return _SetCoderProperties->SetCoderProperties(propIDs, properties, numProperties);
}

STDMETHODIMP CFilterCoder::WriteCoderProperties(ISequentialOutStream *outStream)
{
  return _writeCoderProperties->WriteCoderProperties(outStream);
}

/*
STDMETHODIMP CFilterCoder::ResetSalt()
{
  return _CryptoResetSalt->ResetSalt();
}
*/

STDMETHODIMP CFilterCoder::ResetInitVector()
{
  return _CryptoResetInitVector->ResetInitVector();
}
#endif

STDMETHODIMP CFilterCoder::SetDecoderProperties2(const Byte *data, UInt32 size)
{
  return _setDecoderProperties->SetDecoderProperties2(data, size);
}
