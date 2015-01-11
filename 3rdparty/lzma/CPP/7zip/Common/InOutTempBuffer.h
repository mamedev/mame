// InOutTempBuffer.h

#ifndef __IN_OUT_TEMP_BUFFER_H
#define __IN_OUT_TEMP_BUFFER_H

#include "../../Common/MyCom.h"
#include "../../Windows/FileDir.h"

#include "../IStream.h"

class CInOutTempBuffer
{
  NWindows::NFile::NDirectory::CTempFile _tempFile;
  NWindows::NFile::NIO::COutFile _outFile;
  Byte *_buf;
  UInt32 _bufPos;
  bool _tempFileCreated;
  UInt64 _size;
  UInt32 _crc;

  bool WriteToFile(const void *data, UInt32 size);
public:
  CInOutTempBuffer();
  ~CInOutTempBuffer();
  void Create();

  void InitWriting();
  bool Write(const void *data, UInt32 size);

  HRESULT WriteToStream(ISequentialOutStream *stream);
  UInt64 GetDataSize() const { return _size; }
};

class CSequentialOutTempBufferImp:
  public ISequentialOutStream,
  public CMyUnknownImp
{
  CInOutTempBuffer *_buf;
public:
  void Init(CInOutTempBuffer *buffer)  { _buf = buffer; }
  MY_UNKNOWN_IMP

  STDMETHOD(Write)(const void *data, UInt32 size, UInt32 *processedSize);
};

#endif
