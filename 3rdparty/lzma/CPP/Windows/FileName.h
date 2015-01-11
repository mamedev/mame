// Windows/FileName.h

#ifndef __WINDOWS_FILE_NAME_H
#define __WINDOWS_FILE_NAME_H

#include "../Common/MyString.h"

namespace NWindows {
namespace NFile {
namespace NName {

void NormalizeDirPathPrefix(FString &dirPath); // ensures that it ended with '\\', if dirPath is not epmty
void NormalizeDirPathPrefix(UString &dirPath);

}}}

#endif
