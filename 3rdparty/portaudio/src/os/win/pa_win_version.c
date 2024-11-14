/*
 * $Id$
 * Portable Audio I/O Library
 * Win32 platform-specific support functions
 *
 * Based on the Open Source API proposed by Ross Bencina
 * Copyright (c) 1999-2008 Ross Bencina
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * The text above constitutes the entire PortAudio license; however,
 * the PortAudio community also makes the following non-binding requests:
 *
 * Any person wishing to distribute modifications to the Software is
 * requested to send the modifications to the original developer so that
 * they can be incorporated into the canonical version. It is also
 * requested that these non-binding requests be included along with the
 * license above.
 */

/** @file
 @ingroup win_src

 @brief Portable implementation of Windows OS version getter.
*/

#include <windows.h>

#include "pa_win_version.h"
#include "pa_debugprint.h"

// WinRT
#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)
    #define PA_WINRT
#endif

// Alternative way for checking Windows version, allows to check version of Windows 8.1 and up
#ifndef PA_WINRT
static BOOL IsWindowsVersionOrGreater(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor)
{
    typedef ULONGLONG (NTAPI *LPFN_VERSETCONDITIONMASK)(ULONGLONG ConditionMask, DWORD TypeMask, BYTE Condition);
    typedef BOOL (WINAPI *LPFN_VERIFYVERSIONINFO)(LPOSVERSIONINFOEXA lpVersionInformation, DWORD dwTypeMask, DWORDLONG dwlConditionMask);

    LPFN_VERSETCONDITIONMASK fnVerSetConditionMask;
    LPFN_VERIFYVERSIONINFO fnVerifyVersionInfo;
    OSVERSIONINFOEXA osvi = { sizeof(osvi), 0, 0, 0, 0, {0}, 0, 0 };
    DWORDLONG dwlConditionMask;

    fnVerSetConditionMask = (LPFN_VERSETCONDITIONMASK)GetProcAddress(GetModuleHandleA("kernel32"), "VerSetConditionMask");
    fnVerifyVersionInfo = (LPFN_VERIFYVERSIONINFO)GetProcAddress(GetModuleHandleA("kernel32"), "VerifyVersionInfoA");

    if ((fnVerSetConditionMask == NULL) || (fnVerifyVersionInfo == NULL))
        return FALSE;

    dwlConditionMask = fnVerSetConditionMask(
        fnVerSetConditionMask(
            fnVerSetConditionMask(
                0, VER_MAJORVERSION,     VER_GREATER_EQUAL),
                   VER_MINORVERSION,     VER_GREATER_EQUAL),
                   VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);

    osvi.dwMajorVersion    = wMajorVersion;
    osvi.dwMinorVersion    = wMinorVersion;
    osvi.wServicePackMajor = wServicePackMajor;

    return (fnVerifyVersionInfo(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlConditionMask) != FALSE);
}
#endif

static PaOsVersion GetOsVersion()
{
    PaOsVersion version = paOsVersionWindowsUnknown;

#ifndef PA_WINRT
    DWORD dwMajorVersion = 0xFFFFFFFFU, dwMinorVersion = 0, dwPlatformId = 0, dwBuild = 0;

    // Can be missing in some MinGW distributions
#ifndef NT_SUCCESS
    typedef LONG NTSTATUS;
#endif

#ifndef VER_PLATFORM_WIN32_NT
    #define VER_PLATFORM_WIN32_NT 2
#endif

    // RTL_OSVERSIONINFOW equals OSVERSIONINFOW but it is missing inb MinGW winnt.h header,
    // thus use OSVERSIONINFOW for greater portability
    typedef NTSTATUS (WINAPI *LPFN_RTLGETVERSION)(POSVERSIONINFOW lpVersionInformation);
    LPFN_RTLGETVERSION fnRtlGetVersion;

    #define NTSTATUS_SUCCESS ((NTSTATUS)0x00000000L)

    // RtlGetVersion is able to provide true Windows version (Windows 10 may be reported as Windows 8
    // by GetVersion API) unless Windows is running app in Compatibility mode when shim is replacing
    // real Windows version (https://techcommunity.microsoft.com/t5/ask-the-performance-team/demystifying-shims-or-using-the-app-compat-toolkit-to-make-your/ba-p/374947).
    // In our case we obey Compatibility mode and do not try to bypass it.
    if ((fnRtlGetVersion = (LPFN_RTLGETVERSION)GetProcAddress(GetModuleHandleA("ntdll"), "RtlGetVersion")) != NULL)
    {
        OSVERSIONINFOW ver = { sizeof(OSVERSIONINFOW), 0, 0, 0, 0, {0} };

        if (fnRtlGetVersion(&ver) == NTSTATUS_SUCCESS)
        {
            dwMajorVersion = ver.dwMajorVersion;
            dwMinorVersion = ver.dwMinorVersion;
            dwPlatformId   = ver.dwPlatformId;
            dwBuild        = ver.dwBuildNumber;
        }

        PA_DEBUG(("getting Windows version with RtlGetVersion(): major=%d, minor=%d, build=%d, platform=%d\n",
            dwMajorVersion, dwMinorVersion, dwBuild, dwPlatformId));
    }

    #undef NTSTATUS_SUCCESS

    // Fallback to GetVersion if RtlGetVersion is missing
    if (dwMajorVersion == 0xFFFFFFFFU)
    {
        typedef DWORD (WINAPI *LPFN_GETVERSION)(VOID);
        LPFN_GETVERSION fnGetVersion;

        if ((fnGetVersion = (LPFN_GETVERSION)GetProcAddress(GetModuleHandleA("kernel32"), "GetVersion")) != NULL)
        {
            DWORD dwVersion = fnGetVersion();

            dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
            dwMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));
            dwPlatformId   = VER_PLATFORM_WIN32_NT;

            if (dwVersion < 0x80000000)
                dwBuild = (DWORD)(HIWORD(dwVersion));

            PA_DEBUG(("getting Windows version with GetVersion(): major=%d, minor=%d, build=%d, platform=%d\n",
                dwMajorVersion, dwMinorVersion, dwBuild, dwPlatformId));
        }
    }

    // Version numbers reference:
    // https://learn.microsoft.com/en-us/windows/win32/sysinfo/operating-system-version
    // https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-osversioninfoexa
    if (dwMajorVersion != 0xFFFFFFFFU)
    {
        switch (dwMajorVersion)
        {
        case 0:
        case 1:
        case 2:
        case 3:
            break; // skip lower
        case 4:
            version = (dwPlatformId == VER_PLATFORM_WIN32_NT ? paOsVersionWindowsNT4 : paOsVersionWindows9x);
            break;
        case 5:
            switch (dwMinorVersion)
            {
            case 0:  version = paOsVersionWindows2000;            break;
            case 1:  version = paOsVersionWindowsXP;              break;
            case 2:  version = paOsVersionWindowsXPServer2003;    break;
            default: version = paOsVersionWindowsXPServer2003;    break; // shall not happen
            }
            break;
        case 6:
            switch (dwMinorVersion)
            {
            case 0:  version = paOsVersionWindowsVistaServer2008; break;
            case 1:  version = paOsVersionWindows7Server2008R2;   break;
            case 2:  version = paOsVersionWindows8Server2012;     break;
            case 3:  version = paOsVersionWindows8_1Server2012R2; break;
            default: version = paOsVersionWindows8_1Server2012R2; break; // shall not happen
            }
            break;
        case 10:
            // note: Windows 11 can be detected by dwBuild >= 22000
            version = paOsVersionWindows10Server2016;
            break;
        default:
            version = paOsVersionWindowsFuture;
            break;
        }
    }
    // Fallback to VerifyVersionInfo if RtlGetVersion and GetVersion are missing
    else
    {
        PA_DEBUG(("getting Windows version with VerifyVersionInfo()\n"));

        if (IsWindowsVersionOrGreater(10, 0, 0))
            version = paOsVersionWindows10Server2016;
        else
        if (IsWindowsVersionOrGreater(6, 3, 0))
            version = paOsVersionWindows8_1Server2012R2;
        else
        if (IsWindowsVersionOrGreater(6, 2, 0))
            version = paOsVersionWindows8Server2012;
        else
        if (IsWindowsVersionOrGreater(6, 1, 0))
            version = paOsVersionWindows7Server2008R2;
        else
        if (IsWindowsVersionOrGreater(6, 0, 0))
            version = paOsVersionWindowsVistaServer2008;
        else
            version = paOsVersionWindowsFuture;
    }
#else
    #if (_WIN32_WINNT >= _WIN32_WINNT_WIN10)
        version = paOsVersionWindows10Server2016;
    #else
        version = paOsVersionWindows8Server2012;
    #endif
#endif

    PA_DEBUG(("Windows version=%d\n", version));

    return version;
}

PaOsVersion PaWinUtil_GetOsVersion()
{
    static PaOsVersion version = paOsVersionWindowsUnknown;

    if (version == paOsVersionWindowsUnknown)
        version = GetOsVersion();

    return version;
}
