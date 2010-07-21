/***************************************************************************

    options.h

    Core options code code

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#include "osdcore.h"
#include "corefile.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* unadorned option names */
#define MAX_UNADORNED_OPTIONS		16
#define OPTION_UNADORNED(x)			(((x) < MAX_UNADORNED_OPTIONS) ? option_unadorned[x] : "")

/* option flags */
#define OPTION_BOOLEAN				0x0001			/* option is a boolean value */
#define OPTION_DEPRECATED			0x0002			/* option is deprecated */
#define OPTION_COMMAND				0x0004			/* option is a command */
#define OPTION_HEADER				0x0008			/* text-only header */
#define OPTION_INTERNAL				0x0010			/* option is internal-only */
#define OPTION_REPEATS				0x0020			/* unadorned option repeats */

/* option priorities */
#define OPTION_PRIORITY_DEFAULT		0				/* defaults are at 0 priority */
#define OPTION_PRIORITY_LOW			50				/* low priority */
#define OPTION_PRIORITY_NORMAL		100				/* normal priority */
#define OPTION_PRIORITY_HIGH		150				/* high priority */
#define OPTION_PRIORITY_MAXIMUM		255				/* maximum priority */



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* opaque type representing a collection of options */
typedef struct _core_options core_options;

/* opaque type use for enumeration of options */
typedef struct _options_enumerator options_enumerator;

/* describes a single option with its description and default value */
typedef struct _options_entry options_entry;
struct _options_entry
{
	const char *		name;				/* name on the command line */
	const char *		defvalue;			/* default value of this argument */
	UINT32				flags;				/* flags to describe the option */
	const char *		description;		/* description for -showusage */
};

/* output messages are one of the following types */
enum _options_message
{
	OPTMSG_INFO,
	OPTMSG_WARNING,
	OPTMSG_ERROR,
	OPTMSG_COUNT
};
typedef enum _options_message options_message;

/* option ranges are one of the following types */
enum _options_range_type
{
	OPTION_RANGE_NONE,
	OPTION_RANGE_INT,
	OPTION_RANGE_FLOAT
};
typedef enum _options_range_type options_range_type;



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

extern const char *const option_unadorned[MAX_UNADORNED_OPTIONS];



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- options collection management ----- */

/* create a new collection of options */
core_options *options_create(void (*fail)(const char *message));

/* free a collection of options */
void options_free(core_options *opts);

/* set a callback for a particular class of message */
void options_set_output_callback(core_options *opts, options_message msgtype, void (*callback)(const char *s));

/* revert options at or below a certain priority back to their defaults */
void options_revert(core_options *opts, int priority);

/* copy one collection of options into another */
int options_copy(core_options *dest_opts, core_options *src_opts);

/* compare two collections of options */
int options_equal(core_options *opts1, core_options *opts2);



/* ----- option definitions ----- */

/* add a set of entries to an options collection */
int options_add_entries(core_options *opts, const options_entry *entrylist);

/* set the default value for a particular option entry */
int options_set_option_default_value(core_options *opts, const char *name, const char *defvalue);

/* set a callback for a particular option entry */
int options_set_option_callback(core_options *opts, const char *name, void (*callback)(core_options *opts, const char *arg));



/* ----- option data extraction ----- */

/* parse option data from a command line */
int options_parse_command_line(core_options *opts, int argc, char **argv, int priority);

/*  set option value and execute callback call */
int options_force_option_callback(core_options *opts, const char *optionname, const char *newval, int priority);

/* parse option data from an INI file */
int options_parse_ini_file(core_options *opts, core_file *inifile, int priority);



/* ----- options output ----- */

/* output option data to an INI file */
void options_output_ini_file(core_options *opts, core_file *inifile);

/* output differing option data to an INI file */
void options_output_diff_ini_file(core_options *opts, core_options *baseopts, core_file *inifile);

/* output option data to a standard file handle */
void options_output_ini_stdfile(core_options *opts, FILE *inifile);

/* output help using the specified output function */
void options_output_help(core_options *opts, void (*output)(const char *s));



/* ----- options reading ----- */

/* read an option as a string */
const char *options_get_string(core_options *opts, const char *name);

/* read an option as a boolean */
int options_get_bool(core_options *opts, const char *name);

/* read an option as an integer */
int options_get_int(core_options *opts, const char *name);

/* read an option as a floating point value */
float options_get_float(core_options *opts, const char *name);

/* read an option as a string */
UINT32 options_get_seqid(core_options *opts, const char *name);



/* ----- options setting ----- */

/* set an option as a string */
void options_set_string(core_options *opts, const char *name, const char *value, int priority);

/* set an option as a boolean */
void options_set_bool(core_options *opts, const char *name, int value, int priority);

/* set an option as an integer */
void options_set_int(core_options *opts, const char *name, int value, int priority);

/* set an option as a floating point value */
void options_set_float(core_options *opts, const char *name, float value, int priority);



/* ----- option definition queries ----- */

/* begin enumerating option definitions */
options_enumerator *options_enumerator_begin(core_options *opts);

/* get the next option in sequence */
const char *options_enumerator_next(options_enumerator *enumerator);

/* free memory allocated for enumeration */
void options_enumerator_free(options_enumerator *enumerator);

/* get the type of range for a given option */
options_range_type options_get_range_type(core_options *opts, const char *name);

/* return an integer range for a given option */
void options_get_range_int(core_options *opts, const char *name, int *minval, int *maxval);

/* return a floating point range for a given option */
void options_get_range_float(core_options *opts, const char *name, float *minval, float *maxval);


#endif /* __OPTIONS_H__ */
