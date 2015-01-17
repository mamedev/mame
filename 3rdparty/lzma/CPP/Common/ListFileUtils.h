// Common/ListFileUtils.h

#ifndef __COMMON_LIST_FILE_UTILS_H
#define __COMMON_LIST_FILE_UTILS_H

#include "MyString.h"
#include "Types.h"

bool ReadNamesFromListFile(CFSTR fileName, UStringVector &strings, UINT codePage = CP_OEMCP);

#endif
