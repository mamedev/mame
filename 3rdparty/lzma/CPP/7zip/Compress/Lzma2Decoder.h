// Lzma2Decoder.h

#ifndef __LZMA2_DECODER_H
#define __LZMA2_DECODER_H

#include "../../../C/Lzma2Dec.h"

#include "../../Common/MyCom.h"

#include "../ICoder.h"

namespace NCompress {
namespace NLzma2 {

class CDecoder:
  public ICompressCoder,
  public ICompressSetDecoderProperties2,
  public ICompressGetInStreamProcessedSize,
  #ifndef NO_READ_FROM_CODER
  public ICompressSetInStream,
  public ICompressSetOutStreamSize,
  public ISequentialInStream,
  #endif
  public CMyUnknownImp
{
  CMyComPtr<ISequentialInStream> _inStream;
  Byte *_inBuf;
  UInt32 _inPos;
  UInt32 _inSize;
  CLzma2Dec _state;
  bool _outSizeDefined;
  UInt64 _outSize;
  UInt64 _inSizeProcessed;
  UInt64 _outSizeProcessed;
public:

  #ifndef NO_READ_FROM_CODER
  MY_UNKNOWN_IMP5(
      ICompressSetDecoderProperties2,
      ICompressGetInStreamProcessedSize,
      ICompressSetInStream,
      ICompressSetOutStreamSize,
      ISequentialInStream)
  #else
  MY_UNKNOWN_IMP2(
      ICompressSetDecoderProperties2,
      ICompressGetInStreamProcessedSize)
  #endif

  STDMETHOD(Code)(ISequentialInStream *inStream,
      ISequentialOutStream *outStream, const UInt64 *_inSize, const UInt64 *outSize,
      ICompressProgressInfo *progress);

  STDMETHOD(SetDecoderProperties2)(const Byte *data, UInt32 size);

  STDMETHOD(GetInStreamProcessedSize)(UInt64 *value);

  STDMETHOD(SetInStream)(ISequentialInStream *inStream);
  STDMETHOD(ReleaseInStream)();
  STDMETHOD(SetOutStreamSize)(const UInt64 *outSize);

  #ifndef NO_READ_FROM_CODER
  STDMETHOD(Read)(void *data, UInt32 size, UInt32 *processedSize);
  #endif

  CDecoder();
  virtual ~CDecoder();

};

}}

#endif
