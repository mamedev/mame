// DummyOutStream.h

#ifndef __DUMMYOUTSTREAM_H
#define __DUMMYOUTSTREAM_H

#include "../../IStream.h"
#include "Common/MyCom.h"

class CDummyOutStream:
  public ISequentialOutStream,
  public CMyUnknownImp
{
  CMyComPtr<ISequentialOutStream> _stream;
  UInt64 _size;
public:
  void SetStream(ISequentialOutStream *outStream) { _stream = outStream; }
  void ReleaseStream() { _stream.Release(); }
  void Init() { _size = 0; }
  MY_UNKNOWN_IMP
  STDMETHOD(Write)(const void *data, UInt32 size, UInt32 *processedSize);
  UInt64 GetSize() const { return _size; }
};

#endif
