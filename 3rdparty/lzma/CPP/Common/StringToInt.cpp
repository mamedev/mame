// Common/StringToInt.cpp

#include "StdAfx.h"

#include "StringToInt.h"

UInt64 ConvertStringToUInt64(const char *s, const char **end)
{
  UInt64 result = 0;
  for (;;)
  {
    char c = *s;
    if (c < '0' || c > '9')
    {
      if (end != NULL)
        *end = s;
      return result;
    }
    result *= 10;
    result += (c - '0');
    s++;
  }
}

UInt64 ConvertOctStringToUInt64(const char *s, const char **end)
{
  UInt64 result = 0;
  for (;;)
  {
    char c = *s;
    if (c < '0' || c > '7')
    {
      if (end != NULL)
        *end = s;
      return result;
    }
    result <<= 3;
    result += (c - '0');
    s++;
  }
}

UInt64 ConvertHexStringToUInt64(const char *s, const char **end)
{
  UInt64 result = 0;
  for (;;)
  {
    char c = *s;
    UInt32 v;
    if (c >= '0' && c <= '9') v = (c - '0');
    else if (c >= 'A' && c <= 'F') v = 10 + (c - 'A');
    else if (c >= 'a' && c <= 'f') v = 10 + (c - 'a');
    else
    {
      if (end != NULL)
        *end = s;
      return result;
    }
    result <<= 4;
    result |= v;
    s++;
  }
}


UInt64 ConvertStringToUInt64(const wchar_t *s, const wchar_t **end)
{
  UInt64 result = 0;
  for (;;)
  {
    wchar_t c = *s;
    if (c < '0' || c > '9')
    {
      if (end != NULL)
        *end = s;
      return result;
    }
    result *= 10;
    result += (c - '0');
    s++;
  }
}


Int64 ConvertStringToInt64(const char *s, const char **end)
{
  if (*s == '-')
    return -(Int64)ConvertStringToUInt64(s + 1, end);
  return ConvertStringToUInt64(s, end);
}

Int64 ConvertStringToInt64(const wchar_t *s, const wchar_t **end)
{
  if (*s == L'-')
    return -(Int64)ConvertStringToUInt64(s + 1, end);
  return ConvertStringToUInt64(s, end);
}
