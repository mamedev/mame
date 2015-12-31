// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    mess.c

    Specific (per target) constants

****************************************************************************/

#include "emu.h"

#define APPNAME                 "MESS"
#define APPNAME_LOWER           "mess"
#define CONFIGNAME              "mess"
#define APPLONGNAME             "M.E.S.S."
#define FULLLONGNAME            "Multi Emulator Super System"
#define CAPGAMENOUN             "MACHINE"
#define CAPSTARTGAMENOUN        "Machine"
#define GAMENOUN                "machine"
#define GAMESNOUN               "machines"
#define COPYRIGHT               "Copyright Nicola Salmoria\nand the MAME team\nhttp://mamedev.org"
#define COPYRIGHT_INFO          "Copyright Nicola Salmoria and the MAME team"
#define DISCLAIMER              "This software reproduces, more or less faithfully, the behaviour of a wide range\n" \
								"of machines. But hardware is useless without software, so images of the ROMs and\n" \
								"other media which run on that hardware are also required.\n"
#define USAGE                   "Usage:  %s [%s] [media] [software] [options]"
#define XML_ROOT                "mame"
#define XML_TOP                 "machine"
#define STATE_MAGIC_NUM         "MAMESAVE"

const char * emulator_info::get_appname() { return APPNAME;}
const char * emulator_info::get_appname_lower() { return APPNAME_LOWER;}
const char * emulator_info::get_configname() { return CONFIGNAME;}
const char * emulator_info::get_applongname() { return APPLONGNAME;}
const char * emulator_info::get_fulllongname() { return FULLLONGNAME;}
const char * emulator_info::get_capgamenoun() { return CAPGAMENOUN;}
const char * emulator_info::get_capstartgamenoun() { return CAPSTARTGAMENOUN;}
const char * emulator_info::get_gamenoun() { return GAMENOUN;}
const char * emulator_info::get_gamesnoun() { return GAMESNOUN;}
const char * emulator_info::get_copyright() { return COPYRIGHT;}
const char * emulator_info::get_copyright_info() { return COPYRIGHT_INFO;}
const char * emulator_info::get_disclaimer() { return DISCLAIMER;}
const char * emulator_info::get_usage() { return USAGE;}
const char * emulator_info::get_xml_root() { return XML_ROOT;}
const char * emulator_info::get_xml_top() { return XML_TOP;}
const char * emulator_info::get_state_magic_num() { return STATE_MAGIC_NUM;}
void emulator_info::printf_usage(const char *par1, const char *par2) { osd_printf_info(USAGE, par1, par2); }
