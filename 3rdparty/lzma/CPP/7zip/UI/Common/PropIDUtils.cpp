// PropIDUtils.cpp

#include "StdAfx.h"

#include "Common/IntToString.h"

#include "Windows/FileFind.h"
#include "Windows/PropVariantConversions.h"

#include "../../PropID.h"

#include "PropIDUtils.h"

using namespace NWindows;

void ConvertUInt32ToHex(UInt32 value, wchar_t *s)
{
  for (int i = 0; i < 8; i++)
  {
    int t = value & 0xF;
    value >>= 4;
    s[7 - i] = (wchar_t)((t < 10) ? (L'0' + t) : (L'A' + (t - 10)));
  }
  s[8] = L'\0';
}

static const char g_WinAttrib[17] = "RHS8DAdNTsrCOnE_";
/*
0 READONLY
1 HIDDEN
3 SYSTEM

4 DIRECTORY
5 ARCHIVE
6 DEVICE
7 NORMAL
8 TEMPORARY
9 SPARSE_FILE
10 REPARSE_POINT
11 COMPRESSED
12 OFFLINE
13 NOT_CONTENT_INDEXED
14 ENCRYPTED

16 VIRTUAL
*/

static const char kPosixTypes[16] = { '0', 'p', 'c', '3', 'd', '5', 'b', '7', '-', '9', 'l', 'B', 's', 'D', 'E', 'F' };
#define MY_ATTR_CHAR(a, n, c) ((a )& (1 << (n))) ? c : L'-';

UString ConvertPropertyToString(const PROPVARIANT &prop, PROPID propID, bool full)
{
  switch(propID)
  {
    case kpidCTime:
    case kpidATime:
    case kpidMTime:
    {
      if (prop.vt != VT_FILETIME)
        break;
      FILETIME localFileTime;
      if ((prop.filetime.dwHighDateTime == 0 &&
          prop.filetime.dwLowDateTime == 0) ||
          !::FileTimeToLocalFileTime(&prop.filetime, &localFileTime))
        return UString();
      return ConvertFileTimeToString(localFileTime, true, full);
    }
    case kpidCRC:
    {
      if (prop.vt != VT_UI4)
        break;
      wchar_t temp[12];
      ConvertUInt32ToHex(prop.ulVal, temp);
      return temp;
    }
    case kpidAttrib:
    {
      if (prop.vt != VT_UI4)
        break;
      UInt32 a = prop.ulVal;
      wchar_t sz[32];
      int pos = 0;
      for (int i = 0; i < 16; i++)
        if (a & (1 << i) && i != 7)
          sz[pos++] = g_WinAttrib[i];
      sz[pos] = '\0';
      return sz;
    }
    case kpidPosixAttrib:
    {
      if (prop.vt != VT_UI4)
        break;
      UString res;
      UInt32 a = prop.ulVal;
      wchar_t temp[16];

      temp[0] = kPosixTypes[(a >> 12) & 0xF];
      for (int i = 6; i >= 0; i -= 3)
      {
        temp[7 - i] = MY_ATTR_CHAR(a, i + 2, L'r');
        temp[8 - i] = MY_ATTR_CHAR(a, i + 1, L'w');
        temp[9 - i] = MY_ATTR_CHAR(a, i + 0, L'x');
      }
      if ((a & 0x800) != 0) temp[3] = ((a & (1 << 6)) ? 's' : 'S');
      if ((a & 0x400) != 0) temp[6] = ((a & (1 << 3)) ? 's' : 'S');
      if ((a & 0x200) != 0) temp[9] = ((a & (1 << 0)) ? 't' : 'T');
      temp[10] = 0;
      res = temp;

      a &= ~(UInt32)0xFFFF;
      if (a != 0)
      {
        ConvertUInt32ToHex(a, temp);
        res = UString(temp) + L' ' + res;
      }
      return res;
    }
  }
  return ConvertPropVariantToString(prop);
}
