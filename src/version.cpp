// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    version.c

    Version string source file for MAME.

***************************************************************************/

#define BARE_BUILD_VERSION "0.174"

extern const char bare_build_version[];
extern const char build_version[];
const char bare_build_version[] = BARE_BUILD_VERSION;
#if defined(GIT_VERSION)
#define VERSION_TO_STRING(s) XVERSION_TO_STRING(s)
#define XVERSION_TO_STRING(ver) #ver
const char build_version[] = BARE_BUILD_VERSION " (" VERSION_TO_STRING(GIT_VERSION) ")";
#else
const char build_version[] = BARE_BUILD_VERSION;
#endif
