// Windows/FileDir.h

#ifndef __WINDOWS_FILE_DIR_H
#define __WINDOWS_FILE_DIR_H

#include "../Common/MyString.h"
#include "FileIO.h"

namespace NWindows {
namespace NFile {
namespace NDirectory {

#ifdef WIN_LONG_PATH
bool GetLongPaths(CFSTR s1, CFSTR s2, UString &d1, UString &d2);
#endif

bool MyGetWindowsDirectory(FString &path);
bool MyGetSystemDirectory(FString &path);

bool SetDirTime(CFSTR fileName, const FILETIME *cTime, const FILETIME *aTime, const FILETIME *mTime);
bool MySetFileAttributes(CFSTR fileName, DWORD fileAttributes);
bool MyMoveFile(CFSTR existFileName, CFSTR newFileName);
bool MyRemoveDirectory(CFSTR path);
bool MyCreateDirectory(CFSTR path);
bool CreateComplexDirectory(CFSTR path);
bool DeleteFileAlways(CFSTR name);
bool RemoveDirectoryWithSubItems(const FString &path);

bool MyGetFullPathName(CFSTR path, FString &resFullPath);
bool GetFullPathAndSplit(CFSTR path, FString &resDirPrefix, FString &resFileName);
bool GetOnlyDirPrefix(CFSTR path, FString &resDirPrefix);

#ifndef UNDER_CE

bool MySetCurrentDirectory(CFSTR path);
bool MyGetCurrentDirectory(FString &resultPath);

#endif

bool MyGetTempPath(FString &resultPath);

class CTempFile
{
  bool _mustBeDeleted;
  FString _path;
  void DisableDeleting() { _mustBeDeleted = false; }
public:
  CTempFile(): _mustBeDeleted(false) {}
  ~CTempFile() { Remove(); }
  const FString &GetPath() const { return _path; }
  bool Create(CFSTR pathPrefix, NIO::COutFile *outFile); // pathPrefix is not folder prefix
  bool CreateRandomInTempFolder(CFSTR namePrefix, NIO::COutFile *outFile);
  bool Remove();
  bool MoveTo(CFSTR name, bool deleteDestBefore);
};

class CTempDir
{
  bool _mustBeDeleted;
  FString _path;
public:
  CTempDir(): _mustBeDeleted(false) {}
  ~CTempDir() { Remove();  }
  const FString &GetPath() const { return _path; }
  void DisableDeleting() { _mustBeDeleted = false; }
  bool Create(CFSTR namePrefix) ;
  bool Remove();
};

}}}

#endif
