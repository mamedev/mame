// Windows/Error.h

#ifndef __WINDOWS_ERROR_H
#define __WINDOWS_ERROR_H

#include "Common/MyString.h"

namespace NWindows {
namespace NError {

UString MyFormatMessageW(DWORD errorCode);

}}

#endif
