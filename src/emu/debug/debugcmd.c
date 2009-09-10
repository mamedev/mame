/*********************************************************************

    debugcmd.c

    Debugger command interface engine.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "driver.h"
#include "debugcmd.h"
#include "debugcmt.h"
#include "debugcon.h"
#include "debugcpu.h"
#include "express.h"
#include "debughlp.h"
#include "debugvw.h"
#include "render.h"
#include <ctype.h>



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MAX_GLOBALS		1000



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _global_entry global_entry;
struct _global_entry
{
	void *		base;
	UINT32		size;
};

typedef struct _cheat_map cheat_map;
struct _cheat_map
{
	UINT64		offset;
	UINT64		first_value;
	UINT64		previous_value;
	UINT8		state:1;
	UINT8		undo:7;
};

typedef struct _cheat_system cheat_system;
struct _cheat_system
{
	char			cpu;
	UINT64			length;
	UINT8			width;
	cheat_map *		cheatmap;
	UINT8			undo;
	UINT8			signed_cheat;
};


/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static global_entry global_array[MAX_GLOBALS];
static cheat_system cheat;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void debug_command_exit(running_machine *machine);

static UINT64 execute_min(void *globalref, void *ref, UINT32 params, const UINT64 *param);
static UINT64 execute_max(void *globalref, void *ref, UINT32 params, const UINT64 *param);
static UINT64 execute_if(void *globalref, void *ref, UINT32 params, const UINT64 *param);

static UINT64 global_get(void *globalref, void *ref);
static void global_set(void *globalref, void *ref, UINT64 value);

static void execute_help(running_machine *machine, int ref, int params, const char **param);
static void execute_print(running_machine *machine, int ref, int params, const char **param);
static void execute_printf(running_machine *machine, int ref, int params, const char **param);
static void execute_logerror(running_machine *machine, int ref, int params, const char **param);
static void execute_tracelog(running_machine *machine, int ref, int params, const char **param);
static void execute_quit(running_machine *machine, int ref, int params, const char **param);
static void execute_do(running_machine *machine, int ref, int params, const char **param);
static void execute_step(running_machine *machine, int ref, int params, const char **param);
static void execute_over(running_machine *machine, int ref, int params, const char **param);
static void execute_out(running_machine *machine, int ref, int params, const char **param);
static void execute_go(running_machine *machine, int ref, int params, const char **param);
static void execute_go_vblank(running_machine *machine, int ref, int params, const char **param);
static void execute_go_interrupt(running_machine *machine, int ref, int params, const char **param);
static void execute_go_time(running_machine *machine, int ref, int params, const char *param[]);
static void execute_focus(running_machine *machine, int ref, int params, const char **param);
static void execute_ignore(running_machine *machine, int ref, int params, const char **param);
static void execute_observe(running_machine *machine, int ref, int params, const char **param);
static void execute_next(running_machine *machine, int ref, int params, const char **param);
static void execute_comment(running_machine *machine, int ref, int params, const char **param);
static void execute_comment_del(running_machine *machine, int ref, int params, const char **param);
static void execute_comment_save(running_machine *machine, int ref, int params, const char **param);
static void execute_bpset(running_machine *machine, int ref, int params, const char **param);
static void execute_bpclear(running_machine *machine, int ref, int params, const char **param);
static void execute_bpdisenable(running_machine *machine, int ref, int params, const char **param);
static void execute_bplist(running_machine *machine, int ref, int params, const char **param);
static void execute_wpset(running_machine *machine, int ref, int params, const char **param);
static void execute_wpclear(running_machine *machine, int ref, int params, const char **param);
static void execute_wpdisenable(running_machine *machine, int ref, int params, const char **param);
static void execute_wplist(running_machine *machine, int ref, int params, const char **param);
static void execute_hotspot(running_machine *machine, int ref, int params, const char **param);
static void execute_save(running_machine *machine, int ref, int params, const char **param);
static void execute_dump(running_machine *machine, int ref, int params, const char **param);
static void execute_cheatinit(running_machine *machine, int ref, int params, const char **param);
static void execute_cheatnext(running_machine *machine, int ref, int params, const char **param);
static void execute_cheatlist(running_machine *machine, int ref, int params, const char **param);
static void execute_cheatundo(running_machine *machine, int ref, int params, const char **param);
static void execute_dasm(running_machine *machine, int ref, int params, const char **param);
static void execute_find(running_machine *machine, int ref, int params, const char **param);
static void execute_trace(running_machine *machine, int ref, int params, const char **param);
static void execute_traceover(running_machine *machine, int ref, int params, const char **param);
static void execute_traceflush(running_machine *machine, int ref, int params, const char **param);
static void execute_history(running_machine *machine, int ref, int params, const char **param);
static void execute_snap(running_machine *machine, int ref, int params, const char **param);
static void execute_source(running_machine *machine, int ref, int params, const char **param);
static void execute_map(running_machine *machine, int ref, int params, const char **param);
static void execute_memdump(running_machine *machine, int ref, int params, const char **param);
static void execute_symlist(running_machine *machine, int ref, int params, const char **param);
static void execute_softreset(running_machine *machine, int ref, int params, const char **param);
static void execute_hardreset(running_machine *machine, int ref, int params, const char **param);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    cheat_address_is_valid - return TRUE if the
    given address is valid for cheating
-------------------------------------------------*/

INLINE int cheat_address_is_valid(const address_space *space, offs_t address)
{
	return debug_cpu_translate(space, TRANSLATE_READ, &address) && (memory_get_write_ptr(space, address) != NULL);
}


/*-------------------------------------------------
    cheat_sign_extend - sign-extend a value to
    the current cheat width, if signed
-------------------------------------------------*/

INLINE UINT64 cheat_sign_extend(const cheat_system *cheatsys, UINT64 value)
{
	if (cheatsys->signed_cheat)
	{
		switch (cheatsys->width)
		{
			case 1:	value = (INT8)value;	break;
			case 2:	value = (INT16)value;	break;
			case 4:	value = (INT32)value;	break;
		}
	}
	return value;
}


/*-------------------------------------------------
    cheat_read_extended - read a value from memory
    in the given address space, sign-extending
    if necessary
-------------------------------------------------*/

INLINE UINT64 cheat_read_extended(const cheat_system *cheatsys, const address_space *space, offs_t address)
{
	return cheat_sign_extend(cheatsys, debug_read_memory(space, address, cheatsys->width, TRUE));
}



/***************************************************************************
    INITIALIZATION
***************************************************************************/

/*-------------------------------------------------
    debug_command_init - initializes the command
    system
-------------------------------------------------*/

void debug_command_init(running_machine *machine)
{
	symbol_table *symtable = debug_cpu_get_global_symtable(machine);
	const device_config *cpu;
	const char *name;
	int itemnum;

	/* add a few simple global functions */
	symtable_add_function(symtable, "min", NULL, 2, 2, execute_min);
	symtable_add_function(symtable, "max", NULL, 2, 2, execute_max);
	symtable_add_function(symtable, "if", NULL, 3, 3, execute_if);

	/* add all single-entry save state globals */
	for (itemnum = 0; itemnum < MAX_GLOBALS; itemnum++)
	{
		UINT32 valsize, valcount;
		void *base;

		/* stop when we run out of items */
		name = state_save_get_indexed_item(machine, itemnum, &base, &valsize, &valcount);
		if (name == NULL)
			break;

		/* if this is a single-entry global, add it */
		if (valcount == 1 && strstr(name, "/globals/"))
		{
			char symname[100];
			sprintf(symname, ".%s", strrchr(name, '/') + 1);
			global_array[itemnum].base = base;
			global_array[itemnum].size = valsize;
			symtable_add_register(symtable, symname, &global_array, global_get, global_set);
		}
	}

	/* add all the commands */
	debug_console_register_command(machine, "help",      CMDFLAG_NONE, 0, 0, 1, execute_help);
	debug_console_register_command(machine, "print",     CMDFLAG_NONE, 0, 1, MAX_COMMAND_PARAMS, execute_print);
	debug_console_register_command(machine, "printf",    CMDFLAG_NONE, 0, 1, MAX_COMMAND_PARAMS, execute_printf);
	debug_console_register_command(machine, "logerror",  CMDFLAG_NONE, 0, 1, MAX_COMMAND_PARAMS, execute_logerror);
	debug_console_register_command(machine, "tracelog",  CMDFLAG_NONE, 0, 1, MAX_COMMAND_PARAMS, execute_tracelog);
	debug_console_register_command(machine, "quit",      CMDFLAG_NONE, 0, 0, 0, execute_quit);
	debug_console_register_command(machine, "do",        CMDFLAG_NONE, 0, 1, 1, execute_do);
	debug_console_register_command(machine, "step",      CMDFLAG_NONE, 0, 0, 1, execute_step);
	debug_console_register_command(machine, "s",         CMDFLAG_NONE, 0, 0, 1, execute_step);
	debug_console_register_command(machine, "over",      CMDFLAG_NONE, 0, 0, 1, execute_over);
	debug_console_register_command(machine, "o",         CMDFLAG_NONE, 0, 0, 1, execute_over);
	debug_console_register_command(machine, "out" ,      CMDFLAG_NONE, 0, 0, 0, execute_out);
	debug_console_register_command(machine, "go",        CMDFLAG_NONE, 0, 0, 1, execute_go);
	debug_console_register_command(machine, "g",         CMDFLAG_NONE, 0, 0, 1, execute_go);
	debug_console_register_command(machine, "gvblank",   CMDFLAG_NONE, 0, 0, 0, execute_go_vblank);
	debug_console_register_command(machine, "gv",        CMDFLAG_NONE, 0, 0, 0, execute_go_vblank);
	debug_console_register_command(machine, "gint",      CMDFLAG_NONE, 0, 0, 1, execute_go_interrupt);
	debug_console_register_command(machine, "gi",        CMDFLAG_NONE, 0, 0, 1, execute_go_interrupt);
	debug_console_register_command(machine, "gtime",     CMDFLAG_NONE, 0, 0, 1, execute_go_time);
	debug_console_register_command(machine, "gt",        CMDFLAG_NONE, 0, 0, 1, execute_go_time);
	debug_console_register_command(machine, "next",      CMDFLAG_NONE, 0, 0, 0, execute_next);
	debug_console_register_command(machine, "n",         CMDFLAG_NONE, 0, 0, 0, execute_next);
	debug_console_register_command(machine, "focus",     CMDFLAG_NONE, 0, 1, 1, execute_focus);
	debug_console_register_command(machine, "ignore",    CMDFLAG_NONE, 0, 0, MAX_COMMAND_PARAMS, execute_ignore);
	debug_console_register_command(machine, "observe",   CMDFLAG_NONE, 0, 0, MAX_COMMAND_PARAMS, execute_observe);

	debug_console_register_command(machine, "comadd",	CMDFLAG_NONE, 0, 1, 2, execute_comment);
	debug_console_register_command(machine, "//",        CMDFLAG_NONE, 0, 1, 2, execute_comment);
	debug_console_register_command(machine, "comdelete",	CMDFLAG_NONE, 0, 1, 1, execute_comment_del);
	debug_console_register_command(machine, "comsave", 	CMDFLAG_NONE, 0, 0, 0, execute_comment_save);

	debug_console_register_command(machine, "bpset",     CMDFLAG_NONE, 0, 1, 3, execute_bpset);
	debug_console_register_command(machine, "bp",        CMDFLAG_NONE, 0, 1, 3, execute_bpset);
	debug_console_register_command(machine, "bpclear",   CMDFLAG_NONE, 0, 0, 1, execute_bpclear);
	debug_console_register_command(machine, "bpdisable", CMDFLAG_NONE, 0, 0, 1, execute_bpdisenable);
	debug_console_register_command(machine, "bpenable",  CMDFLAG_NONE, 1, 0, 1, execute_bpdisenable);
	debug_console_register_command(machine, "bplist",    CMDFLAG_NONE, 0, 0, 0, execute_bplist);

	debug_console_register_command(machine, "wpset",     CMDFLAG_NONE, ADDRESS_SPACE_PROGRAM, 3, 5, execute_wpset);
	debug_console_register_command(machine, "wp",        CMDFLAG_NONE, ADDRESS_SPACE_PROGRAM, 3, 5, execute_wpset);
	debug_console_register_command(machine, "wpdset",    CMDFLAG_NONE, ADDRESS_SPACE_DATA, 3, 5, execute_wpset);
	debug_console_register_command(machine, "wpd",       CMDFLAG_NONE, ADDRESS_SPACE_DATA, 3, 5, execute_wpset);
	debug_console_register_command(machine, "wpiset",    CMDFLAG_NONE, ADDRESS_SPACE_IO, 3, 5, execute_wpset);
	debug_console_register_command(machine, "wpi",       CMDFLAG_NONE, ADDRESS_SPACE_IO, 3, 5, execute_wpset);
	debug_console_register_command(machine, "wpclear",   CMDFLAG_NONE, 0, 0, 1, execute_wpclear);
	debug_console_register_command(machine, "wpdisable", CMDFLAG_NONE, 0, 0, 1, execute_wpdisenable);
	debug_console_register_command(machine, "wpenable",  CMDFLAG_NONE, 1, 0, 1, execute_wpdisenable);
	debug_console_register_command(machine, "wplist",    CMDFLAG_NONE, 0, 0, 0, execute_wplist);

	debug_console_register_command(machine, "hotspot",   CMDFLAG_NONE, 0, 0, 3, execute_hotspot);

	debug_console_register_command(machine, "save",      CMDFLAG_NONE, ADDRESS_SPACE_PROGRAM, 3, 4, execute_save);
	debug_console_register_command(machine, "saved",     CMDFLAG_NONE, ADDRESS_SPACE_DATA, 3, 4, execute_save);
	debug_console_register_command(machine, "savei",     CMDFLAG_NONE, ADDRESS_SPACE_IO, 3, 4, execute_save);

	debug_console_register_command(machine, "dump",      CMDFLAG_NONE, ADDRESS_SPACE_PROGRAM, 3, 6, execute_dump);
	debug_console_register_command(machine, "dumpd",     CMDFLAG_NONE, ADDRESS_SPACE_DATA, 3, 6, execute_dump);
	debug_console_register_command(machine, "dumpi",     CMDFLAG_NONE, ADDRESS_SPACE_IO, 3, 6, execute_dump);

	debug_console_register_command(machine, "cheatinit", CMDFLAG_NONE, 0, 0, 4, execute_cheatinit);
	debug_console_register_command(machine, "ci",        CMDFLAG_NONE, 0, 0, 4, execute_cheatinit);

	debug_console_register_command(machine, "cheatrange",CMDFLAG_NONE, 1, 2, 2, execute_cheatinit);
	debug_console_register_command(machine, "cr",        CMDFLAG_NONE, 1, 2, 2, execute_cheatinit);

	debug_console_register_command(machine, "cheatnext", CMDFLAG_NONE, 0, 1, 2, execute_cheatnext);
	debug_console_register_command(machine, "cn",        CMDFLAG_NONE, 0, 1, 2, execute_cheatnext);
	debug_console_register_command(machine, "cheatnextf",CMDFLAG_NONE, 1, 1, 2, execute_cheatnext);
	debug_console_register_command(machine, "cnf",       CMDFLAG_NONE, 1, 1, 2, execute_cheatnext);

	debug_console_register_command(machine, "cheatlist", CMDFLAG_NONE, 0, 0, 1, execute_cheatlist);
	debug_console_register_command(machine, "cl",        CMDFLAG_NONE, 0, 0, 1, execute_cheatlist);

	debug_console_register_command(machine, "cheatundo", CMDFLAG_NONE, 0, 0, 0, execute_cheatundo);
	debug_console_register_command(machine, "cu",        CMDFLAG_NONE, 0, 0, 0, execute_cheatundo);

	debug_console_register_command(machine, "f",         CMDFLAG_KEEP_QUOTES, ADDRESS_SPACE_PROGRAM, 3, MAX_COMMAND_PARAMS, execute_find);
	debug_console_register_command(machine, "find",      CMDFLAG_KEEP_QUOTES, ADDRESS_SPACE_PROGRAM, 3, MAX_COMMAND_PARAMS, execute_find);
	debug_console_register_command(machine, "fd",        CMDFLAG_KEEP_QUOTES, ADDRESS_SPACE_DATA, 3, MAX_COMMAND_PARAMS, execute_find);
	debug_console_register_command(machine, "findd",     CMDFLAG_KEEP_QUOTES, ADDRESS_SPACE_DATA, 3, MAX_COMMAND_PARAMS, execute_find);
	debug_console_register_command(machine, "fi",        CMDFLAG_KEEP_QUOTES, ADDRESS_SPACE_IO, 3, MAX_COMMAND_PARAMS, execute_find);
	debug_console_register_command(machine, "findi",     CMDFLAG_KEEP_QUOTES, ADDRESS_SPACE_IO, 3, MAX_COMMAND_PARAMS, execute_find);

	debug_console_register_command(machine, "dasm",      CMDFLAG_NONE, 0, 3, 5, execute_dasm);

	debug_console_register_command(machine, "trace",     CMDFLAG_NONE, 0, 1, 3, execute_trace);
	debug_console_register_command(machine, "traceover", CMDFLAG_NONE, 0, 1, 3, execute_traceover);
	debug_console_register_command(machine, "traceflush",CMDFLAG_NONE, 0, 0, 0, execute_traceflush);

	debug_console_register_command(machine, "history",   CMDFLAG_NONE, 0, 0, 2, execute_history);

	debug_console_register_command(machine, "snap",      CMDFLAG_NONE, 0, 0, 1, execute_snap);

	debug_console_register_command(machine, "source",    CMDFLAG_NONE, 0, 1, 1, execute_source);

	debug_console_register_command(machine, "map",		CMDFLAG_NONE, ADDRESS_SPACE_PROGRAM, 1, 1, execute_map);
	debug_console_register_command(machine, "mapd",		CMDFLAG_NONE, ADDRESS_SPACE_DATA, 1, 1, execute_map);
	debug_console_register_command(machine, "mapi",		CMDFLAG_NONE, ADDRESS_SPACE_IO, 1, 1, execute_map);
	debug_console_register_command(machine, "memdump",	CMDFLAG_NONE, 0, 0, 1, execute_memdump);

	debug_console_register_command(machine, "symlist",	CMDFLAG_NONE, 0, 0, 1, execute_symlist);

	debug_console_register_command(machine, "softreset",	CMDFLAG_NONE, 0, 0, 1, execute_softreset);
	debug_console_register_command(machine, "hardreset",	CMDFLAG_NONE, 0, 0, 1, execute_hardreset);

	/* ask all the CPUs if they would like to register functions or symbols */
	for (cpu = machine->firstcpu; cpu != NULL; cpu = cpu_next(cpu))
	{
		cpu_debug_init_func debug_init;
		debug_init = (cpu_debug_init_func)device_get_info_fct(cpu, CPUINFO_FCT_DEBUG_INIT);
		if (debug_init != NULL)
			(*debug_init)(cpu);
	}

	add_exit_callback(machine, debug_command_exit);

	/* set up the initial debugscript if specified */
	name = options_get_string(mame_options(), OPTION_DEBUGSCRIPT);
	if (name[0] != 0)
		debug_cpu_source_script(machine, name);
}


/*-------------------------------------------------
    debug_command_exit - exit-time cleanup
-------------------------------------------------*/

static void debug_command_exit(running_machine *machine)
{
	const device_config *cpu;

	/* turn off all traces */
	for (cpu = machine->firstcpu; cpu != NULL; cpu = cpu_next(cpu))
		debug_cpu_trace(cpu, NULL, 0, NULL);

	if (cheat.length)
		free(cheat.cheatmap);
}



/***************************************************************************
    GLOBAL FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    execute_min - return the minimum of two values
-------------------------------------------------*/

static UINT64 execute_min(void *globalref, void *ref, UINT32 params, const UINT64 *param)
{
	return (param[0] < param[1]) ? param[0] : param[1];
}


/*-------------------------------------------------
    execute_max - return the maximum of two values
-------------------------------------------------*/

static UINT64 execute_max(void *globalref, void *ref, UINT32 params, const UINT64 *param)
{
	return (param[0] > param[1]) ? param[0] : param[1];
}


/*-------------------------------------------------
    execute_if - if (a) return b; else return c;
-------------------------------------------------*/

static UINT64 execute_if(void *globalref, void *ref, UINT32 params, const UINT64 *param)
{
	return param[0] ? param[1] : param[2];
}



/***************************************************************************
    GLOBAL ACCESSORS
***************************************************************************/

/*-------------------------------------------------
    global_get - symbol table getter for globals
-------------------------------------------------*/

static UINT64 global_get(void *globalref, void *ref)
{
	global_entry *global = (global_entry *)ref;
	switch (global->size)
	{
		case 1:		return *(UINT8 *)global->base;
		case 2:		return *(UINT16 *)global->base;
		case 4:		return *(UINT32 *)global->base;
		case 8:		return *(UINT64 *)global->base;
	}
	return ~0;
}


/*-------------------------------------------------
    global_set - symbol table setter for globals
-------------------------------------------------*/

static void global_set(void *globalref, void *ref, UINT64 value)
{
	global_entry *global = (global_entry *)ref;
	switch (global->size)
	{
		case 1:		*(UINT8 *)global->base = value;	break;
		case 2:		*(UINT16 *)global->base = value;	break;
		case 4:		*(UINT32 *)global->base = value;	break;
		case 8:		*(UINT64 *)global->base = value;	break;
	}
}



/***************************************************************************
    PARAMETER VALIDATION HELPERS
***************************************************************************/

/*-------------------------------------------------
    debug_command_parameter_number - validates a
    number parameter
-------------------------------------------------*/

int debug_command_parameter_number(running_machine *machine, const char *param, UINT64 *result)
{
	EXPRERR err;

	/* NULL parameter does nothing and returns no error */
	if (param == NULL)
		return TRUE;

	/* evaluate the expression; success if no error */
	err = expression_evaluate(param, debug_cpu_get_visible_symtable(machine), &debug_expression_callbacks, machine, result);
	if (err == EXPRERR_NONE)
		return TRUE;

	/* print an error pointing to the character that caused it */
	debug_console_printf(machine, "Error in expression: %s\n", param);
	debug_console_printf(machine, "                     %*s^", EXPRERR_ERROR_OFFSET(err), "");
	debug_console_printf(machine, "%s\n", exprerr_to_string(err));
	return FALSE;
}


/*-------------------------------------------------
    debug_command_parameter_cpu - validates a
    parameter as a cpu
-------------------------------------------------*/

int debug_command_parameter_cpu(running_machine *machine, const char *param, const device_config **result)
{
	UINT64 cpunum;
	EXPRERR err;

	/* if no parameter, use the visible CPU */
	if (param == NULL)
	{
		*result = debug_cpu_get_visible_cpu(machine);
		if (*result == NULL)
		{
			debug_console_printf(machine, "No valid CPU is currently selected\n");
			return FALSE;
		}
		return TRUE;
	}

	/* first look for a tag match */
	*result = cputag_get_cpu(machine, param);
	if (*result != NULL)
		return TRUE;

	/* then evaluate as an expression; on an error assume it was a tag */
	err = expression_evaluate(param, debug_cpu_get_visible_symtable(machine), &debug_expression_callbacks, machine, &cpunum);
	if (err != EXPRERR_NONE)
	{
		debug_console_printf(machine, "Unable to find CPU '%s'\n", param);
		return FALSE;
	}

	/* if we got a valid one, return */
	*result = device_list_find_by_index(machine->config->devicelist, CPU, cpunum);
	if (*result != NULL)
		return TRUE;

	/* if out of range, complain */
	debug_console_printf(machine, "Invalid CPU index %d\n", (UINT32)cpunum);
	return FALSE;
}


/*-------------------------------------------------
    debug_command_parameter_cpu_space - validates
    a parameter as a cpu and retrieves the given
    address space
-------------------------------------------------*/

int debug_command_parameter_cpu_space(running_machine *machine, const char *param, int spacenum, const address_space **result)
{
	const device_config *cpu;

	/* first do the standard CPU thing */
	if (!debug_command_parameter_cpu(machine, param, &cpu))
		return FALSE;

	/* fetch the space pointer */
	*result = cpu_get_address_space(cpu, spacenum);
	if (*result == NULL)
	{
		debug_console_printf(machine, "No %s memory space found for CPU '%s'\n", address_space_names[spacenum], cpu->tag);
		return FALSE;
	}
	return TRUE;
}


/*-------------------------------------------------
    debug_command_parameter_expression - validates
    an expression parameter
-------------------------------------------------*/

static int debug_command_parameter_expression(running_machine *machine, const char *param, parsed_expression **result)
{
	EXPRERR err;

	/* NULL parameter does nothing and returns no error */
	if (param == NULL)
	{
		*result = NULL;
		return TRUE;
	}

	/* parse the expression; success if no error */
	err = expression_parse(param, debug_cpu_get_visible_symtable(machine), &debug_expression_callbacks, machine, result);
	if (err == EXPRERR_NONE)
		return TRUE;

	/* output an error */
	debug_console_printf(machine, "Error in expression: %s\n", param);
	debug_console_printf(machine, "                     %*s^", EXPRERR_ERROR_OFFSET(err), "");
	debug_console_printf(machine, "%s\n", exprerr_to_string(err));
	return FALSE;
}


/*-------------------------------------------------
    debug_command_parameter_command - validates a
    command parameter
-------------------------------------------------*/

static int debug_command_parameter_command(running_machine *machine, const char *param)
{
	CMDERR err;

	/* NULL parameter does nothing and returns no error */
	if (param == NULL)
		return TRUE;

	/* validate the comment; success if no error */
	err = debug_console_validate_command(machine, param);
	if (err == CMDERR_NONE)
		return TRUE;

	/* output an error */
	debug_console_printf(machine, "Error in command: %s\n", param);
	debug_console_printf(machine, "                  %*s^", CMDERR_ERROR_OFFSET(err), "");
	debug_console_printf(machine, "%s\n", debug_cmderr_to_string(err));
	return 0;
}



/***************************************************************************
    COMMAND HELPERS
***************************************************************************/

/*-------------------------------------------------
    execute_help - execute the help command
-------------------------------------------------*/

static void execute_help(running_machine *machine, int ref, int params, const char *param[])
{
	if (params == 0)
		debug_console_printf_wrap(machine, 80, "%s\n", debug_get_help(""));
	else
		debug_console_printf_wrap(machine, 80, "%s\n", debug_get_help(param[0]));
}


/*-------------------------------------------------
    execute_print - execute the print command
-------------------------------------------------*/

static void execute_print(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 values[MAX_COMMAND_PARAMS];
	int i;

	/* validate the other parameters */
	for (i = 0; i < params; i++)
		if (!debug_command_parameter_number(machine, param[i], &values[i]))
			return;

	/* then print each one */
	for (i = 0; i < params; i++)
		debug_console_printf(machine, "%s", core_i64_hex_format(values[i], 0));
	debug_console_printf(machine, "\n");
}


/*-------------------------------------------------
    mini_printf - safe printf to a buffer
-------------------------------------------------*/

static int mini_printf(running_machine *machine, char *buffer, const char *format, int params, UINT64 *param)
{
	const char *f = format;
	char *p = buffer;

	/* parse the string looking for % signs */
	for (;;)
	{
		char c = *f++;
		if (!c) break;

		/* escape sequences */
		if (c == '\\')
		{
			c = *f++;
			if (!c) break;
			switch (c)
			{
				case '\\':	*p++ = c;		break;
				case 'n':	*p++ = '\n';	break;
				default:					break;
			}
			continue;
		}

		/* formatting */
		else if (c == '%')
		{
			int width = 0;
			int zerofill = 0;

			/* parse out the width */
			for (;;)
			{
				c = *f++;
				if (!c || c < '0' || c > '9') break;
				if (c == '0' && width == 0)
					zerofill = 1;
				width = width * 10 + (c - '0');
			}
			if (!c) break;

			/* get the format */
			switch (c)
			{
				case '%':
					*p++ = c;
					break;

				case 'X':
				case 'x':
					if (params == 0)
					{
						debug_console_printf(machine, "Not enough parameters for format!\n");
						return 0;
					}
					if ((UINT32)(*param >> 32) != 0)
						p += sprintf(p, zerofill ? "%0*X" : "%*X", (width <= 8) ? 1 : width - 8, (UINT32)(*param >> 32));
					else if (width > 8)
						p += sprintf(p, zerofill ? "%0*X" : "%*X", width - 8, 0);
					p += sprintf(p, zerofill ? "%0*X" : "%*X", (width < 8) ? width : 8, (UINT32)*param);
					param++;
					params--;
					break;

				case 'D':
				case 'd':
					if (params == 0)
					{
						debug_console_printf(machine, "Not enough parameters for format!\n");
						return 0;
					}
					p += sprintf(p, zerofill ? "%0*d" : "%*d", width, (UINT32)*param);
					param++;
					params--;
					break;
			}
		}

		/* normal stuff */
		else
			*p++ = c;
	}

	/* NULL-terminate and exit */
	*p = 0;
	return 1;
}


/*-------------------------------------------------
    execute_printf - execute the printf command
-------------------------------------------------*/

static void execute_printf(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 values[MAX_COMMAND_PARAMS];
	char buffer[1024];
	int i;

	/* validate the other parameters */
	for (i = 1; i < params; i++)
		if (!debug_command_parameter_number(machine, param[i], &values[i]))
			return;

	/* then do a printf */
	if (mini_printf(machine, buffer, param[0], params - 1, &values[1]))
		debug_console_printf(machine, "%s\n", buffer);
}


/*-------------------------------------------------
    execute_logerror - execute the logerror command
-------------------------------------------------*/

static void execute_logerror(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 values[MAX_COMMAND_PARAMS];
	char buffer[1024];
	int i;

	/* validate the other parameters */
	for (i = 1; i < params; i++)
		if (!debug_command_parameter_number(machine, param[i], &values[i]))
			return;

	/* then do a printf */
	if (mini_printf(machine, buffer, param[0], params - 1, &values[1]))
		logerror("%s", buffer);
}


/*-------------------------------------------------
    execute_tracelog - execute the tracelog command
-------------------------------------------------*/

static void execute_tracelog(running_machine *machine, int ref, int params, const char *param[])
{
	FILE *file = cpu_get_debug_data(debug_cpu_get_visible_cpu(machine))->trace.file;
	UINT64 values[MAX_COMMAND_PARAMS];
	char buffer[1024];
	int i;

	/* if no tracefile, skip */
	if (!file)
		return;

	/* validate the other parameters */
	for (i = 1; i < params; i++)
		if (!debug_command_parameter_number(machine, param[i], &values[i]))
			return;

	/* then do a printf */
	if (mini_printf(machine, buffer, param[0], params - 1, &values[1]))
		fprintf(file, "%s", buffer);
}


/*-------------------------------------------------
    execute_quit - execute the quit command
-------------------------------------------------*/

static void execute_quit(running_machine *machine, int ref, int params, const char *param[])
{
	mame_printf_error("Exited via the debugger\n");
	mame_schedule_exit(machine);
}


/*-------------------------------------------------
    execute_do - execute the do command
-------------------------------------------------*/

static void execute_do(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 dummy;
	debug_command_parameter_number(machine, param[0], &dummy);
}


/*-------------------------------------------------
    execute_step - execute the step command
-------------------------------------------------*/

static void execute_step(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 steps = 1;

	/* if we have a parameter, use it instead */
	if (!debug_command_parameter_number(machine, param[0], &steps))
		return;

	debug_cpu_single_step(machine, steps);
}


/*-------------------------------------------------
    execute_over - execute the over command
-------------------------------------------------*/

static void execute_over(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 steps = 1;

	/* if we have a parameter, use it instead */
	if (!debug_command_parameter_number(machine, param[0], &steps))
		return;

	debug_cpu_single_step_over(machine, steps);
}


/*-------------------------------------------------
    execute_out - execute the out command
-------------------------------------------------*/

static void execute_out(running_machine *machine, int ref, int params, const char *param[])
{
	debug_cpu_single_step_out(machine);
}


/*-------------------------------------------------
    execute_go - execute the go command
-------------------------------------------------*/

static void execute_go(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 addr = ~0;

	/* if we have a parameter, use it instead */
	if (!debug_command_parameter_number(machine, param[0], &addr))
		return;

	debug_cpu_go(machine, addr);
}


/*-------------------------------------------------
    execute_go_vblank - execute the govblank
    command
-------------------------------------------------*/

static void execute_go_vblank(running_machine *machine, int ref, int params, const char *param[])
{
	debug_cpu_go_vblank(machine);
}


/*-------------------------------------------------
    execute_go_interrupt - execute the goint command
-------------------------------------------------*/

static void execute_go_interrupt(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 irqline = -1;

	/* if we have a parameter, use it instead */
	if (!debug_command_parameter_number(machine, param[0], &irqline))
		return;

	debug_cpu_go_interrupt(machine, irqline);
}


/*-------------------------------------------------
    execute_go_time - execute the gtime command
-------------------------------------------------*/

static void execute_go_time(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 milliseconds = -1;

	/* if we have a parameter, use it instead */
	if (!debug_command_parameter_number(machine, param[0], &milliseconds))
		return;

	debug_cpu_go_milliseconds(machine, milliseconds);
}


/*-------------------------------------------------
    execute_next - execute the next command
-------------------------------------------------*/

static void execute_next(running_machine *machine, int ref, int params, const char *param[])
{
	debug_cpu_next_cpu(machine);
}


/*-------------------------------------------------
    execute_focus - execute the focus command
-------------------------------------------------*/

static void execute_focus(running_machine *machine, int ref, int params, const char *param[])
{
	const device_config *scancpu;
	const device_config *cpu;

	/* validate params */
	if (!debug_command_parameter_cpu(machine, param[0], &cpu))
		return;

	/* first clear the ignore flag on the focused CPU */
	debug_cpu_ignore_cpu(cpu, 0);

	/* then loop over CPUs and set the ignore flags on all other CPUs */
	for (scancpu = machine->firstcpu; scancpu != NULL; scancpu = cpu_next(scancpu))
		if (scancpu != cpu)
			debug_cpu_ignore_cpu(scancpu, 1);
	debug_console_printf(machine, "Now focused on CPU '%s'\n", cpu->tag);
}


/*-------------------------------------------------
    execute_ignore - execute the ignore command
-------------------------------------------------*/

static void execute_ignore(running_machine *machine, int ref, int params, const char *param[])
{
	const device_config *cpuwhich[MAX_COMMAND_PARAMS];
	const device_config *cpu;
	int paramnum;
	char buffer[100];
	int buflen = 0;

	/* if there are no parameters, dump the ignore list */
	if (params == 0)
	{
		/* loop over all CPUs */
		for (cpu = machine->firstcpu; cpu != NULL; cpu = cpu_next(cpu))
		{
			const cpu_debug_data *cpuinfo = cpu_get_debug_data(cpu);

			/* build up a comma-separated list */
			if ((cpuinfo->flags & DEBUG_FLAG_OBSERVING) == 0)
			{
				if (buflen == 0)
					buflen += sprintf(&buffer[buflen], "Currently ignoring CPU '%s'", cpu->tag);
				else
					buflen += sprintf(&buffer[buflen], ", '%s'", cpu->tag);
			}
		}

		/* special message for none */
		if (buflen == 0)
			sprintf(&buffer[buflen], "Not currently ignoring any CPUs");
		debug_console_printf(machine, "%s\n", buffer);
	}

	/* otherwise clear the ignore flag on all requested CPUs */
	else
	{
		/* validate parameters */
		for (paramnum = 0; paramnum < params; paramnum++)
			if (!debug_command_parameter_cpu(machine, param[paramnum], &cpuwhich[paramnum]))
				return;

		/* set the ignore flags */
		for (paramnum = 0; paramnum < params; paramnum++)
		{
			/* make sure this isn't the last live CPU */
			for (cpu = machine->firstcpu; cpu != NULL; cpu = cpu_next(cpu))
				if (cpu != cpuwhich[paramnum] && (cpu_get_debug_data(cpu)->flags & DEBUG_FLAG_OBSERVING) != 0)
					break;
			if (cpu == NULL)
			{
				debug_console_printf(machine, "Can't ignore all CPUs!\n");
				return;
			}

			debug_cpu_ignore_cpu(cpuwhich[paramnum], 1);
			debug_console_printf(machine, "Now ignoring CPU '%s'\n", cpuwhich[paramnum]->tag);
		}
	}
}


/*-------------------------------------------------
    execute_observe - execute the observe command
-------------------------------------------------*/

static void execute_observe(running_machine *machine, int ref, int params, const char *param[])
{
	const device_config *cpuwhich[MAX_COMMAND_PARAMS];
	const device_config *cpu;
	int paramnum;
	char buffer[100];
	int buflen = 0;

	/* if there are no parameters, dump the ignore list */
	if (params == 0)
	{
		/* loop over all CPUs */
		for (cpu = machine->firstcpu; cpu != NULL; cpu = cpu_next(cpu))
		{
			const cpu_debug_data *cpuinfo = cpu_get_debug_data(cpu);

			/* build up a comma-separated list */
			if ((cpuinfo->flags & DEBUG_FLAG_OBSERVING) != 0)
			{
				if (buflen == 0)
					buflen += sprintf(&buffer[buflen], "Currently observing CPU '%s'", cpu->tag);
				else
					buflen += sprintf(&buffer[buflen], ", '%s'", cpu->tag);
			}
		}

		/* special message for none */
		if (buflen == 0)
			buflen += sprintf(&buffer[buflen], "Not currently observing any CPUs");
		debug_console_printf(machine, "%s\n", buffer);
	}

	/* otherwise set the ignore flag on all requested CPUs */
	else
	{
		/* validate parameters */
		for (paramnum = 0; paramnum < params; paramnum++)
			if (!debug_command_parameter_cpu(machine, param[paramnum], &cpuwhich[paramnum]))
				return;

		/* clear the ignore flags */
		for (paramnum = 0; paramnum < params; paramnum++)
		{
			debug_cpu_ignore_cpu(cpuwhich[paramnum], 0);
			debug_console_printf(machine, "Now observing CPU '%s'\n", cpuwhich[paramnum]->tag);
		}
	}
}


/*-------------------------------------------------
    execute_comment - add a comment to a line
-------------------------------------------------*/

static void execute_comment(running_machine *machine, int ref, int params, const char *param[])
{
	const device_config *cpu;
	UINT64 address;

	/* param 1 is the address for the comment */
	if (!debug_command_parameter_number(machine, param[0], &address))
		return;

	/* CPU parameter is implicit */
	if (!debug_command_parameter_cpu(machine, NULL, &cpu))
		return;

	/* make sure param 2 exists */
	if (strlen(param[1]) == 0)
	{
		debug_console_printf(machine, "Error : comment text empty\n");
		return;
	}

	/* Now try adding the comment */
	debug_comment_add(cpu, address, param[1], 0x00ff0000, debug_comment_get_opcode_crc32(cpu, address));
	debug_view_update_type(cpu->machine, DVT_DISASSEMBLY);
}


/*------------------------------------------------------
    execute_comment_del - remove a comment from an addr
--------------------------------------------------------*/

static void execute_comment_del(running_machine *machine, int ref, int params, const char *param[])
{
	const device_config *cpu;
	UINT64 address;

	/* param 1 can either be a command or the address for the comment */
	if (!debug_command_parameter_number(machine, param[0], &address))
		return;

	/* CPU parameter is implicit */
	if (!debug_command_parameter_cpu(machine, NULL, &cpu))
		return;

	/* If it's a number, it must be an address */
	/* The bankoff and cbn will be pulled from what's currently active */
	debug_comment_remove(cpu, address, debug_comment_get_opcode_crc32(cpu, address));
	debug_view_update_type(cpu->machine, DVT_DISASSEMBLY);
}


/*-------------------------------------------------
    execute_comment - add a comment to a line
-------------------------------------------------*/

static void execute_comment_save(running_machine *machine, int ref, int params, const char *param[])
{
	if (debug_comment_save(machine))
		debug_console_printf(machine, "Comments successfully saved\n");
}


/*-------------------------------------------------
    execute_bpset - execute the breakpoint set
    command
-------------------------------------------------*/

static void execute_bpset(running_machine *machine, int ref, int params, const char *param[])
{
	parsed_expression *condition = NULL;
	const device_config *cpu;
	const char *action = NULL;
	UINT64 address;
	int bpnum;

	/* param 1 is the address */
	if (!debug_command_parameter_number(machine, param[0], &address))
		return;

	/* param 2 is the condition */
	if (!debug_command_parameter_expression(machine, param[1], &condition))
		return;

	/* param 3 is the action */
	if (!debug_command_parameter_command(machine, action = param[2]))
		return;

	/* CPU is implicit */
	if (!debug_command_parameter_cpu(machine, NULL, &cpu))
		return;

	/* set the breakpoint */
	bpnum = debug_cpu_breakpoint_set(cpu, address, condition, action);
	debug_console_printf(machine, "Breakpoint %X set\n", bpnum);
}


/*-------------------------------------------------
    execute_bpclear - execute the breakpoint
    clear command
-------------------------------------------------*/

static void execute_bpclear(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 bpindex;

	/* if 0 parameters, clear all */
	if (params == 0)
	{
		const device_config *cpu;

		for (cpu = machine->firstcpu; cpu != NULL; cpu = cpu_next(cpu))
		{
			const cpu_debug_data *cpuinfo = cpu_get_debug_data(cpu);
			debug_cpu_breakpoint *bp;
			while ((bp = cpuinfo->bplist) != NULL)
				debug_cpu_breakpoint_clear(machine, bp->index);
		}
		debug_console_printf(machine, "Cleared all breakpoints\n");
	}

	/* otherwise, clear the specific one */
	else if (!debug_command_parameter_number(machine, param[0], &bpindex))
		return;
	else
	{
		int found = debug_cpu_breakpoint_clear(machine, bpindex);
		if (found)
			debug_console_printf(machine, "Breakpoint %X cleared\n", (UINT32)bpindex);
		else
			debug_console_printf(machine, "Invalid breakpoint number %X\n", (UINT32)bpindex);
	}
}


/*-------------------------------------------------
    execute_bpdisenable - execute the breakpoint
    disable/enable commands
-------------------------------------------------*/

static void execute_bpdisenable(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 bpindex;

	/* if 0 parameters, clear all */
	if (params == 0)
	{
		const device_config *cpu;

		for (cpu = machine->firstcpu; cpu != NULL; cpu = cpu_next(cpu))
		{
			const cpu_debug_data *cpuinfo = cpu_get_debug_data(cpu);
			debug_cpu_breakpoint *bp;
			for (bp = cpuinfo->bplist; bp != NULL; bp = bp->next)
				debug_cpu_breakpoint_enable(machine, bp->index, ref);
		}
		if (ref == 0)
			debug_console_printf(machine, "Disabled all breakpoints\n");
		else
			debug_console_printf(machine, "Enabled all breakpoints\n");
	}

	/* otherwise, clear the specific one */
	else if (!debug_command_parameter_number(machine, param[0], &bpindex))
		return;
	else
	{
		int found = debug_cpu_breakpoint_enable(machine, bpindex, ref);
		if (found)
			debug_console_printf(machine, "Breakpoint %X %s\n", (UINT32)bpindex, ref ? "enabled" : "disabled");
		else
			debug_console_printf(machine, "Invalid breakpoint number %X\n", (UINT32)bpindex);
	}
}


/*-------------------------------------------------
    execute_bplist - execute the breakpoint list
    command
-------------------------------------------------*/

static void execute_bplist(running_machine *machine, int ref, int params, const char *param[])
{
	const device_config *cpu;
	int printed = 0;
	char buffer[256];

	/* loop over all CPUs */
	for (cpu = machine->firstcpu; cpu != NULL; cpu = cpu_next(cpu))
	{
		const cpu_debug_data *cpuinfo = cpu_get_debug_data(cpu);
		const address_space *space = cpu_get_address_space(cpu, ADDRESS_SPACE_PROGRAM);

		if (cpuinfo->bplist != NULL)
		{
			debug_cpu_breakpoint *bp;

			debug_console_printf(machine, "CPU '%s' breakpoints:\n", cpu->tag);

			/* loop over the breakpoints */
			for (bp = cpuinfo->bplist; bp != NULL; bp = bp->next)
			{
				int buflen;
				buflen = sprintf(buffer, "%c%4X @ %s", bp->enabled ? ' ' : 'D', bp->index, core_i64_hex_format(bp->address, space->logaddrchars));
				if (bp->condition)
					buflen += sprintf(&buffer[buflen], " if %s", expression_original_string(bp->condition));
				if (bp->action)
					buflen += sprintf(&buffer[buflen], " do %s", bp->action);
				debug_console_printf(machine, "%s\n", buffer);
				printed++;
			}
		}
	}

	if (!printed)
		debug_console_printf(machine, "No breakpoints currently installed\n");
}


/*-------------------------------------------------
    execute_wpset - execute the watchpoint set
    command
-------------------------------------------------*/

static void execute_wpset(running_machine *machine, int ref, int params, const char *param[])
{
	parsed_expression *condition = NULL;
	const address_space *space;
	const char *action = NULL;
	UINT64 address, length;
	int type = 0;
	int wpnum;

	/* param 1 is the address */
	if (!debug_command_parameter_number(machine, param[0], &address))
		return;

	/* param 2 is the length */
	if (!debug_command_parameter_number(machine, param[1], &length))
		return;

	/* param 3 is the type */
	if (!strcmp(param[2], "r"))
		type = WATCHPOINT_READ;
	else if (!strcmp(param[2], "w"))
		type = WATCHPOINT_WRITE;
	else if (!strcmp(param[2], "rw") || !strcmp(param[2], "wr"))
		type = WATCHPOINT_READWRITE;
	else
	{
		debug_console_printf(machine, "Invalid watchpoint type: expected r, w, or rw\n");
		return;
	}

	/* param 4 is the condition */
	if (!debug_command_parameter_expression(machine, param[3], &condition))
		return;

	/* param 5 is the action */
	if (!debug_command_parameter_command(machine, action = param[4]))
		return;

	/* CPU is implicit */
	if (!debug_command_parameter_cpu_space(machine, NULL, ref, &space))
		return;

	/* set the watchpoint */
	wpnum = debug_cpu_watchpoint_set(space, type, address, length, condition, action);
	debug_console_printf(machine, "Watchpoint %X set\n", wpnum);
}


/*-------------------------------------------------
    execute_wpclear - execute the watchpoint
    clear command
-------------------------------------------------*/

static void execute_wpclear(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 wpindex;

	/* if 0 parameters, clear all */
	if (params == 0)
	{
		const device_config *cpu;

		for (cpu = machine->firstcpu; cpu != NULL; cpu = cpu_next(cpu))
		{
			const cpu_debug_data *cpuinfo = cpu_get_debug_data(cpu);
			int spacenum;

			for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
			{
				debug_cpu_watchpoint *wp;
				while ((wp = cpuinfo->wplist[spacenum]) != NULL)
					debug_cpu_watchpoint_clear(machine, wp->index);
			}
		}
		debug_console_printf(machine, "Cleared all watchpoints\n");
	}

	/* otherwise, clear the specific one */
	else if (!debug_command_parameter_number(machine, param[0], &wpindex))
		return;
	else
	{
		int found = debug_cpu_watchpoint_clear(machine, wpindex);
		if (found)
			debug_console_printf(machine, "Watchpoint %X cleared\n", (UINT32)wpindex);
		else
			debug_console_printf(machine, "Invalid watchpoint number %X\n", (UINT32)wpindex);
	}
}


/*-------------------------------------------------
    execute_wpdisenable - execute the watchpoint
    disable/enable commands
-------------------------------------------------*/

static void execute_wpdisenable(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 wpindex;

	/* if 0 parameters, clear all */
	if (params == 0)
	{
		const device_config *cpu;

		for (cpu = machine->firstcpu; cpu != NULL; cpu = cpu_next(cpu))
		{
			const cpu_debug_data *cpuinfo = cpu_get_debug_data(cpu);
			int spacenum;

			for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
			{
				debug_cpu_watchpoint *wp;
				for (wp = cpuinfo->wplist[spacenum]; wp != NULL; wp = wp->next)
					debug_cpu_watchpoint_enable(machine, wp->index, ref);
			}
		}
		if (ref == 0)
			debug_console_printf(machine, "Disabled all watchpoints\n");
		else
			debug_console_printf(machine, "Enabled all watchpoints\n");
	}

	/* otherwise, clear the specific one */
	else if (!debug_command_parameter_number(machine, param[0], &wpindex))
		return;
	else
	{
		int found = debug_cpu_watchpoint_enable(machine, wpindex, ref);
		if (found)
			debug_console_printf(machine, "Watchpoint %X %s\n", (UINT32)wpindex, ref ? "enabled" : "disabled");
		else
			debug_console_printf(machine, "Invalid watchpoint number %X\n", (UINT32)wpindex);
	}
}


/*-------------------------------------------------
    execute_wplist - execute the watchpoint list
    command
-------------------------------------------------*/

static void execute_wplist(running_machine *machine, int ref, int params, const char *param[])
{
	const device_config *cpu;
	int printed = 0;
	char buffer[256];

	/* loop over all CPUs */
	for (cpu = machine->firstcpu; cpu != NULL; cpu = cpu_next(cpu))
	{
		const cpu_debug_data *cpuinfo = cpu_get_debug_data(cpu);
		int spacenum;

		for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
			if (cpuinfo->wplist[spacenum] != NULL)
			{
				static const char *const types[] = { "unkn ", "read ", "write", "r/w  " };
				const address_space *space = cpu_get_address_space(cpu, spacenum);
				debug_cpu_watchpoint *wp;

				debug_console_printf(machine, "CPU '%s' %s space watchpoints:\n", cpu->tag, address_space_names[spacenum]);

				/* loop over the watchpoints */
				for (wp = cpuinfo->wplist[spacenum]; wp != NULL; wp = wp->next)
				{
					int buflen;
					buflen = sprintf(buffer, "%c%4X @ %s-%s %s", wp->enabled ? ' ' : 'D', wp->index,
							core_i64_hex_format(memory_byte_to_address(space, wp->address), space->addrchars),
							core_i64_hex_format(memory_byte_to_address_end(space, wp->address + wp->length) - 1, space->addrchars),
							types[wp->type & 3]);
					if (wp->condition)
						buflen += sprintf(&buffer[buflen], " if %s", expression_original_string(wp->condition));
					if (wp->action)
						buflen += sprintf(&buffer[buflen], " do %s", wp->action);
					debug_console_printf(machine, "%s\n", buffer);
					printed++;
				}
			}
	}

	if (!printed)
		debug_console_printf(machine, "No watchpoints currently installed\n");
}


/*-------------------------------------------------
    execute_hotspot - execute the hotspot
    command
-------------------------------------------------*/

static void execute_hotspot(running_machine *machine, int ref, int params, const char *param[])
{
	const device_config *cpu;
	UINT64 threshhold;
	UINT64 count;

	/* if no params, and there are live hotspots, clear them */
	if (params == 0)
	{
		int cleared = FALSE;

		/* loop over CPUs and find live spots */
		for (cpu = machine->firstcpu; cpu != NULL; cpu = cpu_next(cpu))
		{
			const cpu_debug_data *cpuinfo = cpu_get_debug_data(cpu);

			if (cpuinfo->hotspots != NULL)
			{
				debug_cpu_hotspot_track(cpuinfo->device, 0, 0);
				debug_console_printf(machine, "Cleared hotspot tracking on CPU '%s'\n", cpu->tag);
				cleared = TRUE;
			}
		}

		/* if we cleared, we're done */
		if (cleared)
			return;
	}

	/* extract parameters */
	count = 64;
	threshhold = 250;
	if (!debug_command_parameter_cpu(machine, (params > 0) ? param[0] : NULL, &cpu))
		return;
	if (!debug_command_parameter_number(machine, param[1], &count))
		return;
	if (!debug_command_parameter_number(machine, param[2], &threshhold))
		return;

	/* attempt to install */
	if (debug_cpu_hotspot_track(cpu, count, threshhold))
		debug_console_printf(machine, "Now tracking hotspots on CPU '%s' using %d slots with a threshhold of %d\n", cpu->tag, (int)count, (int)threshhold);
	else
		debug_console_printf(machine, "Error setting up the hotspot tracking\n");
}


/*-------------------------------------------------
    execute_save - execute the save command
-------------------------------------------------*/

static void execute_save(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 offset, endoffset, length;
	const address_space *space;
	FILE *f;
	UINT64 i;

	/* validate parameters */
	if (!debug_command_parameter_number(machine, param[1], &offset))
		return;
	if (!debug_command_parameter_number(machine, param[2], &length))
		return;
	if (!debug_command_parameter_cpu_space(machine, (params > 3) ? param[3] : NULL, ref, &space))
		return;

	/* determine the addresses to write */
	endoffset = memory_address_to_byte(space, offset + length - 1) & space->bytemask;
	offset = memory_address_to_byte(space, offset) & space->bytemask;

	/* open the file */
	f = fopen(param[0], "wb");
	if (!f)
	{
		debug_console_printf(machine, "Error opening file '%s'\n", param[0]);
		return;
	}

	/* now write the data out */
	for (i = offset; i <= endoffset; i++)
	{
		UINT8 byte = debug_read_byte(space, i, TRUE);
		fwrite(&byte, 1, 1, f);
	}

	/* close the file */
	fclose(f);
	debug_console_printf(machine, "Data saved successfully\n");
}


/*-------------------------------------------------
    execute_dump - execute the dump command
-------------------------------------------------*/

static void execute_dump(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 offset, endoffset, length, width = 0, ascii = 1;
	const address_space *space;
	FILE *f = NULL;
	UINT64 i, j;

	/* validate parameters */
	if (!debug_command_parameter_number(machine, param[1], &offset))
		return;
	if (!debug_command_parameter_number(machine, param[2], &length))
		return;
	if (!debug_command_parameter_number(machine, param[3], &width))
		return;
	if (!debug_command_parameter_number(machine, param[4], &ascii))
		return;
	if (!debug_command_parameter_cpu_space(machine, (params > 5) ? param[5] : NULL, ref, &space))
		return;

	/* further validation */
	if (width == 0)
		width = space->dbits / 8;
	if (width < memory_address_to_byte(space, 1))
		width = memory_address_to_byte(space, 1);
	if (width != 1 && width != 2 && width != 4 && width != 8)
	{
		debug_console_printf(machine, "Invalid width! (must be 1,2,4 or 8)\n");
		return;
	}
	endoffset = memory_address_to_byte(space, offset + length - 1) & space->bytemask;
	offset = memory_address_to_byte(space, offset) & space->bytemask;

	/* open the file */
	f = fopen(param[0], "w");
	if (!f)
	{
		debug_console_printf(machine, "Error opening file '%s'\n", param[0]);
		return;
	}

	/* now write the data out */
	for (i = offset; i <= endoffset; i += 16)
	{
		char output[200];
		int outdex = 0;

		/* print the address */
		outdex += sprintf(&output[outdex], "%s: ", core_i64_hex_format((UINT32)memory_byte_to_address(space, i), space->logaddrchars));

		/* print the bytes */
		for (j = 0; j < 16; j += width)
		{
			if (i + j <= endoffset)
			{
				offs_t curaddr = i + j;
				if (debug_cpu_translate(space, TRANSLATE_READ_DEBUG, &curaddr))
				{
					UINT64 value = debug_read_memory(space, i + j, width, TRUE);
					outdex += sprintf(&output[outdex], " %s", core_i64_hex_format(value, width * 2));
				}
				else
					outdex += sprintf(&output[outdex], " %.*s", (int)width * 2, "****************");
			}
			else
				outdex += sprintf(&output[outdex], " %*s", (int)width * 2, "");
		}

		/* print the ASCII */
		if (ascii)
		{
			outdex += sprintf(&output[outdex], "  ");
			for (j = 0; j < 16 && (i + j) <= endoffset; j++)
			{
				offs_t curaddr = i + j;
				if (debug_cpu_translate(space, TRANSLATE_READ_DEBUG, &curaddr))
				{
					UINT8 byte = debug_read_byte(space, i + j, TRUE);
					outdex += sprintf(&output[outdex], "%c", (byte >= 32 && byte < 128) ? byte : '.');
				}
				else
					outdex += sprintf(&output[outdex], " ");
			}
		}

		/* output the result */
		fprintf(f, "%s\n", output);
	}

	/* close the file */
	fclose(f);
	debug_console_printf(machine, "Data dumped successfully\n");
}


/*-------------------------------------------------
   execute_cheatinit - initialize the cheat system
-------------------------------------------------*/

static void execute_cheatinit(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 offset, endoffset, length = 0, real_length = 0;
	const address_space *space;
	UINT32 active_cheat = 0;
	UINT64 curaddr;

	/* validate parameters */
	if (!debug_command_parameter_cpu_space(machine, (params > 3) ? param[3] : NULL, ADDRESS_SPACE_PROGRAM, &space))
		return;

	if (ref == 0)
	{
		if (params > 0)
		{
			if (!strcmp(param[0], "sb"))
			{
				cheat.width = 1;
				cheat.signed_cheat = TRUE;
			}
			else if (!strcmp(param[0], "sw"))
			{
				cheat.width = 2;
				cheat.signed_cheat = TRUE;
			}
			else if (!strcmp(param[0], "sd"))
			{
				cheat.width = 4;
				cheat.signed_cheat = TRUE;
			}
			else if (!strcmp(param[0], "sq"))
			{
				cheat.width = 8;
				cheat.signed_cheat = TRUE;
			}
			else if (!strcmp(param[0], "ub"))
			{
				cheat.width = 1;
				cheat.signed_cheat = FALSE;
			}
			else if (!strcmp(param[0], "uw"))
			{
				cheat.width = 2;
				cheat.signed_cheat = FALSE;
			}
			else if (!strcmp(param[0], "ud"))
			{
				cheat.width = 4;
				cheat.signed_cheat = FALSE;
			}
			else if (!strcmp(param[0], "uq"))
			{
				cheat.width = 8;
				cheat.signed_cheat = FALSE;
			}
			else
			{
				debug_console_printf(machine, "Invalid type: expected ub, uw, ud, uq, sb, sw, sd or sq\n");
				return;
			}
		}
		else
		{
			cheat.width = 1;
			cheat.signed_cheat = FALSE;
		}
	}

	if (params > 1)
	{
		if (!debug_command_parameter_number(machine, param[(ref == 0) ? 1 : 0], &offset))
			return;
		if (!debug_command_parameter_number(machine, param[(ref == 0) ? 2 : 1], &length))
			return;
	}
	else
	{
		/* initialize entire memory  */
		offset = 0;
		length = space->bytemask + 1;
	}

	endoffset = memory_address_to_byte(space, offset + length - 1) & space->bytemask;
	offset = memory_address_to_byte(space, offset) & space->bytemask;

	/* counts the writable bytes in the area */
	for (curaddr = offset; curaddr <= endoffset; curaddr += cheat.width)
		if (cheat_address_is_valid(space, curaddr))
			real_length++;

	if (real_length == 0)
	{
		debug_console_printf(machine, "No writable bytes found in this area\n");
		return;
	}

	if (ref == 0)
	{
		/* initialize new cheat system */
		if (cheat.cheatmap != NULL)
			free(cheat.cheatmap);
		cheat.cheatmap = (cheat_map *)malloc(real_length * sizeof(cheat_map));
		if (cheat.cheatmap == NULL)
		{
			debug_console_printf(machine, "Unable of allocate the necessary memory\n");
			return;
		}

		cheat.length = real_length;
		cheat.undo = 0;
		cheat.cpu = (params > 3) ? *param[3] : '0';
	}
	else
	{
		/* add range to cheat system */
		cheat_map * cheatmap_bak = cheat.cheatmap;

		if (cheat.cpu == 0)
		{
			debug_console_printf(machine, "Use cheatinit before cheatrange\n");
			return;
		}

		if (!debug_command_parameter_cpu_space(machine, &cheat.cpu, ADDRESS_SPACE_PROGRAM, &space))
			return;

		cheat.cheatmap = (cheat_map *)realloc(cheat.cheatmap, (cheat.length + real_length) * sizeof(cheat_map));
		if (cheat.cheatmap == NULL)
		{
			cheat.cheatmap = cheatmap_bak;
			debug_console_printf(machine, "Unable of allocate the necessary memory\n");
			return;
		}

		active_cheat = cheat.length;
		cheat.length += real_length;
	}

	/* initialize cheatmap in the selected space */
	for (curaddr = offset; curaddr <= endoffset; curaddr += cheat.width)
		if (cheat_address_is_valid(space, curaddr))
		{
			cheat.cheatmap[active_cheat].previous_value = cheat_read_extended(&cheat, space, curaddr);
			cheat.cheatmap[active_cheat].first_value = cheat.cheatmap[active_cheat].previous_value;
			cheat.cheatmap[active_cheat].offset = curaddr;
			cheat.cheatmap[active_cheat].state = 1;
			cheat.cheatmap[active_cheat].undo = 0;
			active_cheat++;
		}

	debug_console_printf(machine, "%u cheat initialized\n", active_cheat);
}


/*-------------------------------------------------
    execute_cheatnext - execute the search
-------------------------------------------------*/

static void execute_cheatnext(running_machine *machine, int ref, int params, const char *param[])
{
	const address_space *space;
	UINT64 cheatindex;
	UINT32 active_cheat = 0;
	UINT8 condition;
	UINT64 comp_value = 0;

	enum
	{
		CHEAT_ALL = 0,
		CHEAT_EQUAL,
		CHEAT_NOTEQUAL,
		CHEAT_EQUALTO,
		CHEAT_NOTEQUALTO,
		CHEAT_DECREASE,
		CHEAT_INCREASE,
		CHEAT_DECREASE_OR_EQUAL,
		CHEAT_INCREASE_OR_EQUAL,
		CHEAT_DECREASEOF,
		CHEAT_INCREASEOF,
		CHEAT_SMALLEROF,
		CHEAT_GREATEROF
	};

	if (cheat.cpu == 0)
	{
		debug_console_printf(machine, "Use cheatinit before cheatnext\n");
		return;
	}

	if (!debug_command_parameter_cpu_space(machine, &cheat.cpu, ADDRESS_SPACE_PROGRAM, &space))
		return;

	if (params > 1 && !debug_command_parameter_number(machine, param[1], &comp_value))
		return;
	comp_value = cheat_sign_extend(&cheat, comp_value);

	/* decode contidion */
	if (!strcmp(param[0], "all"))
		condition = CHEAT_ALL;
	else if (!strcmp(param[0], "equal") || !strcmp(param[0], "eq"))
		condition = (params > 1) ? CHEAT_EQUALTO : CHEAT_EQUAL;
	else if (!strcmp(param[0], "notequal") || !strcmp(param[0], "ne"))
		condition = (params > 1) ? CHEAT_NOTEQUALTO : CHEAT_NOTEQUAL;
	else if (!strcmp(param[0], "decrease") || !strcmp(param[0], "de") || !strcmp(param[0], "-"))
		condition = (params > 1) ? CHEAT_DECREASEOF : CHEAT_DECREASE;
	else if (!strcmp(param[0], "increase") || !strcmp(param[0], "in") || !strcmp(param[0], "+"))
		condition = (params > 1) ? CHEAT_INCREASEOF : CHEAT_INCREASE;
	else if (!strcmp(param[0], "decreaseorequal") || !strcmp(param[0], "deeq"))
		condition = CHEAT_DECREASE_OR_EQUAL;
	else if (!strcmp(param[0], "increaseorequal") || !strcmp(param[0], "ineq"))
		condition = CHEAT_INCREASE_OR_EQUAL;
	else if (!strcmp(param[0], "smallerof") || !strcmp(param[0], "lt") || !strcmp(param[0], "<"))
		condition = CHEAT_SMALLEROF;
	else if (!strcmp(param[0], "greaterof") || !strcmp(param[0], "gt") || !strcmp(param[0], ">"))
		condition = CHEAT_GREATEROF;
	else
	{
		debug_console_printf(machine, "Invalid condition type\n");
		return;
	}

	cheat.undo++;

	/* execute the search */
	for (cheatindex = 0; cheatindex < cheat.length; cheatindex += 1)
		if (cheat.cheatmap[cheatindex].state == 1)
		{
			UINT64 cheat_value = cheat_read_extended(&cheat, space, cheat.cheatmap[cheatindex].offset);
			UINT64 comp_byte = (ref == 0) ? cheat.cheatmap[cheatindex].previous_value : cheat.cheatmap[cheatindex].first_value;
			UINT8 disable_byte = FALSE;

			switch (condition)
			{
				case CHEAT_ALL:
					break;

				case CHEAT_EQUAL:
					disable_byte = (cheat_value != comp_byte);
					break;

				case CHEAT_NOTEQUAL:
					disable_byte = (cheat_value == comp_byte);
					break;

				case CHEAT_EQUALTO:
					disable_byte = (cheat_value != comp_value);
					break;

				case CHEAT_NOTEQUALTO:
					disable_byte = (cheat_value == comp_value);
					break;

				case CHEAT_DECREASE:
					if (cheat.signed_cheat)
						disable_byte = ((INT64)cheat_value >= (INT64)comp_byte);
					else
						disable_byte = ((UINT64)cheat_value >= (UINT64)comp_byte);
					break;

				case CHEAT_INCREASE:
					if (cheat.signed_cheat)
						disable_byte = ((INT64)cheat_value <= (INT64)comp_byte);
					else
						disable_byte = ((UINT64)cheat_value <= (UINT64)comp_byte);
					break;

				case CHEAT_DECREASE_OR_EQUAL:
					if (cheat.signed_cheat)
						disable_byte = ((INT64)cheat_value > (INT64)comp_byte);
					else
						disable_byte = ((UINT64)cheat_value > (UINT64)comp_byte);
					break;

				case CHEAT_INCREASE_OR_EQUAL:
					if (cheat.signed_cheat)
						disable_byte = ((INT64)cheat_value < (INT64)comp_byte);
					else
						disable_byte = ((UINT64)cheat_value < (UINT64)comp_byte);
					break;

				case CHEAT_DECREASEOF:
					disable_byte = (cheat_value != comp_byte - comp_value);
					break;

				case CHEAT_INCREASEOF:
					disable_byte = (cheat_value != comp_byte + comp_value);
					break;

				case CHEAT_SMALLEROF:
					if (cheat.signed_cheat)
						disable_byte = ((INT64)cheat_value >= (INT64)comp_value);
					else
						disable_byte = ((UINT64)cheat_value >= (UINT64)comp_value);
					break;

				case CHEAT_GREATEROF:
					if (cheat.signed_cheat)
						disable_byte = ((INT64)cheat_value <= (INT64)comp_value);
					else
						disable_byte = ((UINT64)cheat_value <= (UINT64)comp_value);
					break;
			}

			if (disable_byte)
			{
				cheat.cheatmap[cheatindex].state = 0;
				cheat.cheatmap[cheatindex].undo = cheat.undo;
			}
			else
				active_cheat++;

			/* update previous value */
			cheat.cheatmap[cheatindex].previous_value = cheat_value;
		}

	if (active_cheat <= 5)
		execute_cheatlist(machine, 0, 0, NULL);

	debug_console_printf(machine, "%u cheats found\n", active_cheat);
}


/*-------------------------------------------------
    execute_cheatlist - show a list of active cheat
-------------------------------------------------*/

static void execute_cheatlist(running_machine *machine, int ref, int params, const char *param[])
{
	char spaceletter, sizeletter;
	const address_space *space;
	UINT32 active_cheat = 0;
	UINT64 cheatindex;
	UINT64 sizemask;
	FILE *f = NULL;

	if (!debug_command_parameter_cpu_space(machine, &cheat.cpu, ADDRESS_SPACE_PROGRAM, &space))
		return;

	if (params > 0)
		f = fopen(param[0], "w");

	switch (space->spacenum)
	{
		default:
		case ADDRESS_SPACE_PROGRAM:	spaceletter = 'p';	break;
		case ADDRESS_SPACE_DATA:	spaceletter = 'd';	break;
		case ADDRESS_SPACE_IO:		spaceletter = 'i';	break;
	}

	switch (cheat.width)
	{
		default:
		case 1:						sizeletter = 'b';	sizemask = 0xff;					break;
		case 2:						sizeletter = 'w';	sizemask = 0xffff;					break;
		case 4:						sizeletter = 'd';	sizemask = 0xffffffff;				break;
		case 8:						sizeletter = 'q';	sizemask = U64(0xffffffffffffffff);	break;
	}

	/* write the cheat list */
	for (cheatindex = 0; cheatindex < cheat.length; cheatindex += 1)
	{
		if (cheat.cheatmap[cheatindex].state == 1)
		{
			UINT64 value = cheat_read_extended(&cheat, space, cheat.cheatmap[cheatindex].offset) & sizemask;
			offs_t address = memory_byte_to_address(space, cheat.cheatmap[cheatindex].offset);

			if (params > 0)
			{
				active_cheat++;
				fprintf(f, "  <cheat desc=\"Possibility %d : %s (%s)\">\n", active_cheat, core_i64_hex_format(address, space->logaddrchars), core_i64_hex_format(value, cheat.width * 2));
				fprintf(f, "    <script state=\"run\">\n");
				fprintf(f, "      <action>maincpu.%c%c@%s=%s</action>\n", spaceletter, sizeletter, core_i64_hex_format(address, space->logaddrchars), core_i64_hex_format(cheat.cheatmap[cheatindex].first_value & sizemask, cheat.width * 2));
				fprintf(f, "    </script>\n");
				fprintf(f, "  </cheat>\n\n");
			}
			else
				debug_console_printf(machine, "Address=%s Start=%s Current=%s\n", core_i64_hex_format(address, space->logaddrchars), core_i64_hex_format(cheat.cheatmap[cheatindex].first_value & sizemask, cheat.width * 2), core_i64_hex_format(value, cheat.width * 2));
		}
	}
	if (params > 0)
		fclose(f);
}


/*-------------------------------------------------
    execute_cheatundo - undo the last search
-------------------------------------------------*/

static void execute_cheatundo(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 cheatindex;
	UINT32 undo_count = 0;

	if (cheat.undo > 0)
	{
		for (cheatindex = 0; cheatindex < cheat.length; cheatindex += 1)
		{
			if (cheat.cheatmap[cheatindex].undo == cheat.undo)
			{
				cheat.cheatmap[cheatindex].state = 1;
				cheat.cheatmap[cheatindex].undo = 0;
				undo_count++;
			}
		}

		cheat.undo--;
		debug_console_printf(machine, "%u cheat reactivated\n", undo_count);
	}
	else
		debug_console_printf(machine, "Maximum undo reached\n");
}


/*-------------------------------------------------
    execute_find - execute the find command
-------------------------------------------------*/

static void execute_find(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 offset, endoffset, length;
	const address_space *space;
	UINT64 data_to_find[256];
	UINT8 data_size[256];
	int cur_data_size;
	int data_count = 0;
	int found = 0;
	UINT64 i, j;

	/* validate parameters */
	if (!debug_command_parameter_number(machine, param[0], &offset))
		return;
	if (!debug_command_parameter_number(machine, param[1], &length))
		return;
	if (!debug_command_parameter_cpu_space(machine, NULL, ref, &space))
		return;

	/* further validation */
	endoffset = memory_address_to_byte(space, offset + length - 1) & space->bytemask;
	offset = memory_address_to_byte(space, offset) & space->bytemask;
	cur_data_size = memory_address_to_byte(space, 1);
	if (cur_data_size == 0)
		cur_data_size = 1;

	/* parse the data parameters */
	for (i = 2; i < params; i++)
	{
		const char *pdata = param[i];

		/* check for a string */
		if (pdata[0] == '"' && pdata[strlen(pdata) - 1] == '"')
		{
			for (j = 1; j < strlen(pdata) - 1; j++)
			{
				data_to_find[data_count] = pdata[j];
				data_size[data_count++] = 1;
			}
		}

		/* otherwise, validate as a number */
		else
		{
			/* check for a 'b','w','d',or 'q' prefix */
			data_size[data_count] = cur_data_size;
			if (tolower((UINT8)pdata[0]) == 'b' && pdata[1] == '.') { data_size[data_count] = cur_data_size = 1; pdata += 2; }
			if (tolower((UINT8)pdata[0]) == 'w' && pdata[1] == '.') { data_size[data_count] = cur_data_size = 2; pdata += 2; }
			if (tolower((UINT8)pdata[0]) == 'd' && pdata[1] == '.') { data_size[data_count] = cur_data_size = 4; pdata += 2; }
			if (tolower((UINT8)pdata[0]) == 'q' && pdata[1] == '.') { data_size[data_count] = cur_data_size = 8; pdata += 2; }

			/* look for a wildcard */
			if (!strcmp(pdata, "?"))
				data_size[data_count++] |= 0x10;

			/* otherwise, validate as a number */
			else if (!debug_command_parameter_number(machine, pdata, &data_to_find[data_count++]))
				return;
		}
	}

	/* now search */
	for (i = offset; i <= endoffset; i += data_size[0])
	{
		int suboffset = 0;
		int match = 1;

		/* find the entire string */
		for (j = 0; j < data_count && match; j++)
		{
			switch (data_size[j])
			{
				case 1:	match = ((UINT8)debug_read_byte(space, i + suboffset, TRUE) == (UINT8)data_to_find[j]);	break;
				case 2:	match = ((UINT16)debug_read_word(space, i + suboffset, TRUE) == (UINT16)data_to_find[j]);	break;
				case 4:	match = ((UINT32)debug_read_dword(space, i + suboffset, TRUE) == (UINT32)data_to_find[j]);	break;
				case 8:	match = ((UINT64)debug_read_qword(space, i + suboffset, TRUE) == (UINT64)data_to_find[j]);	break;
				default:	/* all other cases are wildcards */		break;
			}
			suboffset += data_size[j] & 0x0f;
		}

		/* did we find it? */
		if (match)
		{
			found++;
			debug_console_printf(machine, "Found at %s\n", core_i64_hex_format((UINT32)memory_byte_to_address(space, i), space->addrchars));
		}
	}

	/* print something if not found */
	if (found == 0)
		debug_console_printf(machine, "Not found\n");
}


/*-------------------------------------------------
    execute_dasm - execute the dasm command
-------------------------------------------------*/

static void execute_dasm(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 offset, length, bytes = 1;
	int minbytes, maxbytes, byteswidth;
	const address_space *space;
	FILE *f = NULL;
	int i, j;

	/* validate parameters */
	if (!debug_command_parameter_number(machine, param[1], &offset))
		return;
	if (!debug_command_parameter_number(machine, param[2], &length))
		return;
	if (!debug_command_parameter_number(machine, param[3], &bytes))
		return;
	if (!debug_command_parameter_cpu_space(machine, (params > 4) ? param[4] : NULL, ADDRESS_SPACE_PROGRAM, &space))
		return;

	/* determine the width of the bytes */
	minbytes = cpu_get_min_opcode_bytes(space->cpu);
	maxbytes = cpu_get_max_opcode_bytes(space->cpu);
	byteswidth = 0;
	if (bytes)
	{
		byteswidth = (maxbytes + (minbytes - 1)) / minbytes;
		byteswidth *= (2 * minbytes) + 1;
	}

	/* open the file */
	f = fopen(param[0], "w");
	if (!f)
	{
		debug_console_printf(machine, "Error opening file '%s'\n", param[0]);
		return;
	}

	/* now write the data out */
	for (i = 0; i < length; )
	{
		int pcbyte = memory_address_to_byte(space, offset + i) & space->bytemask;
		char output[200+DEBUG_COMMENT_MAX_LINE_LENGTH], disasm[200];
		const char *comment;
		offs_t tempaddr;
		int outdex = 0;
		int numbytes = 0;

		/* print the address */
		outdex += sprintf(&output[outdex], "%s: ", core_i64_hex_format((UINT32)memory_byte_to_address(space, pcbyte), space->logaddrchars));

		/* make sure we can translate the address */
		tempaddr = pcbyte;
		if (debug_cpu_translate(space, TRANSLATE_FETCH_DEBUG, &tempaddr))
		{
			UINT8 opbuf[64], argbuf[64];

			/* fetch the bytes up to the maximum */
			for (numbytes = 0; numbytes < maxbytes; numbytes++)
			{
				opbuf[numbytes] = debug_read_opcode(space, pcbyte + numbytes, 1, FALSE);
				argbuf[numbytes] = debug_read_opcode(space, pcbyte + numbytes, 1, TRUE);
			}

			/* disassemble the result */
			i += numbytes = debug_cpu_disassemble(space->cpu, disasm, offset + i, opbuf, argbuf) & DASMFLAG_LENGTHMASK;
		}

		/* print the bytes */
		if (bytes)
		{
			int startdex = outdex;
			numbytes = memory_address_to_byte(space, numbytes);
			for (j = 0; j < numbytes; j += minbytes)
				outdex += sprintf(&output[outdex], "%s ", core_i64_hex_format(debug_read_opcode(space, pcbyte + j, minbytes, FALSE), minbytes * 2));
			if (outdex - startdex < byteswidth)
				outdex += sprintf(&output[outdex], "%*s", byteswidth - (outdex - startdex), "");
			outdex += sprintf(&output[outdex], "  ");
		}

		/* add the disassembly */
		sprintf(&output[outdex], "%s", disasm);

		/* attempt to add the comment */
		comment = debug_comment_get_text(space->cpu, tempaddr, debug_comment_get_opcode_crc32(space->cpu, tempaddr));
		if (comment != NULL)
		{
			/* somewhat arbitrary guess as to how long most disassembly lines will be [column 60] */
			if (strlen(output) < 60)
			{
				/* pad the comment space out to 60 characters and null-terminate */
				for (outdex = (int)strlen(output); outdex < 60; outdex++)
					output[outdex] = ' ' ;
				output[outdex] = 0 ;

				sprintf(&output[strlen(output)], "// %s", comment) ;
			}
			else
				sprintf(&output[strlen(output)], "\t// %s", comment) ;
		}

		/* output the result */
		fprintf(f, "%s\n", output);
	}

	/* close the file */
	fclose(f);
	debug_console_printf(machine, "Data dumped successfully\n");
}


/*-------------------------------------------------
    execute_trace_internal - functionality for
    trace over and trace info
-------------------------------------------------*/

static void execute_trace_internal(running_machine *machine, int ref, int params, const char *param[], int trace_over)
{
	const char *action = NULL, *filename = param[0];
	const device_config *cpu;
	FILE *f = NULL;
	const char *mode;

	/* validate parameters */
	if (!debug_command_parameter_cpu(machine, (params > 1) ? param[1] : NULL, &cpu))
		return;
	if (!debug_command_parameter_command(machine, action = param[2]))
		return;

	/* further validation */
	if (mame_stricmp(filename, "off") == 0)
		filename = NULL;

	/* open the file */
	if (filename)
	{
		mode = "w";

		/* opening for append? */
		if ((filename[0] == '>') && (filename[1] == '>'))
		{
			mode = "a";
			filename += 2;
		}

		f = fopen(filename, mode);
		if (!f)
		{
			debug_console_printf(machine, "Error opening file '%s'\n", param[0]);
			return;
		}
	}

	/* do it */
	debug_cpu_trace(cpu, f, trace_over, action);
	if (f)
		debug_console_printf(machine, "Tracing CPU '%s' to file %s\n", cpu->tag, filename);
	else
		debug_console_printf(machine, "Stopped tracing on CPU '%s'\n", cpu->tag);
}


/*-------------------------------------------------
    execute_trace - execute the trace command
-------------------------------------------------*/

static void execute_trace(running_machine *machine, int ref, int params, const char *param[])
{
	execute_trace_internal(machine, ref, params, param, 0);
}


/*-------------------------------------------------
    execute_traceover - execute the trace over command
-------------------------------------------------*/

static void execute_traceover(running_machine *machine, int ref, int params, const char *param[])
{
	execute_trace_internal(machine, ref, params, param, 1);
}


/*-------------------------------------------------
    execute_traceflush - execute the trace flush command
-------------------------------------------------*/

static void execute_traceflush(running_machine *machine, int ref, int params, const char *param[])
{
	debug_cpu_flush_traces(machine);
}


/*-------------------------------------------------
    execute_history - execute the history command
-------------------------------------------------*/

static void execute_history(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 count = DEBUG_HISTORY_SIZE;
	const cpu_debug_data *cpuinfo;
	const address_space *space;
	int i;

	/* validate parameters */
	if (!debug_command_parameter_cpu_space(machine, (params > 0) ? param[0] : NULL, ADDRESS_SPACE_PROGRAM, &space))
		return;
	if (!debug_command_parameter_number(machine, param[1], &count))
		return;

	/* further validation */
	if (count > DEBUG_HISTORY_SIZE)
		count = DEBUG_HISTORY_SIZE;

	cpuinfo = cpu_get_debug_data(space->cpu);

	/* loop over lines */
	for (i = 0; i < count; i++)
	{
		offs_t pc = cpuinfo->pc_history[(cpuinfo->pc_history_index + DEBUG_HISTORY_SIZE - count + i) % DEBUG_HISTORY_SIZE];
		int maxbytes = cpu_get_max_opcode_bytes(space->cpu);
		UINT8 opbuf[64], argbuf[64];
		char buffer[200];
		offs_t pcbyte;
		int numbytes;

		/* fetch the bytes up to the maximum */
		pcbyte = memory_address_to_byte(space, pc) & space->bytemask;
		for (numbytes = 0; numbytes < maxbytes; numbytes++)
		{
			opbuf[numbytes] = debug_read_opcode(space, pcbyte + numbytes, 1, FALSE);
			argbuf[numbytes] = debug_read_opcode(space, pcbyte + numbytes, 1, TRUE);
		}

		debug_cpu_disassemble(space->cpu, buffer, pc, opbuf, argbuf);

		debug_console_printf(machine, "%s: %s\n", core_i64_hex_format(pc, space->logaddrchars), buffer);
	}
}


/*-------------------------------------------------
    execute_snap - execute the snapshot command
-------------------------------------------------*/

static void execute_snap(running_machine *machine, int ref, int params, const char *param[])
{
	/* if no params, use the default behavior */
	if (params == 0)
	{
		video_save_active_screen_snapshots(machine);
		debug_console_printf(machine, "Saved snapshot\n");
	}

	/* otherwise, we have to open the file ourselves */
	else
	{
		file_error filerr;
		mame_file *fp;
		const char *filename = param[0];
		int scrnum = (params > 1) ? atoi(param[1]) : 0;
		astring *fname;

		const device_config *screen = device_list_find_by_index(machine->config->devicelist, VIDEO_SCREEN, scrnum);

		if ((screen == NULL) || !render_is_live_screen(screen))
		{
			debug_console_printf(machine, "Invalid screen number '%d'\n", scrnum);
			return;
		}

		fname = astring_dupc(filename);
		if (astring_findc(fname, 0, ".png") == -1)
			astring_catc(fname, ".png");
		filerr = mame_fopen(SEARCHPATH_SCREENSHOT, astring_c(fname), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, &fp);
		astring_free(fname);

		if (filerr != FILERR_NONE)
		{
			debug_console_printf(machine, "Error creating file '%s'\n", filename);
			return;
		}

		video_screen_save_snapshot(screen->machine, screen, fp);
		mame_fclose(fp);
		debug_console_printf(machine, "Saved screen #%d snapshot as '%s'\n", scrnum, filename);
	}
}


/*-------------------------------------------------
    execute_source - execute the source command
-------------------------------------------------*/

static void execute_source(running_machine *machine, int ref, int params, const char *param[])
{
	debug_cpu_source_script(machine, param[0]);
}


/*-------------------------------------------------
    execute_map - execute the map command
-------------------------------------------------*/

static void execute_map(running_machine *machine, int ref, int params, const char *param[])
{
	const address_space *space;
	offs_t taddress;
	UINT64 address;
	int intention;

	/* validate parameters */
	if (!debug_command_parameter_number(machine, param[0], &address))
		return;

	/* CPU is implicit */
	if (!debug_command_parameter_cpu_space(machine, NULL, ref, &space))
		return;

	/* do the translation first */
	for (intention = TRANSLATE_READ_DEBUG; intention <= TRANSLATE_FETCH_DEBUG; intention++)
	{
		static const char *const intnames[] = { "Read", "Write", "Fetch" };
		taddress = memory_address_to_byte(space, address) & space->bytemask;
		if (debug_cpu_translate(space, intention, &taddress))
		{
			const char *mapname = memory_get_handler_string(space, intention == TRANSLATE_WRITE_DEBUG, taddress);
			debug_console_printf(machine, "%7s: %s logical == %s physical -> %s\n", intnames[intention & 3], core_i64_hex_format(address, space->logaddrchars), core_i64_hex_format(memory_byte_to_address(space, taddress), space->addrchars), mapname);
		}
		else
			debug_console_printf(machine, "%7s: %s logical is unmapped\n", intnames[intention & 3], core_i64_hex_format(address, space->logaddrchars));
	}
}


/*-------------------------------------------------
    execute_memdump - execute the memdump command
-------------------------------------------------*/

static void execute_memdump(running_machine *machine, int ref, int params, const char **param)
{
	FILE *file;
	const char *filename;

	filename = (params == 0) ? "memdump.log" : param[0];

	debug_console_printf(machine, "Dumping memory to %s\n", filename);

	file = fopen(filename, "w");
	if (file)
	{
		memory_dump(machine, file);
		fclose(file);
	}
}


/*-------------------------------------------------
    execute_symlist - execute the symlist command
-------------------------------------------------*/

static int CLIB_DECL symbol_sort_compare(const void *item1, const void *item2)
{
	const char *str1 = *(const char **)item1;
	const char *str2 = *(const char **)item2;
	return strcmp(str1, str2);
}

static void execute_symlist(running_machine *machine, int ref, int params, const char **param)
{
	const device_config *cpu = NULL;
	const char *namelist[1000];
	symbol_table *symtable;
	int symnum, count = 0;

	/* validate parameters */
	if (!debug_command_parameter_cpu(machine, param[0], &cpu))
		return;

	if (cpu != NULL)
	{
		symtable = debug_cpu_get_symtable(cpu);
		debug_console_printf(machine, "CPU '%s' symbols:\n", cpu->tag);
	}
	else
	{
		symtable = debug_cpu_get_global_symtable(machine);
		debug_console_printf(machine, "Global symbols:\n");
	}

	/* gather names for all symbols */
	for (symnum = 0; symnum < 100000; symnum++)
	{
		const symbol_entry *entry;
		const char *name = symtable_find_indexed(symtable, symnum, &entry);

		/* if we didn't get anything, we're done */
		if (name == NULL)
			break;

		/* only display "register" type symbols */
		if (entry->type == SMT_REGISTER)
		{
			namelist[count++] = name;
			if (count >= ARRAY_LENGTH(namelist))
				break;
		}
	}

	/* sort the symbols */
	if (count > 1)
		qsort((void *)namelist, count, sizeof(namelist[0]), symbol_sort_compare);

	/* iterate over symbols and print out relevant ones */
	for (symnum = 0; symnum < count; symnum++)
	{
		const symbol_entry *entry = symtable_find(symtable, namelist[symnum]);
		UINT64 value = (*entry->info.reg.getter)(symtable_get_globalref(entry->table), entry->ref);
		assert(entry != NULL);

		/* only display "register" type symbols */
		debug_console_printf(machine, "%s = %s", namelist[symnum], core_i64_hex_format(value, 0));
		if (entry->info.reg.setter == NULL)
			debug_console_printf(machine, "  (read-only)");
		debug_console_printf(machine, "\n");
	}
}


/*-------------------------------------------------
    execute_softreset - execute the softreset command
-------------------------------------------------*/

static void execute_softreset(running_machine *machine, int ref, int params, const char **param)
{
	mame_schedule_soft_reset(machine);
}


/*-------------------------------------------------
    execute_hardreset - execute the hardreset command
-------------------------------------------------*/

static void execute_hardreset(running_machine *machine, int ref, int params, const char **param)
{
	mame_schedule_hard_reset(machine);
}
