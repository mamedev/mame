// Windows/PropVariantConversions.h

#ifndef __PROP_VARIANT_CONVERSIONS_H
#define __PROP_VARIANT_CONVERSIONS_H

#include "Common/MyString.h"
#include "Common/Types.h"

bool ConvertFileTimeToString(const FILETIME &ft, char *s, bool includeTime = true, bool includeSeconds = true);
UString ConvertFileTimeToString(const FILETIME &ft, bool includeTime = true, bool includeSeconds = true);
UString ConvertPropVariantToString(const PROPVARIANT &prop);
UInt64 ConvertPropVariantToUInt64(const PROPVARIANT &prop);

#endif
