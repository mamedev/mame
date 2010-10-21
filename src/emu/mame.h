/***************************************************************************

    mame.h

    Controls execution of the core MAME system.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __MAME_H__
#define __MAME_H__

#include <time.h>



//**************************************************************************
//    CONSTANTS
//**************************************************************************

// return values from run_game
enum
{
	MAMERR_NONE				= 0,	/* no error */
	MAMERR_FAILED_VALIDITY	= 1,	/* failed validity checks */
	MAMERR_MISSING_FILES	= 2,	/* missing files */
	MAMERR_FATALERROR		= 3,	/* some other fatal error */
	MAMERR_DEVICE			= 4,	/* device initialization error (MESS-specific) */
	MAMERR_NO_SUCH_GAME		= 5,	/* game was specified but doesn't exist */
	MAMERR_INVALID_CONFIG	= 6,	/* some sort of error in configuration */
	MAMERR_IDENT_NONROMS	= 7,	/* identified all non-ROM files */
	MAMERR_IDENT_PARTIAL	= 8,	/* identified some files but not all */
	MAMERR_IDENT_NONE		= 9		/* identified no files */
};


// MESS vs. MAME abstractions
#ifndef MESS
#define APPNAME					"MAME"
#define APPNAME_LOWER			"mame"
#define CONFIGNAME				"mame"
#define APPLONGNAME				"M.A.M.E."
#define CAPGAMENOUN				"GAME"
#define CAPSTARTGAMENOUN		"Game"
#define GAMENOUN				"game"
#define GAMESNOUN				"games"
#define HISTORYNAME				"History"
#define COPYRIGHT				"Copyright Nicola Salmoria\nand the MAME team\nhttp://mamedev.org"
#else
#define APPNAME					"MESS"
#define APPNAME_LOWER			"mess"
#define CONFIGNAME				"mess"
#define APPLONGNAME				"M.E.S.S."
#define CAPGAMENOUN				"SYSTEM"
#define CAPSTARTGAMENOUN		"System"
#define GAMENOUN				"system"
#define GAMESNOUN				"systems"
#define HISTORYNAME				"System Info"
#define COPYRIGHT				"Copyright the MESS team\nhttp://mess.org"
#endif



//**************************************************************************
//    TYPE DEFINITIONS
//**************************************************************************

// output channel callback
typedef void (*output_callback_func)(void *param, const char *format, va_list argptr);



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

extern const char mame_disclaimer[];

extern const char build_version[];



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- core system management ----- */

/* execute as configured by the OPTION_GAMENAME option on the specified options */
int mame_execute(osd_interface &osd, core_options *options);

/* accesses the core_options for the currently running emulation */
core_options *mame_options(void);

/* set mame options, used by validate option */
void set_mame_options(core_options *options);

/* return true if the given machine is valid */
int mame_is_valid_machine(running_machine *machine);



/* ----- output management ----- */

/* set the output handler for a channel, returns the current one */
void mame_set_output_channel(output_channel channel, output_callback_func callback, void *param, output_callback_func *prevcb, void **prevparam);

/* built-in default callbacks */
void mame_file_output_callback(void *param, const char *format, va_list argptr);
void mame_null_output_callback(void *param, const char *format, va_list argptr);

/* calls to be used by the code */
void mame_printf_error(const char *format, ...) ATTR_PRINTF(1,2);
void mame_printf_warning(const char *format, ...) ATTR_PRINTF(1,2);
void mame_printf_info(const char *format, ...) ATTR_PRINTF(1,2);
void mame_printf_verbose(const char *format, ...) ATTR_PRINTF(1,2);
void mame_printf_debug(const char *format, ...) ATTR_PRINTF(1,2);

/* discourage the use of printf directly */
/* sadly, can't do this because of the ATTR_PRINTF under GCC */
/*
#undef printf
#define printf !MUST_USE_MAME_PRINTF_*_CALLS_WITHIN_THE_CORE!
*/


/* ----- miscellaneous bits & pieces ----- */

/* parse the configured INI files */
void mame_parse_ini_files(core_options *options, const game_driver *driver);


// pop-up a user visible message
void CLIB_DECL popmessage(const char *format,...) ATTR_PRINTF(1,2);

// log to the standard error.log file
void CLIB_DECL logerror(const char *format,...) ATTR_PRINTF(1,2);


#endif	/* __MAME_H__ */
