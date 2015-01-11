// ArchiveName.cpp

#include "StdAfx.h"

#include "Windows/FileDir.h"
#include "Windows/FileFind.h"

#include "ExtractingFilePath.h"

using namespace NWindows;

static FString CreateArchiveName2(const FString &srcName, bool fromPrev, bool keepName)
{
  FString resultName = FTEXT("Archive");
  if (fromPrev)
  {
    FString dirPrefix;
    if (NFile::NDirectory::GetOnlyDirPrefix(srcName, dirPrefix))
    {
      if (dirPrefix.Length() > 0)
        if (dirPrefix.Back() == FCHAR_PATH_SEPARATOR)
        {
          dirPrefix.DeleteBack();
          NFile::NFind::CFileInfo fileInfo;
          if (fileInfo.Find(dirPrefix))
            resultName = fileInfo.Name;
        }
    }
  }
  else
  {
    NFile::NFind::CFileInfo fileInfo;
    if (!fileInfo.Find(srcName))
      // return resultName;
      return srcName;
    resultName = fileInfo.Name;
    if (!fileInfo.IsDir() && !keepName)
    {
      int dotPos = resultName.ReverseFind('.');
      if (dotPos > 0)
      {
        FString archiveName2 = resultName.Left(dotPos);
        if (archiveName2.ReverseFind(FTEXT('.')) < 0)
          resultName = archiveName2;
      }
    }
  }
  return resultName;
}

UString CreateArchiveName(const UString &srcName, bool fromPrev, bool keepName)
{
  return GetCorrectFsPath(fs2us(CreateArchiveName2(us2fs(srcName), fromPrev, keepName)));
}
