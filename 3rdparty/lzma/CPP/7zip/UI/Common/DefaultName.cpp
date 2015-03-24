// DefaultName.cpp

#include "StdAfx.h"

#include "DefaultName.h"

static UString GetDefaultName3(const UString &fileName,
    const UString &extension, const UString &addSubExtension)
{
  int extLength = extension.Length();
  int fileNameLength = fileName.Length();
  if (fileNameLength > extLength + 1)
  {
    int dotPos = fileNameLength - (extLength + 1);
    if (fileName[dotPos] == '.')
      if (extension.CompareNoCase(fileName.Mid(dotPos + 1)) == 0)
        return fileName.Left(dotPos) + addSubExtension;
  }
  int dotPos = fileName.ReverseFind(L'.');
  if (dotPos > 0)
    return fileName.Left(dotPos) + addSubExtension;

  if (addSubExtension.IsEmpty())
    return fileName + L"~";
  else
    return fileName + addSubExtension;
}

UString GetDefaultName2(const UString &fileName,
    const UString &extension, const UString &addSubExtension)
{
  UString name = GetDefaultName3(fileName, extension, addSubExtension);
  name.TrimRight();
  return name;
}
