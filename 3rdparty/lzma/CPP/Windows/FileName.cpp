// Windows/FileName.cpp

#include "StdAfx.h"

#include "FileName.h"

namespace NWindows {
namespace NFile {
namespace NName {

void NormalizeDirPathPrefix(FString &dirPath)
{
  if (dirPath.IsEmpty())
    return;
  if (dirPath.ReverseFind(FCHAR_PATH_SEPARATOR) != dirPath.Length() - 1)
    dirPath += FCHAR_PATH_SEPARATOR;
}

#ifndef USE_UNICODE_FSTRING
void NormalizeDirPathPrefix(UString &dirPath)
{
  if (dirPath.IsEmpty())
    return;
  if (dirPath.ReverseFind(WCHAR_PATH_SEPARATOR) != dirPath.Length() - 1)
    dirPath += WCHAR_PATH_SEPARATOR;
}
#endif

}}}
