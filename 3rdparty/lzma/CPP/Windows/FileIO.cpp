// Windows/FileIO.cpp

#include "StdAfx.h"

#include "FileIO.h"

#ifndef _UNICODE
extern bool g_IsNT;
#endif

namespace NWindows {
namespace NFile {

#ifdef SUPPORT_DEVICE_FILE

bool IsDeviceName(CFSTR n)
{
  #ifdef UNDER_CE
  int len = (int)MyStringLen(n);
  if (len < 5 || len > 5 || memcmp(n, FTEXT("DSK"), 3 * sizeof(FCHAR)) != 0)
    return false;
  if (n[4] != ':')
    return false;
  // for reading use SG_REQ sg; if (DeviceIoControl(dsk, IOCTL_DISK_READ));
  #else
  if (n[0] != '\\' || n[1] != '\\' || n[2] != '.' ||  n[3] != '\\')
    return false;
  int len = (int)MyStringLen(n);
  if (len == 6 && n[5] == ':')
    return true;
  if (len < 18 || len > 22 || memcmp(n + 4, FTEXT("PhysicalDrive"), 13 * sizeof(FCHAR)) != 0)
    return false;
  for (int i = 17; i < len; i++)
    if (n[i] < '0' || n[i] > '9')
      return false;
  #endif
  return true;
}

#endif

#if defined(WIN_LONG_PATH) && defined(_UNICODE)
#define WIN_LONG_PATH2
#endif

#ifdef WIN_LONG_PATH
bool GetLongPathBase(CFSTR s, UString &res)
{
  res.Empty();
  int len = MyStringLen(s);
  FChar c = s[0];
  if (len < 1 || c == '\\' || c == '.' && (len == 1 || len == 2 && s[1] == '.'))
    return true;
  UString curDir;
  bool isAbs = false;
  if (len > 3)
    isAbs = (s[1] == ':' && s[2] == '\\' && (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z'));

  if (!isAbs)
  {
    WCHAR temp[MAX_PATH + 2];
    temp[0] = 0;
    DWORD needLength = ::GetCurrentDirectoryW(MAX_PATH + 1, temp);
    if (needLength == 0 || needLength > MAX_PATH)
      return false;
    curDir = temp;
    if (curDir.Back() != L'\\')
      curDir += L'\\';
  }
  res = UString(L"\\\\?\\") + curDir + fs2us(s);
  return true;
}

bool GetLongPath(CFSTR path, UString &longPath)
{
  if (GetLongPathBase(path, longPath))
    return !longPath.IsEmpty();
  return false;
}
#endif

namespace NIO {

CFileBase::~CFileBase() { Close(); }

bool CFileBase::Create(CFSTR fileName, DWORD desiredAccess,
    DWORD shareMode, DWORD creationDisposition, DWORD flagsAndAttributes)
{
  if (!Close())
    return false;

  #ifdef SUPPORT_DEVICE_FILE
  IsDeviceFile = false;
  #endif

  #ifndef _UNICODE
  if (!g_IsNT)
  {
    _handle = ::CreateFile(fs2fas(fileName), desiredAccess, shareMode,
        (LPSECURITY_ATTRIBUTES)NULL, creationDisposition, flagsAndAttributes, (HANDLE)NULL);
  }
  else
  #endif
  {
    _handle = ::CreateFileW(fs2us(fileName), desiredAccess, shareMode,
        (LPSECURITY_ATTRIBUTES)NULL, creationDisposition, flagsAndAttributes, (HANDLE)NULL);
    #ifdef WIN_LONG_PATH
    if (_handle == INVALID_HANDLE_VALUE)
    {
      UString longPath;
      if (GetLongPath(fileName, longPath))
        _handle = ::CreateFileW(longPath, desiredAccess, shareMode,
            (LPSECURITY_ATTRIBUTES)NULL, creationDisposition, flagsAndAttributes, (HANDLE)NULL);
    }
    #endif
  }
  return (_handle != INVALID_HANDLE_VALUE);
}

bool CFileBase::Close()
{
  if (_handle == INVALID_HANDLE_VALUE)
    return true;
  if (!::CloseHandle(_handle))
    return false;
  _handle = INVALID_HANDLE_VALUE;
  return true;
}

bool CFileBase::GetPosition(UInt64 &position) const
{
  return Seek(0, FILE_CURRENT, position);
}

bool CFileBase::GetLength(UInt64 &length) const
{
  #ifdef SUPPORT_DEVICE_FILE
  if (IsDeviceFile && LengthDefined)
  {
    length = Length;
    return true;
  }
  #endif

  DWORD sizeHigh;
  DWORD sizeLow = ::GetFileSize(_handle, &sizeHigh);
  if (sizeLow == 0xFFFFFFFF)
    if (::GetLastError() != NO_ERROR)
      return false;
  length = (((UInt64)sizeHigh) << 32) + sizeLow;
  return true;
}

bool CFileBase::Seek(Int64 distanceToMove, DWORD moveMethod, UInt64 &newPosition) const
{
  #ifdef SUPPORT_DEVICE_FILE
  if (IsDeviceFile && LengthDefined && moveMethod == FILE_END)
  {
    distanceToMove += Length;
    moveMethod = FILE_BEGIN;
  }
  #endif

  LARGE_INTEGER value;
  value.QuadPart = distanceToMove;
  value.LowPart = ::SetFilePointer(_handle, value.LowPart, &value.HighPart, moveMethod);
  if (value.LowPart == 0xFFFFFFFF)
    if (::GetLastError() != NO_ERROR)
      return false;
  newPosition = value.QuadPart;
  return true;
}

bool CFileBase::Seek(UInt64 position, UInt64 &newPosition)
{
  return Seek(position, FILE_BEGIN, newPosition);
}

bool CFileBase::SeekToBegin()
{
  UInt64 newPosition;
  return Seek(0, newPosition);
}

bool CFileBase::SeekToEnd(UInt64 &newPosition)
{
  return Seek(0, FILE_END, newPosition);
}

/*
bool CFileBase::GetFileInformation(CByHandleFileInfo &fi) const
{
  BY_HANDLE_FILE_INFORMATION wfi;
  if (!::GetFileInformationByHandle(_handle, &wfi))
    return false;
  fi.Attrib = wfi.dwFileAttributes;
  fi.CTime = wfi.ftCreationTime;
  fi.ATime = wfi.ftLastAccessTime;
  fi.MTime = wfi.ftLastWriteTime;
  fi.Size = (((UInt64)wfi.nFileSizeHigh) << 32) + wfi.nFileSizeLow;
  fi.VolumeSerialNumber = wfi.dwVolumeSerialNumber;
  fi.NumLinks = wfi.nNumberOfLinks;
  fi.FileIndex = (((UInt64)wfi.nFileIndexHigh) << 32) + wfi.nFileIndexLow;
  return true;
}
*/

/////////////////////////
// CInFile

#ifdef SUPPORT_DEVICE_FILE
void CInFile::GetDeviceLength()
{
  if (_handle != INVALID_HANDLE_VALUE && IsDeviceFile)
  {
    #ifdef UNDER_CE
    LengthDefined = true;
    Length = 128 << 20;

    #else
    PARTITION_INFORMATION partInfo;
    LengthDefined = true;
    Length = 0;

    if (GetPartitionInfo(&partInfo))
      Length = partInfo.PartitionLength.QuadPart;
    else
    {
      DISK_GEOMETRY geom;
      if (!GetGeometry(&geom))
        if (!GetCdRomGeometry(&geom))
          LengthDefined = false;
      if (LengthDefined)
        Length = geom.Cylinders.QuadPart * geom.TracksPerCylinder * geom.SectorsPerTrack * geom.BytesPerSector;
    }
    // SeekToBegin();
    #endif
  }
}

// ((desiredAccess & (FILE_WRITE_DATA | FILE_APPEND_DATA | GENERIC_WRITE)) == 0 &&

#define MY_DEVICE_EXTRA_CODE \
  IsDeviceFile = IsDeviceName(fileName); \
  GetDeviceLength();
#else
#define MY_DEVICE_EXTRA_CODE
#endif

bool CInFile::Open(CFSTR fileName, DWORD shareMode, DWORD creationDisposition, DWORD flagsAndAttributes)
{
  bool res = Create(fileName, GENERIC_READ, shareMode, creationDisposition, flagsAndAttributes);
  MY_DEVICE_EXTRA_CODE
  return res;
}

bool CInFile::OpenShared(CFSTR fileName, bool shareForWrite)
{ return Open(fileName, FILE_SHARE_READ | (shareForWrite ? FILE_SHARE_WRITE : 0), OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL); }

bool CInFile::Open(CFSTR fileName)
  { return OpenShared(fileName, false); }

// ReadFile and WriteFile functions in Windows have BUG:
// If you Read or Write 64MB or more (probably min_failure_size = 64MB - 32KB + 1)
// from/to Network file, it returns ERROR_NO_SYSTEM_RESOURCES
// (Insufficient system resources exist to complete the requested service).

// Probably in some version of Windows there are problems with other sizes:
// for 32 MB (maybe also for 16 MB).
// And message can be "Network connection was lost"

static UInt32 kChunkSizeMax = (1 << 22);

bool CInFile::Read1(void *data, UInt32 size, UInt32 &processedSize)
{
  DWORD processedLoc = 0;
  bool res = BOOLToBool(::ReadFile(_handle, data, size, &processedLoc, NULL));
  processedSize = (UInt32)processedLoc;
  return res;
}

bool CInFile::ReadPart(void *data, UInt32 size, UInt32 &processedSize)
{
  if (size > kChunkSizeMax)
    size = kChunkSizeMax;
  return Read1(data, size, processedSize);
}

bool CInFile::Read(void *data, UInt32 size, UInt32 &processedSize)
{
  processedSize = 0;
  do
  {
    UInt32 processedLoc = 0;
    bool res = ReadPart(data, size, processedLoc);
    processedSize += processedLoc;
    if (!res)
      return false;
    if (processedLoc == 0)
      return true;
    data = (void *)((unsigned char *)data + processedLoc);
    size -= processedLoc;
  }
  while (size > 0);
  return true;
}

/////////////////////////
// COutFile

static inline DWORD GetCreationDisposition(bool createAlways)
  { return createAlways? CREATE_ALWAYS: CREATE_NEW; }

bool COutFile::Open(CFSTR fileName, DWORD shareMode, DWORD creationDisposition, DWORD flagsAndAttributes)
  { return CFileBase::Create(fileName, GENERIC_WRITE, shareMode, creationDisposition, flagsAndAttributes); }

bool COutFile::Open(CFSTR fileName, DWORD creationDisposition)
  { return Open(fileName, FILE_SHARE_READ, creationDisposition, FILE_ATTRIBUTE_NORMAL); }

bool COutFile::Create(CFSTR fileName, bool createAlways)
  { return Open(fileName, GetCreationDisposition(createAlways)); }

bool COutFile::SetTime(const FILETIME *cTime, const FILETIME *aTime, const FILETIME *mTime)
  { return BOOLToBool(::SetFileTime(_handle, cTime, aTime, mTime)); }

bool COutFile::SetMTime(const FILETIME *mTime) {  return SetTime(NULL, NULL, mTime); }

bool COutFile::WritePart(const void *data, UInt32 size, UInt32 &processedSize)
{
  if (size > kChunkSizeMax)
    size = kChunkSizeMax;
  DWORD processedLoc = 0;
  bool res = BOOLToBool(::WriteFile(_handle, data, size, &processedLoc, NULL));
  processedSize = (UInt32)processedLoc;
  return res;
}

bool COutFile::Write(const void *data, UInt32 size, UInt32 &processedSize)
{
  processedSize = 0;
  do
  {
    UInt32 processedLoc = 0;
    bool res = WritePart(data, size, processedLoc);
    processedSize += processedLoc;
    if (!res)
      return false;
    if (processedLoc == 0)
      return true;
    data = (const void *)((const unsigned char *)data + processedLoc);
    size -= processedLoc;
  }
  while (size > 0);
  return true;
}

bool COutFile::SetEndOfFile() { return BOOLToBool(::SetEndOfFile(_handle)); }

bool COutFile::SetLength(UInt64 length)
{
  UInt64 newPosition;
  if (!Seek(length, newPosition))
    return false;
  if (newPosition != length)
    return false;
  return SetEndOfFile();
}

}}}
