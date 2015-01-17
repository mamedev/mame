// Bcj2Coder.cpp

#include "StdAfx.h"

#include "../../../C/Alloc.h"

#include "Bcj2Coder.h"

namespace NCompress {
namespace NBcj2 {

inline bool IsJcc(Byte b0, Byte b1) { return (b0 == 0x0F && (b1 & 0xF0) == 0x80); }
inline bool IsJ(Byte b0, Byte b1) { return ((b1 & 0xFE) == 0xE8 || IsJcc(b0, b1)); }
inline unsigned GetIndex(Byte b0, Byte b1) { return ((b1 == 0xE8) ? b0 : ((b1 == 0xE9) ? 256 : 257)); }

#ifndef EXTRACT_ONLY

static const int kBufferSize = 1 << 17;

static bool inline Test86MSByte(Byte b)
{
  return (b == 0 || b == 0xFF);
}

bool CEncoder::Create()
{
  if (!_mainStream.Create(1 << 18))
    return false;
  if (!_callStream.Create(1 << 18))
    return false;
  if (!_jumpStream.Create(1 << 18))
    return false;
  if (!_rangeEncoder.Create(1 << 20))
    return false;
  if (_buffer == 0)
  {
    _buffer = (Byte *)MidAlloc(kBufferSize);
    if (_buffer == 0)
      return false;
  }
  return true;
}

CEncoder::~CEncoder()
{
  ::MidFree(_buffer);
}

HRESULT CEncoder::Flush()
{
  RINOK(_mainStream.Flush());
  RINOK(_callStream.Flush());
  RINOK(_jumpStream.Flush());
  _rangeEncoder.FlushData();
  return _rangeEncoder.FlushStream();
}

const UInt32 kDefaultLimit = (1 << 24);

HRESULT CEncoder::CodeReal(ISequentialInStream **inStreams, const UInt64 **inSizes, UInt32 numInStreams,
    ISequentialOutStream **outStreams, const UInt64 ** /* outSizes */, UInt32 numOutStreams,
    ICompressProgressInfo *progress)
{
  if (numInStreams != 1 || numOutStreams != 4)
    return E_INVALIDARG;

  if (!Create())
    return E_OUTOFMEMORY;

  bool sizeIsDefined = false;
  UInt64 inSize = 0;
  if (inSizes != NULL)
    if (inSizes[0] != NULL)
    {
      inSize = *inSizes[0];
      if (inSize <= kDefaultLimit)
        sizeIsDefined = true;
    }

  CCoderReleaser releaser(this);

  ISequentialInStream *inStream = inStreams[0];

  _mainStream.SetStream(outStreams[0]);
  _mainStream.Init();
  _callStream.SetStream(outStreams[1]);
  _callStream.Init();
  _jumpStream.SetStream(outStreams[2]);
  _jumpStream.Init();
  _rangeEncoder.SetStream(outStreams[3]);
  _rangeEncoder.Init();
  for (int i = 0; i < 256 + 2; i++)
    _statusEncoder[i].Init();

  CMyComPtr<ICompressGetSubStreamSize> getSubStreamSize;
  {
    inStream->QueryInterface(IID_ICompressGetSubStreamSize, (void **)&getSubStreamSize);
  }

  UInt32 nowPos = 0;
  UInt64 nowPos64 = 0;
  UInt32 bufferPos = 0;

  Byte prevByte = 0;

  UInt64 subStreamIndex = 0;
  UInt64 subStreamStartPos  = 0;
  UInt64 subStreamEndPos = 0;

  for (;;)
  {
    UInt32 processedSize = 0;
    for (;;)
    {
      UInt32 size = kBufferSize - (bufferPos + processedSize);
      UInt32 processedSizeLoc;
      if (size == 0)
        break;
      RINOK(inStream->Read(_buffer + bufferPos + processedSize, size, &processedSizeLoc));
      if (processedSizeLoc == 0)
        break;
      processedSize += processedSizeLoc;
    }
    UInt32 endPos = bufferPos + processedSize;
    
    if (endPos < 5)
    {
      // change it
      for (bufferPos = 0; bufferPos < endPos; bufferPos++)
      {
        Byte b = _buffer[bufferPos];
        _mainStream.WriteByte(b);
        UInt32 index;
        if (b == 0xE8)
          index = prevByte;
        else if (b == 0xE9)
          index = 256;
        else if (IsJcc(prevByte, b))
          index = 257;
        else
        {
          prevByte = b;
          continue;
        }
        _statusEncoder[index].Encode(&_rangeEncoder, 0);
        prevByte = b;
      }
      return Flush();
    }

    bufferPos = 0;

    UInt32 limit = endPos - 5;
    while(bufferPos <= limit)
    {
      Byte b = _buffer[bufferPos];
      _mainStream.WriteByte(b);
      if (!IsJ(prevByte, b))
      {
        bufferPos++;
        prevByte = b;
        continue;
      }
      Byte nextByte = _buffer[bufferPos + 4];
      UInt32 src =
        (UInt32(nextByte) << 24) |
        (UInt32(_buffer[bufferPos + 3]) << 16) |
        (UInt32(_buffer[bufferPos + 2]) << 8) |
        (_buffer[bufferPos + 1]);
      UInt32 dest = (nowPos + bufferPos + 5) + src;
      // if (Test86MSByte(nextByte))
      bool convert;
      if (getSubStreamSize != NULL)
      {
        UInt64 currentPos = (nowPos64 + bufferPos);
        while (subStreamEndPos < currentPos)
        {
          UInt64 subStreamSize;
          HRESULT result = getSubStreamSize->GetSubStreamSize(subStreamIndex, &subStreamSize);
          if (result == S_OK)
          {
            subStreamStartPos = subStreamEndPos;
            subStreamEndPos += subStreamSize;
            subStreamIndex++;
          }
          else if (result == S_FALSE || result == E_NOTIMPL)
          {
            getSubStreamSize.Release();
            subStreamStartPos = 0;
            subStreamEndPos = subStreamStartPos - 1;
          }
          else
            return result;
        }
        if (getSubStreamSize == NULL)
        {
          if (sizeIsDefined)
            convert = (dest < inSize);
          else
            convert = Test86MSByte(nextByte);
        }
        else if (subStreamEndPos - subStreamStartPos > kDefaultLimit)
          convert = Test86MSByte(nextByte);
        else
        {
          UInt64 dest64 = (currentPos + 5) + Int64(Int32(src));
          convert = (dest64 >= subStreamStartPos && dest64 < subStreamEndPos);
        }
      }
      else if (sizeIsDefined)
        convert = (dest < inSize);
      else
        convert = Test86MSByte(nextByte);
      unsigned index = GetIndex(prevByte, b);
      if (convert)
      {
        _statusEncoder[index].Encode(&_rangeEncoder, 1);
        bufferPos += 5;
        COutBuffer &s = (b == 0xE8) ? _callStream : _jumpStream;
        for (int i = 24; i >= 0; i -= 8)
          s.WriteByte((Byte)(dest >> i));
        prevByte = nextByte;
      }
      else
      {
        _statusEncoder[index].Encode(&_rangeEncoder, 0);
        bufferPos++;
        prevByte = b;
      }
    }
    nowPos += bufferPos;
    nowPos64 += bufferPos;

    if (progress != NULL)
    {
      /*
      const UInt64 compressedSize =
        _mainStream.GetProcessedSize() +
        _callStream.GetProcessedSize() +
        _jumpStream.GetProcessedSize() +
        _rangeEncoder.GetProcessedSize();
      */
      RINOK(progress->SetRatioInfo(&nowPos64, NULL));
    }
 
    UInt32 i = 0;
    while(bufferPos < endPos)
      _buffer[i++] = _buffer[bufferPos++];
    bufferPos = i;
  }
}

STDMETHODIMP CEncoder::Code(ISequentialInStream **inStreams, const UInt64 **inSizes, UInt32 numInStreams,
    ISequentialOutStream **outStreams, const UInt64 **outSizes, UInt32 numOutStreams,
    ICompressProgressInfo *progress)
{
  try
  {
    return CodeReal(inStreams, inSizes, numInStreams, outStreams, outSizes,numOutStreams, progress);
  }
  catch(const COutBufferException &e) { return e.ErrorCode; }
  catch(...) { return S_FALSE; }
}

#endif


STDMETHODIMP CDecoder::SetInBufSize(UInt32 streamIndex, UInt32 size) { _inBufSizes[streamIndex] = size; return S_OK; }
STDMETHODIMP CDecoder::SetOutBufSize(UInt32 , UInt32 size) { _outBufSize = size; return S_OK; }

CDecoder::CDecoder():
  _outBufSize(1 << 16)
{
  _inBufSizes[0] = 1 << 20;
  _inBufSizes[1] = 1 << 20;
  _inBufSizes[2] = 1 << 20;
  _inBufSizes[3] = 1 << 20;
}

HRESULT CDecoder::CodeReal(ISequentialInStream **inStreams, const UInt64 ** /* inSizes */, UInt32 numInStreams,
    ISequentialOutStream **outStreams, const UInt64 ** /* outSizes */, UInt32 numOutStreams,
    ICompressProgressInfo *progress)
{
  if (numInStreams != 4 || numOutStreams != 1)
    return E_INVALIDARG;

  if (!_mainInStream.Create(_inBufSizes[0]))
    return E_OUTOFMEMORY;
  if (!_callStream.Create(_inBufSizes[1]))
    return E_OUTOFMEMORY;
  if (!_jumpStream.Create(_inBufSizes[2]))
    return E_OUTOFMEMORY;
  if (!_rangeDecoder.Create(_inBufSizes[3]))
    return E_OUTOFMEMORY;
  if (!_outStream.Create(_outBufSize))
    return E_OUTOFMEMORY;

  CCoderReleaser releaser(this);

  _mainInStream.SetStream(inStreams[0]);
  _callStream.SetStream(inStreams[1]);
  _jumpStream.SetStream(inStreams[2]);
  _rangeDecoder.SetStream(inStreams[3]);
  _outStream.SetStream(outStreams[0]);

  _mainInStream.Init();
  _callStream.Init();
  _jumpStream.Init();
  _rangeDecoder.Init();
  _outStream.Init();

  for (int i = 0; i < 256 + 2; i++)
    _statusDecoder[i].Init();

  Byte prevByte = 0;
  UInt32 processedBytes = 0;
  for (;;)
  {
    if (processedBytes >= (1 << 20) && progress != NULL)
    {
      /*
      const UInt64 compressedSize =
        _mainInStream.GetProcessedSize() +
        _callStream.GetProcessedSize() +
        _jumpStream.GetProcessedSize() +
        _rangeDecoder.GetProcessedSize();
      */
      const UInt64 nowPos64 = _outStream.GetProcessedSize();
      RINOK(progress->SetRatioInfo(NULL, &nowPos64));
      processedBytes = 0;
    }
    UInt32 i;
    Byte b = 0;
    const UInt32 kBurstSize = (1 << 18);
    for (i = 0; i < kBurstSize; i++)
    {
      if (!_mainInStream.ReadByte(b))
        return Flush();
      _outStream.WriteByte(b);
      if (IsJ(prevByte, b))
        break;
      prevByte = b;
    }
    processedBytes += i;
    if (i == kBurstSize)
      continue;
    unsigned index = GetIndex(prevByte, b);
    if (_statusDecoder[index].Decode(&_rangeDecoder) == 1)
    {
      UInt32 src = 0;
      CInBuffer &s = (b == 0xE8) ? _callStream : _jumpStream;
      for (int i = 0; i < 4; i++)
      {
        Byte b0;
        if(!s.ReadByte(b0))
          return S_FALSE;
        src <<= 8;
        src |= ((UInt32)b0);
      }
      UInt32 dest = src - (UInt32(_outStream.GetProcessedSize()) + 4) ;
      _outStream.WriteByte((Byte)(dest));
      _outStream.WriteByte((Byte)(dest >> 8));
      _outStream.WriteByte((Byte)(dest >> 16));
      _outStream.WriteByte((Byte)(dest >> 24));
      prevByte = (Byte)(dest >> 24);
      processedBytes += 4;
    }
    else
      prevByte = b;
  }
}

STDMETHODIMP CDecoder::Code(ISequentialInStream **inStreams, const UInt64 **inSizes, UInt32 numInStreams,
    ISequentialOutStream **outStreams, const UInt64 **outSizes, UInt32 numOutStreams,
    ICompressProgressInfo *progress)
{
  try
  {
    return CodeReal(inStreams, inSizes, numInStreams, outStreams, outSizes,numOutStreams, progress);
  }
  catch(const CInBufferException &e) { return e.ErrorCode; }
  catch(const COutBufferException &e) { return e.ErrorCode; }
  catch(...) { return S_FALSE; }
}

}}
