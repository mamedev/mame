// Common/UTFConvert.h

#ifndef __COMMON_UTFCONVERT_H
#define __COMMON_UTFCONVERT_H

#include "MyString.h"

bool ConvertUTF8ToUnicode(const AString &utfString, UString &resultString);
bool ConvertUnicodeToUTF8(const UString &unicodeString, AString &resultString);

#endif
