// Windows/FileFind.cpp

#include "StdAfx.h"

#include "FileFind.h"
#include "FileIO.h"
#ifndef _UNICODE
#include "../Common/StringConvert.h"
#endif

#ifndef _UNICODE
extern bool g_IsNT;
#endif

namespace NWindows {
namespace NFile {

#ifdef SUPPORT_DEVICE_FILE
bool IsDeviceName(CFSTR n);
#endif

#if defined(WIN_LONG_PATH)
bool GetLongPath(CFSTR fileName, UString &res);
#endif

namespace NFind {

bool CFileInfo::IsDots() const
{
  if (!IsDir() || Name.IsEmpty())
    return false;
  if (Name[0] != FTEXT('.'))
    return false;
  return Name.Length() == 1 || (Name.Length() == 2 && Name[1] == FTEXT('.'));
}

#define WIN_FD_TO_MY_FI(fi, fd) \
  fi.Attrib = fd.dwFileAttributes; \
  fi.CTime = fd.ftCreationTime; \
  fi.ATime = fd.ftLastAccessTime; \
  fi.MTime = fd.ftLastWriteTime; \
  fi.Size = (((UInt64)fd.nFileSizeHigh) << 32) + fd.nFileSizeLow; \
  fi.IsDevice = false;

  /*
  #ifdef UNDER_CE
  fi.ObjectID = fd.dwOID;
  #else
  fi.ReparseTag = fd.dwReserved0;
  #endif
  */

static void ConvertWIN32_FIND_DATA_To_FileInfo(const WIN32_FIND_DATAW &fd, CFileInfo &fi)
{
  WIN_FD_TO_MY_FI(fi, fd);
  fi.Name = us2fs(fd.cFileName);
}

#ifndef _UNICODE

static inline UINT GetCurrentCodePage() { return ::AreFileApisANSI() ? CP_ACP : CP_OEMCP; }

static void ConvertWIN32_FIND_DATA_To_FileInfo(const WIN32_FIND_DATA &fd, CFileInfo &fi)
{
  WIN_FD_TO_MY_FI(fi, fd);
  fi.Name = fas2fs(fd.cFileName);
}
#endif
  
////////////////////////////////
// CFindFile

bool CFindFile::Close()
{
  if (_handle == INVALID_HANDLE_VALUE)
    return true;
  if (!::FindClose(_handle))
    return false;
  _handle = INVALID_HANDLE_VALUE;
  return true;
}

bool CFindFile::FindFirst(CFSTR wildcard, CFileInfo &fi)
{
  if (!Close())
    return false;
  #ifndef _UNICODE
  if (!g_IsNT)
  {
    WIN32_FIND_DATAA fd;
    _handle = ::FindFirstFileA(fs2fas(wildcard), &fd);
    if (_handle == INVALID_HANDLE_VALUE)
      return false;
    ConvertWIN32_FIND_DATA_To_FileInfo(fd, fi);
  }
  else
  #endif
  {
    WIN32_FIND_DATAW fd;
    _handle = ::FindFirstFileW(fs2us(wildcard), &fd);
    #ifdef WIN_LONG_PATH
    if (_handle == INVALID_HANDLE_VALUE)
    {
      UString longPath;
      if (GetLongPath(wildcard, longPath))
        _handle = ::FindFirstFileW(longPath, &fd);
    }
    #endif
    if (_handle == INVALID_HANDLE_VALUE)
      return false;
    ConvertWIN32_FIND_DATA_To_FileInfo(fd, fi);
  }
  return true;
}

bool CFindFile::FindNext(CFileInfo &fi)
{
  #ifndef _UNICODE
  if (!g_IsNT)
  {
    WIN32_FIND_DATAA fd;
    if (!::FindNextFileA(_handle, &fd))
      return false;
    ConvertWIN32_FIND_DATA_To_FileInfo(fd, fi);
  }
  else
  #endif
  {
    WIN32_FIND_DATAW fd;
    if (!::FindNextFileW(_handle, &fd))
      return false;
    ConvertWIN32_FIND_DATA_To_FileInfo(fd, fi);
  }
  return true;
}

#define MY_CLEAR_FILETIME(ft) ft.dwLowDateTime = ft.dwHighDateTime = 0;

void CFileInfoBase::Clear()
{
  Size = 0;
  MY_CLEAR_FILETIME(CTime);
  MY_CLEAR_FILETIME(ATime);
  MY_CLEAR_FILETIME(MTime);
  Attrib = 0;
}
  
bool CFileInfo::Find(CFSTR wildcard)
{
  #ifdef SUPPORT_DEVICE_FILE
  if (IsDeviceName(wildcard))
  {
    Clear();
    IsDevice = true;
    NIO::CInFile inFile;
    if (!inFile.Open(wildcard))
      return false;
    Name = wildcard + 4;
    if (inFile.LengthDefined)
      Size = inFile.Length;
    return true;
  }
  #endif
  CFindFile finder;
  if (finder.FindFirst(wildcard, *this))
    return true;
  #ifdef _WIN32
  {
    DWORD lastError = GetLastError();
    if (lastError == ERROR_BAD_NETPATH || lastError == ERROR_FILE_NOT_FOUND)
    {
      int len = MyStringLen(wildcard);
      if (len > 2 && wildcard[0] == '\\' && wildcard[1] == '\\')
      {
        int pos = FindCharPosInString(wildcard + 2, FTEXT('\\'));
        if (pos >= 0)
        {
          pos += 2 + 1;
          len -= pos;
          CFSTR remString = wildcard + pos;
          int pos2 = FindCharPosInString(remString, FTEXT('\\'));
          FString s = wildcard;
          if (pos2 < 0 || pos2 == len - 1)
          {
            FString s = wildcard;
            if (pos2 < 0)
            {
              pos2 = len;
              s += FTEXT('\\');
            }
            s += FCHAR_ANY_MASK;
            if (finder.FindFirst(s, *this))
              if (Name == FTEXT("."))
              {
                Name = s.Mid(pos, pos2);
                return true;
              }
            ::SetLastError(lastError);
          }
        }
      }
    }
  }
  #endif
  return false;
}

bool DoesFileExist(CFSTR name)
{
  CFileInfo fi;
  return fi.Find(name) && !fi.IsDir();
}

bool DoesDirExist(CFSTR name)
{
  CFileInfo fi;
  return fi.Find(name) && fi.IsDir();
}
bool DoesFileOrDirExist(CFSTR name)
{
  CFileInfo fi;
  return fi.Find(name);
}

bool CEnumerator::NextAny(CFileInfo &fi)
{
  if (_findFile.IsHandleAllocated())
    return _findFile.FindNext(fi);
  else
    return _findFile.FindFirst(_wildcard, fi);
}

bool CEnumerator::Next(CFileInfo &fi)
{
  for (;;)
  {
    if (!NextAny(fi))
      return false;
    if (!fi.IsDots())
      return true;
  }
}

bool CEnumerator::Next(CFileInfo &fi, bool &found)
{
  if (Next(fi))
  {
    found = true;
    return true;
  }
  found = false;
  return (::GetLastError() == ERROR_NO_MORE_FILES);
}

////////////////////////////////
// CFindChangeNotification
// FindFirstChangeNotification can return 0. MSDN doesn't tell about it.

bool CFindChangeNotification::Close()
{
  if (!IsHandleAllocated())
    return true;
  if (!::FindCloseChangeNotification(_handle))
    return false;
  _handle = INVALID_HANDLE_VALUE;
  return true;
}
           
HANDLE CFindChangeNotification::FindFirst(CFSTR pathName, bool watchSubtree, DWORD notifyFilter)
{
  #ifndef _UNICODE
  if (!g_IsNT)
    _handle = ::FindFirstChangeNotification(fs2fas(pathName), BoolToBOOL(watchSubtree), notifyFilter);
  else
  #endif
  {
    _handle = ::FindFirstChangeNotificationW(fs2us(pathName), BoolToBOOL(watchSubtree), notifyFilter);
    #ifdef WIN_LONG_PATH
    if (!IsHandleAllocated())
    {
      UString longPath;
      if (GetLongPath(pathName, longPath))
        _handle = ::FindFirstChangeNotificationW(longPath, BoolToBOOL(watchSubtree), notifyFilter);
    }
    #endif
  }
  return _handle;
}

#ifndef UNDER_CE

bool MyGetLogicalDriveStrings(CObjectVector<FString> &driveStrings)
{
  driveStrings.Clear();
  #ifndef _UNICODE
  if (!g_IsNT)
  {
    driveStrings.Clear();
    UINT32 size = GetLogicalDriveStrings(0, NULL);
    if (size == 0)
      return false;
    AString buf;
    UINT32 newSize = GetLogicalDriveStrings(size, buf.GetBuffer(size));
    if (newSize == 0 || newSize > size)
      return false;
    AString s;
    for (UINT32 i = 0; i < newSize; i++)
    {
      char c = buf[i];
      if (c == '\0')
      {
        driveStrings.Add(fas2fs(s));
        s.Empty();
      }
      else
        s += c;
    }
    return s.IsEmpty();
  }
  else
  #endif
  {
    UINT32 size = GetLogicalDriveStringsW(0, NULL);
    if (size == 0)
      return false;
    UString buf;
    UINT32 newSize = GetLogicalDriveStringsW(size, buf.GetBuffer(size));
    if (newSize == 0 || newSize > size)
      return false;
    UString s;
    for (UINT32 i = 0; i < newSize; i++)
    {
      WCHAR c = buf[i];
      if (c == L'\0')
      {
        driveStrings.Add(us2fs(s));
        s.Empty();
      }
      else
        s += c;
    }
    return s.IsEmpty();
  }
}

#endif

}}}
