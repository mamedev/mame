// CWrappers.h

#ifndef __C_WRAPPERS_H
#define __C_WRAPPERS_H

#include "../ICoder.h"
#include "../../Common/MyCom.h"

struct CCompressProgressWrap
{
  ICompressProgress p;
  ICompressProgressInfo *Progress;
  HRESULT Res;
  CCompressProgressWrap(ICompressProgressInfo *progress);
};

struct CSeqInStreamWrap
{
  ISeqInStream p;
  ISequentialInStream *Stream;
  HRESULT Res;
  CSeqInStreamWrap(ISequentialInStream *stream);
};

struct CSeekInStreamWrap
{
  ISeekInStream p;
  IInStream *Stream;
  HRESULT Res;
  CSeekInStreamWrap(IInStream *stream);
};

struct CSeqOutStreamWrap
{
  ISeqOutStream p;
  ISequentialOutStream *Stream;
  HRESULT Res;
  UInt64 Processed;
  CSeqOutStreamWrap(ISequentialOutStream *stream);
};

HRESULT SResToHRESULT(SRes res);

struct CByteInBufWrap
{
  IByteIn p;
  const Byte *Cur;
  const Byte *Lim;
  Byte *Buf;
  UInt32 Size;
  ISequentialInStream *Stream;
  UInt64 Processed;
  bool Extra;
  HRESULT Res;
  
  CByteInBufWrap();
  ~CByteInBufWrap() { Free();  }
  void Free();
  bool Alloc(UInt32 size);
  void Init()
  {
    Lim = Cur = Buf;
    Processed = 0;
    Extra = false;
    Res = S_OK;
  }
  UInt64 GetProcessed() const { return Processed + (Cur - Buf); }
  Byte ReadByteFromNewBlock();
  Byte ReadByte()
  {
    if (Cur != Lim)
      return *Cur++;
    return ReadByteFromNewBlock();
  }
};

struct CByteOutBufWrap
{
  IByteOut p;
  Byte *Cur;
  const Byte *Lim;
  Byte *Buf;
  size_t Size;
  ISequentialOutStream *Stream;
  UInt64 Processed;
  HRESULT Res;
  
  CByteOutBufWrap();
  ~CByteOutBufWrap() { Free();  }
  void Free();
  bool Alloc(size_t size);
  void Init()
  {
    Cur = Buf;
    Lim = Buf + Size;
    Processed = 0;
    Res = S_OK;
  }
  UInt64 GetProcessed() const { return Processed + (Cur - Buf); }
  HRESULT Flush();
  void WriteByte(Byte b)
  {
    *Cur++ = b;
    if (Cur == Lim)
      Flush();
  }
};

#endif
