// ExtractingFilePath.h

#ifndef __EXTRACTING_FILE_PATH_H
#define __EXTRACTING_FILE_PATH_H

#include "Common/MyString.h"

UString MakePathNameFromParts(const UStringVector &parts);
void MakeCorrectPath(UStringVector &pathParts);
UString GetCorrectFsPath(const UString &path);
UString GetCorrectFullFsPath(const UString &path);

#endif
