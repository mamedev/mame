// WorkDir.cpp

#include "StdAfx.h"

#include "Common/StringConvert.h"
#include "Common/Wildcard.h"

#include "Windows/FileName.h"

#include "WorkDir.h"

using namespace NWindows;
using namespace NFile;
using namespace NDirectory;

FString GetWorkDir(const NWorkDir::CInfo &workDirInfo, const FString &path, FString &fileName)
{
  NWorkDir::NMode::EEnum mode = workDirInfo.Mode;
  #ifndef UNDER_CE
  if (workDirInfo.ForRemovableOnly)
  {
    mode = NWorkDir::NMode::kCurrent;
    FString prefix = path.Left(3);
    if (prefix[1] == FTEXT(':') && prefix[2] == FTEXT('\\'))
    {
      UINT driveType = GetDriveType(GetSystemString(prefix, ::AreFileApisANSI() ? CP_ACP : CP_OEMCP));
      if (driveType == DRIVE_CDROM || driveType == DRIVE_REMOVABLE)
        mode = workDirInfo.Mode;
    }
    /*
    CParsedPath parsedPath;
    parsedPath.ParsePath(archiveName);
    UINT driveType = GetDriveType(parsedPath.Prefix);
    if ((driveType != DRIVE_CDROM) && (driveType != DRIVE_REMOVABLE))
      mode = NZipSettings::NWorkDir::NMode::kCurrent;
    */
  }
  #endif
  int pos = path.ReverseFind(FCHAR_PATH_SEPARATOR) + 1;
  fileName = path.Mid(pos);
  switch (mode)
  {
    case NWorkDir::NMode::kCurrent:
    {
      return path.Left(pos);;
    }
    case NWorkDir::NMode::kSpecified:
    {
      FString tempDir = workDirInfo.Path;
      NName::NormalizeDirPathPrefix(tempDir);
      return tempDir;
    }
    default:
    {
      FString tempDir;
      if (!MyGetTempPath(tempDir))
        throw 141717;
      return tempDir;
    }
  }
}

HRESULT CWorkDirTempFile::CreateTempFile(const FString &originalPath)
{
  NWorkDir::CInfo workDirInfo;
  workDirInfo.Load();
  FString namePart;
  FString workDir = GetWorkDir(workDirInfo, originalPath, namePart);
  CreateComplexDirectory(workDir);
  CTempFile tempFile;
  _outStreamSpec = new COutFileStream;
  OutStream = _outStreamSpec;
  if (!_tempFile.Create(workDir + namePart, &_outStreamSpec->File))
  {
    DWORD error = GetLastError();
    return error ? error : E_FAIL;
  }
  _originalPath = originalPath;
  return S_OK;
}

HRESULT CWorkDirTempFile::MoveToOriginal(bool deleteOriginal)
{
  OutStream.Release();
  if (!_tempFile.MoveTo(_originalPath, deleteOriginal))
  {
    DWORD error = GetLastError();
    return error ? error : E_FAIL;
  }
  return S_OK;
}
