// Compress/CopyCoder.h

#ifndef __COMPRESS_COPY_CODER_H
#define __COMPRESS_COPY_CODER_H

#include "../../Common/MyCom.h"

#include "../ICoder.h"

namespace NCompress {

class CCopyCoder:
  public ICompressCoder,
  public ICompressGetInStreamProcessedSize,
  public CMyUnknownImp
{
  Byte *_buffer;
public:
  UInt64 TotalSize;
  CCopyCoder(): TotalSize(0), _buffer(0) {};
  ~CCopyCoder();

  MY_UNKNOWN_IMP1(ICompressGetInStreamProcessedSize)

  STDMETHOD(Code)(ISequentialInStream *inStream, ISequentialOutStream *outStream,
      const UInt64 *inSize, const UInt64 *outSize, ICompressProgressInfo *progress);
  STDMETHOD(GetInStreamProcessedSize)(UInt64 *value);
};

HRESULT CopyStream(ISequentialInStream *inStream, ISequentialOutStream *outStream, ICompressProgressInfo *progress);

}

#endif
