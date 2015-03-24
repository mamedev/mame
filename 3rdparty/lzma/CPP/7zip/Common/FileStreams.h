// FileStreams.h

#ifndef __FILE_STREAMS_H
#define __FILE_STREAMS_H

#ifdef _WIN32
#define USE_WIN_FILE
#endif

#include "../../Common/MyString.h"

#ifdef USE_WIN_FILE
#include "../../Windows/FileIO.h"
#else
#include "../../Common/C_FileIO.h"
#endif

#include "../../Common/MyCom.h"

#include "../IStream.h"

class CInFileStream:
  public IInStream,
  public IStreamGetSize,
  public CMyUnknownImp
{
public:
  #ifdef USE_WIN_FILE
  NWindows::NFile::NIO::CInFile File;
  #ifdef SUPPORT_DEVICE_FILE
  UInt64 VirtPos;
  UInt64 PhyPos;
  UInt64 BufferStartPos;
  Byte *Buffer;
  UInt32 BufferSize;
  #endif
  #else
  NC::NFile::NIO::CInFile File;
  #endif
  virtual ~CInFileStream();

  #ifdef SUPPORT_DEVICE_FILE
  CInFileStream();
  #endif
  
  bool Open(CFSTR fileName)
  {
    return File.Open(fileName);
  }
  
  bool OpenShared(CFSTR fileName, bool shareForWrite)
  {
    return File.OpenShared(fileName, shareForWrite);
  }

  MY_UNKNOWN_IMP2(IInStream, IStreamGetSize)

  STDMETHOD(Read)(void *data, UInt32 size, UInt32 *processedSize);
  STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition);

  STDMETHOD(GetSize)(UInt64 *size);
};

class CStdInFileStream:
  public ISequentialInStream,
  public CMyUnknownImp
{
public:
  MY_UNKNOWN_IMP

  virtual ~CStdInFileStream() {}
  STDMETHOD(Read)(void *data, UInt32 size, UInt32 *processedSize);
};

class COutFileStream:
  public IOutStream,
  public CMyUnknownImp
{
public:
  #ifdef USE_WIN_FILE
  NWindows::NFile::NIO::COutFile File;
  #else
  NC::NFile::NIO::COutFile File;
  #endif
  virtual ~COutFileStream() {}
  bool Create(CFSTR fileName, bool createAlways)
  {
    ProcessedSize = 0;
    return File.Create(fileName, createAlways);
  }
  bool Open(CFSTR fileName, DWORD creationDisposition)
  {
    ProcessedSize = 0;
    return File.Open(fileName, creationDisposition);
  }

  HRESULT Close();
  
  UInt64 ProcessedSize;

  #ifdef USE_WIN_FILE
  bool SetTime(const FILETIME *cTime, const FILETIME *aTime, const FILETIME *mTime)
  {
    return File.SetTime(cTime, aTime, mTime);
  }
  bool SetMTime(const FILETIME *mTime) {  return File.SetMTime(mTime); }
  #endif


  MY_UNKNOWN_IMP1(IOutStream)

  STDMETHOD(Write)(const void *data, UInt32 size, UInt32 *processedSize);
  STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition);
  STDMETHOD(SetSize)(UInt64 newSize);
};

class CStdOutFileStream:
  public ISequentialOutStream,
  public CMyUnknownImp
{
public:
  MY_UNKNOWN_IMP

  virtual ~CStdOutFileStream() {}
  STDMETHOD(Write)(const void *data, UInt32 size, UInt32 *processedSize);
};

#endif
