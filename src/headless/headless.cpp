/**
 * Headless Mame
 */

#include "emu.h"

#define APPNAME                 "HMAME"
#define APPNAME_LOWER           "hmame"
#define CONFIGNAME              "hmame"
#define COPYRIGHT               "none"
#define COPYRIGHT_INFO          "none"

const char * emulator_info::get_appname() { return APPNAME;}
const char * emulator_info::get_appname_lower() { return APPNAME_LOWER;}
const char * emulator_info::get_configname() { return CONFIGNAME;}
const char * emulator_info::get_copyright() { return COPYRIGHT;}
const char * emulator_info::get_copyright_info() { return COPYRIGHT_INFO;}
