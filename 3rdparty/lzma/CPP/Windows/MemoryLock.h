// Windows/MemoryLock.h

#ifndef __WINDOWS_MEMORYLOCK_H
#define __WINDOWS_MEMORYLOCK_H

namespace NWindows {
namespace NSecurity {

#ifndef UNDER_CE
bool EnableLockMemoryPrivilege(bool enable = true);
#endif

}}

#endif
