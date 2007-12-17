/*********************************************************************

    debugcmd.h

    Debugger command interface engine.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
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
#include <stdarg.h>
#include <ctype.h>



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MAX_GLOBALS		1000



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static struct
{
	void *		base;
	UINT32		size;
} global_array[MAX_GLOBALS];



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void debug_command_exit(running_machine *machine);

static UINT64 execute_min(UINT32 ref, UINT32 params, UINT64 *param);
static UINT64 execute_max(UINT32 ref, UINT32 params, UINT64 *param);
static UINT64 execute_if(UINT32 ref, UINT32 params, UINT64 *param);

static UINT64 global_get(UINT32 ref);
static void global_set(UINT32 ref, UINT64 value);

static void execute_help(int ref, int params, const char **param);
static void execute_print(int ref, int params, const char **param);
static void execute_printf(int ref, int params, const char **param);
static void execute_logerror(int ref, int params, const char **param);
static void execute_tracelog(int ref, int params, const char **param);
static void execute_quit(int ref, int params, const char **param);
static void execute_do(int ref, int params, const char **param);
static void execute_step(int ref, int params, const char **param);
static void execute_over(int ref, int params, const char **param);
static void execute_out(int ref, int params, const char **param);
static void execute_go(int ref, int params, const char **param);
static void execute_go_vblank(int ref, int params, const char **param);
static void execute_go_interrupt(int ref, int params, const char **param);
static void execute_go_time(int ref, int params, const char *param[]);
static void execute_focus(int ref, int params, const char **param);
static void execute_ignore(int ref, int params, const char **param);
static void execute_observe(int ref, int params, const char **param);
static void execute_next(int ref, int params, const char **param);
static void execute_comment(int ref, int params, const char **param);
static void execute_comment_del(int ref, int params, const char **param);
static void execute_comment_save(int ref, int params, const char **param);
static void execute_bpset(int ref, int params, const char **param);
static void execute_bpclear(int ref, int params, const char **param);
static void execute_bpdisenable(int ref, int params, const char **param);
static void execute_bplist(int ref, int params, const char **param);
static void execute_wpset(int ref, int params, const char **param);
static void execute_wpclear(int ref, int params, const char **param);
static void execute_wpdisenable(int ref, int params, const char **param);
static void execute_wplist(int ref, int params, const char **param);
static void execute_hotspot(int ref, int params, const char **param);
static void execute_save(int ref, int params, const char **param);
static void execute_dump(int ref, int params, const char **param);
static void execute_dasm(int ref, int params, const char **param);
static void execute_find(int ref, int params, const char **param);
static void execute_trace(int ref, int params, const char **param);
static void execute_traceover(int ref, int params, const char **param);
static void execute_traceflush(int ref, int params, const char **param);
static void execute_history(int ref, int params, const char **param);
static void execute_snap(int ref, int params, const char **param);
static void execute_source(int ref, int params, const char **param);
static void execute_map(int ref, int params, const char **param);
static void execute_memdump(int ref, int params, const char **param);
static void execute_symlist(int ref, int params, const char **param);
static void execute_softreset(int ref, int params, const char **param);
static void execute_hardreset(int ref, int params, const char **param);



/***************************************************************************

    Initialization

***************************************************************************/

/*-------------------------------------------------
    debug_command_init - initializes the command
    system
-------------------------------------------------*/

void debug_command_init(running_machine *machine)
{
	int cpunum, itemnum;
	const char *name;

	/* add a few simple global functions */
	symtable_add_function(global_symtable, "min", 0, 2, 2, execute_min);
	symtable_add_function(global_symtable, "max", 0, 2, 2, execute_max);
	symtable_add_function(global_symtable, "if", 0, 3, 3, execute_if);

	/* add all single-entry save state globals */
	for (itemnum = 0; itemnum < MAX_GLOBALS; itemnum++)
	{
		UINT32 valsize, valcount;
		void *base;

		/* stop when we run out of items */
		name = state_save_get_indexed_item(itemnum, &base, &valsize, &valcount);
		if (name == NULL)
			break;

		/* if this is a single-entry global, add it */
		if (valcount == 1 && strstr(name, "/globals/"))
		{
			char symname[100];
			sprintf(symname, ".%s", strrchr(name, '/') + 1);
			global_array[itemnum].base = base;
			global_array[itemnum].size = valsize;
			symtable_add_register(global_symtable, symname, itemnum, global_get, global_set);
		}
	}

	/* add all the commands */
	debug_console_register_command("help",      CMDFLAG_NONE, 0, 0, 1, execute_help);
	debug_console_register_command("print",     CMDFLAG_NONE, 0, 1, MAX_COMMAND_PARAMS, execute_print);
	debug_console_register_command("printf",    CMDFLAG_NONE, 0, 1, MAX_COMMAND_PARAMS, execute_printf);
	debug_console_register_command("logerror",  CMDFLAG_NONE, 0, 1, MAX_COMMAND_PARAMS, execute_logerror);
	debug_console_register_command("tracelog",  CMDFLAG_NONE, 0, 1, MAX_COMMAND_PARAMS, execute_tracelog);
	debug_console_register_command("quit",      CMDFLAG_NONE, 0, 0, 0, execute_quit);
	debug_console_register_command("do",        CMDFLAG_NONE, 0, 1, 1, execute_do);
	debug_console_register_command("step",      CMDFLAG_NONE, 0, 0, 1, execute_step);
	debug_console_register_command("s",         CMDFLAG_NONE, 0, 0, 1, execute_step);
	debug_console_register_command("over",      CMDFLAG_NONE, 0, 0, 1, execute_over);
	debug_console_register_command("o",         CMDFLAG_NONE, 0, 0, 1, execute_over);
	debug_console_register_command("out" ,      CMDFLAG_NONE, 0, 0, 0, execute_out);
	debug_console_register_command("go",        CMDFLAG_NONE, 0, 0, 1, execute_go);
	debug_console_register_command("g",         CMDFLAG_NONE, 0, 0, 1, execute_go);
	debug_console_register_command("gvblank",   CMDFLAG_NONE, 0, 0, 0, execute_go_vblank);
	debug_console_register_command("gv",        CMDFLAG_NONE, 0, 0, 0, execute_go_vblank);
	debug_console_register_command("gint",      CMDFLAG_NONE, 0, 0, 1, execute_go_interrupt);
	debug_console_register_command("gi",        CMDFLAG_NONE, 0, 0, 1, execute_go_interrupt);
	debug_console_register_command("gtime",     CMDFLAG_NONE, 0, 0, 1, execute_go_time);
	debug_console_register_command("gt",        CMDFLAG_NONE, 0, 0, 1, execute_go_time);
	debug_console_register_command("next",      CMDFLAG_NONE, 0, 0, 0, execute_next);
	debug_console_register_command("n",         CMDFLAG_NONE, 0, 0, 0, execute_next);
	debug_console_register_command("focus",     CMDFLAG_NONE, 0, 1, 1, execute_focus);
	debug_console_register_command("ignore",    CMDFLAG_NONE, 0, 0, MAX_COMMAND_PARAMS, execute_ignore);
	debug_console_register_command("observe",   CMDFLAG_NONE, 0, 0, MAX_COMMAND_PARAMS, execute_observe);

	debug_console_register_command("comadd",	CMDFLAG_NONE, 0, 1, 2, execute_comment);
	debug_console_register_command("//",        CMDFLAG_NONE, 0, 1, 2, execute_comment);
	debug_console_register_command("comdelete",	CMDFLAG_NONE, 0, 1, 1, execute_comment_del);
	debug_console_register_command("comsave", 	CMDFLAG_NONE, 0, 0, 0, execute_comment_save);

	debug_console_register_command("bpset",     CMDFLAG_NONE, 0, 1, 3, execute_bpset);
	debug_console_register_command("bp",        CMDFLAG_NONE, 0, 1, 3, execute_bpset);
	debug_console_register_command("bpclear",   CMDFLAG_NONE, 0, 0, 1, execute_bpclear);
	debug_console_register_command("bpdisable", CMDFLAG_NONE, 0, 0, 1, execute_bpdisenable);
	debug_console_register_command("bpenable",  CMDFLAG_NONE, 1, 0, 1, execute_bpdisenable);
	debug_console_register_command("bplist",    CMDFLAG_NONE, 0, 0, 0, execute_bplist);

	debug_console_register_command("wpset",     CMDFLAG_NONE, ADDRESS_SPACE_PROGRAM, 3, 5, execute_wpset);
	debug_console_register_command("wp",        CMDFLAG_NONE, ADDRESS_SPACE_PROGRAM, 3, 5, execute_wpset);
	debug_console_register_command("wpdset",    CMDFLAG_NONE, ADDRESS_SPACE_DATA, 3, 5, execute_wpset);
	debug_console_register_command("wpd",       CMDFLAG_NONE, ADDRESS_SPACE_DATA, 3, 5, execute_wpset);
	debug_console_register_command("wpiset",    CMDFLAG_NONE, ADDRESS_SPACE_IO, 3, 5, execute_wpset);
	debug_console_register_command("wpi",       CMDFLAG_NONE, ADDRESS_SPACE_IO, 3, 5, execute_wpset);
	debug_console_register_command("wpclear",   CMDFLAG_NONE, 0, 0, 1, execute_wpclear);
	debug_console_register_command("wpdisable", CMDFLAG_NONE, 0, 0, 1, execute_wpdisenable);
	debug_console_register_command("wpenable",  CMDFLAG_NONE, 1, 0, 1, execute_wpdisenable);
	debug_console_register_command("wplist",    CMDFLAG_NONE, 0, 0, 0, execute_wplist);

	debug_console_register_command("hotspot",   CMDFLAG_NONE, 0, 0, 3, execute_hotspot);

	debug_console_register_command("save",      CMDFLAG_NONE, ADDRESS_SPACE_PROGRAM, 3, 4, execute_save);
	debug_console_register_command("saved",     CMDFLAG_NONE, ADDRESS_SPACE_DATA, 3, 4, execute_save);
	debug_console_register_command("savei",     CMDFLAG_NONE, ADDRESS_SPACE_IO, 3, 4, execute_save);

	debug_console_register_command("dump",      CMDFLAG_NONE, ADDRESS_SPACE_PROGRAM, 3, 6, execute_dump);
	debug_console_register_command("dumpd",     CMDFLAG_NONE, ADDRESS_SPACE_DATA, 3, 6, execute_dump);
	debug_console_register_command("dumpi",     CMDFLAG_NONE, ADDRESS_SPACE_IO, 3, 6, execute_dump);

	debug_console_register_command("f",         CMDFLAG_KEEP_QUOTES, ADDRESS_SPACE_PROGRAM, 3, MAX_COMMAND_PARAMS, execute_find);
	debug_console_register_command("find",      CMDFLAG_KEEP_QUOTES, ADDRESS_SPACE_PROGRAM, 3, MAX_COMMAND_PARAMS, execute_find);
	debug_console_register_command("fd",        CMDFLAG_KEEP_QUOTES, ADDRESS_SPACE_DATA, 3, MAX_COMMAND_PARAMS, execute_find);
	debug_console_register_command("findd",     CMDFLAG_KEEP_QUOTES, ADDRESS_SPACE_DATA, 3, MAX_COMMAND_PARAMS, execute_find);
	debug_console_register_command("fi",        CMDFLAG_KEEP_QUOTES, ADDRESS_SPACE_IO, 3, MAX_COMMAND_PARAMS, execute_find);
	debug_console_register_command("findi",     CMDFLAG_KEEP_QUOTES, ADDRESS_SPACE_IO, 3, MAX_COMMAND_PARAMS, execute_find);

	debug_console_register_command("dasm",      CMDFLAG_NONE, 0, 3, 5, execute_dasm);

	debug_console_register_command("trace",     CMDFLAG_NONE, 0, 1, 3, execute_trace);
	debug_console_register_command("traceover", CMDFLAG_NONE, 0, 1, 3, execute_traceover);
	debug_console_register_command("traceflush",CMDFLAG_NONE, 0, 0, 0, execute_traceflush);

	debug_console_register_command("history",   CMDFLAG_NONE, 0, 0, 2, execute_history);

	debug_console_register_command("snap",      CMDFLAG_NONE, 0, 0, 1, execute_snap);

	debug_console_register_command("source",    CMDFLAG_NONE, 0, 1, 1, execute_source);

	debug_console_register_command("map",		CMDFLAG_NONE, ADDRESS_SPACE_PROGRAM, 1, 1, execute_map);
	debug_console_register_command("mapd",		CMDFLAG_NONE, ADDRESS_SPACE_DATA, 1, 1, execute_map);
	debug_console_register_command("mapi",		CMDFLAG_NONE, ADDRESS_SPACE_IO, 1, 1, execute_map);
	debug_console_register_command("memdump",	CMDFLAG_NONE, 0, 0, 1, execute_memdump);

	debug_console_register_command("symlist",	CMDFLAG_NONE, 0, 0, 1, execute_symlist);

	debug_console_register_command("softreset",	CMDFLAG_NONE, 0, 0, 1, execute_softreset);
	debug_console_register_command("hardreset",	CMDFLAG_NONE, 0, 0, 1, execute_hardreset);

	/* ask all the CPUs if they would like to register functions or symbols */
	for (cpunum = 0; cpunum < cpu_gettotalcpu(); cpunum++)
	{
		void (*setup_commands)(void);
		setup_commands = cpunum_get_info_fct(cpunum, CPUINFO_PTR_DEBUG_SETUP_COMMANDS);
		if (setup_commands)
			setup_commands();
	}

	add_exit_callback(machine, debug_command_exit);

	/* set up the initial debugscript if specified */
	name = options_get_string(mame_options(), OPTION_DEBUGSCRIPT);
	if (name[0] != 0)
		debug_source_script(name);
}


/*-------------------------------------------------
    debug_command_exit - exit-time cleanup
-------------------------------------------------*/

static void debug_command_exit(running_machine *machine)
{
	int cpunum;

	/* turn off all traces */
	for (cpunum = 0; cpunum < cpu_gettotalcpu(); cpunum++)
		debug_cpu_trace(cpunum, NULL, 0, NULL);
}



/***************************************************************************

    Simple Global Functions

***************************************************************************/

/*-------------------------------------------------
    execute_min - return the minimum of two values
-------------------------------------------------*/

static UINT64 execute_min(UINT32 ref, UINT32 params, UINT64 *param)
{
	return (param[0] < param[1]) ? param[0] : param[1];
}


/*-------------------------------------------------
    execute_max - return the maximum of two values
-------------------------------------------------*/

static UINT64 execute_max(UINT32 ref, UINT32 params, UINT64 *param)
{
	return (param[0] > param[1]) ? param[0] : param[1];
}


/*-------------------------------------------------
    execute_if - if (a) return b; else return c;
-------------------------------------------------*/

static UINT64 execute_if(UINT32 ref, UINT32 params, UINT64 *param)
{
	return param[0] ? param[1] : param[2];
}



/***************************************************************************

    Global accessors

***************************************************************************/

/*-------------------------------------------------
    global_get - symbol table getter for globals
-------------------------------------------------*/

static UINT64 global_get(UINT32 ref)
{
	assert(ref < MAX_GLOBALS);
	switch (global_array[ref].size)
	{
		case 1:		return *(UINT8 *)global_array[ref].base;
		case 2:		return *(UINT16 *)global_array[ref].base;
		case 4:		return *(UINT32 *)global_array[ref].base;
		case 8:		return *(UINT64 *)global_array[ref].base;
	}
	return ~0;
}


/*-------------------------------------------------
    global_set - symbol table setter for globals
-------------------------------------------------*/

static void global_set(UINT32 ref, UINT64 value)
{
	assert(ref < MAX_GLOBALS);
	switch (global_array[ref].size)
	{
		case 1:		*(UINT8 *)global_array[ref].base = value;	break;
		case 2:		*(UINT16 *)global_array[ref].base = value;	break;
		case 4:		*(UINT32 *)global_array[ref].base = value;	break;
		case 8:		*(UINT64 *)global_array[ref].base = value;	break;
	}
}



/***************************************************************************

    Parameter Validation Helpers

***************************************************************************/

/*-------------------------------------------------
    debug_command_parameter_number - validates a
    number parameter
-------------------------------------------------*/

int debug_command_parameter_number(const char *param, UINT64 *result)
{
	EXPRERR err = expression_evaluate(param, debug_get_cpu_info(cpu_getactivecpu())->symtable, result);
	if (err == EXPRERR_NONE)
		return 1;
	debug_console_printf("Error in expression: %s\n", param);
	debug_console_printf("                     %*s^", EXPRERR_ERROR_OFFSET(err), "");
	debug_console_printf("%s\n", exprerr_to_string(err));
	return 0;
}


/*-------------------------------------------------
    debug_command_parameter_expression - validates
    an expression parameter
-------------------------------------------------*/

static int debug_command_parameter_expression(const char *param, parsed_expression **result)
{
	EXPRERR err = expression_parse(param, debug_get_cpu_info(cpu_getactivecpu())->symtable, result);
	if (err == EXPRERR_NONE)
		return 1;
	debug_console_printf("Error in expression: %s\n", param);
	debug_console_printf("                     %*s^", EXPRERR_ERROR_OFFSET(err), "");
	debug_console_printf("%s\n", exprerr_to_string(err));
	return 0;
}


/*-------------------------------------------------
    debug_command_parameter_command - validates a
    command parameter
-------------------------------------------------*/

static int debug_command_parameter_command(const char *param)
{
	CMDERR err = debug_console_validate_command(param);
	if (err == CMDERR_NONE)
		return 1;
	debug_console_printf("Error in command: %s\n", param);
	debug_console_printf("                  %*s^", CMDERR_ERROR_OFFSET(err), "");
	debug_console_printf("%s\n", debug_cmderr_to_string(err));
	return 0;
}



/***************************************************************************

    Command Helpers

***************************************************************************/

/*-------------------------------------------------
    execute_help - execute the help command
-------------------------------------------------*/

static void execute_help(int ref, int params, const char *param[])
{
	if (params == 0)
		debug_console_printf_wrap(80, "%s\n", debug_get_help(""));
	else
		debug_console_printf_wrap(80, "%s\n", debug_get_help(param[0]));
}


/*-------------------------------------------------
    execute_print - execute the print command
-------------------------------------------------*/

static void execute_print(int ref, int params, const char *param[])
{
	UINT64 values[MAX_COMMAND_PARAMS];
	int i;

	/* validate the other parameters */
	for (i = 0; i < params; i++)
		if (!debug_command_parameter_number(param[i], &values[i]))
			return;

	/* then print each one */
	for (i = 0; i < params; i++)
	{
		if ((values[i] >> 32) != 0)
			debug_console_printf("%X%08X ", (UINT32)(values[i] >> 32), (UINT32)values[i]);
		else
			debug_console_printf("%X ", (UINT32)values[i]);
	}
	debug_console_printf("\n");
}


/*-------------------------------------------------
    mini_printf - safe printf to a buffer
-------------------------------------------------*/

static int mini_printf(char *buffer, const char *format, int params, UINT64 *param)
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
						debug_console_printf("Not enough parameters for format!\n");
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
						debug_console_printf("Not enough parameters for format!\n");
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

static void execute_printf(int ref, int params, const char *param[])
{
	UINT64 values[MAX_COMMAND_PARAMS];
	char buffer[1024];
	int i;

	/* validate the other parameters */
	for (i = 1; i < params; i++)
		if (!debug_command_parameter_number(param[i], &values[i]))
			return;

	/* then do a printf */
	if (mini_printf(buffer, param[0], params - 1, &values[1]))
		debug_console_printf("%s\n", buffer);
}


/*-------------------------------------------------
    execute_logerror - execute the logerror command
-------------------------------------------------*/

static void execute_logerror(int ref, int params, const char *param[])
{
	UINT64 values[MAX_COMMAND_PARAMS];
	char buffer[1024];
	int i;

	/* validate the other parameters */
	for (i = 1; i < params; i++)
		if (!debug_command_parameter_number(param[i], &values[i]))
			return;

	/* then do a printf */
	if (mini_printf(buffer, param[0], params - 1, &values[1]))
		logerror("%s", buffer);
}


/*-------------------------------------------------
    execute_tracelog - execute the tracelog command
-------------------------------------------------*/

static void execute_tracelog(int ref, int params, const char *param[])
{
	FILE *file = debug_get_cpu_info(cpu_getactivecpu())->trace.file;
	UINT64 values[MAX_COMMAND_PARAMS];
	char buffer[1024];
	int i;

	/* if no tracefile, skip */
	if (!file)
		return;

	/* validate the other parameters */
	for (i = 1; i < params; i++)
		if (!debug_command_parameter_number(param[i], &values[i]))
			return;

	/* then do a printf */
	if (mini_printf(buffer, param[0], params - 1, &values[1]))
		fprintf(file, "%s", buffer);
}


/*-------------------------------------------------
    execute_quit - execute the quit command
-------------------------------------------------*/

static void execute_quit(int ref, int params, const char *param[])
{
	mame_printf_error("Exited via the debugger\n");
	mame_schedule_exit(Machine);
}


/*-------------------------------------------------
    execute_do - execute the do command
-------------------------------------------------*/

static void execute_do(int ref, int params, const char *param[])
{
	UINT64 dummy;
	debug_command_parameter_number(param[0], &dummy);
}


/*-------------------------------------------------
    execute_step - execute the step command
-------------------------------------------------*/

static void execute_step(int ref, int params, const char *param[])
{
	UINT64 steps = 1;

	/* if we have a parameter, use it instead */
	if (params > 0 && !debug_command_parameter_number(param[0], &steps))
		return;

	debug_cpu_single_step(steps);
}


/*-------------------------------------------------
    execute_over - execute the over command
-------------------------------------------------*/

static void execute_over(int ref, int params, const char *param[])
{
	UINT64 steps = 1;

	/* if we have a parameter, use it instead */
	if (params > 0 && !debug_command_parameter_number(param[0], &steps))
		return;

	debug_cpu_single_step_over(steps);
}


/*-------------------------------------------------
    execute_out - execute the out command
-------------------------------------------------*/

static void execute_out(int ref, int params, const char *param[])
{
	debug_cpu_single_step_out();
}


/*-------------------------------------------------
    execute_go - execute the go command
-------------------------------------------------*/

static void execute_go(int ref, int params, const char *param[])
{
	UINT64 addr = ~0;

	/* if we have a parameter, use it instead */
	if (params > 0 && !debug_command_parameter_number(param[0], &addr))
		return;

	debug_cpu_go(addr);
}


/*-------------------------------------------------
    execute_go_vblank - execute the govblank
    command
-------------------------------------------------*/

static void execute_go_vblank(int ref, int params, const char *param[])
{
	debug_cpu_go_vblank();
}


/*-------------------------------------------------
    execute_go_interrupt - execute the goint command
-------------------------------------------------*/

static void execute_go_interrupt(int ref, int params, const char *param[])
{
	UINT64 irqline = -1;

	/* if we have a parameter, use it instead */
	if (params > 0 && !debug_command_parameter_number(param[0], &irqline))
		return;

	debug_cpu_go_interrupt(irqline);
}


/*-------------------------------------------------
    execute_go_time - execute the gtime command
-------------------------------------------------*/

static void execute_go_time(int ref, int params, const char *param[])
{
	UINT64 milliseconds = -1;

	/* if we have a parameter, use it instead */
	if (params > 0 && !debug_command_parameter_number(param[0], &milliseconds))
		return;

	debug_cpu_go_milliseconds(milliseconds);
}


/*-------------------------------------------------
    execute_next - execute the next command
-------------------------------------------------*/

static void execute_next(int ref, int params, const char *param[])
{
	debug_cpu_next_cpu();
}


/*-------------------------------------------------
    execute_focus - execute the focus command
-------------------------------------------------*/

static void execute_focus(int ref, int params, const char *param[])
{
	UINT64 cpuwhich;
	int cpunum;

	/* validate params */
	if (!debug_command_parameter_number(param[0], &cpuwhich))
		return;
	if (cpuwhich >= cpu_gettotalcpu())
	{
		debug_console_printf("Invalid CPU number!\n");
		return;
	}

	/* first clear the ignore flag on the focused CPU */
	debug_cpu_ignore_cpu(cpuwhich, 0);

	/* then loop over CPUs and set the ignore flags on all other CPUs */
	for (cpunum = 0; cpunum < MAX_CPU; cpunum++)
	{
		const debug_cpu_info *info = debug_get_cpu_info(cpunum);
		if (info && info->valid && cpunum != cpuwhich)
			debug_cpu_ignore_cpu(cpunum, 1);
	}
	debug_console_printf("Now focused on CPU %d\n", (int)cpuwhich);
}


/*-------------------------------------------------
    execute_ignore - execute the ignore command
-------------------------------------------------*/

static void execute_ignore(int ref, int params, const char *param[])
{
	UINT64 cpuwhich[MAX_COMMAND_PARAMS];
	int cpunum, paramnum;
	char buffer[100];
	int buflen = 0;

	/* if there are no parameters, dump the ignore list */
	if (params == 0)
	{
		/* loop over all CPUs */
		for (cpunum = 0; cpunum < MAX_CPU; cpunum++)
		{
			const debug_cpu_info *info = debug_get_cpu_info(cpunum);

			/* build up a comma-separated list */
			if (info && info->valid && info->ignoring)
			{
				if (buflen == 0) buflen += sprintf(&buffer[buflen], "Currently ignoring CPU %d", cpunum);
				else buflen += sprintf(&buffer[buflen], ",%d", cpunum);
			}
		}

		/* special message for none */
		if (buflen == 0)
			sprintf(&buffer[buflen], "Not currently ignoring any CPUs");
		debug_console_printf("%s\n", buffer);
	}

	/* otherwise clear the ignore flag on all requested CPUs */
	else
	{
		/* validate parameters */
		for (paramnum = 0; paramnum < params; paramnum++)
		{
			if (!debug_command_parameter_number(param[paramnum], &cpuwhich[paramnum]))
				return;
			if (cpuwhich[paramnum] >= cpu_gettotalcpu())
			{
				debug_console_printf("Invalid CPU number! (%d)\n", (int)cpuwhich[paramnum]);
				return;
			}
		}

		/* set the ignore flags */
		for (paramnum = 0; paramnum < params; paramnum++)
		{
			/* make sure this isn't the last live CPU */
			for (cpunum = 0; cpunum < MAX_CPU; cpunum++)
			{
				const debug_cpu_info *info = debug_get_cpu_info(cpunum);
				if (cpunum != cpuwhich[paramnum] && info && info->valid && !info->ignoring)
					break;
			}
			if (cpunum == MAX_CPU)
			{
				debug_console_printf("Can't ignore all CPUs!\n");
				return;
			}

			debug_cpu_ignore_cpu(cpuwhich[paramnum], 1);
			debug_console_printf("Now ignoring CPU %d\n", (int)cpuwhich[paramnum]);
		}
	}
}


/*-------------------------------------------------
    execute_observe - execute the observe command
-------------------------------------------------*/

static void execute_observe(int ref, int params, const char *param[])
{
	UINT64 cpuwhich[MAX_COMMAND_PARAMS];
	int cpunum, paramnum;
	char buffer[100];
	int buflen = 0;

	/* if there are no parameters, dump the ignore list */
	if (params == 0)
	{
		/* loop over all CPUs */
		for (cpunum = 0; cpunum < MAX_CPU; cpunum++)
		{
			const debug_cpu_info *info = debug_get_cpu_info(cpunum);

			/* build up a comma-separated list */
			if (info && info->valid && !info->ignoring)
			{
				if (buflen == 0) buflen += sprintf(&buffer[buflen], "Currently observing CPU %d", cpunum);
				else buflen += sprintf(&buffer[buflen], ",%d", cpunum);
			}
		}

		/* special message for none */
		if (buflen == 0)
			buflen += sprintf(&buffer[buflen], "Not currently observing any CPUs");
		debug_console_printf("%s\n", buffer);
	}

	/* otherwise set the ignore flag on all requested CPUs */
	else
	{
		/* validate parameters */
		for (paramnum = 0; paramnum < params; paramnum++)
		{
			if (!debug_command_parameter_number(param[paramnum], &cpuwhich[paramnum]))
				return;
			if (cpuwhich[paramnum] >= cpu_gettotalcpu())
			{
				debug_console_printf("Invalid CPU number! (%d)\n", (int)cpuwhich[paramnum]);
				return;
			}
		}

		/* clear the ignore flags */
		for (paramnum = 0; paramnum < params; paramnum++)
		{
			debug_cpu_ignore_cpu(cpuwhich[paramnum], 0);
			debug_console_printf("Now observing CPU %d\n", (int)cpuwhich[paramnum]);
		}
	}
}


/*-------------------------------------------------
    execute_comment - add a comment to a line
-------------------------------------------------*/

static void execute_comment(int ref, int params, const char *param[])
{
	UINT64 address;

	/* param 1 is the address for the comment */
	if (!debug_command_parameter_number(param[0], &address))
		return;

	/* make sure param 2 exists */
	if (strlen(param[1]) == 0)
	{
		debug_console_printf("Error : comment text empty\n");
		return;
	}

	/* Now try adding the comment */
	debug_comment_add(cpu_getactivecpu(), address, param[1], 0x00ff0000, debug_comment_get_opcode_crc32(address));
	debug_view_update_type(DVT_DISASSEMBLY);
}


/*------------------------------------------------------
    execute_comment_del - remove a comment from an addr
--------------------------------------------------------*/

static void execute_comment_del(int ref, int params, const char *param[])
{
	UINT64 address;

	/* param 1 can either be a command or the address for the comment */
	if (!debug_command_parameter_number(param[0], &address))
		return;

	/* If it's a number, it must be an address */
	/* The bankoff and cbn will be pulled from what's currently active */
	debug_comment_remove(cpu_getactivecpu(), address, debug_comment_get_opcode_crc32(address));
	debug_view_update_type(DVT_DISASSEMBLY);
}


/*-------------------------------------------------
    execute_comment - add a comment to a line
-------------------------------------------------*/

static void execute_comment_save(int ref, int params, const char *param[])
{
	if (debug_comment_save())
		debug_console_printf("Comments successfully saved\n");
}


/*-------------------------------------------------
    execute_bpset - execute the breakpoint set
    command
-------------------------------------------------*/

static void execute_bpset(int ref, int params, const char *param[])
{
	parsed_expression *condition = NULL;
	const char *action = NULL;
	UINT64 address;
	int bpnum;

	/* make sure that there is an active CPU */
	if (cpu_getactivecpu() < 0)
	{
		debug_console_printf("No active CPU!\n");
		return;
	}

	/* param 1 is the address */
	if (!debug_command_parameter_number(param[0], &address))
		return;

	/* param 2 is the condition */
	if (params > 1 && !debug_command_parameter_expression(param[1], &condition))
		return;

	/* param 3 is the action */
	if (params > 2 && !debug_command_parameter_command(action = param[2]))
		return;

	/* set the breakpoint */
	bpnum = debug_breakpoint_set(cpu_getactivecpu(), address, condition, action);
	debug_console_printf("Breakpoint %X set\n", bpnum);
}


/*-------------------------------------------------
    execute_bpclear - execute the breakpoint
    clear command
-------------------------------------------------*/

static void execute_bpclear(int ref, int params, const char *param[])
{
	UINT64 bpindex;

	/* if 0 parameters, clear all */
	if (params == 0)
	{
		int cpunum;

		for (cpunum = 0; cpunum < MAX_CPU; cpunum++)
		{
			const debug_cpu_info *cpuinfo = debug_get_cpu_info(cpunum);
			if (cpuinfo->valid)
			{
				debug_cpu_breakpoint *bp;
				while ((bp = cpuinfo->first_bp) != NULL)
					debug_breakpoint_clear(bp->index);
			}
		}
		debug_console_printf("Cleared all breakpoints\n");
	}

	/* otherwise, clear the specific one */
	else if (!debug_command_parameter_number(param[0], &bpindex))
		return;
	else
	{
		int found = debug_breakpoint_clear(bpindex);
		if (found)
			debug_console_printf("Breakpoint %X cleared\n", (UINT32)bpindex);
		else
			debug_console_printf("Invalid breakpoint number %X\n", (UINT32)bpindex);
	}
}


/*-------------------------------------------------
    execute_bpdisenable - execute the breakpoint
    disable/enable commands
-------------------------------------------------*/

static void execute_bpdisenable(int ref, int params, const char *param[])
{
	UINT64 bpindex;

	/* if 0 parameters, clear all */
	if (params == 0)
	{
		int cpunum;

		for (cpunum = 0; cpunum < MAX_CPU; cpunum++)
		{
			const debug_cpu_info *cpuinfo = debug_get_cpu_info(cpunum);
			if (cpuinfo->valid)
			{
				debug_cpu_breakpoint *bp;
				for (bp = cpuinfo->first_bp; bp; bp = bp->next)
					debug_breakpoint_enable(bp->index, ref);
			}
		}
		if (ref == 0)
			debug_console_printf("Disabled all breakpoints\n");
		else
			debug_console_printf("Enabled all breakpoints\n");
	}

	/* otherwise, clear the specific one */
	else if (!debug_command_parameter_number(param[0], &bpindex))
		return;
	else
	{
		int found = debug_breakpoint_enable(bpindex, ref);
		if (found)
			debug_console_printf("Breakpoint %X %s\n", (UINT32)bpindex, ref ? "enabled" : "disabled");
		else
			debug_console_printf("Invalid breakpoint number %X\n", (UINT32)bpindex);
	}
}


/*-------------------------------------------------
    execute_bplist - execute the breakpoint list
    command
-------------------------------------------------*/

static void execute_bplist(int ref, int params, const char *param[])
{
	int cpunum, printed = 0;
	char buffer[256];

	/* loop over all CPUs */
	for (cpunum = 0; cpunum < MAX_CPU; cpunum++)
	{
		const debug_cpu_info *cpuinfo = debug_get_cpu_info(cpunum);

		if (cpuinfo->valid && cpuinfo->first_bp)
		{
			debug_cpu_breakpoint *bp;

			debug_console_printf("CPU %d breakpoints:\n", cpunum);

			/* loop over the breakpoints */
			for (bp = cpuinfo->first_bp; bp; bp = bp->next)
			{
				int buflen;
				buflen = sprintf(buffer, "%c%4X @ %08X", bp->enabled ? ' ' : 'D', bp->index, bp->address);
				if (bp->condition)
					buflen += sprintf(&buffer[buflen], " if %s", expression_original_string(bp->condition));
				if (bp->action)
					buflen += sprintf(&buffer[buflen], " do %s", bp->action);
				debug_console_printf("%s\n", buffer);
				printed++;
			}
		}
	}

	if (!printed)
		debug_console_printf("No breakpoints currently installed\n");
}


/*-------------------------------------------------
    execute_wpset - execute the watchpoint set
    command
-------------------------------------------------*/

static void execute_wpset(int ref, int params, const char *param[])
{
	parsed_expression *condition = NULL;
	const char *action = NULL;
	UINT64 address, length;
	int type = 0;
	int wpnum;

	/* param 1 is the address */
	if (!debug_command_parameter_number(param[0], &address))
		return;

	/* param 2 is the length */
	if (!debug_command_parameter_number(param[1], &length))
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
		debug_console_printf("Invalid watchpoint type: expected r, w, or rw\n");
		return;
	}

	/* param 4 is the condition */
	if (params > 3 && !debug_command_parameter_expression(param[3], &condition))
		return;

	/* param 5 is the action */
	if (params > 4 && !debug_command_parameter_command(action = param[4]))
		return;

	/* set the watchpoint */
	wpnum = debug_watchpoint_set(cpu_getactivecpu(), ref, type, address, length, condition, action);
	debug_console_printf("Watchpoint %X set\n", wpnum);
}


/*-------------------------------------------------
    execute_wpclear - execute the watchpoint
    clear command
-------------------------------------------------*/

static void execute_wpclear(int ref, int params, const char *param[])
{
	UINT64 wpindex;

	/* if 0 parameters, clear all */
	if (params == 0)
	{
		int cpunum;

		for (cpunum = 0; cpunum < MAX_CPU; cpunum++)
		{
			const debug_cpu_info *cpuinfo = debug_get_cpu_info(cpunum);
			if (cpuinfo->valid)
			{
				int spacenum;

				for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
				{
					debug_cpu_watchpoint *wp;
					while ((wp = cpuinfo->space[spacenum].first_wp) != NULL)
						debug_watchpoint_clear(wp->index);
				}
			}
		}
		debug_console_printf("Cleared all watchpoints\n");
	}

	/* otherwise, clear the specific one */
	else if (!debug_command_parameter_number(param[0], &wpindex))
		return;
	else
	{
		int found = debug_watchpoint_clear(wpindex);
		if (found)
			debug_console_printf("Watchpoint %X cleared\n", (UINT32)wpindex);
		else
			debug_console_printf("Invalid watchpoint number %X\n", (UINT32)wpindex);
	}
}


/*-------------------------------------------------
    execute_wpdisenable - execute the watchpoint
    disable/enable commands
-------------------------------------------------*/

static void execute_wpdisenable(int ref, int params, const char *param[])
{
	UINT64 wpindex;

	/* if 0 parameters, clear all */
	if (params == 0)
	{
		int cpunum;

		for (cpunum = 0; cpunum < MAX_CPU; cpunum++)
		{
			const debug_cpu_info *cpuinfo = debug_get_cpu_info(cpunum);
			if (cpuinfo->valid)
			{
				int spacenum;

				for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
				{
					debug_cpu_watchpoint *wp;
					for (wp = cpuinfo->space[spacenum].first_wp; wp; wp = wp->next)
						debug_watchpoint_enable(wp->index, ref);
				}
			}
		}
		if (ref == 0)
			debug_console_printf("Disabled all watchpoints\n");
		else
			debug_console_printf("Enabled all watchpoints\n");
	}

	/* otherwise, clear the specific one */
	else if (!debug_command_parameter_number(param[0], &wpindex))
		return;
	else
	{
		int found = debug_watchpoint_enable(wpindex, ref);
		if (found)
			debug_console_printf("Watchpoint %X %s\n", (UINT32)wpindex, ref ? "enabled" : "disabled");
		else
			debug_console_printf("Invalid watchpoint number %X\n", (UINT32)wpindex);
	}
}


/*-------------------------------------------------
    execute_wplist - execute the watchpoint list
    command
-------------------------------------------------*/

static void execute_wplist(int ref, int params, const char *param[])
{
	static const char *spacenames[ADDRESS_SPACES] = { "program", "data", "I/O" };
	int cpunum, printed = 0;
	char buffer[256];

	/* loop over all CPUs */
	for (cpunum = 0; cpunum < MAX_CPU; cpunum++)
	{
		const debug_cpu_info *cpuinfo = debug_get_cpu_info(cpunum);

		if (cpuinfo->valid)
		{
			int spacenum;

			for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
			{
				if (cpuinfo->space[spacenum].first_wp)
				{
					static const char *types[] = { "unkn ", "read ", "write", "r/w  " };
					debug_cpu_watchpoint *wp;

					debug_console_printf("CPU %d %s space watchpoints:\n", cpunum, spacenames[spacenum]);

					/* loop over the watchpoints */
					for (wp = cpuinfo->space[spacenum].first_wp; wp; wp = wp->next)
					{
						int buflen;
						buflen = sprintf(buffer, "%c%4X @ %08X-%08X %s", wp->enabled ? ' ' : 'D',
								wp->index, BYTE2ADDR(wp->address, cpuinfo, spacenum), BYTE2ADDR(wp->address + wp->length, cpuinfo, spacenum) - 1, types[wp->type & 3]);
						if (wp->condition)
							buflen += sprintf(&buffer[buflen], " if %s", expression_original_string(wp->condition));
						if (wp->action)
							buflen += sprintf(&buffer[buflen], " do %s", wp->action);
						debug_console_printf("%s\n", buffer);
						printed++;
					}
				}
			}
		}
	}

	if (!printed)
		debug_console_printf("No watchpoints currently installed\n");
}


/*-------------------------------------------------
    execute_hotspot - execute the hotspot
    command
-------------------------------------------------*/

static void execute_hotspot(int ref, int params, const char *param[])
{
	UINT64 threshhold;
	UINT64 cpunum;
	UINT64 count;

	/* if no params, and there are live hotspots, clear them */
	if (params == 0)
	{
		int cleared = FALSE;

		/* loop over CPUs and find live spots */
		for (cpunum = 0; cpunum < MAX_CPU; cpunum++)
		{
			const debug_cpu_info *cpuinfo = debug_get_cpu_info(cpunum);

			if (cpuinfo->valid && cpuinfo->hotspots)
			{
				debug_hotspot_track(cpunum, 0, 0);
				debug_console_printf("Cleared hotspot tracking on CPU %d\n", (int)cpunum);
				cleared = TRUE;
			}
		}

		/* if we cleared, we're done */
		if (cleared)
			return;
	}

	/* extract parameters */
	cpunum = cpu_getactivecpu();
	count = 64;
	threshhold = 250;
	if (params > 0 && !debug_command_parameter_number(param[0], &cpunum))
		return;
	if (params > 1 && !debug_command_parameter_number(param[1], &count))
		return;
	if (params > 2 && !debug_command_parameter_number(param[2], &threshhold))
		return;

	/* attempt to install */
	if (debug_hotspot_track(cpunum, count, threshhold))
		debug_console_printf("Now tracking hotspots on CPU %d using %d slots with a threshhold of %d\n", (int)cpunum, (int)count, (int)threshhold);
	else
		debug_console_printf("Error setting up the hotspot tracking\n");
}


/*-------------------------------------------------
    execute_save - execute the save command
-------------------------------------------------*/

static void execute_save(int ref, int params, const char *param[])
{
	UINT64 offset, endoffset, length, cpunum = cpu_getactivecpu();
	const debug_cpu_info *info;
	int spacenum = ref;
	FILE *f;
	UINT64 i;

	/* validate parameters */
	if (!debug_command_parameter_number(param[1], &offset))
		return;
	if (!debug_command_parameter_number(param[2], &length))
		return;
	if (params > 3 && !debug_command_parameter_number(param[3], &cpunum))
		return;

	/* determine the addresses to write */
	info = debug_get_cpu_info(cpunum);
	endoffset = ADDR2BYTE_MASKED(offset + length - 1, info, spacenum);
	offset = ADDR2BYTE_MASKED(offset, info, spacenum);

	/* open the file */
	f = fopen(param[0], "wb");
	if (!f)
	{
		debug_console_printf("Error opening file '%s'\n", param[0]);
		return;
	}

	/* now write the data out */
	cpuintrf_push_context(cpunum);
	for (i = offset; i <= endoffset; i++)
	{
		UINT8 byte = debug_read_byte(spacenum, i, TRUE);
		fwrite(&byte, 1, 1, f);
	}
	cpuintrf_pop_context();

	/* close the file */
	fclose(f);
	debug_console_printf("Data saved successfully\n");
}


/*-------------------------------------------------
    execute_dump - execute the dump command
-------------------------------------------------*/

static void execute_dump(int ref, int params, const char *param[])
{
	UINT64 offset, endoffset, length, width = 0, ascii = 1, cpunum = cpu_getactivecpu();
	const debug_cpu_info *info;
	int spacenum = ref;
	FILE *f = NULL;
	UINT64 i, j;

	/* validate parameters */
	if (!debug_command_parameter_number(param[1], &offset))
		return;
	if (!debug_command_parameter_number(param[2], &length))
		return;
	if (params > 3 && !debug_command_parameter_number(param[3], &width))
		return;
	if (params > 4 && !debug_command_parameter_number(param[4], &ascii))
		return;
	if (params > 5 && !debug_command_parameter_number(param[5], &cpunum))
		return;

	/* further validation */
	if (cpunum >= cpu_gettotalcpu())
	{
		debug_console_printf("Invalid CPU number!\n");
		return;
	}
	info = debug_get_cpu_info(cpunum);
	if (width == 0)
		width = info->space[spacenum].databytes;
	if (width < ADDR2BYTE(1, info, spacenum))
		width = ADDR2BYTE(1, info, spacenum);
	if (width != 1 && width != 2 && width != 4 && width != 8)
	{
		debug_console_printf("Invalid width! (must be 1,2,4 or 8)\n");
		return;
	}
	endoffset = ADDR2BYTE_MASKED(offset + length - 1, info, spacenum);
	offset = ADDR2BYTE_MASKED(offset, info, spacenum);

	/* open the file */
	f = fopen(param[0], "w");
	if (!f)
	{
		debug_console_printf("Error opening file '%s'\n", param[0]);
		return;
	}

	/* now write the data out */
	cpuintrf_push_context(cpunum);
	for (i = offset; i <= endoffset; i += 16)
	{
		char output[200];
		int outdex = 0;

		/* print the address */
		outdex += sprintf(&output[outdex], "%0*X: ", info->space[spacenum].logchars, (UINT32)BYTE2ADDR(i, info, spacenum));

		/* print the bytes */
		switch (width)
		{
			case 1:
				for (j = 0; j < 16; j++)
				{
					if (i + j <= endoffset)
					{
						offs_t curaddr = i + j;
						if (!info->translate || (*info->translate)(spacenum, &curaddr))
						{
							UINT8 byte = debug_read_byte(ref, i + j, TRUE);
							outdex += sprintf(&output[outdex], " %02X", byte);
						}
						else
							outdex += sprintf(&output[outdex], " **");
					}
					else
						outdex += sprintf(&output[outdex], "   ");
				}
				break;

			case 2:
				for (j = 0; j < 16; j += 2)
				{
					if (i + j <= endoffset)
					{
						offs_t curaddr = i + j;
						if (!info->translate || (*info->translate)(spacenum, &curaddr))
						{
							UINT16 word = debug_read_word(ref, i + j, TRUE);
							outdex += sprintf(&output[outdex], " %04X", word);
						}
						else
							outdex += sprintf(&output[outdex], " ****");
					}
					else
						outdex += sprintf(&output[outdex], "     ");
				}
				break;

			case 4:
				for (j = 0; j < 16; j += 4)
				{
					if (i + j <= endoffset)
					{
						offs_t curaddr = i + j;
						if (!info->translate || (*info->translate)(spacenum, &curaddr))
						{
							UINT32 dword = debug_read_dword(ref, i + j, TRUE);
							outdex += sprintf(&output[outdex], " %08X", dword);
						}
						else
							outdex += sprintf(&output[outdex], " ********");
					}
					else
						outdex += sprintf(&output[outdex], "         ");
				}
				break;

			case 8:
				for (j = 0; j < 16; j += 8)
				{
					if (i + j <= endoffset)
					{
						offs_t curaddr = i + j;
						if (!info->translate || (*info->translate)(spacenum, &curaddr))
						{
							UINT64 qword = debug_read_qword(ref, i + j, TRUE);
							outdex += sprintf(&output[outdex], " %08X%08X", (UINT32)(qword >> 32), (UINT32)qword);
						}
						else
							outdex += sprintf(&output[outdex], " ****************");
					}
					else
						outdex += sprintf(&output[outdex], "                 ");
				}
				break;
		}

		/* print the ASCII */
		if (ascii)
		{
			outdex += sprintf(&output[outdex], "  ");
			for (j = 0; j < 16 && (i + j) <= endoffset; j++)
			{
				offs_t curaddr = i + j;
				if (!info->translate || (*info->translate)(spacenum, &curaddr))
				{
					UINT8 byte = debug_read_byte(ref, i + j, TRUE);
					outdex += sprintf(&output[outdex], "%c", (byte >= 32 && byte < 128) ? byte : '.');
				}
				else
					outdex += sprintf(&output[outdex], " ");
			}
		}

		/* output the result */
		fprintf(f, "%s\n", output);
	}
	cpuintrf_pop_context();

	/* close the file */
	fclose(f);
	debug_console_printf("Data dumped successfully\n");
}


/*-------------------------------------------------
    execute_find - execute the find command
-------------------------------------------------*/

static void execute_find(int ref, int params, const char *param[])
{
	UINT64 offset, endoffset, length, cpunum = cpu_getactivecpu();
	const debug_cpu_info *info;
	UINT64 data_to_find[256];
	UINT8 data_size[256];
	int cur_data_size;
	int data_count = 0;
	int spacenum = ref;
	int found = 0;
	UINT64 i, j;

	/* validate parameters */
	if (!debug_command_parameter_number(param[0], &offset))
		return;
	if (!debug_command_parameter_number(param[1], &length))
		return;

	/* further validation */
	if (cpunum >= cpu_gettotalcpu())
	{
		debug_console_printf("Invalid CPU number!\n");
		return;
	}
	info = debug_get_cpu_info(cpunum);
	endoffset = ADDR2BYTE_MASKED(offset + length - 1, info, spacenum);
	offset = ADDR2BYTE_MASKED(offset, info, spacenum);
	cur_data_size = ADDR2BYTE(1, info, spacenum);
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
			if (tolower(pdata[0]) == 'b' && pdata[1] == '.') { data_size[data_count] = cur_data_size = 1; pdata += 2; }
			if (tolower(pdata[0]) == 'w' && pdata[1] == '.') { data_size[data_count] = cur_data_size = 2; pdata += 2; }
			if (tolower(pdata[0]) == 'd' && pdata[1] == '.') { data_size[data_count] = cur_data_size = 4; pdata += 2; }
			if (tolower(pdata[0]) == 'q' && pdata[1] == '.') { data_size[data_count] = cur_data_size = 8; pdata += 2; }

			/* look for a wildcard */
			if (!strcmp(pdata, "?"))
				data_size[data_count++] |= 0x10;

			/* otherwise, validate as a number */
			else if (!debug_command_parameter_number(pdata, &data_to_find[data_count++]))
				return;
		}
	}

	/* now search */
	cpuintrf_push_context(cpunum);
	for (i = offset; i <= endoffset; i += data_size[0])
	{
		int suboffset = 0;
		int match = 1;

		/* find the entire string */
		for (j = 0; j < data_count && match; j++)
		{
			switch (data_size[j])
			{
			case 1:	match = ((UINT8)debug_read_byte(spacenum, i + suboffset, TRUE) == (UINT8)data_to_find[j]);	break;
			case 2:	match = ((UINT16)debug_read_word(spacenum, i + suboffset, TRUE) == (UINT16)data_to_find[j]);	break;
			case 4:	match = ((UINT32)debug_read_dword(spacenum, i + suboffset, TRUE) == (UINT32)data_to_find[j]);	break;
			case 8:	match = ((UINT64)debug_read_qword(spacenum, i + suboffset, TRUE) == (UINT64)data_to_find[j]);	break;
				default:	/* all other cases are wildcards */		break;
			}
			suboffset += data_size[j] & 0x0f;
		}

		/* did we find it? */
		if (match)
		{
			found++;
			debug_console_printf("Found at %*X\n", info->space[spacenum].logchars, (UINT32)BYTE2ADDR(i, info, spacenum));
		}
	}
	cpuintrf_pop_context();

	/* print something if not found */
	if (found == 0)
		debug_console_printf("Not found\n");
}


/*-------------------------------------------------
    execute_dasm - execute the dasm command
-------------------------------------------------*/

static void execute_dasm(int ref, int params, const char *param[])
{
	UINT64 offset, length, bytes = 1, cpunum = cpu_getactivecpu();
	const debug_cpu_info *info;
	int minbytes, maxbytes, byteswidth;
	FILE *f = NULL;
	int i, j;

	/* validate parameters */
	if (!debug_command_parameter_number(param[1], &offset))
		return;
	if (!debug_command_parameter_number(param[2], &length))
		return;
	if (params > 3 && !debug_command_parameter_number(param[3], &bytes))
		return;
	if (params > 4 && !debug_command_parameter_number(param[4], &cpunum))
		return;

	/* further validation */
	if (cpunum >= cpu_gettotalcpu())
	{
		debug_console_printf("Invalid CPU number!\n");
		return;
	}
	info = debug_get_cpu_info(cpunum);

	/* determine the width of the bytes */
	minbytes = cpunum_min_instruction_bytes(cpunum);
	maxbytes = cpunum_max_instruction_bytes(cpunum);
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
		debug_console_printf("Error opening file '%s'\n", param[0]);
		return;
	}

	/* now write the data out */
	cpuintrf_push_context(cpunum);
	for (i = 0; i < length; )
	{
		int pcbyte = ADDR2BYTE_MASKED(offset + i, info, ADDRESS_SPACE_PROGRAM);
		char output[200+DEBUG_COMMENT_MAX_LINE_LENGTH], disasm[200];
		const char *comment;
		offs_t tempaddr;
		int outdex = 0;
		int numbytes = 0;

		/* print the address */
		outdex += sprintf(&output[outdex], "%0*X: ", info->space[ADDRESS_SPACE_PROGRAM].logchars, (UINT32)BYTE2ADDR(pcbyte, info, ADDRESS_SPACE_PROGRAM));

		/* make sure we can translate the address */
		tempaddr = pcbyte;
		if (!info->translate || (*info->translate)(ADDRESS_SPACE_PROGRAM, &tempaddr))
		{
			UINT8 opbuf[64], argbuf[64];

			/* fetch the bytes up to the maximum */
			for (numbytes = 0; numbytes < maxbytes; numbytes++)
			{
				opbuf[numbytes] = debug_read_opcode(pcbyte + numbytes, 1, FALSE);
				argbuf[numbytes] = debug_read_opcode(pcbyte + numbytes, 1, TRUE);
			}

			/* disassemble the result */
			i += numbytes = activecpu_dasm(disasm, offset + i, opbuf, argbuf) & DASMFLAG_LENGTHMASK;
		}

		/* print the bytes */
		if (bytes)
		{
			int startdex = outdex;
			numbytes = ADDR2BYTE(numbytes, info, ADDRESS_SPACE_PROGRAM);
			switch (minbytes)
			{
				case 1:
					for (j = 0; j < numbytes; j++)
						outdex += sprintf(&output[outdex], "%02X ", (UINT32)debug_read_opcode(pcbyte + j, 1, FALSE));
					break;

				case 2:
					for (j = 0; j < numbytes; j += 2)
						outdex += sprintf(&output[outdex], "%04X ", (UINT32)debug_read_opcode(pcbyte + j, 2, FALSE));
					break;

				case 4:
					for (j = 0; j < numbytes; j += 4)
						outdex += sprintf(&output[outdex], "%08X ", (UINT32)debug_read_opcode(pcbyte + j, 4, FALSE));
					break;

				case 8:
					for (j = 0; j < numbytes; j += 8)
					{
						UINT64 val = debug_read_opcode(pcbyte + j, 8, FALSE);
						outdex += sprintf(&output[outdex], "%08X%08X ", (UINT32)(val >> 32), (UINT32)val);
					}
					break;
			}
			if (outdex - startdex < byteswidth)
				outdex += sprintf(&output[outdex], "%*s", byteswidth - (outdex - startdex), "");
			outdex += sprintf(&output[outdex], "  ");
		}

		/* add the disassembly */
		sprintf(&output[outdex], "%s", disasm);

		/* attempt to add the comment */
		comment = debug_comment_get_text(cpunum, tempaddr, debug_comment_get_opcode_crc32(tempaddr));
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
	cpuintrf_pop_context();

	/* close the file */
	fclose(f);
	debug_console_printf("Data dumped successfully\n");
}


/*-------------------------------------------------
    execute_trace_internal - functionality for
    trace over and trace info
-------------------------------------------------*/

static void execute_trace_internal(int ref, int params, const char *param[], int trace_over)
{
	const char *action = NULL, *filename = param[0];
	FILE *f = NULL;
	const char *mode;
	UINT64 cpunum;

	cpunum = cpu_getactivecpu();

	/* validate parameters */
	if (params > 1 && !debug_command_parameter_number(param[1], &cpunum))
		return;
	if (params > 2 && !debug_command_parameter_command(action = param[2]))
		return;

	/* further validation */
	if (!mame_stricmp(filename, "off"))
		filename = NULL;
	if (cpunum >= cpu_gettotalcpu())
	{
		debug_console_printf("Invalid CPU number!\n");
		return;
	}

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
			debug_console_printf("Error opening file '%s'\n", param[0]);
			return;
		}
	}

	/* do it */
	debug_cpu_trace(cpunum, f, trace_over, action);
	if (f)
		debug_console_printf("Tracing CPU %d to file %s\n", (int)cpunum, filename);
	else
		debug_console_printf("Stopped tracing on CPU %d\n", (int)cpunum);
}


/*-------------------------------------------------
    execute_trace - execute the trace command
-------------------------------------------------*/

static void execute_trace(int ref, int params, const char *param[])
{
	execute_trace_internal(ref, params, param, 0);
}


/*-------------------------------------------------
    execute_traceover - execute the trace over command
-------------------------------------------------*/

static void execute_traceover(int ref, int params, const char *param[])
{
	execute_trace_internal(ref, params, param, 1);
}


/*-------------------------------------------------
    execute_traceflush - execute the trace flush command
-------------------------------------------------*/

static void execute_traceflush(int ref, int params, const char *param[])
{
	debug_flush_traces();
}


/*-------------------------------------------------
    execute_history - execute the history command
-------------------------------------------------*/

static void execute_history(int ref, int params, const char *param[])
{
	UINT64 count = DEBUG_HISTORY_SIZE;
	const debug_cpu_info *info;
	UINT64 cpunum;
	int i;

	cpunum = cpu_getactivecpu();

	/* validate parameters */
	if (params > 0 && !debug_command_parameter_number(param[0], &cpunum))
		return;
	if (params > 1 && !debug_command_parameter_number(param[1], &count))
		return;

	/* further validation */
	if (cpunum >= cpu_gettotalcpu())
	{
		debug_console_printf("Invalid CPU number!\n");
		return;
	}
	if (count > DEBUG_HISTORY_SIZE)
		count = DEBUG_HISTORY_SIZE;

	info = debug_get_cpu_info(cpunum);

	/* loop over lines */
	cpuintrf_push_context(cpunum);
	for (i = 0; i < count; i++)
	{
		offs_t pc = info->pc_history[(info->pc_history_index + DEBUG_HISTORY_SIZE - count + i) % DEBUG_HISTORY_SIZE];
		int maxbytes = activecpu_max_instruction_bytes();
		UINT8 opbuf[64], argbuf[64];
		char buffer[200];
		offs_t pcbyte;
		int numbytes;

		/* fetch the bytes up to the maximum */
		pcbyte = ADDR2BYTE_MASKED(pc, info, ADDRESS_SPACE_PROGRAM);
		for (numbytes = 0; numbytes < maxbytes; numbytes++)
		{
			opbuf[numbytes] = debug_read_opcode(pcbyte + numbytes, 1, FALSE);
			argbuf[numbytes] = debug_read_opcode(pcbyte + numbytes, 1, TRUE);
		}

		activecpu_dasm(buffer, pc, opbuf, argbuf);

		debug_console_printf("%0*X: %s\n", info->space[ADDRESS_SPACE_PROGRAM].logchars, pc, buffer);
	}
	cpuintrf_pop_context();
}


/*-------------------------------------------------
    execute_snap - execute the trace over command
-------------------------------------------------*/

static void execute_snap(int ref, int params, const char *param[])
{
	/* if no params, use the default behavior */
	if (params == 0)
	{
		video_save_active_screen_snapshots(Machine);
		debug_console_printf("Saved snapshot\n");
	}

	/* otherwise, we have to open the file ourselves */
	else
	{
		file_error filerr;
		mame_file *fp;
		const char *filename = param[0];
		int scrnum = (params > 1) ? atoi(param[1]) : 0;
		UINT32 mask = render_get_live_screens_mask();
		astring *fname;

		if (scrnum < 0 || scrnum >= MAX_SCREENS || !(mask & (1 << scrnum)))
		{
			debug_console_printf("Invalid screen number '%d'\n", scrnum);
			return;
		}

		fname = astring_dupc(filename);
		if (astring_findc(fname, 0, ".png") == -1)
			astring_catc(fname, ".png");
		filerr = mame_fopen(SEARCHPATH_SCREENSHOT, astring_c(fname), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, &fp);
		astring_free(fname);

		if (filerr != FILERR_NONE)
		{
			debug_console_printf("Error creating file '%s'\n", filename);
			return;
		}

		video_screen_save_snapshot(Machine, fp, scrnum);
		mame_fclose(fp);
		debug_console_printf("Saved screen #%d snapshot as '%s'\n", scrnum, filename);
	}
}


/*-------------------------------------------------
    execute_source - execute the source command
-------------------------------------------------*/

static void execute_source(int ref, int params, const char *param[])
{
	debug_source_script(param[0]);
}


/*-------------------------------------------------
    execute_map - execute the map command
-------------------------------------------------*/

static void execute_map(int ref, int params, const char *param[])
{
	UINT64 address, cpunum = cpu_getactivecpu();
	const debug_cpu_info *info;
	int spacenum = ref;
	offs_t taddress;

	/* validate parameters */
	if (!debug_command_parameter_number(param[0], &address))
		return;
	info = debug_get_cpu_info(cpunum);

	/* do the translation first */
	taddress = ADDR2BYTE_MASKED(address, info, spacenum);
	if (info->translate)
	{
		if ((*info->translate)(spacenum, &taddress))
			debug_console_printf("%08X logical -> %08X physical\n", (UINT32)address, BYTE2ADDR(taddress, info, spacenum));
		else
		{
			debug_console_printf("%08X logical -> unmapped\n", (UINT32)address);
			return;
		}
	}
	else
		debug_console_printf("%08X physical\n", BYTE2ADDR(taddress, info, spacenum));

	/* now do the mapping */
	debug_console_printf("  -> read: %s\n", memory_get_handler_string(0, cpunum, spacenum, taddress));
	debug_console_printf("  -> write: %s\n", memory_get_handler_string(1, cpunum, spacenum, taddress));
}


/*-------------------------------------------------
    execute_memdump - execute the memdump command
-------------------------------------------------*/

static void execute_memdump(int ref, int params, const char **param)
{
	FILE *file;
	const char *filename;

	filename = (params == 0) ? "memdump.log" : param[0];

	debug_console_printf("Dumping memory to %s\n", filename);

	file = fopen(filename, "w");
	if (file)
	{
		memory_dump(file);
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

static void execute_symlist(int ref, int params, const char **param)
{
	const char *namelist[1000];
	symbol_table *symtable;
	UINT64 cpunum = 100000;
	int symnum, count = 0;

	/* validate parameters */
	if (params > 0 && !debug_command_parameter_number(param[0], &cpunum))
		return;
	if (cpunum != 100000 && cpunum >= cpu_gettotalcpu())
	{
		debug_console_printf("Invalid CPU number!\n");
		return;
	}
	symtable = (cpunum == 100000) ? global_symtable : debug_get_cpu_info(cpunum)->symtable;

	if (symtable == global_symtable)
		debug_console_printf("Global symbols:\n");
	else
		debug_console_printf("CPU #%d symbols:\n", (UINT32)cpunum);

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
		UINT64 value = (*entry->info.reg.getter)(entry->ref);
		assert(entry != NULL);

		/* only display "register" type symbols */
		debug_console_printf("%s = ", namelist[symnum]);
		if ((value >> 32) != 0)
			debug_console_printf("%X%08X", (UINT32)(value >> 32), (UINT32)value);
		else
			debug_console_printf("%X", (UINT32)value);
		if (entry->info.reg.setter == NULL)
			debug_console_printf("  (read-only)");
		debug_console_printf("\n");
	}
}


/*-------------------------------------------------
    execute_softreset - execute the softreset command
-------------------------------------------------*/

static void execute_softreset(int ref, int params, const char **param)
{
	mame_schedule_soft_reset(Machine);
}


/*-------------------------------------------------
    execute_hardreset - execute the hardreset command
-------------------------------------------------*/

static void execute_hardreset(int ref, int params, const char **param)
{
	mame_schedule_hard_reset(Machine);
}
