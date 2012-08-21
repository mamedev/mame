/***************************************************************************

    mess.c

    Specific (per target) constants

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************/
#include "emu.h"

#define APPNAME					"MESS"
#define APPNAME_LOWER			"mess"
#define CONFIGNAME				"mess"
#define APPLONGNAME				"M.E.S.S."
#define FULLLONGNAME			"Multi Emulator Super System"
#define CAPGAMENOUN				"SYSTEM"
#define CAPSTARTGAMENOUN		"System"
#define GAMENOUN				"system"
#define GAMESNOUN				"systems"
#define COPYRIGHT				"Copyright the MESS team\nhttp://mess.org"
#define COPYRIGHT_INFO			"Copyright the MESS team\n\n" \
								"MESS is based on MAME Source code\n" \
								"Copyright Nicola Salmoria and the MAME team"
#define DISCLAIMER				"MESS is an emulator: it reproduces, more or less faithfully, the behaviour of\n"\
								"several computer and console systems. But hardware is useless without software\n" \
								"so a file dump of the ROM, cartridges, discs, and cassettes which run on that\n" \
								"hardware is required. Such files, like any other commercial software, are\n" \
								"copyrighted material and it is therefore illegal to use them if you don't own\n" \
								"the original media from which the files are derived. Needless to say, these\n" \
								"files are not distributed together with MESS. Distribution of MESS together\n" \
								"with these files is a violation of copyright law and should be promptly\n" \
								"reported to the authors so that appropriate legal action can be taken.\n"
#define USAGE					"Usage:  %s [%s] [media] [software] [options]"
#define XML_ROOT			    "mess"
#define XML_TOP 				"machine"
#define STATE_MAGIC_NUM			"MESSSAVE"

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
void emulator_info::printf_usage(const char *par1, const char *par2) { mame_printf_info(USAGE, par1, par2); }
