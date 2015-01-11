// Common/StringToInt.h

#ifndef __COMMON_STRING_TO_INT_H
#define __COMMON_STRING_TO_INT_H

#include "Types.h"

UInt64 ConvertStringToUInt64(const char *s, const char **end);
UInt64 ConvertOctStringToUInt64(const char *s, const char **end);
UInt64 ConvertHexStringToUInt64(const char *s, const char **end);
UInt64 ConvertStringToUInt64(const wchar_t *s, const wchar_t **end);

Int64 ConvertStringToInt64(const char *s, const char **end);
Int64 ConvertStringToInt64(const wchar_t *s, const wchar_t **end);

#endif
