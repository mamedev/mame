// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    version.c

    Version string source file for MAME.

***************************************************************************/

#define BARE_BUILD_VERSION "0.172"

extern const char bare_build_version[];
extern const char build_version[];
const char bare_build_version[] = BARE_BUILD_VERSION;
const char build_version[] = BARE_BUILD_VERSION " (" __DATE__")";
