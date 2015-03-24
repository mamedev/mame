// FilePathAutoRename.cpp

#include "StdAfx.h"

#include "Common/Defs.h"
#include "Common/IntToString.h"

#include "Windows/FileFind.h"

#include "FilePathAutoRename.h"

using namespace NWindows;

static bool MakeAutoName(const FString &name,
    const FString &extension, unsigned value, FString &path)
{
  FChar number[16];
  ConvertUInt32ToString(value, number);
  path = name;
  path += number;
  path += extension;
  return NFile::NFind::DoesFileOrDirExist(path);
}

bool AutoRenamePath(FString &fullProcessedPath)
{
  FString path;
  int dotPos = fullProcessedPath.ReverseFind(FTEXT('.'));

  int slashPos = fullProcessedPath.ReverseFind(FTEXT('/'));
  #ifdef _WIN32
  int slash1Pos = fullProcessedPath.ReverseFind(FTEXT('\\'));
  slashPos = MyMax(slashPos, slash1Pos);
  #endif

  FString name, extension;
  if (dotPos > slashPos && dotPos > 0)
  {
    name = fullProcessedPath.Left(dotPos);
    extension = fullProcessedPath.Mid(dotPos);
  }
  else
    name = fullProcessedPath;
  name += L'_';
  unsigned left = 1, right = (1 << 30);
  while (left != right)
  {
    unsigned mid = (left + right) / 2;
    if (MakeAutoName(name, extension, mid, path))
      left = mid + 1;
    else
      right = mid;
  }
  return !MakeAutoName(name, extension, right, fullProcessedPath);
}
