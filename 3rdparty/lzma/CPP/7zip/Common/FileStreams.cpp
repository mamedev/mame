// FileStreams.cpp

#include "StdAfx.h"

#ifndef _WIN32
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#endif

#ifdef SUPPORT_DEVICE_FILE
#include "../../../C/Alloc.h"
#include "../../Common/Defs.h"
#endif

#include "FileStreams.h"

static inline HRESULT ConvertBoolToHRESULT(bool result)
{
  #ifdef _WIN32
  if (result)
    return S_OK;
  DWORD lastError = ::GetLastError();
  if (lastError == 0)
    return E_FAIL;
  return HRESULT_FROM_WIN32(lastError);
  #else
  return result ? S_OK: E_FAIL;
  #endif
}

#ifdef SUPPORT_DEVICE_FILE

static const UInt32 kClusterSize = 1 << 18;
CInFileStream::CInFileStream():
  VirtPos(0),
  PhyPos(0),
  Buffer(0),
  BufferSize(0)
{
}

#endif

CInFileStream::~CInFileStream()
{
  #ifdef SUPPORT_DEVICE_FILE
  MidFree(Buffer);
  #endif
}

STDMETHODIMP CInFileStream::Read(void *data, UInt32 size, UInt32 *processedSize)
{
  #ifdef USE_WIN_FILE
  
  #ifdef SUPPORT_DEVICE_FILE
  if (processedSize != NULL)
    *processedSize = 0;
  if (size == 0)
    return S_OK;
  if (File.IsDeviceFile)
  {
    if (File.LengthDefined)
    {
      if (VirtPos >= File.Length)
        return VirtPos == File.Length ? S_OK : E_FAIL;
      UInt64 rem = File.Length - VirtPos;
      if (size > rem)
        size = (UInt32)rem;
    }
    for (;;)
    {
      const UInt32 mask = kClusterSize - 1;
      UInt64 mask2 = ~(UInt64)mask;
      UInt64 alignedPos = VirtPos & mask2;
      if (BufferSize > 0 && BufferStartPos == alignedPos)
      {
        UInt32 pos = (UInt32)VirtPos & mask;
        if (pos >= BufferSize)
          return S_OK;
        UInt32 rem = MyMin(BufferSize - pos, size);
        memcpy(data, Buffer + pos, rem);
        VirtPos += rem;
        if (processedSize != NULL)
          *processedSize += rem;
        return S_OK;
      }
      
      bool useBuffer = false;
      if ((VirtPos & mask) != 0 || ((ptrdiff_t)data & mask) != 0 )
        useBuffer = true;
      else
      {
        UInt64 end = VirtPos + size;
        if ((end & mask) != 0)
        {
          end &= mask2;
          if (end <= VirtPos)
            useBuffer = true;
          else
            size = (UInt32)(end - VirtPos);
        }
      }
      if (!useBuffer)
        break;
      if (alignedPos != PhyPos)
      {
        UInt64 realNewPosition;
        bool result = File.Seek(alignedPos, FILE_BEGIN, realNewPosition);
        if (!result)
          return ConvertBoolToHRESULT(result);
        PhyPos = realNewPosition;
      }

      BufferStartPos = alignedPos;
      UInt32 readSize = kClusterSize;
      if (File.LengthDefined)
        readSize = (UInt32)MyMin(File.Length - PhyPos, (UInt64)kClusterSize);

      if (Buffer == 0)
      {
        Buffer = (Byte *)MidAlloc(kClusterSize);
        if (Buffer == 0)
          return E_OUTOFMEMORY;
      }
      bool result = File.Read1(Buffer, readSize, BufferSize);
      if (!result)
        return ConvertBoolToHRESULT(result);

      if (BufferSize == 0)
        return S_OK;
      PhyPos += BufferSize;
    }

    if (VirtPos != PhyPos)
    {
      UInt64 realNewPosition;
      bool result = File.Seek(VirtPos, FILE_BEGIN, realNewPosition);
      if (!result)
        return ConvertBoolToHRESULT(result);
      PhyPos = VirtPos = realNewPosition;
    }
  }
  #endif

  UInt32 realProcessedSize;
  bool result = File.ReadPart(data, size, realProcessedSize);
  if (processedSize != NULL)
    *processedSize = realProcessedSize;
  #ifdef SUPPORT_DEVICE_FILE
  VirtPos += realProcessedSize;
  PhyPos += realProcessedSize;
  #endif
  return ConvertBoolToHRESULT(result);
  
  #else
  
  if (processedSize != NULL)
    *processedSize = 0;
  ssize_t res = File.Read(data, (size_t)size);
  if (res == -1)
    return E_FAIL;
  if (processedSize != NULL)
    *processedSize = (UInt32)res;
  return S_OK;

  #endif
}

#ifdef UNDER_CE
STDMETHODIMP CStdInFileStream::Read(void *data, UInt32 size, UInt32 *processedSize)
{
  size_t s2 = fread(data, 1, size, stdout);
  if (processedSize != 0)
    *processedSize = s2;
  return (s2 = size) ? S_OK : E_FAIL;
}
#else
STDMETHODIMP CStdInFileStream::Read(void *data, UInt32 size, UInt32 *processedSize)
{
  #ifdef _WIN32
  
  DWORD realProcessedSize;
  UInt32 sizeTemp = (1 << 20);
  if (sizeTemp > size)
    sizeTemp = size;
  BOOL res = ::ReadFile(GetStdHandle(STD_INPUT_HANDLE), data, sizeTemp, &realProcessedSize, NULL);
  if (processedSize != NULL)
    *processedSize = realProcessedSize;
  if (res == FALSE && GetLastError() == ERROR_BROKEN_PIPE)
    return S_OK;
  return ConvertBoolToHRESULT(res != FALSE);
  
  #else

  if (processedSize != NULL)
    *processedSize = 0;
  ssize_t res;
  do
  {
    res = read(0, data, (size_t)size);
  }
  while (res < 0 && (errno == EINTR));
  if (res == -1)
    return E_FAIL;
  if (processedSize != NULL)
    *processedSize = (UInt32)res;
  return S_OK;
  
  #endif
}
  
#endif

STDMETHODIMP CInFileStream::Seek(Int64 offset, UInt32 seekOrigin,
    UInt64 *newPosition)
{
  if (seekOrigin >= 3)
    return STG_E_INVALIDFUNCTION;

  #ifdef USE_WIN_FILE

  #ifdef SUPPORT_DEVICE_FILE
  if (File.IsDeviceFile)
  {
    UInt64 newVirtPos = offset;
    switch(seekOrigin)
    {
      case STREAM_SEEK_SET: break;
      case STREAM_SEEK_CUR: newVirtPos += VirtPos; break;
      case STREAM_SEEK_END: newVirtPos += File.Length; break;
      default: return STG_E_INVALIDFUNCTION;
    }
    VirtPos = newVirtPos;
    if (newPosition)
      *newPosition = newVirtPos;
    return S_OK;
  }
  #endif
  
  UInt64 realNewPosition;
  bool result = File.Seek(offset, seekOrigin, realNewPosition);
  
  #ifdef SUPPORT_DEVICE_FILE
  PhyPos = VirtPos = realNewPosition;
  #endif

  if (newPosition != NULL)
    *newPosition = realNewPosition;
  return ConvertBoolToHRESULT(result);
  
  #else
  
  off_t res = File.Seek((off_t)offset, seekOrigin);
  if (res == -1)
    return E_FAIL;
  if (newPosition != NULL)
    *newPosition = (UInt64)res;
  return S_OK;
  
  #endif
}

STDMETHODIMP CInFileStream::GetSize(UInt64 *size)
{
  return ConvertBoolToHRESULT(File.GetLength(*size));
}


//////////////////////////
// COutFileStream

HRESULT COutFileStream::Close()
{
  return ConvertBoolToHRESULT(File.Close());
}

STDMETHODIMP COutFileStream::Write(const void *data, UInt32 size, UInt32 *processedSize)
{
  #ifdef USE_WIN_FILE

  UInt32 realProcessedSize;
  bool result = File.WritePart(data, size, realProcessedSize);
  ProcessedSize += realProcessedSize;
  if (processedSize != NULL)
    *processedSize = realProcessedSize;
  return ConvertBoolToHRESULT(result);
  
  #else
  
  if (processedSize != NULL)
    *processedSize = 0;
  ssize_t res = File.Write(data, (size_t)size);
  if (res == -1)
    return E_FAIL;
  if (processedSize != NULL)
    *processedSize = (UInt32)res;
  ProcessedSize += res;
  return S_OK;
  
  #endif
}
  
STDMETHODIMP COutFileStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition)
{
  if (seekOrigin >= 3)
    return STG_E_INVALIDFUNCTION;
  #ifdef USE_WIN_FILE

  UInt64 realNewPosition;
  bool result = File.Seek(offset, seekOrigin, realNewPosition);
  if (newPosition != NULL)
    *newPosition = realNewPosition;
  return ConvertBoolToHRESULT(result);
  
  #else
  
  off_t res = File.Seek((off_t)offset, seekOrigin);
  if (res == -1)
    return E_FAIL;
  if (newPosition != NULL)
    *newPosition = (UInt64)res;
  return S_OK;
  
  #endif
}

STDMETHODIMP COutFileStream::SetSize(UInt64 newSize)
{
  #ifdef USE_WIN_FILE
  UInt64 currentPos;
  if (!File.Seek(0, FILE_CURRENT, currentPos))
    return E_FAIL;
  bool result = File.SetLength(newSize);
  UInt64 currentPos2;
  result = result && File.Seek(currentPos, currentPos2);
  return result ? S_OK : E_FAIL;
  #else
  return E_FAIL;
  #endif
}

#ifdef UNDER_CE
STDMETHODIMP CStdOutFileStream::Write(const void *data, UInt32 size, UInt32 *processedSize)
{
  size_t s2 = fwrite(data, 1, size, stdout);
  if (processedSize != 0)
    *processedSize = s2;
  return (s2 = size) ? S_OK : E_FAIL;
}
#else
STDMETHODIMP CStdOutFileStream::Write(const void *data, UInt32 size, UInt32 *processedSize)
{
  if (processedSize != NULL)
    *processedSize = 0;

  #ifdef _WIN32
  UInt32 realProcessedSize;
  BOOL res = TRUE;
  if (size > 0)
  {
    // Seems that Windows doesn't like big amounts writing to stdout.
    // So we limit portions by 32KB.
    UInt32 sizeTemp = (1 << 15);
    if (sizeTemp > size)
      sizeTemp = size;
    res = ::WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),
        data, sizeTemp, (DWORD *)&realProcessedSize, NULL);
    size -= realProcessedSize;
    data = (const void *)((const Byte *)data + realProcessedSize);
    if (processedSize != NULL)
      *processedSize += realProcessedSize;
  }
  return ConvertBoolToHRESULT(res != FALSE);

  #else
  
  ssize_t res;
  do
  {
    res = write(1, data, (size_t)size);
  }
  while (res < 0 && (errno == EINTR));
  if (res == -1)
    return E_FAIL;
  if (processedSize != NULL)
    *processedSize = (UInt32)res;
  return S_OK;
  
  return S_OK;
  #endif
}

#endif
