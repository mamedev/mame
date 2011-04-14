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
#define FULLLONGNAME			"Multiple Arcade Machine Emulator"
#define CAPGAMENOUN				"GAME"
#define CAPSTARTGAMENOUN		"Game"
#define GAMENOUN				"game"
#define GAMESNOUN				"games"
#define COPYRIGHT				"Copyright Nicola Salmoria\nand the MAME team\nhttp://mamedev.org"
#define COPYRIGHT_INFO			"Copyright Nicola Salmoria and the MAME team"
#define DISCLAIMER				"MAME is an emulator: it reproduces, more or less faithfully, the behaviour of\n" \
								"several arcade machines. But hardware is useless without software, so an image\n" \
								"of the ROMs which run on that hardware is required. Such ROMs, like any other\n" \
								"commercial software, are copyrighted material and it is therefore illegal to\n" \
								"use them if you don't own the original arcade machine. Needless to say, ROMs\n" \
								"are not distributed together with MAME. Distribution of MAME together with ROM\n" \
								"images is a violation of copyright law and should be promptly reported to the\n" \
								"authors so that appropriate legal action can be taken.\n"
#define USAGE					"Usage:  %s [%s] [options]"
#define XML_ROOT 			    "mame"
#define XML_TOP 				"game"
#define STATE_MAGIC_NUM			'M', 'A', 'M', 'E', 'S', 'A', 'V', 'E'
#else
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
#define XML_ROOT 			    "mess"
#define XML_TOP 				"machine"	
#define STATE_MAGIC_NUM			'M', 'E', 'S', 'S', 'S', 'A', 'V', 'E'
#endif



//**************************************************************************
//    TYPE DEFINITIONS
//**************************************************************************

// output channel callback
typedef void (*output_callback_func)(void *param, const char *format, va_list argptr);



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

extern const char build_version[];



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- core system management ----- */

/* execute as configured by the OPTION_SYSTEMNAME option on the specified options */
int mame_execute(emu_options &options, osd_interface &osd);

/* return true if the given machine is valid */
int mame_is_valid_machine(running_machine &machine);



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

// pop-up a user visible message
void CLIB_DECL popmessage(const char *format,...) ATTR_PRINTF(1,2);

// log to the standard error.log file
void CLIB_DECL logerror(const char *format,...) ATTR_PRINTF(1,2);


#endif	/* __MAME_H__ */
