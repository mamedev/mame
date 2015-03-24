// InBuffer.h

#ifndef __INBUFFER_H
#define __INBUFFER_H

#include "../IStream.h"
#include "../../Common/MyCom.h"
#include "../../Common/MyException.h"

#ifndef _NO_EXCEPTIONS
struct CInBufferException: public CSystemException
{
  CInBufferException(HRESULT errorCode): CSystemException(errorCode) {}
};
#endif

class CInBuffer
{
  Byte *_buffer;
  Byte *_bufferLimit;
  Byte *_bufferBase;
  CMyComPtr<ISequentialInStream> _stream;
  UInt64 _processedSize;
  UInt32 _bufferSize;
  bool _wasFinished;

  bool ReadBlock();
  Byte ReadBlock2();

public:
  #ifdef _NO_EXCEPTIONS
  HRESULT ErrorCode;
  #endif

  CInBuffer();
  ~CInBuffer() { Free(); }

  bool Create(UInt32 bufferSize);
  void Free();
  
  void SetStream(ISequentialInStream *stream);
  void Init();
  void ReleaseStream() { _stream.Release(); }

  bool ReadByte(Byte &b)
  {
    if (_buffer >= _bufferLimit)
      if (!ReadBlock())
        return false;
    b = *_buffer++;
    return true;
  }
  Byte ReadByte()
  {
    if (_buffer >= _bufferLimit)
      return ReadBlock2();
    return *_buffer++;
  }
  UInt32 ReadBytes(Byte *buf, UInt32 size)
  {
    if ((UInt32)(_bufferLimit - _buffer) >= size)
    {
      for (UInt32 i = 0; i < size; i++)
        buf[i] = _buffer[i];
      _buffer += size;
      return size;
    }
    for (UInt32 i = 0; i < size; i++)
    {
      if (_buffer >= _bufferLimit)
        if (!ReadBlock())
          return i;
      buf[i] = *_buffer++;
    }
    return size;
  }
  UInt64 GetProcessedSize() const { return _processedSize + (_buffer - _bufferBase); }
  bool WasFinished() const { return _wasFinished; }
};

#endif
