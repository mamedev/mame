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
	MAMERR_NONE             = 0,    /* no error */
	MAMERR_FAILED_VALIDITY  = 1,    /* failed validity checks */
	MAMERR_MISSING_FILES    = 2,    /* missing files */
	MAMERR_FATALERROR       = 3,    /* some other fatal error */
	MAMERR_DEVICE           = 4,    /* device initialization error (MESS-specific) */
	MAMERR_NO_SUCH_GAME     = 5,    /* game was specified but doesn't exist */
	MAMERR_INVALID_CONFIG   = 6,    /* some sort of error in configuration */
	MAMERR_IDENT_NONROMS    = 7,    /* identified all non-ROM files */
	MAMERR_IDENT_PARTIAL    = 8,    /* identified some files but not all */
	MAMERR_IDENT_NONE       = 9     /* identified no files */
};




//**************************************************************************
//    TYPE DEFINITIONS
//**************************************************************************

// output channel callback
typedef delegate<void (const char *, va_list)> output_delegate;

class emulator_info
{
public:
	// construction/destruction
	emulator_info() {};

	static const char * get_appname();
	static const char * get_appname_lower();
	static const char * get_configname();
	static const char * get_applongname();
	static const char * get_fulllongname();
	static const char * get_capgamenoun();
	static const char * get_capstartgamenoun();
	static const char * get_gamenoun();
	static const char * get_gamesnoun();
	static const char * get_copyright();
	static const char * get_copyright_info();
	static const char * get_disclaimer();
	static const char * get_usage();
	static const char * get_xml_root();
	static const char * get_xml_top();
	static const char * get_state_magic_num();
	static void printf_usage(const char *par1, const char *par2);
};



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



/* ----- output management ----- */

/* set the output handler for a channel, returns the current one */
output_delegate mame_set_output_channel(output_channel channel, output_delegate callback);

/* built-in default callbacks */
void mame_file_output_callback(FILE *file, const char *format, va_list argptr);
void mame_null_output_callback(FILE *param, const char *format, va_list argptr);

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
void CLIB_DECL vlogerror(const char *format, va_list arg);


#endif  /* __MAME_H__ */
