// StreamBinder.cpp

#include "StdAfx.h"

#include "../../Common/MyCom.h"

#include "StreamBinder.h"

class CBinderInStream:
  public ISequentialInStream,
  public CMyUnknownImp
{
  CStreamBinder *_binder;
public:
  MY_UNKNOWN_IMP
  STDMETHOD(Read)(void *data, UInt32 size, UInt32 *processedSize);
  ~CBinderInStream() { _binder->CloseRead(); }
  CBinderInStream(CStreamBinder *binder): _binder(binder) {}
};

STDMETHODIMP CBinderInStream::Read(void *data, UInt32 size, UInt32 *processedSize)
  { return _binder->Read(data, size, processedSize); }

class CBinderOutStream:
  public ISequentialOutStream,
  public CMyUnknownImp
{
  CStreamBinder *_binder;
public:
  MY_UNKNOWN_IMP
  STDMETHOD(Write)(const void *data, UInt32 size, UInt32 *processedSize);
  ~CBinderOutStream() { _binder->CloseWrite(); }
  CBinderOutStream(CStreamBinder *binder): _binder(binder) {}
};

STDMETHODIMP CBinderOutStream::Write(const void *data, UInt32 size, UInt32 *processedSize)
  { return _binder->Write(data, size, processedSize); }



WRes CStreamBinder::CreateEvents()
{
  RINOK(_canWrite_Event.Create(true));
  RINOK(_canRead_Event.Create());
  return _readingWasClosed_Event.Create();
}

void CStreamBinder::ReInit()
{
  _waitWrite = true;
  _canRead_Event.Reset();
  _readingWasClosed_Event.Reset();
  ProcessedSize = 0;
}


void CStreamBinder::CreateStreams(ISequentialInStream **inStream, ISequentialOutStream **outStream)
{
  _waitWrite = true;
  _bufSize = 0;
  _buf = NULL;
  ProcessedSize = 0;

  CBinderInStream *inStreamSpec = new CBinderInStream(this);
  CMyComPtr<ISequentialInStream> inStreamLoc(inStreamSpec);
  *inStream = inStreamLoc.Detach();

  CBinderOutStream *outStreamSpec = new CBinderOutStream(this);
  CMyComPtr<ISequentialOutStream> outStreamLoc(outStreamSpec);
  *outStream = outStreamLoc.Detach();
}

// (_canRead_Event && _bufSize == 0) means that stream is finished.

HRESULT CStreamBinder::Read(void *data, UInt32 size, UInt32 *processedSize)
{
  if (processedSize)
    *processedSize = 0;
  if (size != 0)
  {
    if (_waitWrite)
    {
      RINOK(_canRead_Event.Lock());
      _waitWrite = false;
    }
    if (size > _bufSize)
      size = _bufSize;
    if (size != 0)
    {
      memcpy(data, _buf, size);
      _buf = ((const Byte *)_buf) + size;
      ProcessedSize += size;
      if (processedSize)
        *processedSize = size;
      _bufSize -= size;
      if (_bufSize == 0)
      {
        _waitWrite = true;
        _canRead_Event.Reset();
        _canWrite_Event.Set();
      }
    }
  }
  return S_OK;
}

HRESULT CStreamBinder::Write(const void *data, UInt32 size, UInt32 *processedSize)
{
  if (processedSize)
    *processedSize = 0;
  if (size != 0)
  {
    _buf = data;
    _bufSize = size;
    _canWrite_Event.Reset();
    _canRead_Event.Set();

    HANDLE events[2] = { _canWrite_Event, _readingWasClosed_Event };
    DWORD waitResult = ::WaitForMultipleObjects(2, events, FALSE, INFINITE);
    if (waitResult != WAIT_OBJECT_0 + 0)
      return S_FALSE;
    if (processedSize)
      *processedSize = size;
  }
  return S_OK;
}
