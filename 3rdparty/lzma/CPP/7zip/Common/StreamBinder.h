// StreamBinder.h

#ifndef __STREAM_BINDER_H
#define __STREAM_BINDER_H

#include "../../Windows/Synchronization.h"

#include "../IStream.h"

class CStreamBinder
{
  NWindows::NSynchronization::CManualResetEvent _canWrite_Event;
  NWindows::NSynchronization::CManualResetEvent _canRead_Event;
  NWindows::NSynchronization::CManualResetEvent _readingWasClosed_Event;
  bool _waitWrite;
  UInt32 _bufSize;
  const void *_buf;
public:
  UInt64 ProcessedSize;

  WRes CreateEvents();
  void CreateStreams(ISequentialInStream **inStream, ISequentialOutStream **outStream);
  void ReInit();
  HRESULT Read(void *data, UInt32 size, UInt32 *processedSize);
  HRESULT Write(const void *data, UInt32 size, UInt32 *processedSize);
  void CloseRead() { _readingWasClosed_Event.Set(); }
  void CloseWrite()
  {
    // _bufSize must be = 0
    _canRead_Event.Set();
  }
};

#endif
