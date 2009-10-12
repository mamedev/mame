/***************************************************************************

    options.c

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

#include <stdarg.h>
#include <ctype.h>
#include "options.h"
#include "astring.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MAX_ENTRY_NAMES		4



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* forward references */
typedef struct _options_data options_data;


/* range parameter for minimum/maximum values */
typedef union _options_range_parameter options_range_parameter;
union _options_range_parameter
{
	float 					f;
	int 					i;
};


/* hash table entries */
typedef struct _options_hash_entry options_hash_entry;
struct _options_hash_entry
{
	options_hash_entry *	next;				/* link to the next entry */
	astring *				name;				/* name under the current link */
	options_data *			data;				/* link to associated options_data */
};


/* information about a single entry in the options */
struct _options_data
{
	options_hash_entry		links[MAX_ENTRY_NAMES]; /* array of hash table entries (one per name) */
	options_data *			next;				/* link to the next data */
	UINT32					flags;				/* flags from the entry */
	UINT32					seqid;				/* sequence ID; bumped on each change */
	int						error_reported;		/* have we reported an error on this option yet? */
	int						priority;			/* priority of the data set */
	astring *				data;				/* data for this item */
	astring *				defdata;			/* default data for this item */
	const char *			description;		/* description for this item */
	options_range_type		range_type;			/* the type of range to apply to this item */
	options_range_parameter	range_minimum;		/* the minimum of the range */
	options_range_parameter	range_maximum;		/* the maximum of the range */
	void					(*callback)(core_options *opts, const char *arg);	/* callback to be invoked when parsing */
};


/* structure holding information about a collection of options */
struct _core_options
{
	void (*output[OPTMSG_COUNT])(const char *s);/* output callbacks */

	/* linked list, for sequential iteration */
	options_data *			datalist;			/* linked list for sequential iteration */
	options_data **			datalist_nextptr;	/* pointer to pointer to tail entry */

	/* hashtable, for fast lookup */
	options_hash_entry *	hashtable[101];		/* hash table for fast lookup */
};


/* information about an in-progress options enumeration */
struct _options_enumerator
{
	options_data *			current;
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static options_data *find_entry_data(core_options *opts, const char *string, int is_command_line);
static void update_data(core_options *opts, options_data *data, const char *newdata, int priority);
static int parse_option_name(core_options *opts, const char *srcstring, options_data *data);

static void message(core_options *opts, options_message msgtype, const char *format, ...) ATTR_PRINTF(3,4);
static UINT32 hash_value(core_options *opts, const char *str);
static void output_printf(void (*output)(const char *s), const char *format, ...) ATTR_PRINTF(2,3);



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

const char *const option_unadorned[MAX_UNADORNED_OPTIONS] =
{
	"<UNADORNED0>",
	"<UNADORNED1>",
	"<UNADORNED2>",
	"<UNADORNED3>",
	"<UNADORNED4>",
	"<UNADORNED5>",
	"<UNADORNED6>",
	"<UNADORNED7>",
	"<UNADORNED8>",
	"<UNADORNED9>",
	"<UNADORNED10>",
	"<UNADORNED11>",
	"<UNADORNED12>",
	"<UNADORNED13>",
	"<UNADORNED14>",
	"<UNADORNED15>"
};



/***************************************************************************
    OPTIONS COLLECTION MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    options_create - creates a new instance of
    core options
-------------------------------------------------*/

core_options *options_create(void (*fail)(const char *message))
{
	/* allocate memory for the option block */
	core_options *opts = (core_options *)malloc(sizeof(*opts));
	if (opts == NULL)
		goto error;

	/* and set up the structure */
	memset(opts, 0, sizeof(*opts));
	opts->datalist_nextptr = &opts->datalist;
	return opts;

error:
	return NULL;
}


/*-------------------------------------------------
    options_free - frees an options object
-------------------------------------------------*/

void options_free(core_options *opts)
{
	options_data *data, *next;

	/* loop over data items and free them */
	for (data = opts->datalist; data != NULL; data = next)
	{
		int linknum;

		next = data->next;

		/* free names */
		for (linknum = 0; linknum < ARRAY_LENGTH(data->links); linknum++)
			if (data->links[linknum].name != NULL)
				astring_free(data->links[linknum].name);

		/* free strings */
		astring_free(data->data);
		astring_free(data->defdata);

		/* free the data itself */
		free(data);
	}

	/* free the options itself */
	free(opts);
}


/*-------------------------------------------------
    options_set_output_callback - installs a callback
    to be invoked when data is outputted in the
    process of outputting options
-------------------------------------------------*/

void options_set_output_callback(core_options *opts, options_message msgtype, void (*callback)(const char *s))
{
	opts->output[msgtype] = callback;
}


/*-------------------------------------------------
    options_revert - revert options at or below
    a certain priority back to their defaults
-------------------------------------------------*/

void options_revert(core_options *opts, int priority)
{
	options_data *data;

	/* iterate over options and revert to defaults if below the given priority */
	for (data = opts->datalist; data != NULL; data = data->next)
		if (data->priority <= priority)
		{
			astring_cpy(data->data, data->defdata);
			data->priority = OPTION_PRIORITY_DEFAULT;
		}
}


/*-------------------------------------------------
    options_copy - copy options from one core_options
    to another
-------------------------------------------------*/

int options_copy(core_options *dest_opts, core_options *src_opts)
{
	options_data *data;

	/* iterate over options in the destination */
	for (data = dest_opts->datalist; data != NULL; data = data->next)
		if (!(data->flags & OPTION_HEADER))
		{
			options_data *srcdata = find_entry_data(src_opts, astring_c(data->links[0].name), FALSE);

			/* if the option exists in the source, set it in the destination */
			if (srcdata != NULL)
				options_set_string(dest_opts, astring_c(srcdata->links[0].name), astring_c(srcdata->data), srcdata->priority);
		}

	return TRUE;
}


/*-------------------------------------------------
    options_equal - compare two sets of options
-------------------------------------------------*/

int options_equal(core_options *opts1, core_options *opts2)
{
	options_data *data;

	/* iterate over options in the first list */
	for (data = opts1->datalist; data != NULL; data = data->next)
		if (!(data->flags & OPTION_HEADER))
		{
			const char *value1 = options_get_string(opts1, astring_c(data->links[0].name));
			const char *value2 = options_get_string(opts2, astring_c(data->links[0].name));

			/* if the values differ, return false */
			if (strcmp(value1, value2) != 0)
				return FALSE;
		}

	return TRUE;
}



/***************************************************************************
    OPTION DEFINITIONS
***************************************************************************/

/*-------------------------------------------------
    options_add_entries - add entries to the
    current options sets
-------------------------------------------------*/

int options_add_entries(core_options *opts, const options_entry *entrylist)
{
	/* loop over entries until we hit a NULL name */
	for ( ; entrylist->name != NULL || (entrylist->flags & OPTION_HEADER); entrylist++)
	{
		options_data *match = NULL;
		int i;

		/* allocate a new item */
		options_data *data = (options_data *)malloc(sizeof(*data));
		if (data == NULL)
			return FALSE;
		memset(data, 0, sizeof(*data));

		/* parse the option name */
		if (entrylist->name != NULL)
			parse_option_name(opts, entrylist->name, data);

		/* do we match an existing entry? */
		for (i = 0; i < ARRAY_LENGTH(data->links) && match == NULL; i++)
			if (data->links[i].name != NULL)
				match = find_entry_data(opts, astring_c(data->links[i].name), FALSE);

		/* if so, throw away this entry and replace the data */
		if (match != NULL)
		{
			/* free what we've allocated so far */
			for (i = 0; i < ARRAY_LENGTH(data->links); i++)
				if (data->links[i].name != NULL)
					astring_free(data->links[i].name);
			free(data);

			/* use the matching entry as our data */
			data = match;
		}

		/* otherwise, finish making the new entry */
		else
		{
			/* allocate strings */
			data->data = astring_alloc();
			data->defdata = astring_alloc();

			/* copy the flags, and set the value equal to the default */
			data->flags = entrylist->flags;
			data->description = entrylist->description;

			/* add us to the end of the sequential list */
			*opts->datalist_nextptr = data;
			opts->datalist_nextptr = &data->next;

			/* add each name to the appropriate hash table with a link back to us */
			for (i = 0; i < ARRAY_LENGTH(data->links); i++)
				if (data->links[i].name != NULL)
				{
					int hash_entry = hash_value(opts, astring_c(data->links[i].name));

					/* set up link */
					data->links[i].data = data;
					data->links[i].next = opts->hashtable[hash_entry];
					opts->hashtable[hash_entry] = &data->links[i];
				}
		}

		/* copy in the data and default data values */
		if (entrylist->defvalue != NULL)
		{
			astring_cpyc(data->data, entrylist->defvalue);
			astring_cpyc(data->defdata, entrylist->defvalue);
		}
		data->priority = OPTION_PRIORITY_DEFAULT;
	}
	return TRUE;
}


/*-------------------------------------------------
    options_set_option_default_value - change the
    default value of an option
-------------------------------------------------*/

int options_set_option_default_value(core_options *opts, const char *name, const char *defvalue)
{
	options_data *data = find_entry_data(opts, name, TRUE);

	/* if we don't have an entry for this, fail */
	if (data == NULL)
		return FALSE;

	/* update the data and default data; note that we assume that data == defdata */
	astring_cpyc(data->data, defvalue);
	astring_cpyc(data->defdata, defvalue);
	data->priority = OPTION_PRIORITY_DEFAULT;
	return TRUE;
}


/*-------------------------------------------------
    options_set_option_callback - specifies a
    callback to be invoked when parsing options
-------------------------------------------------*/

int options_set_option_callback(core_options *opts, const char *name, void (*callback)(core_options *opts, const char *arg))
{
	options_data *data = find_entry_data(opts, name, TRUE);

	/* if we don't find an entry, fail */
	if (data == NULL)
		return FALSE;

	/* set the callback */
	data->callback = callback;
	return TRUE;
}



/***************************************************************************
    OPTION DATA EXTRACTION
***************************************************************************/

/*-------------------------------------------------
    options_parse_command_line - parse a series
    of command line arguments
-------------------------------------------------*/

int options_parse_command_line(core_options *opts, int argc, char **argv, int priority)
{
	int unadorned_index = 0;
	int arg;

	/* loop over commands, looking for options */
	for (arg = 1; arg < argc; arg++)
	{
		const char *optionname, *newdata;
		options_data *data;
		int is_unadorned;

		/* determine the entry name to search for */
		is_unadorned = (argv[arg][0] != '-');
		if (!is_unadorned)
			optionname = &argv[arg][1];
		else
			optionname = OPTION_UNADORNED(unadorned_index);

		/* find our entry */
		data = find_entry_data(opts, optionname, TRUE);
		if (data == NULL)
		{
			message(opts, OPTMSG_ERROR, "Error: unknown option: %s\n", argv[arg]);
			return 1;
		}

		/* if unadorned, we have to bump the count (unless if the option repeats) */
		if (is_unadorned && !(data->flags & OPTION_REPEATS))
			unadorned_index++;

		/* get the data for this argument, special casing booleans */
		if ((data->flags & (OPTION_BOOLEAN | OPTION_COMMAND)) != 0)
			newdata = (strncmp(&argv[arg][1], "no", 2) == 0) ? "0" : "1";
		else if (argv[arg][0] != '-')
			newdata = argv[arg];
		else if (arg + 1 < argc)
			newdata = argv[++arg];
		else
		{
			message(opts, OPTMSG_ERROR, "Error: option %s expected a parameter\n", argv[arg]);
			return 1;
		}

		/* if the option is deprecated or internal, don't process further */
		if ((data->flags & (OPTION_DEPRECATED | OPTION_INTERNAL)) != 0)
			continue;

		/* invoke callback, if present */
		if (data->callback != NULL)
			(*data->callback)(opts, newdata);

		/* allocate a new copy of data for this */
		update_data(opts, data, newdata, priority);
	}
	return 0;
}


/*-------------------------------------------------
    options_parse_ini_file - parse a series
    of entries in an INI file
-------------------------------------------------*/

int options_parse_ini_file(core_options *opts, core_file *inifile, int priority)
{
	char buffer[4096];

	/* loop over data */
	while (core_fgets(buffer, ARRAY_LENGTH(buffer), inifile) != NULL)
	{
		char *optionname, *optiondata, *temp;
		options_data *data;
		int inquotes = FALSE;

		/* find the name */
		for (optionname = buffer; *optionname != 0; optionname++)
			if (!isspace((UINT8)*optionname))
				break;

		/* skip comments */
		if (*optionname == 0 || *optionname == '#')
			continue;

		/* scan forward to find the first space */
		for (temp = optionname; *temp != 0; temp++)
			if (isspace((UINT8)*temp))
				break;

		/* if we hit the end early, print a warning and continue */
		if (*temp == 0)
		{
			message(opts, OPTMSG_WARNING, "Warning: invalid line in INI: %s", buffer);
			continue;
		}

		/* NULL-terminate */
		*temp++ = 0;
		optiondata = temp;

		/* scan the data, stopping when we hit a comment */
		for (temp = optiondata; *temp != 0; temp++)
		{
			if (*temp == '"')
				inquotes = !inquotes;
			if (*temp == '#' && !inquotes)
				break;
		}
		*temp = 0;

		/* find our entry */
		data = find_entry_data(opts, optionname, FALSE);
		if (data == NULL)
		{
			message(opts, OPTMSG_WARNING, "Warning: unknown option in INI: %s\n", optionname);
			continue;
		}
		if ((data->flags & (OPTION_DEPRECATED | OPTION_INTERNAL)) != 0)
			continue;

		/* allocate a new copy of data for this */
		update_data(opts, data, optiondata, priority);
	}
	return 0;
}



/***************************************************************************
    OPTIONS OUTPUT
***************************************************************************/

/*-------------------------------------------------
    options_output_diff_ini_file - output the diff
    of the current state from a base state to an
    INI file
-------------------------------------------------*/

void options_output_diff_ini_file(core_options *opts, core_options *baseopts, core_file *inifile)
{
	options_data *data;
	const char *last_header = NULL;
	const char *name;
	const char *value;
	options_data *basedata;

	/* loop over all items */
	for (data = opts->datalist; data != NULL; data = data->next)
	{
		/* header: record description */
		if ((data->flags & OPTION_HEADER) != 0)
			last_header = data->description;

		/* otherwise, output entries for all non-deprecated and non-command items (if not in baseopts) */
		else if ((data->flags & (OPTION_DEPRECATED | OPTION_INTERNAL | OPTION_COMMAND)) == 0)
		{
			/* get name and data of this value */
			name = astring_c(data->links[0].name);
			value = astring_c(data->data);

			/* look up counterpart in baseopts, if baseopts is specified */
			basedata = (baseopts != NULL) ? find_entry_data(baseopts, name, FALSE) : NULL;

			/* is our data different, or not in baseopts? */
			if ((basedata == NULL) || (strcmp(value, astring_c(basedata->data)) != 0))
			{
				/* output header, if we have one */
				if (last_header != NULL)
				{
					core_fprintf(inifile, "\n#\n# %s\n#\n", last_header);
					last_header = NULL;
				}

				/* and finally output the data */
				if (strchr(value, ' ') != NULL)
					core_fprintf(inifile, "%-25s \"%s\"\n", name, value);
				else
					core_fprintf(inifile, "%-25s %s\n", name, value);
			}
		}
	}
}


/*-------------------------------------------------
    options_output_ini_file - output the current
    state to an INI file
-------------------------------------------------*/

void options_output_ini_file(core_options *opts, core_file *inifile)
{
	options_output_diff_ini_file(opts, NULL, inifile);
}


/*-------------------------------------------------
    options_output_ini_file - output the current
    state to an INI file
-------------------------------------------------*/

void options_output_ini_stdfile(core_options *opts, FILE *inifile)
{
	options_data *data;

	/* loop over all items */
	for (data = opts->datalist; data != NULL; data = data->next)
	{
		/* header: just print */
		if ((data->flags & OPTION_HEADER) != 0)
			fprintf(inifile, "\n#\n# %s\n#\n", data->description);

		/* otherwise, output entries for all non-deprecated and non-command items */
		else if ((data->flags & (OPTION_DEPRECATED | OPTION_INTERNAL | OPTION_COMMAND)) == 0)
		{
			if (astring_chr(data->data, 0, ' ') != -1)
				fprintf(inifile, "%-25s \"%s\"\n", astring_c(data->links[0].name), astring_c(data->data));
			else
				fprintf(inifile, "%-25s %s\n", astring_c(data->links[0].name), astring_c(data->data));
		}
	}
}


/*-------------------------------------------------
    options_output_help - output option help to
    a file
-------------------------------------------------*/

void options_output_help(core_options *opts, void (*output)(const char *))
{
	options_data *data;

	/* loop over all items */
	for (data = opts->datalist; data != NULL; data = data->next)
	{
		/* header: just print */
		if ((data->flags & OPTION_HEADER) != 0)
			output_printf(output, "\n#\n# %s\n#\n", data->description);

		/* otherwise, output entries for all non-deprecated items */
		else if ((data->flags & (OPTION_DEPRECATED | OPTION_INTERNAL)) == 0 && data->description != NULL)
			output_printf(output, "-%-20s%s\n", astring_c(data->links[0].name), data->description);
	}
}



/***************************************************************************
    OPTIONS READING
***************************************************************************/

/*-------------------------------------------------
    options_get_string - return data formatted
    as a string
-------------------------------------------------*/

const char *options_get_string(core_options *opts, const char *name)
{
	options_data *data = find_entry_data(opts, name, FALSE);
	const char *value = "";

	/* error if not found */
	if (data == NULL)
		message(opts, OPTMSG_ERROR, "Unexpected option %s queried\n", name);

	/* copy if non-NULL */
	else
		value = astring_c(data->data);

	return value;
}


/*-------------------------------------------------
    options_get_bool - return data formatted as
    a boolean
-------------------------------------------------*/

int options_get_bool(core_options *opts, const char *name)
{
	options_data *data = find_entry_data(opts, name, FALSE);
	int value = FALSE;

	/* error if not found */
	if (data == NULL)
		message(opts, OPTMSG_ERROR, "Unexpected boolean option %s queried\n", name);

	/* also error if we don't have a valid boolean value */
	else if (sscanf(astring_c(data->data), "%d", &value) != 1 || value < 0 || value > 1)
	{
		options_set_string(opts, name, astring_c(data->defdata), 0);
		sscanf(astring_c(data->data), "%d", &value);
		if (!data->error_reported)
		{
			message(opts, OPTMSG_ERROR, "Illegal boolean value for %s; reverting to %d\n", astring_c(data->links[0].name), value);
			data->error_reported = TRUE;
		}
	}
	return value;
}


/*-------------------------------------------------
    options_get_int - return data formatted as
    an integer
-------------------------------------------------*/

int options_get_int(core_options *opts, const char *name)
{
	options_data *data = find_entry_data(opts, name, FALSE);
	int value = 0;

	/* error if not found */
	if (data == NULL)
		message(opts, OPTMSG_ERROR, "Unexpected integer option %s queried\n", name);

	/* also error if we don't have a valid integer value */
	else if (sscanf(astring_c(data->data), "%d", &value) != 1)
	{
		options_set_string(opts, name, astring_c(data->defdata), 0);
		sscanf(astring_c(data->data), "%d", &value);
		if (!data->error_reported)
		{
			message(opts, OPTMSG_ERROR, "Illegal integer value for %s; reverting to %d\n", astring_c(data->links[0].name), value);
			data->error_reported = TRUE;
		}
	}
	return value;
}


/*-------------------------------------------------
    options_get_float - return data formatted as
    a float
-------------------------------------------------*/

float options_get_float(core_options *opts, const char *name)
{
	options_data *data = find_entry_data(opts, name, FALSE);
	float value = 0;

	/* error if not found */
	if (data == NULL)
		message(opts, OPTMSG_ERROR, "Unexpected float option %s queried\n", name);

	/* also error if we don't have a valid floating point value */
	else if (sscanf(astring_c(data->data), "%f", &value) != 1)
	{
		options_set_string(opts, name, astring_c(data->defdata), 0);
		sscanf(astring_c(data->data), "%f", &value);
		if (!data->error_reported)
		{
			message(opts, OPTMSG_ERROR, "Illegal float value for %s; reverting to %f\n", astring_c(data->links[0].name), (double)value);
			data->error_reported = TRUE;
		}
	}
	return value;
}


/*-------------------------------------------------
    options_get_seqid - return the seqid for an
    entry
-------------------------------------------------*/

UINT32 options_get_seqid(core_options *opts, const char *name)
{
	options_data *data = find_entry_data(opts, name, FALSE);
	return (data == NULL) ? 0 : data->seqid;
}



/***************************************************************************
    OPTIONS SETTING
***************************************************************************/

/*-------------------------------------------------
    options_set_string - set a string value
-------------------------------------------------*/

void options_set_string(core_options *opts, const char *name, const char *value, int priority)
{
	options_data *data = find_entry_data(opts, name, FALSE);
	update_data(opts, data, value, priority);
}


/*-------------------------------------------------
    options_set_bool - set a boolean value
-------------------------------------------------*/

void options_set_bool(core_options *opts, const char *name, int value, int priority)
{
	char temp[4];
	sprintf(temp, "%d", value ? 1 : 0);
	options_set_string(opts, name, temp, priority);
}


/*-------------------------------------------------
    options_set_int - set an integer value
-------------------------------------------------*/

void options_set_int(core_options *opts, const char *name, int value, int priority)
{
	char temp[20];
	sprintf(temp, "%d", value);
	options_set_string(opts, name, temp, priority);
}


/*-------------------------------------------------
    options_set_float - set a float value
-------------------------------------------------*/

void options_set_float(core_options *opts, const char *name, float value, int priority)
{
	char temp[100];
	sprintf(temp, "%f", value);
	options_set_string(opts, name, temp, priority);
}



/***************************************************************************
    OPTION DEFINITION QUERIES
***************************************************************************/

/*-------------------------------------------------
    options_enumerator_begin - retrieve the range of
    a float option
-------------------------------------------------*/

options_enumerator *options_enumerator_begin(core_options *opts)
{
	options_enumerator *enumerator;

	/* allocate memory for the enumerator */
	enumerator = (options_enumerator *)malloc(sizeof(*enumerator));
	if (enumerator == NULL)
		return NULL;

	/* start at the head of the list */
	enumerator->current = opts->datalist;
	return enumerator;
}



/*-------------------------------------------------
    options_enumerator_next - returns the current
    option and advances the enumerator
-------------------------------------------------*/

const char *options_enumerator_next(options_enumerator *enumerator)
{
	astring *option_name = NULL;

	/* be sure to skip over false options */
	while (option_name == NULL && enumerator->current != NULL)
	{
		/* retrieve the current option name and advance the enumerator */
		option_name = enumerator->current->links[0].name;
		enumerator->current = enumerator->current->next;
	}
	return (option_name != NULL) ? astring_c(option_name) : NULL;
}



/*-------------------------------------------------
    options_enumerator_free - disposes an options
    enumerator
-------------------------------------------------*/

void options_enumerator_free(options_enumerator *enumerator)
{
	free(enumerator);
}


/*-------------------------------------------------
    options_get_range_type - determine the type
    of range for a particular option
-------------------------------------------------*/

options_range_type options_get_range_type(core_options *opts, const char *name)
{
	options_data *data = find_entry_data(opts, name, FALSE);
	return data->range_type;
}


/*-------------------------------------------------
    options_get_range_int - retrieve the range of
    an integer option
-------------------------------------------------*/

void options_get_range_int(core_options *opts, const char *name, int *minval, int *maxval)
{
	options_data *data = find_entry_data(opts, name, FALSE);
	*minval = data->range_minimum.i;
	*maxval = data->range_maximum.i;
}



/*-------------------------------------------------
    options_get_range_float - retrieve the range of
    a float option
-------------------------------------------------*/

void options_get_range_float(core_options *opts, const char *name, float *minval, float *maxval)
{
	options_data *data = find_entry_data(opts, name, FALSE);
	*minval = data->range_minimum.f;
	*maxval = data->range_maximum.f;
}



/***************************************************************************
    INTERNAL UTILITIES
***************************************************************************/

/*-------------------------------------------------
    find_entry_data - locate an entry whose name
    matches the given string
-------------------------------------------------*/

static options_data *find_entry_data(core_options *opts, const char *string, int is_command_line)
{
	int hash_entry = hash_value(opts, string);
	options_hash_entry *link;

	/* scan all entries */
	for (link = opts->hashtable[hash_entry]; link != NULL; link = link->next)
		if (!(link->data->flags & OPTION_HEADER) && link->name != NULL && astring_cmpc(link->name, string) == 0)
			return link->data;

	/* haven't found it?  if we are prefixed with "no", then try to search for that */
	if (is_command_line && string[0] == 'n' && string[1] == 'o')
	{
		options_data *data = find_entry_data(opts, &string[2], FALSE);
		if (data != NULL && (data->flags & OPTION_BOOLEAN))
			return data;
	}

	/* didn't find it at all */
	return NULL;
}


/*-------------------------------------------------
    update_data - update the data value for a
    given entry
-------------------------------------------------*/

static void update_data(core_options *opts, options_data *data, const char *newdata, int priority)
{
	const char *dataend = newdata + strlen(newdata) - 1;
	const char *datastart = newdata;
	float f;
	int i;

	/* strip off leading/trailing spaces */
	while (isspace((UINT8)*datastart) && datastart <= dataend)
		datastart++;
	while (isspace((UINT8)*dataend) && datastart <= dataend)
		dataend--;

	/* strip off quotes */
	if (datastart != dataend && *datastart == '"' && *dataend == '"')
		datastart++, dataend--;

	/* check against range */
	switch (data->range_type)
	{
		case OPTION_RANGE_NONE:
			/* do nothing */
			break;

		case OPTION_RANGE_INT:
			/* check against integer range */
			i = 0;
			if (sscanf(datastart, "%d", &i) != 1)
			{
				message(opts, OPTMSG_ERROR, "Illegal integer value for %s; keeping value of %s\n", astring_c(data->links[0].name), astring_c(data->data));
				data->error_reported = TRUE;
				return;
			}
			if (i < data->range_minimum.i || i > data->range_maximum.i)
			{
				message(opts, OPTMSG_ERROR, "Invalid %s value (must be between %i and %i); keeping value of %s\n",
					astring_c(data->links[0].name), data->range_minimum.i, data->range_maximum.i, astring_c(data->data));
				data->error_reported = TRUE;
				return;
			}
			break;

		case OPTION_RANGE_FLOAT:
			/* check against float range */
			f = 0;
			if (sscanf(datastart, "%f", &f) != 1)
			{
				message(opts, OPTMSG_ERROR, "Illegal float value for %s; keeping value of %s\n", astring_c(data->links[0].name), astring_c(data->data));
				data->error_reported = TRUE;
				return;
			}
			if (f < data->range_minimum.f || f > data->range_maximum.f)
			{
				message(opts, OPTMSG_ERROR, "Invalid %s value (must be between %f and %f); keeping value of %s\n",
					astring_c(data->links[0].name), data->range_minimum.f, data->range_maximum.f, astring_c(data->data));
				data->error_reported = TRUE;
				return;
			}
			break;
	}

	/* ignore if we don't have priority */
	if (priority < data->priority)
		return;

	/* allocate a copy of the data */
	astring_cpych(data->data, datastart, dataend + 1 - datastart);
	data->priority = priority;

	/* bump the seqid and clear the error reporting */
	data->seqid++;
	data->error_reported = FALSE;
}


/*-------------------------------------------------
    parse_option_name - read data from an option_entry
    name into an option_data structure
-------------------------------------------------*/

static int parse_option_name(core_options *opts, const char *srcstring, options_data *data)
{
	const char *start;
	const char *end = NULL;
	int curentry;

	/* start with the original string and loop over entries */
	start = srcstring;
	for (curentry = 0; curentry < ARRAY_LENGTH(data->links); curentry++)
	{
		/* find the end of this entry and copy the string */
		for (end = start; *end != 0 && *end != ';' && *end != '('; end++)
			;
		data->links[curentry].name = astring_dupch(start, end - start);

		/* if we hit the end of the source, stop */
		if (*end != ';')
			break;
		start = end + 1;
	}

	/* have we found a range? */
	if (end != NULL && *end == '(')
	{
		if (sscanf(end, "(%d-%d)", &data->range_minimum.i, &data->range_maximum.i) == 2)
			data->range_type = OPTION_RANGE_INT;
		else if (sscanf(end, "(%f-%f)", &data->range_minimum.f, &data->range_maximum.f) == 2)
			data->range_type = OPTION_RANGE_FLOAT;
	}
	return curentry;
}


/*-------------------------------------------------
    message - outputs a message to a listener
-------------------------------------------------*/

static void message(core_options *opts, options_message msgtype, const char *format, ...)
{
	char buf[1024];
	va_list argptr;

	/* output a message if there is a non-NULL handler for it */
	if (opts->output[msgtype] != NULL)
	{
		va_start(argptr, format);
		vsprintf(buf, format, argptr);
		va_end(argptr);

		(*opts->output[msgtype])(buf);
	}
}


/*-------------------------------------------------
    hash_value - computes the hash value for a string
-------------------------------------------------*/

static UINT32 hash_value(core_options *opts, const char *str)
{
    UINT32 hash = 5381;
    int c;

    while ((c = *str++) != 0)
        hash = ((hash << 5) + hash) + c;

    return hash % ARRAY_LENGTH(opts->hashtable);
}


/*-------------------------------------------------
    output_printf - outputs an arbitrary message
    to a callback
-------------------------------------------------*/

static void output_printf(void (*output)(const char *s), const char *format, ...)
{
	char buf[1024];
	va_list argptr;

	va_start(argptr, format);
	vsprintf(buf, format, argptr);
	va_end(argptr);

	output(buf);
}
