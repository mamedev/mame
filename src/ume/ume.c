/***************************************************************************

    ume.c

    Specific (per target) constants

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************/
#include "emu.h"

#define APPNAME					"UME"
#define APPNAME_LOWER			"ume"
#define CONFIGNAME				"ume"
#define APPLONGNAME				"U.M.E."
#define FULLLONGNAME			"Universal Machine Emulator"
#define CAPGAMENOUN				"GAME"
#define CAPSTARTGAMENOUN		"Game"
#define GAMENOUN				"game"
#define GAMESNOUN				"games"
#define COPYRIGHT				"Copyright Nicola Salmoria\nand the MAME and MESS teams"
#define COPYRIGHT_INFO			"Copyright Nicola Salmoria and the MAME and MESS teams"
#define DISCLAIMER				"This is promo build of combined MAME and MESS emulators made in order to\n" \
								"promote both projects and show possibilities to new potential users.\n\n" \
								"Note that bugs noticed in this build should not be forwarded to development\n"\
								"teams unless those are confirmed in original builds as well.\n"
#define USAGE					"Usage:  %s [%s] [media] [software] [options]"
#define XML_ROOT			    "mame"
#define XML_TOP 				"game"
#define STATE_MAGIC_NUM			"MAMESAVE"

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
