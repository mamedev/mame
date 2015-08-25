/**
 * \file   os_getversioninfo.c
 * \brief  Retrieve operating system version information.
 * \author Copyright (c) 2011-2012 Jason Perkins and the Premake project
 */

#include "premake.h"
#include <stdlib.h>
#include <string.h> // OSX memset

struct OsVersionInfo
{
	int majorversion;
	int minorversion;
	int revision;
	const char* description;
	int isalloc;
};

static void getversion(struct OsVersionInfo* info);


int os_getversion(lua_State* L)
{
	struct OsVersionInfo info;
	memset(&info, 0, sizeof(info));
	getversion(&info);

	lua_newtable(L);

	lua_pushstring(L, "majorversion");
	lua_pushnumber(L, info.majorversion);
	lua_settable(L, -3);

	lua_pushstring(L, "minorversion");
	lua_pushnumber(L, info.minorversion);
	lua_settable(L, -3);

	lua_pushstring(L, "revision");
	lua_pushnumber(L, info.revision);
	lua_settable(L, -3);

	lua_pushstring(L, "description");
	lua_pushstring(L, info.description);
	lua_settable(L, -3);

	if (info.isalloc) {
		free((void*)info.description);
	}

	return 1;
}

/*************************************************************/

#if defined(PLATFORM_WINDOWS)

#if !defined(VER_SUITE_WH_SERVER)
#define VER_SUITE_WH_SERVER   (0x00008000)
#endif

#ifndef SM_SERVERR2
#	define SM_SERVERR2 89
#endif

SYSTEM_INFO getsysteminfo()
{
	typedef void (WINAPI *GetNativeSystemInfoSig)(LPSYSTEM_INFO);
	GetNativeSystemInfoSig nativeSystemInfo = (GetNativeSystemInfoSig)
	GetProcAddress(GetModuleHandle(TEXT("kernel32")), "GetNativeSystemInfo");

	SYSTEM_INFO systemInfo;
	memset(&systemInfo, 0, sizeof(systemInfo));
	if ( nativeSystemInfo ) nativeSystemInfo(&systemInfo);
	else GetSystemInfo(&systemInfo);
	return systemInfo;
}

void getversion(struct OsVersionInfo* info)
{
	OSVERSIONINFOEX versionInfo;
	memset(&versionInfo, 0, sizeof(versionInfo));

	versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetVersionEx((OSVERSIONINFO*)&versionInfo);

	info->majorversion = versionInfo.dwMajorVersion;
	info->minorversion = versionInfo.dwMinorVersion;
	info->revision = versionInfo.wServicePackMajor;

	if (versionInfo.dwMajorVersion == 5 && versionInfo.dwMinorVersion == 0)
	{
		info->description = "Windows 2000";
	}
	else if (versionInfo.dwMajorVersion == 5 && versionInfo.dwMinorVersion == 1)
	{
		info->description = "Windows XP";
	}
	else if (versionInfo.dwMajorVersion == 5 && versionInfo.dwMinorVersion == 2)
	{
		SYSTEM_INFO systemInfo = getsysteminfo();
		if (versionInfo.wProductType == VER_NT_WORKSTATION &&
			systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
		{
			info->description = "Windows XP Professional x64";
		}
		else if (versionInfo.wSuiteMask & VER_SUITE_WH_SERVER)
		{
			info->description = "Windows Home Server";
		}
		else if (GetSystemMetrics(SM_SERVERR2) == 0)
		{
			info->description = "Windows Server 2003";
		}
		else
		{
			info->description = "Windows Server 2003 R2";
		}
	}
	else if (versionInfo.dwMajorVersion == 6 && versionInfo.dwMinorVersion == 0)
	{
		if (versionInfo.wProductType == VER_NT_WORKSTATION)
		{
			info->description = "Windows Vista";
		}
		else
		{
			info->description = "Windows Server 2008";
		}
	}
	else if (versionInfo.dwMajorVersion == 6 && versionInfo.dwMinorVersion == 1 )
	{
		if (versionInfo.wProductType != VER_NT_WORKSTATION)
		{
			info->description = "Windows Server 2008 R2";
		}
		else
		{
			info->description = "Windows 7";
		}
	}
	else
	{
		info->description = "Windows";
	}
}

/*************************************************************/

#elif defined(PLATFORM_MACOSX)

#include <CoreServices/CoreServices.h>

void getversion(struct OsVersionInfo* info)
{
	SInt32 majorversion, minorversion, bugfix;
	Gestalt(gestaltSystemVersionMajor, &majorversion);
	Gestalt(gestaltSystemVersionMinor, &minorversion);
	Gestalt(gestaltSystemVersionBugFix, &bugfix);

	info->majorversion = majorversion;
	info->minorversion = minorversion;
	info->revision = bugfix;

	info->description = "Mac OS X";
	if (info->majorversion == 10)
	{
		switch (info->minorversion)
		{
		case 4:
			info->description = "Mac OS X Tiger";
			break;
		case 5:
			info->description = "Mac OS X Leopard";
			break;
		case 6:
			info->description = "Mac OS X Snow Leopard";
			break;
		case 7:
			info->description = "Mac OS X Lion";
			break;
		}
	}
}

/*************************************************************/

#elif defined(PLATFORM_BSD) || defined(PLATFORM_LINUX) || defined(PLATFORM_SOLARIS)

#include <string.h>
#include <sys/utsname.h>

void getversion(struct OsVersionInfo* info)
{
	struct utsname u;
	char* ver;

	info->majorversion = 0;
	info->minorversion = 0;
	info->revision = 0;

	if (uname(&u))
	{
		// error
		info->description = PLATFORM_STRING;
		return;
	}

#if __GLIBC__
	// When using glibc, info->description gets set to u.sysname,
	// but it isn't passed out of this function, so we need to copy 
	// the string.
	info->description = malloc(strlen(u.sysname) + 1);
	strcpy((char*)info->description, u.sysname);
	info->isalloc = 1;
#else
	info->description = u.sysname;
#endif

	if ((ver = strtok(u.release, ".-")) != NULL)
	{
		info->majorversion = atoi(ver);
		// continue parsing from the previous position
		if ((ver = strtok(NULL, ".-")) != NULL)
		{
			info->minorversion = atoi(ver);
			if ((ver = strtok(NULL, ".-")) != NULL)
				info->revision = atoi(ver);
		}
	}
}

/*************************************************************/

#elif defined(PLATFORM_OS2)

#define INCL_DOS
#include <os2.h>

void getversion(struct OsVersionInfo* info)
{
	ULONG ulMajor;
	ULONG ulMinor;
	ULONG ulRev;

	DosQuerySysInfo(QSV_VERSION_MAJOR, QSV_VERSION_MAJOR, &ulMajor, sizeof(ulMajor));
	DosQuerySysInfo(QSV_VERSION_MINOR, QSV_VERSION_MINOR, &ulMinor, sizeof(ulMinor));
	DosQuerySysInfo(QSV_VERSION_REVISION, QSV_VERSION_REVISION,
					&ulRev, sizeof(ulRev));

	info->majorversion = ulMajor;
	info->minorversion = ulMinor;
	info->revision = ulRev;
	info->description = PLATFORM_STRING;
}

/*************************************************************/

#else

void getversion(struct OsVersionInfo* info)
{
	info->majorversion = 0;
	info->minorversion = 0;
	info->revision = 0;
	info->description = PLATFORM_STRING;
}

#endif

