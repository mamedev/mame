// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    debugcmd.c

    Debugger command interface engine.

*********************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "debugcmd.h"
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

#define MAX_GLOBALS     1000



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct global_entry
{
	void *      base;
	UINT32      size;
};


struct cheat_map
{
	UINT64      offset;
	UINT64      first_value;
	UINT64      previous_value;
	UINT8       state:1;
	UINT8       undo:7;
};


struct cheat_system
{
	char        cpu[2];
	UINT8       width;
	std::vector<cheat_map> cheatmap;
	UINT8       undo;
	UINT8       signed_cheat;
	UINT8       swapped_cheat;
};


struct cheat_region_map
{
	UINT64      offset;
	UINT64      endoffset;
	const char *share;
	UINT8       disabled;
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static global_entry global_array[MAX_GLOBALS];
static cheat_system cheat;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void debug_command_exit(running_machine &machine);

static UINT64 execute_min(symbol_table &table, void *ref, int params, const UINT64 *param);
static UINT64 execute_max(symbol_table &table, void *ref, int params, const UINT64 *param);
static UINT64 execute_if(symbol_table &table, void *ref, int params, const UINT64 *param);

static UINT64 global_get(symbol_table &table, void *ref);
static void global_set(symbol_table &table, void *ref, UINT64 value);

static void execute_help(running_machine &machine, int ref, int params, const char **param);
static void execute_print(running_machine &machine, int ref, int params, const char **param);
static void execute_printf(running_machine &machine, int ref, int params, const char **param);
static void execute_logerror(running_machine &machine, int ref, int params, const char **param);
static void execute_tracelog(running_machine &machine, int ref, int params, const char **param);
static void execute_quit(running_machine &machine, int ref, int params, const char **param);
static void execute_do(running_machine &machine, int ref, int params, const char **param);
static void execute_step(running_machine &machine, int ref, int params, const char **param);
static void execute_over(running_machine &machine, int ref, int params, const char **param);
static void execute_out(running_machine &machine, int ref, int params, const char **param);
static void execute_go(running_machine &machine, int ref, int params, const char **param);
static void execute_go_vblank(running_machine &machine, int ref, int params, const char **param);
static void execute_go_interrupt(running_machine &machine, int ref, int params, const char **param);
static void execute_go_time(running_machine &machine, int ref, int params, const char *param[]);
static void execute_focus(running_machine &machine, int ref, int params, const char **param);
static void execute_ignore(running_machine &machine, int ref, int params, const char **param);
static void execute_observe(running_machine &machine, int ref, int params, const char **param);
static void execute_next(running_machine &machine, int ref, int params, const char **param);
static void execute_comment(running_machine &machine, int ref, int params, const char **param);
static void execute_comment_del(running_machine &machine, int ref, int params, const char **param);
static void execute_comment_save(running_machine &machine, int ref, int params, const char **param);
static void execute_bpset(running_machine &machine, int ref, int params, const char **param);
static void execute_bpclear(running_machine &machine, int ref, int params, const char **param);
static void execute_bpdisenable(running_machine &machine, int ref, int params, const char **param);
static void execute_bplist(running_machine &machine, int ref, int params, const char **param);
static void execute_wpset(running_machine &machine, int ref, int params, const char **param);
static void execute_wpclear(running_machine &machine, int ref, int params, const char **param);
static void execute_wpdisenable(running_machine &machine, int ref, int params, const char **param);
static void execute_wplist(running_machine &machine, int ref, int params, const char **param);
static void execute_rpset(running_machine &machine, int ref, int params, const char **param);
static void execute_rpclear(running_machine &machine, int ref, int params, const char **param);
static void execute_rpdisenable(running_machine &machine, int ref, int params, const char **param);
static void execute_rplist(running_machine &machine, int ref, int params, const char **param);
static void execute_hotspot(running_machine &machine, int ref, int params, const char **param);
static void execute_statesave(running_machine &machine, int ref, int params, const char **param);
static void execute_stateload(running_machine &machine, int ref, int params, const char **param);
static void execute_save(running_machine &machine, int ref, int params, const char **param);
static void execute_load(running_machine &machine, int ref, int params, const char **param);
static void execute_dump(running_machine &machine, int ref, int params, const char **param);
static void execute_cheatinit(running_machine &machine, int ref, int params, const char **param);
static void execute_cheatnext(running_machine &machine, int ref, int params, const char **param);
static void execute_cheatlist(running_machine &machine, int ref, int params, const char **param);
static void execute_cheatundo(running_machine &machine, int ref, int params, const char **param);
static void execute_dasm(running_machine &machine, int ref, int params, const char **param);
static void execute_find(running_machine &machine, int ref, int params, const char **param);
static void execute_trace(running_machine &machine, int ref, int params, const char **param);
static void execute_traceover(running_machine &machine, int ref, int params, const char **param);
static void execute_traceflush(running_machine &machine, int ref, int params, const char **param);
static void execute_history(running_machine &machine, int ref, int params, const char **param);
static void execute_trackpc(running_machine &machine, int ref, int params, const char **param);
static void execute_trackmem(running_machine &machine, int ref, int params, const char **param);
static void execute_pcatmem(running_machine &machine, int ref, int params, const char **param);
static void execute_snap(running_machine &machine, int ref, int params, const char **param);
static void execute_source(running_machine &machine, int ref, int params, const char **param);
static void execute_map(running_machine &machine, int ref, int params, const char **param);
static void execute_memdump(running_machine &machine, int ref, int params, const char **param);
static void execute_symlist(running_machine &machine, int ref, int params, const char **param);
static void execute_softreset(running_machine &machine, int ref, int params, const char **param);
static void execute_hardreset(running_machine &machine, int ref, int params, const char **param);
static void execute_images(running_machine &machine, int ref, int params, const char **param);
static void execute_mount(running_machine &machine, int ref, int params, const char **param);
static void execute_unmount(running_machine &machine, int ref, int params, const char **param);
static void execute_input(running_machine &machine, int ref, int params, const char **param);
static void execute_dumpkbd(running_machine &machine, int ref, int params, const char **param);


/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    cheat_address_is_valid - return TRUE if the
    given address is valid for cheating
-------------------------------------------------*/

INLINE int cheat_address_is_valid(address_space &space, offs_t address)
{
	return debug_cpu_translate(space, TRANSLATE_READ, &address) && (space.get_write_ptr(address) != nullptr);
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
			case 1: value = (INT8)value;    break;
			case 2: value = (INT16)value;   break;
			case 4: value = (INT32)value;   break;
		}
	}
	return value;
}
/*-------------------------------------------------
    cheat_byte_swap - swap a value
-------------------------------------------------*/

INLINE UINT64 cheat_byte_swap(const cheat_system *cheatsys, UINT64 value)
{
	if (cheatsys->swapped_cheat)
	{
		switch (cheatsys->width)
		{
			case 2: value = ((value >> 8) & 0x00ff) | ((value << 8) & 0xff00);  break;
			case 4: value = ((value >> 24) & 0x000000ff) | ((value >> 8) & 0x0000ff00) | ((value << 8) & 0x00ff0000) | ((value << 24) & 0xff000000);    break;
			case 8: value = ((value >> 56) & U64(0x00000000000000ff)) | ((value >> 40) & U64(0x000000000000ff00)) | ((value >> 24) & U64(0x0000000000ff0000)) | ((value >> 8) & U64(0x00000000ff000000)) |
							((value << 8) & U64(0x000000ff00000000)) | ((value << 24) & U64(0x0000ff0000000000)) | ((value << 40) & U64(0x00ff000000000000)) | ((value << 56) & U64(0xff00000000000000));   break;
		}
	}
	return value;
}

/*-------------------------------------------------
    cheat_read_extended - read a value from memory
    in the given address space, sign-extending
    and swapping if necessary
-------------------------------------------------*/

INLINE UINT64 cheat_read_extended(const cheat_system *cheatsys, address_space &space, offs_t address)
{
	return cheat_sign_extend(cheatsys, cheat_byte_swap(cheatsys, debug_read_memory(space, address, cheatsys->width, TRUE)));
}



/***************************************************************************
    INITIALIZATION
***************************************************************************/

/*-------------------------------------------------
    debug_command_init - initializes the command
    system
-------------------------------------------------*/

void debug_command_init(running_machine &machine)
{
	symbol_table *symtable = debug_cpu_get_global_symtable(machine);
	const char *name;
	int itemnum;

	/* add a few simple global functions */
	symtable->add("min", nullptr, 2, 2, execute_min);
	symtable->add("max", nullptr, 2, 2, execute_max);
	symtable->add("if", nullptr, 3, 3, execute_if);

	/* add all single-entry save state globals */
	for (itemnum = 0; itemnum < MAX_GLOBALS; itemnum++)
	{
		UINT32 valsize, valcount;
		void *base;

		/* stop when we run out of items */
		name = machine.save().indexed_item(itemnum, base, valsize, valcount);
		if (name == nullptr)
			break;

		/* if this is a single-entry global, add it */
		if (valcount == 1 && strstr(name, "/globals/"))
		{
			char symname[100];
			sprintf(symname, ".%s", strrchr(name, '/') + 1);
			global_array[itemnum].base = base;
			global_array[itemnum].size = valsize;
			symtable->add(symname, &global_array, global_get, global_set);
		}
	}

	/* add all the commands */
	debug_console_register_command(machine, "help",      CMDFLAG_NONE, 0, 0, 1, execute_help);
	debug_console_register_command(machine, "print",     CMDFLAG_NONE, 0, 1, MAX_COMMAND_PARAMS, execute_print);
	debug_console_register_command(machine, "printf",    CMDFLAG_NONE, 0, 1, MAX_COMMAND_PARAMS, execute_printf);
	debug_console_register_command(machine, "logerror",  CMDFLAG_NONE, 0, 1, MAX_COMMAND_PARAMS, execute_logerror);
	debug_console_register_command(machine, "tracelog",  CMDFLAG_NONE, 0, 1, MAX_COMMAND_PARAMS, execute_tracelog);
	debug_console_register_command(machine, "quit",      CMDFLAG_NONE, 0, 0, 0, execute_quit);
	debug_console_register_command(machine, "exit",      CMDFLAG_NONE, 0, 0, 0, execute_quit);
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

	debug_console_register_command(machine, "comadd",    CMDFLAG_NONE, 0, 1, 2, execute_comment);
	debug_console_register_command(machine, "//",        CMDFLAG_NONE, 0, 1, 2, execute_comment);
	debug_console_register_command(machine, "comdelete", CMDFLAG_NONE, 0, 1, 1, execute_comment_del);
	debug_console_register_command(machine, "comsave",   CMDFLAG_NONE, 0, 0, 0, execute_comment_save);

	debug_console_register_command(machine, "bpset",     CMDFLAG_NONE, 0, 1, 3, execute_bpset);
	debug_console_register_command(machine, "bp",        CMDFLAG_NONE, 0, 1, 3, execute_bpset);
	debug_console_register_command(machine, "bpclear",   CMDFLAG_NONE, 0, 0, 1, execute_bpclear);
	debug_console_register_command(machine, "bpdisable", CMDFLAG_NONE, 0, 0, 1, execute_bpdisenable);
	debug_console_register_command(machine, "bpenable",  CMDFLAG_NONE, 1, 0, 1, execute_bpdisenable);
	debug_console_register_command(machine, "bplist",    CMDFLAG_NONE, 0, 0, 0, execute_bplist);

	debug_console_register_command(machine, "wpset",     CMDFLAG_NONE, AS_PROGRAM, 3, 5, execute_wpset);
	debug_console_register_command(machine, "wp",        CMDFLAG_NONE, AS_PROGRAM, 3, 5, execute_wpset);
	debug_console_register_command(machine, "wpdset",    CMDFLAG_NONE, AS_DATA, 3, 5, execute_wpset);
	debug_console_register_command(machine, "wpd",       CMDFLAG_NONE, AS_DATA, 3, 5, execute_wpset);
	debug_console_register_command(machine, "wpiset",    CMDFLAG_NONE, AS_IO, 3, 5, execute_wpset);
	debug_console_register_command(machine, "wpi",       CMDFLAG_NONE, AS_IO, 3, 5, execute_wpset);
	debug_console_register_command(machine, "wpclear",   CMDFLAG_NONE, 0, 0, 1, execute_wpclear);
	debug_console_register_command(machine, "wpdisable", CMDFLAG_NONE, 0, 0, 1, execute_wpdisenable);
	debug_console_register_command(machine, "wpenable",  CMDFLAG_NONE, 1, 0, 1, execute_wpdisenable);
	debug_console_register_command(machine, "wplist",    CMDFLAG_NONE, 0, 0, 0, execute_wplist);

	debug_console_register_command(machine, "rpset",     CMDFLAG_NONE, 0, 1, 2, execute_rpset);
	debug_console_register_command(machine, "rp",        CMDFLAG_NONE, 0, 1, 2, execute_rpset);
	debug_console_register_command(machine, "rpclear",   CMDFLAG_NONE, 0, 0, 1, execute_rpclear);
	debug_console_register_command(machine, "rpdisable", CMDFLAG_NONE, 0, 0, 1, execute_rpdisenable);
	debug_console_register_command(machine, "rpenable",  CMDFLAG_NONE, 1, 0, 1, execute_rpdisenable);
	debug_console_register_command(machine, "rplist",    CMDFLAG_NONE, 0, 0, 0, execute_rplist);

	debug_console_register_command(machine, "hotspot",   CMDFLAG_NONE, 0, 0, 3, execute_hotspot);

	debug_console_register_command(machine, "statesave", CMDFLAG_NONE, 0, 1, 1, execute_statesave);
	debug_console_register_command(machine, "ss",        CMDFLAG_NONE, 0, 1, 1, execute_statesave);
	debug_console_register_command(machine, "stateload", CMDFLAG_NONE, 0, 1, 1, execute_stateload);
	debug_console_register_command(machine, "sl",        CMDFLAG_NONE, 0, 1, 1, execute_stateload);

	debug_console_register_command(machine, "save",      CMDFLAG_NONE, AS_PROGRAM, 3, 4, execute_save);
	debug_console_register_command(machine, "saved",     CMDFLAG_NONE, AS_DATA, 3, 4, execute_save);
	debug_console_register_command(machine, "savei",     CMDFLAG_NONE, AS_IO, 3, 4, execute_save);

	debug_console_register_command(machine, "load",      CMDFLAG_NONE, AS_PROGRAM, 3, 4, execute_load);
	debug_console_register_command(machine, "loadd",     CMDFLAG_NONE, AS_DATA, 3, 4, execute_load);
	debug_console_register_command(machine, "loadi",     CMDFLAG_NONE, AS_IO, 3, 4, execute_load);

	debug_console_register_command(machine, "dump",      CMDFLAG_NONE, AS_PROGRAM, 3, 6, execute_dump);
	debug_console_register_command(machine, "dumpd",     CMDFLAG_NONE, AS_DATA, 3, 6, execute_dump);
	debug_console_register_command(machine, "dumpi",     CMDFLAG_NONE, AS_IO, 3, 6, execute_dump);

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

	debug_console_register_command(machine, "f",         CMDFLAG_KEEP_QUOTES, AS_PROGRAM, 3, MAX_COMMAND_PARAMS, execute_find);
	debug_console_register_command(machine, "find",      CMDFLAG_KEEP_QUOTES, AS_PROGRAM, 3, MAX_COMMAND_PARAMS, execute_find);
	debug_console_register_command(machine, "fd",        CMDFLAG_KEEP_QUOTES, AS_DATA, 3, MAX_COMMAND_PARAMS, execute_find);
	debug_console_register_command(machine, "findd",     CMDFLAG_KEEP_QUOTES, AS_DATA, 3, MAX_COMMAND_PARAMS, execute_find);
	debug_console_register_command(machine, "fi",        CMDFLAG_KEEP_QUOTES, AS_IO, 3, MAX_COMMAND_PARAMS, execute_find);
	debug_console_register_command(machine, "findi",     CMDFLAG_KEEP_QUOTES, AS_IO, 3, MAX_COMMAND_PARAMS, execute_find);

	debug_console_register_command(machine, "dasm",      CMDFLAG_NONE, 0, 3, 5, execute_dasm);

	debug_console_register_command(machine, "trace",     CMDFLAG_NONE, 0, 1, 3, execute_trace);
	debug_console_register_command(machine, "traceover", CMDFLAG_NONE, 0, 1, 3, execute_traceover);
	debug_console_register_command(machine, "traceflush",CMDFLAG_NONE, 0, 0, 0, execute_traceflush);

	debug_console_register_command(machine, "history",   CMDFLAG_NONE, 0, 0, 2, execute_history);
	debug_console_register_command(machine, "trackpc",   CMDFLAG_NONE, 0, 0, 3, execute_trackpc);

	debug_console_register_command(machine, "trackmem",  CMDFLAG_NONE, 0, 0, 3, execute_trackmem);
	debug_console_register_command(machine, "pcatmemp",  CMDFLAG_NONE, AS_PROGRAM, 1, 2, execute_pcatmem);
	debug_console_register_command(machine, "pcatmemd",  CMDFLAG_NONE, AS_DATA,    1, 2, execute_pcatmem);
	debug_console_register_command(machine, "pcatmemi",  CMDFLAG_NONE, AS_IO,      1, 2, execute_pcatmem);

	debug_console_register_command(machine, "snap",      CMDFLAG_NONE, 0, 0, 1, execute_snap);

	debug_console_register_command(machine, "source",    CMDFLAG_NONE, 0, 1, 1, execute_source);

	debug_console_register_command(machine, "map",       CMDFLAG_NONE, AS_PROGRAM, 1, 1, execute_map);
	debug_console_register_command(machine, "mapd",      CMDFLAG_NONE, AS_DATA, 1, 1, execute_map);
	debug_console_register_command(machine, "mapi",      CMDFLAG_NONE, AS_IO, 1, 1, execute_map);
	debug_console_register_command(machine, "memdump",   CMDFLAG_NONE, 0, 0, 1, execute_memdump);

	debug_console_register_command(machine, "symlist",   CMDFLAG_NONE, 0, 0, 1, execute_symlist);

	debug_console_register_command(machine, "softreset", CMDFLAG_NONE, 0, 0, 1, execute_softreset);
	debug_console_register_command(machine, "hardreset", CMDFLAG_NONE, 0, 0, 1, execute_hardreset);

	debug_console_register_command(machine, "images",    CMDFLAG_NONE, 0, 0, 0, execute_images);
	debug_console_register_command(machine, "mount",     CMDFLAG_NONE, 0, 2, 2, execute_mount);
	debug_console_register_command(machine, "unmount",   CMDFLAG_NONE, 0, 1, 1, execute_unmount);

	debug_console_register_command(machine, "input",     CMDFLAG_NONE, 0, 1, 1, execute_input);
	debug_console_register_command(machine, "dumpkbd",   CMDFLAG_NONE, 0, 0, 1, execute_dumpkbd);

	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(debug_command_exit), &machine));

	/* set up the initial debugscript if specified */
	name = machine.options().debug_script();
	if (name[0] != 0)
		debug_cpu_source_script(machine, name);

	cheat.cpu[0] = cheat.cpu[1] = 0;
}


/*-------------------------------------------------
    debug_command_exit - exit-time cleanup
-------------------------------------------------*/

static void debug_command_exit(running_machine &machine)
{
}



/***************************************************************************
    GLOBAL FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    execute_min - return the minimum of two values
-------------------------------------------------*/

static UINT64 execute_min(symbol_table &table, void *ref, int params, const UINT64 *param)
{
	return (param[0] < param[1]) ? param[0] : param[1];
}


/*-------------------------------------------------
    execute_max - return the maximum of two values
-------------------------------------------------*/

static UINT64 execute_max(symbol_table &table, void *ref, int params, const UINT64 *param)
{
	return (param[0] > param[1]) ? param[0] : param[1];
}


/*-------------------------------------------------
    execute_if - if (a) return b; else return c;
-------------------------------------------------*/

static UINT64 execute_if(symbol_table &table, void *ref, int params, const UINT64 *param)
{
	return param[0] ? param[1] : param[2];
}



/***************************************************************************
    GLOBAL ACCESSORS
***************************************************************************/

/*-------------------------------------------------
    global_get - symbol table getter for globals
-------------------------------------------------*/

static UINT64 global_get(symbol_table &table, void *ref)
{
	global_entry *global = (global_entry *)ref;
	switch (global->size)
	{
		case 1:     return *(UINT8 *)global->base;
		case 2:     return *(UINT16 *)global->base;
		case 4:     return *(UINT32 *)global->base;
		case 8:     return *(UINT64 *)global->base;
	}
	return ~0;
}


/*-------------------------------------------------
    global_set - symbol table setter for globals
-------------------------------------------------*/

static void global_set(symbol_table &table, void *ref, UINT64 value)
{
	global_entry *global = (global_entry *)ref;
	switch (global->size)
	{
		case 1:     *(UINT8 *)global->base = value; break;
		case 2:     *(UINT16 *)global->base = value;    break;
		case 4:     *(UINT32 *)global->base = value;    break;
		case 8:     *(UINT64 *)global->base = value;    break;
	}
}



/***************************************************************************
    PARAMETER VALIDATION HELPERS
***************************************************************************/

/*-------------------------------------------------
    debug_command_parameter_number - validates a
    number parameter
-------------------------------------------------*/

int debug_command_parameter_number(running_machine &machine, const char *param, UINT64 *result)
{
	/* NULL parameter does nothing and returns no error */
	if (param == nullptr)
		return TRUE;

	/* evaluate the expression; success if no error */
	try
	{
		parsed_expression expression(debug_cpu_get_visible_symtable(machine), param, result);
		return TRUE;
	}
	catch (expression_error &error)
	{
		/* print an error pointing to the character that caused it */
		debug_console_printf(machine, "Error in expression: %s\n", param);
		debug_console_printf(machine, "                     %*s^", error.offset(), "");
		debug_console_printf(machine, "%s\n", error.code_string());
		return FALSE;
	}
}


/*-------------------------------------------------
    debug_command_parameter_cpu - validates a
    parameter as a cpu
-------------------------------------------------*/

int debug_command_parameter_cpu(running_machine &machine, const char *param, device_t **result)
{
	UINT64 cpunum;

	/* if no parameter, use the visible CPU */
	if (param == nullptr)
	{
		*result = debug_cpu_get_visible_cpu(machine);
		if (*result == nullptr)
		{
			debug_console_printf(machine, "No valid CPU is currently selected\n");
			return FALSE;
		}
		return TRUE;
	}

	/* first look for a tag match */
	*result = machine.device(param);
	if (*result != nullptr)
		return TRUE;

	/* then evaluate as an expression; on an error assume it was a tag */
	try
	{
		parsed_expression expression(debug_cpu_get_visible_symtable(machine), param, &cpunum);
	}
	catch (expression_error &)
	{
		debug_console_printf(machine, "Unable to find CPU '%s'\n", param);
		return FALSE;
	}

	/* if we got a valid one, return */
	const UINT64 original_cpunum = cpunum;
	execute_interface_iterator iter(machine.root_device());
	for (device_execute_interface *exec = iter.first(); exec != nullptr; exec = iter.next())
	{
		if (cpunum-- == 0)
		{
			*result = &exec->device();
			return TRUE;
		}
	}

	/* if out of range, complain */
	debug_console_printf(machine, "Invalid CPU index %d\n", (UINT32)original_cpunum);
	return FALSE;
}


/*-------------------------------------------------
    debug_command_parameter_cpu_space - validates
    a parameter as a cpu and retrieves the given
    address space
-------------------------------------------------*/

int debug_command_parameter_cpu_space(running_machine &machine, const char *param, int spacenum, address_space *&result)
{
	device_t *cpu;

	/* first do the standard CPU thing */
	if (!debug_command_parameter_cpu(machine, param, &cpu))
		return FALSE;

	/* fetch the space pointer */
	if (!cpu->memory().has_space(spacenum))
	{
		debug_console_printf(machine, "No matching memory space found for CPU '%s'\n", cpu->tag());
		return FALSE;
	}
	result = &cpu->memory().space(spacenum);
	return TRUE;
}


/*-------------------------------------------------
    debug_command_parameter_expression - validates
    an expression parameter
-------------------------------------------------*/

static int debug_command_parameter_expression(running_machine &machine, const char *param, parsed_expression &result)
{
	/* NULL parameter does nothing and returns no error */
	if (param == nullptr)
		return TRUE;

	/* parse the expression; success if no error */
	try
	{
		result.parse(param);
		return TRUE;
	}
	catch (expression_error &err)
	{
		/* output an error */
		debug_console_printf(machine, "Error in expression: %s\n", param);
		debug_console_printf(machine, "                     %*s^", err.offset(), "");
		debug_console_printf(machine, "%s\n", err.code_string());
		return FALSE;
	}
}


/*-------------------------------------------------
    debug_command_parameter_command - validates a
    command parameter
-------------------------------------------------*/

static int debug_command_parameter_command(running_machine &machine, const char *param)
{
	CMDERR err;

	/* NULL parameter does nothing and returns no error */
	if (param == nullptr)
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

static void execute_help(running_machine &machine, int ref, int params, const char *param[])
{
	if (params == 0)
		debug_console_printf_wrap(machine, 80, "%s\n", debug_get_help(""));
	else
		debug_console_printf_wrap(machine, 80, "%s\n", debug_get_help(param[0]));
}


/*-------------------------------------------------
    execute_print - execute the print command
-------------------------------------------------*/

static void execute_print(running_machine &machine, int ref, int params, const char *param[])
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

static int mini_printf(running_machine &machine, char *buffer, const char *format, int params, UINT64 *param)
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
				case '\\':  *p++ = c;       break;
				case 'n':   *p++ = '\n';    break;
				default:                    break;
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

static void execute_printf(running_machine &machine, int ref, int params, const char *param[])
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

static void execute_logerror(running_machine &machine, int ref, int params, const char *param[])
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
		machine.logerror("%s", buffer);
}


/*-------------------------------------------------
    execute_tracelog - execute the tracelog command
-------------------------------------------------*/

static void execute_tracelog(running_machine &machine, int ref, int params, const char *param[])
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
		debug_cpu_get_visible_cpu(machine)->debug()->trace_printf("%s", buffer);
}


/*-------------------------------------------------
    execute_quit - execute the quit command
-------------------------------------------------*/

static void execute_quit(running_machine &machine, int ref, int params, const char *param[])
{
	osd_printf_error("Exited via the debugger\n");
	machine.schedule_exit();
}


/*-------------------------------------------------
    execute_do - execute the do command
-------------------------------------------------*/

static void execute_do(running_machine &machine, int ref, int params, const char *param[])
{
	UINT64 dummy;
	debug_command_parameter_number(machine, param[0], &dummy);
}


/*-------------------------------------------------
    execute_step - execute the step command
-------------------------------------------------*/

static void execute_step(running_machine &machine, int ref, int params, const char *param[])
{
	UINT64 steps = 1;

	/* if we have a parameter, use it instead */
	if (!debug_command_parameter_number(machine, param[0], &steps))
		return;

	debug_cpu_get_visible_cpu(machine)->debug()->single_step(steps);
}


/*-------------------------------------------------
    execute_over - execute the over command
-------------------------------------------------*/

static void execute_over(running_machine &machine, int ref, int params, const char *param[])
{
	UINT64 steps = 1;

	/* if we have a parameter, use it instead */
	if (!debug_command_parameter_number(machine, param[0], &steps))
		return;

	debug_cpu_get_visible_cpu(machine)->debug()->single_step_over(steps);
}


/*-------------------------------------------------
    execute_out - execute the out command
-------------------------------------------------*/

static void execute_out(running_machine &machine, int ref, int params, const char *param[])
{
	debug_cpu_get_visible_cpu(machine)->debug()->single_step_out();
}


/*-------------------------------------------------
    execute_go - execute the go command
-------------------------------------------------*/

static void execute_go(running_machine &machine, int ref, int params, const char *param[])
{
	UINT64 addr = ~0;

	/* if we have a parameter, use it instead */
	if (!debug_command_parameter_number(machine, param[0], &addr))
		return;

	debug_cpu_get_visible_cpu(machine)->debug()->go(addr);
}


/*-------------------------------------------------
    execute_go_vblank - execute the govblank
    command
-------------------------------------------------*/

static void execute_go_vblank(running_machine &machine, int ref, int params, const char *param[])
{
	debug_cpu_get_visible_cpu(machine)->debug()->go_vblank();
}


/*-------------------------------------------------
    execute_go_interrupt - execute the goint command
-------------------------------------------------*/

static void execute_go_interrupt(running_machine &machine, int ref, int params, const char *param[])
{
	UINT64 irqline = -1;

	/* if we have a parameter, use it instead */
	if (!debug_command_parameter_number(machine, param[0], &irqline))
		return;

	debug_cpu_get_visible_cpu(machine)->debug()->go_interrupt(irqline);
}


/*-------------------------------------------------
    execute_go_time - execute the gtime command
-------------------------------------------------*/

static void execute_go_time(running_machine &machine, int ref, int params, const char *param[])
{
	UINT64 milliseconds = -1;

	/* if we have a parameter, use it instead */
	if (!debug_command_parameter_number(machine, param[0], &milliseconds))
		return;

	debug_cpu_get_visible_cpu(machine)->debug()->go_milliseconds(milliseconds);
}


/*-------------------------------------------------
    execute_next - execute the next command
-------------------------------------------------*/

static void execute_next(running_machine &machine, int ref, int params, const char *param[])
{
	debug_cpu_get_visible_cpu(machine)->debug()->go_next_device();
}


/*-------------------------------------------------
    execute_focus - execute the focus command
-------------------------------------------------*/

static void execute_focus(running_machine &machine, int ref, int params, const char *param[])
{
	/* validate params */
	device_t *cpu;
	if (!debug_command_parameter_cpu(machine, param[0], &cpu))
		return;

	/* first clear the ignore flag on the focused CPU */
	cpu->debug()->ignore(false);

	/* then loop over CPUs and set the ignore flags on all other CPUs */
	execute_interface_iterator iter(machine.root_device());
	for (device_execute_interface *exec = iter.first(); exec != nullptr; exec = iter.next())
		if (&exec->device() != cpu)
			exec->device().debug()->ignore(true);
	debug_console_printf(machine, "Now focused on CPU '%s'\n", cpu->tag());
}


/*-------------------------------------------------
    execute_ignore - execute the ignore command
-------------------------------------------------*/

static void execute_ignore(running_machine &machine, int ref, int params, const char *param[])
{
	/* if there are no parameters, dump the ignore list */
	if (params == 0)
	{
		std::string buffer;

		/* loop over all executable devices */
		execute_interface_iterator iter(machine.root_device());
		for (device_execute_interface *exec = iter.first(); exec != nullptr; exec = iter.next())

			/* build up a comma-separated list */
			if (!exec->device().debug()->observing())
			{
				if (buffer.empty())
					strprintf(buffer, "Currently ignoring device '%s'", exec->device().tag());
				else
					strcatprintf(buffer, ", '%s'", exec->device().tag());
			}

		/* special message for none */
		if (buffer.empty())
			strprintf(buffer, "Not currently ignoring any devices");
		debug_console_printf(machine, "%s\n", buffer.c_str());
	}

	/* otherwise clear the ignore flag on all requested CPUs */
	else
	{
		device_t *devicelist[MAX_COMMAND_PARAMS];

		/* validate parameters */
		for (int paramnum = 0; paramnum < params; paramnum++)
			if (!debug_command_parameter_cpu(machine, param[paramnum], &devicelist[paramnum]))
				return;

		/* set the ignore flags */
		for (int paramnum = 0; paramnum < params; paramnum++)
		{
			/* make sure this isn't the last live CPU */
			execute_interface_iterator iter(machine.root_device());
			bool gotone = false;
			for (device_execute_interface *exec = iter.first(); exec != nullptr; exec = iter.next())
				if (&exec->device() != devicelist[paramnum] && exec->device().debug()->observing())
				{
					gotone = true;
					break;
				}
			if (!gotone)
			{
				debug_console_printf(machine, "Can't ignore all devices!\n");
				return;
			}

			devicelist[paramnum]->debug()->ignore(true);
			debug_console_printf(machine, "Now ignoring device '%s'\n", devicelist[paramnum]->tag());
		}
	}
}


/*-------------------------------------------------
    execute_observe - execute the observe command
-------------------------------------------------*/

static void execute_observe(running_machine &machine, int ref, int params, const char *param[])
{
	/* if there are no parameters, dump the ignore list */
	if (params == 0)
	{
		std::string buffer;

		/* loop over all executable devices */
		execute_interface_iterator iter(machine.root_device());
		for (device_execute_interface *exec = iter.first(); exec != nullptr; exec = iter.next())

			/* build up a comma-separated list */
			if (exec->device().debug()->observing())
			{
				if (buffer.empty())
					strprintf(buffer, "Currently observing CPU '%s'", exec->device().tag());
				else
					strcatprintf(buffer, ", '%s'", exec->device().tag());
			}

		/* special message for none */
		if (buffer.empty())
			strprintf(buffer, "Not currently observing any devices");
		debug_console_printf(machine, "%s\n", buffer.c_str());
	}

	/* otherwise set the ignore flag on all requested CPUs */
	else
	{
		device_t *devicelist[MAX_COMMAND_PARAMS];

		/* validate parameters */
		for (int paramnum = 0; paramnum < params; paramnum++)
			if (!debug_command_parameter_cpu(machine, param[paramnum], &devicelist[paramnum]))
				return;

		/* clear the ignore flags */
		for (int paramnum = 0; paramnum < params; paramnum++)
		{
			devicelist[paramnum]->debug()->ignore(false);
			debug_console_printf(machine, "Now observing device '%s'\n", devicelist[paramnum]->tag());
		}
	}
}


/*-------------------------------------------------
    execute_comment - add a comment to a line
-------------------------------------------------*/

static void execute_comment(running_machine &machine, int ref, int params, const char *param[])
{
	device_t *cpu;
	UINT64 address;

	/* param 1 is the address for the comment */
	if (!debug_command_parameter_number(machine, param[0], &address))
		return;

	/* CPU parameter is implicit */
	if (!debug_command_parameter_cpu(machine, nullptr, &cpu))
		return;

	/* make sure param 2 exists */
	if (strlen(param[1]) == 0)
	{
		debug_console_printf(machine, "Error : comment text empty\n");
		return;
	}

	/* Now try adding the comment */
	cpu->debug()->comment_add(address, param[1], 0x00ff0000);
	cpu->machine().debug_view().update_all(DVT_DISASSEMBLY);
}


/*------------------------------------------------------
    execute_comment_del - remove a comment from an addr
--------------------------------------------------------*/

static void execute_comment_del(running_machine &machine, int ref, int params, const char *param[])
{
	device_t *cpu;
	UINT64 address;

	/* param 1 can either be a command or the address for the comment */
	if (!debug_command_parameter_number(machine, param[0], &address))
		return;

	/* CPU parameter is implicit */
	if (!debug_command_parameter_cpu(machine, nullptr, &cpu))
		return;

	/* If it's a number, it must be an address */
	/* The bankoff and cbn will be pulled from what's currently active */
	cpu->debug()->comment_remove(address);
	cpu->machine().debug_view().update_all(DVT_DISASSEMBLY);
}


/*-------------------------------------------------
    execute_comment - add a comment to a line
-------------------------------------------------*/

static void execute_comment_save(running_machine &machine, int ref, int params, const char *param[])
{
	if (debug_comment_save(machine))
		debug_console_printf(machine, "Comments successfully saved\n");
	else
		debug_console_printf(machine, "Comments not saved\n");
}


/*-------------------------------------------------
    execute_bpset - execute the breakpoint set
    command
-------------------------------------------------*/

static void execute_bpset(running_machine &machine, int ref, int params, const char *param[])
{
	device_t *cpu;
	const char *action = nullptr;
	UINT64 address;
	int bpnum;

	/* CPU is implicit */
	if (!debug_command_parameter_cpu(machine, nullptr, &cpu))
		return;

	/* param 1 is the address */
	if (!debug_command_parameter_number(machine, param[0], &address))
		return;

	/* param 2 is the condition */
	parsed_expression condition(&cpu->debug()->symtable());
	if (!debug_command_parameter_expression(machine, param[1], condition))
		return;

	/* param 3 is the action */
	if (!debug_command_parameter_command(machine, action = param[2]))
		return;

	/* set the breakpoint */
	bpnum = cpu->debug()->breakpoint_set(address, (condition.is_empty()) ? nullptr : condition.original_string(), action);
	debug_console_printf(machine, "Breakpoint %X set\n", bpnum);
}


/*-------------------------------------------------
    execute_bpclear - execute the breakpoint
    clear command
-------------------------------------------------*/

static void execute_bpclear(running_machine &machine, int ref, int params, const char *param[])
{
	UINT64 bpindex;

	/* if 0 parameters, clear all */
	if (params == 0)
	{
		device_iterator iter(machine.root_device());
		for (device_t *device = iter.first(); device != nullptr; device = iter.next())
			device->debug()->breakpoint_clear_all();
		debug_console_printf(machine, "Cleared all breakpoints\n");
	}

	/* otherwise, clear the specific one */
	else if (!debug_command_parameter_number(machine, param[0], &bpindex))
		return;
	else
	{
		device_iterator iter(machine.root_device());
		bool found = false;
		for (device_t *device = iter.first(); device != nullptr; device = iter.next())
			if (device->debug()->breakpoint_clear(bpindex))
				found = true;
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

static void execute_bpdisenable(running_machine &machine, int ref, int params, const char *param[])
{
	UINT64 bpindex;

	/* if 0 parameters, clear all */
	if (params == 0)
	{
		device_iterator iter(machine.root_device());
		for (device_t *device = iter.first(); device != nullptr; device = iter.next())
			device->debug()->breakpoint_enable_all(ref);
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
		device_iterator iter(machine.root_device());
		bool found = false;
		for (device_t *device = iter.first(); device != nullptr; device = iter.next())
			if (device->debug()->breakpoint_enable(bpindex, ref))
				found = true;
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

static void execute_bplist(running_machine &machine, int ref, int params, const char *param[])
{
	int printed = 0;
	std::string buffer;

	/* loop over all CPUs */
	device_iterator iter(machine.root_device());
	for (device_t *device = iter.first(); device != nullptr; device = iter.next())
		if (device->debug()->breakpoint_first() != nullptr)
		{
			debug_console_printf(machine, "Device '%s' breakpoints:\n", device->tag());

			/* loop over the breakpoints */
			for (device_debug::breakpoint *bp = device->debug()->breakpoint_first(); bp != nullptr; bp = bp->next())
			{
				strprintf(buffer, "%c%4X @ %s", bp->enabled() ? ' ' : 'D', bp->index(), core_i64_hex_format(bp->address(), device->debug()->logaddrchars()));
				if (std::string(bp->condition()).compare("1") != 0)
					strcatprintf(buffer, " if %s", bp->condition());
				if (std::string(bp->action()).compare("") != 0)
					strcatprintf(buffer, " do %s", bp->action());
				debug_console_printf(machine, "%s\n", buffer.c_str());
				printed++;
			}
		}

	if (printed == 0)
		debug_console_printf(machine, "No breakpoints currently installed\n");
}


/*-------------------------------------------------
    execute_wpset - execute the watchpoint set
    command
-------------------------------------------------*/

static void execute_wpset(running_machine &machine, int ref, int params, const char *param[])
{
	address_space *space;
	const char *action = nullptr;
	UINT64 address, length;
	int type;
	int wpnum;

	/* CPU is implicit */
	if (!debug_command_parameter_cpu_space(machine, nullptr, ref, space))
		return;

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
	parsed_expression condition(&space->device().debug()->symtable());
	if (!debug_command_parameter_expression(machine, param[3], condition))
		return;

	/* param 5 is the action */
	if (!debug_command_parameter_command(machine, action = param[4]))
		return;

	/* set the watchpoint */
	wpnum = space->device().debug()->watchpoint_set(*space, type, address, length, (condition.is_empty()) ? nullptr : condition.original_string(), action);
	debug_console_printf(machine, "Watchpoint %X set\n", wpnum);
}


/*-------------------------------------------------
    execute_wpclear - execute the watchpoint
    clear command
-------------------------------------------------*/

static void execute_wpclear(running_machine &machine, int ref, int params, const char *param[])
{
	UINT64 wpindex;

	/* if 0 parameters, clear all */
	if (params == 0)
	{
		device_iterator iter(machine.root_device());
		for (device_t *device = iter.first(); device != nullptr; device = iter.next())
			device->debug()->watchpoint_clear_all();
		debug_console_printf(machine, "Cleared all watchpoints\n");
	}

	/* otherwise, clear the specific one */
	else if (!debug_command_parameter_number(machine, param[0], &wpindex))
		return;
	else
	{
		device_iterator iter(machine.root_device());
		bool found = false;
		for (device_t *device = iter.first(); device != nullptr; device = iter.next())
			if (device->debug()->watchpoint_clear(wpindex))
				found = true;
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

static void execute_wpdisenable(running_machine &machine, int ref, int params, const char *param[])
{
	UINT64 wpindex;

	/* if 0 parameters, clear all */
	if (params == 0)
	{
		device_iterator iter(machine.root_device());
		for (device_t *device = iter.first(); device != nullptr; device = iter.next())
			device->debug()->watchpoint_enable_all(ref);
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
		device_iterator iter(machine.root_device());
		bool found = false;
		for (device_t *device = iter.first(); device != nullptr; device = iter.next())
			if (device->debug()->watchpoint_enable(wpindex, ref))
				found = true;
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

static void execute_wplist(running_machine &machine, int ref, int params, const char *param[])
{
	int printed = 0;
	std::string buffer;

	/* loop over all CPUs */
	device_iterator iter(machine.root_device());
	for (device_t *device = iter.first(); device != nullptr; device = iter.next())
		for (address_spacenum spacenum = AS_0; spacenum < ADDRESS_SPACES; ++spacenum)
			if (device->debug()->watchpoint_first(spacenum) != nullptr)
			{
				static const char *const types[] = { "unkn ", "read ", "write", "r/w  " };

				debug_console_printf(machine, "Device '%s' %s space watchpoints:\n", device->tag(),
																						device->debug()->watchpoint_first(spacenum)->space().name());

				/* loop over the watchpoints */
				for (device_debug::watchpoint *wp = device->debug()->watchpoint_first(spacenum); wp != nullptr; wp = wp->next())
				{
					strprintf(buffer, "%c%4X @ %s-%s %s", wp->enabled() ? ' ' : 'D', wp->index(),
							core_i64_hex_format(wp->space().byte_to_address(wp->address()), wp->space().addrchars()),
							core_i64_hex_format(wp->space().byte_to_address_end(wp->address() + wp->length()) - 1, wp->space().addrchars()),
							types[wp->type() & 3]);
					if (std::string(wp->condition()).compare("1") != 0)
						strcatprintf(buffer, " if %s", wp->condition());
					if (std::string(wp->action()).compare("") != 0)
						strcatprintf(buffer, " do %s", wp->action());
					debug_console_printf(machine, "%s\n", buffer.c_str());
					printed++;
				}
			}

	if (printed == 0)
		debug_console_printf(machine, "No watchpoints currently installed\n");
}


/*-------------------------------------------------
    execute_rpset - execute the registerpoint set
    command
-------------------------------------------------*/

static void execute_rpset(running_machine &machine, int ref, int params, const char *param[])
{
	device_t *cpu;
	const char *action = nullptr;
	int bpnum;

	/* CPU is implicit */
	if (!debug_command_parameter_cpu(machine, nullptr, &cpu))
		return;

	/* param 1 is the condition */
	parsed_expression condition(&cpu->debug()->symtable());
	if (!debug_command_parameter_expression(machine, param[0], condition))
		return;

	/* param 2 is the action */
	if (!debug_command_parameter_command(machine, action = param[1]))
		return;

	/* set the breakpoint */
	bpnum = cpu->debug()->registerpoint_set(condition.original_string(), action);
	debug_console_printf(machine, "Registerpoint %X set\n", bpnum);
}


/*-------------------------------------------------
    execute_rpclear - execute the registerpoint
    clear command
-------------------------------------------------*/

static void execute_rpclear(running_machine &machine, int ref, int params, const char *param[])
{
	UINT64 rpindex;

	/* if 0 parameters, clear all */
	if (params == 0)
	{
		device_iterator iter(machine.root_device());
		for (device_t *device = iter.first(); device != nullptr; device = iter.next())
			device->debug()->registerpoint_clear_all();
		debug_console_printf(machine, "Cleared all registerpoints\n");
	}

	/* otherwise, clear the specific one */
	else if (!debug_command_parameter_number(machine, param[0], &rpindex))
		return;
	else
	{
		device_iterator iter(machine.root_device());
		bool found = false;
		for (device_t *device = iter.first(); device != nullptr; device = iter.next())
			if (device->debug()->registerpoint_clear(rpindex))
				found = true;
		if (found)
			debug_console_printf(machine, "Registerpoint %X cleared\n", (UINT32)rpindex);
		else
			debug_console_printf(machine, "Invalid registerpoint number %X\n", (UINT32)rpindex);
	}
}


/*-------------------------------------------------
    execute_rpdisenable - execute the registerpoint
    disable/enable commands
-------------------------------------------------*/

static void execute_rpdisenable(running_machine &machine, int ref, int params, const char *param[])
{
	UINT64 rpindex;

	/* if 0 parameters, clear all */
	if (params == 0)
	{
		device_iterator iter(machine.root_device());
		for (device_t *device = iter.first(); device != nullptr; device = iter.next())
			device->debug()->registerpoint_enable_all(ref);
		if (ref == 0)
			debug_console_printf(machine, "Disabled all registerpoints\n");
		else
			debug_console_printf(machine, "Enabled all registeroints\n");
	}

	/* otherwise, clear the specific one */
	else if (!debug_command_parameter_number(machine, param[0], &rpindex))
		return;
	else
	{
		device_iterator iter(machine.root_device());
		bool found = false;
		for (device_t *device = iter.first(); device != nullptr; device = iter.next())
			if (device->debug()->registerpoint_enable(rpindex, ref))
				found = true;
		if (found)
			debug_console_printf(machine, "Registerpoint %X %s\n", (UINT32)rpindex, ref ? "enabled" : "disabled");
		else
			debug_console_printf(machine, "Invalid registerpoint number %X\n", (UINT32)rpindex);
	}
}


/*-------------------------------------------------
    execute_rplist - execute the registerpoint list
    command
-------------------------------------------------*/

static void execute_rplist(running_machine &machine, int ref, int params, const char *param[])
{
	int printed = 0;
	std::string buffer;

	/* loop over all CPUs */
	device_iterator iter(machine.root_device());
	for (device_t *device = iter.first(); device != nullptr; device = iter.next())
		if (device->debug()->registerpoint_first() != nullptr)
		{
			debug_console_printf(machine, "Device '%s' registerpoints:\n", device->tag());

			/* loop over the breakpoints */
			for (device_debug::registerpoint *rp = device->debug()->registerpoint_first(); rp != nullptr; rp = rp->next())
			{
				strprintf(buffer, "%c%4X ", rp->enabled() ? ' ' : 'D', rp->index());
				strcatprintf(buffer, "if %s", rp->condition());
				if (rp->action() != nullptr)
					strcatprintf(buffer, " do %s", rp->action());
				debug_console_printf(machine, "%s\n", buffer.c_str());
				printed++;
			}
		}

	if (printed == 0)
		debug_console_printf(machine, "No registerpoints currently installed\n");
}


/*-------------------------------------------------
    execute_hotspot - execute the hotspot
    command
-------------------------------------------------*/

static void execute_hotspot(running_machine &machine, int ref, int params, const char *param[])
{
	/* if no params, and there are live hotspots, clear them */
	if (params == 0)
	{
		bool cleared = false;

		/* loop over CPUs and find live spots */
		device_iterator iter(machine.root_device());
		for (device_t *device = iter.first(); device != nullptr; device = iter.next())
			if (device->debug()->hotspot_tracking_enabled())
			{
				device->debug()->hotspot_track(0, 0);
				debug_console_printf(machine, "Cleared hotspot tracking on CPU '%s'\n", device->tag());
				cleared = true;
			}

		/* if we cleared, we're done */
		if (cleared)
			return;
	}

	/* extract parameters */
	device_t *device = nullptr;
	if (!debug_command_parameter_cpu(machine, (params > 0) ? param[0] : nullptr, &device))
		return;
	UINT64 count = 64;
	if (!debug_command_parameter_number(machine, param[1], &count))
		return;
	UINT64 threshhold = 250;
	if (!debug_command_parameter_number(machine, param[2], &threshhold))
		return;

	/* attempt to install */
	device->debug()->hotspot_track(count, threshhold);
	debug_console_printf(machine, "Now tracking hotspots on CPU '%s' using %d slots with a threshhold of %d\n", device->tag(), (int)count, (int)threshhold);
}


/*-------------------------------------------------
    execute_statesave - execute the statesave command
-------------------------------------------------*/

static void execute_statesave(running_machine &machine, int ref, int params, const char *param[])
{
	std::string filename(param[0]);
	machine.immediate_save(filename.c_str());
	debug_console_printf(machine, "State save attempted.  Please refer to window message popup for results.\n");
}


/*-------------------------------------------------
    execute_stateload - execute the stateload command
-------------------------------------------------*/

static void execute_stateload(running_machine &machine, int ref, int params, const char *param[])
{
	std::string filename(param[0]);
	machine.immediate_load(filename.c_str());

	// Clear all PC & memory tracks
	device_iterator iter(machine.root_device());
	for (device_t *device = iter.first(); device != nullptr; device = iter.next())
	{
		device->debug()->track_pc_data_clear();
		device->debug()->track_mem_data_clear();
	}
	debug_console_printf(machine, "State load attempted.  Please refer to window message popup for results.\n");
}


/*-------------------------------------------------
    execute_save - execute the save command
-------------------------------------------------*/

static void execute_save(running_machine &machine, int ref, int params, const char *param[])
{
	UINT64 offset, endoffset, length;
	address_space *space;
	FILE *f;
	UINT64 i;

	/* validate parameters */
	if (!debug_command_parameter_number(machine, param[1], &offset))
		return;
	if (!debug_command_parameter_number(machine, param[2], &length))
		return;
	if (!debug_command_parameter_cpu_space(machine, (params > 3) ? param[3] : nullptr, ref, space))
		return;

	/* determine the addresses to write */
	endoffset = space->address_to_byte(offset + length - 1) & space->bytemask();
	offset = space->address_to_byte(offset) & space->bytemask();

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
		UINT8 byte = debug_read_byte(*space, i, TRUE);
		fwrite(&byte, 1, 1, f);
	}

	/* close the file */
	fclose(f);
	debug_console_printf(machine, "Data saved successfully\n");
}


/*-------------------------------------------------
    execute_load - execute the load command
-------------------------------------------------*/

static void execute_load(running_machine &machine, int ref, int params, const char *param[])
{
	UINT64 offset, endoffset, length;
	address_space *space;
	FILE *f;
	UINT64 i;

	/* validate parameters */
	if (!debug_command_parameter_number(machine, param[1], &offset))
		return;
	if (!debug_command_parameter_number(machine, param[2], &length))
		return;
	if (!debug_command_parameter_cpu_space(machine, (params > 3) ? param[3] : nullptr, ref, space))
		return;

	/* determine the addresses to read */
	endoffset = space->address_to_byte(offset + length - 1) & space->bytemask();
	offset = space->address_to_byte(offset) & space->bytemask();

	/* open the file */
	f = fopen(param[0], "rb");
	if (!f)
	{
		debug_console_printf(machine, "Error opening file '%s'\n", param[0]);
		return;
	}

	/* now read the data in, ignore endoffset and load entire file if length has been set to zero (offset-1) */
	UINT8 byte;
	for (i = offset; i <= endoffset || endoffset == offset - 1 ; i++)
	{
		fread(&byte, 1, 1, f);
		/* check if end of file has been reached and stop loading if it has */
		if (feof(f))
			break;
		debug_write_byte(*space, i, byte, TRUE);
	}
	/* close the file */
	fclose(f);
	if ( i == offset)
		debug_console_printf(machine, "Length specified too large, load failed\n");
	else
		debug_console_printf(machine, "Data loaded successfully to memory : 0x%s to 0x%s\n", core_i64_hex_format(offset,0), core_i64_hex_format(i-1,0));
}


/*-------------------------------------------------
    execute_dump - execute the dump command
-------------------------------------------------*/

static void execute_dump(running_machine &machine, int ref, int params, const char *param[])
{
	UINT64 offset, endoffset, length, width = 0, ascii = 1;
	address_space *space;
	FILE *f;
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
	if (!debug_command_parameter_cpu_space(machine, (params > 5) ? param[5] : nullptr, ref, space))
		return;

	/* further validation */
	if (width == 0)
		width = space->data_width() / 8;
	if (width < space->address_to_byte(1))
		width = space->address_to_byte(1);
	if (width != 1 && width != 2 && width != 4 && width != 8)
	{
		debug_console_printf(machine, "Invalid width! (must be 1,2,4 or 8)\n");
		return;
	}
	endoffset = space->address_to_byte(offset + length - 1) & space->bytemask();
	offset = space->address_to_byte(offset) & space->bytemask();

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
		outdex += sprintf(&output[outdex], "%s: ", core_i64_hex_format((UINT32)space->byte_to_address(i), space->logaddrchars()));

		/* print the bytes */
		for (j = 0; j < 16; j += width)
		{
			if (i + j <= endoffset)
			{
				offs_t curaddr = i + j;
				if (debug_cpu_translate(*space, TRANSLATE_READ_DEBUG, &curaddr))
				{
					UINT64 value = debug_read_memory(*space, i + j, width, TRUE);
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
				if (debug_cpu_translate(*space, TRANSLATE_READ_DEBUG, &curaddr))
				{
					UINT8 byte = debug_read_byte(*space, i + j, TRUE);
					outdex += sprintf(&output[outdex], "%c", (byte >= 32 && byte < 127) ? byte : '.');
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

static void execute_cheatinit(running_machine &machine, int ref, int params, const char *param[])
{
	UINT64 offset, length = 0, real_length = 0;
	address_space *space;
	UINT32 active_cheat = 0;
	UINT64 curaddr;
	UINT8 i, region_count = 0;

	address_map_entry *entry;
	cheat_region_map cheat_region[100];

	memset(cheat_region, 0, sizeof(cheat_region));

	/* validate parameters */
	if (!debug_command_parameter_cpu_space(machine, (params > 3) ? param[3] : nullptr, AS_PROGRAM, space))
		return;

	if (ref == 0)
	{
		cheat.width = 1;
		cheat.signed_cheat = FALSE;
		cheat.swapped_cheat = FALSE;
		if (params > 0)
		{
			char *srtpnt = (char*)param[0];

			if (*srtpnt == 's')
				cheat.signed_cheat = TRUE;
			else if (*srtpnt == 'u')
				cheat.signed_cheat = FALSE;
			else
			{
				debug_console_printf(machine, "Invalid sign: expected s or u\n");
				return;
			}

			if (*(++srtpnt) == 'b')
				cheat.width = 1;
			else if (*srtpnt == 'w')
				cheat.width = 2;
			else if (*srtpnt == 'd')
				cheat.width = 4;
			else if (*srtpnt == 'q')
				cheat.width = 8;
			else
			{
				debug_console_printf(machine, "Invalid width: expected b, w, d or q\n");
				return;
			}

			if (*(++srtpnt) == 's')
				cheat.swapped_cheat = TRUE;
			else
				cheat.swapped_cheat = FALSE;
		}
	}

	/* initialize entire memory by default */
	if (params <= 1)
	{
		for (entry = space->map()->m_entrylist.first(); entry != nullptr; entry = entry->next())
		{
			cheat_region[region_count].offset = space->address_to_byte(entry->m_addrstart) & space->bytemask();
			cheat_region[region_count].endoffset = space->address_to_byte(entry->m_addrend) & space->bytemask();
			cheat_region[region_count].share = entry->m_share;
			cheat_region[region_count].disabled = (entry->m_write.m_type == AMH_RAM) ? FALSE : TRUE;

			/* disable double share regions */
			if (entry->m_share != nullptr)
				for (i = 0; i < region_count; i++)
					if (cheat_region[i].share != nullptr)
						if (strcmp(cheat_region[i].share, entry->m_share) == 0)
							cheat_region[region_count].disabled = TRUE;

			region_count++;
		}
	}
	else
	{
		/* validate parameters */
		if (!debug_command_parameter_number(machine, param[(ref == 0) ? 1 : 0], &offset))
			return;
		if (!debug_command_parameter_number(machine, param[(ref == 0) ? 2 : 1], &length))
			return;

		/* force region to the specified range */
		cheat_region[region_count].offset = space->address_to_byte(offset) & space->bytemask();
		cheat_region[region_count].endoffset = space->address_to_byte(offset + length - 1) & space->bytemask();
		cheat_region[region_count].share = nullptr;
		cheat_region[region_count].disabled = FALSE;
		region_count++;
	}

	/* determine the writable extent of each region in total */
	for (i = 0; i < region_count; i++)
		if (!cheat_region[i].disabled)
			for (curaddr = cheat_region[i].offset; curaddr <= cheat_region[i].endoffset; curaddr += cheat.width)
				if (cheat_address_is_valid(*space, curaddr))
					real_length++;

	if (real_length == 0)
	{
		debug_console_printf(machine, "No writable bytes found in this area\n");
		return;
	}

	if (ref == 0)
	{
		/* initialize new cheat system */
		cheat.cheatmap.resize(real_length);
		cheat.undo = 0;
		cheat.cpu[0] = (params > 3) ? *param[3] : '0';
	}
	else
	{
		/* add range to cheat system */
		if (cheat.cpu[0] == 0)
		{
			debug_console_printf(machine, "Use cheatinit before cheatrange\n");
			return;
		}

		if (!debug_command_parameter_cpu_space(machine, cheat.cpu, AS_PROGRAM, space))
			return;

		active_cheat = cheat.cheatmap.size();
		cheat.cheatmap.resize(cheat.cheatmap.size() + real_length);
	}

	/* initialize cheatmap in the selected space */
	for (i = 0; i < region_count; i++)
		if (!cheat_region[i].disabled)
			for (curaddr = cheat_region[i].offset; curaddr <= cheat_region[i].endoffset; curaddr += cheat.width)
				if (cheat_address_is_valid(*space, curaddr))
				{
					cheat.cheatmap[active_cheat].previous_value = cheat_read_extended(&cheat, *space, curaddr);
					cheat.cheatmap[active_cheat].first_value = cheat.cheatmap[active_cheat].previous_value;
					cheat.cheatmap[active_cheat].offset = curaddr;
					cheat.cheatmap[active_cheat].state = 1;
					cheat.cheatmap[active_cheat].undo = 0;
					active_cheat++;
				}

	/* give a detailed init message to avoid searches being mistakingly carried out on the wrong CPU */
	device_t *cpu = nullptr;
	debug_command_parameter_cpu(machine, cheat.cpu, &cpu);
	debug_console_printf(machine, "%u cheat initialized for CPU index %s ( aka %s )\n", active_cheat, cheat.cpu, cpu->tag());
}


/*-------------------------------------------------
    execute_cheatnext - execute the search
-------------------------------------------------*/

static void execute_cheatnext(running_machine &machine, int ref, int params, const char *param[])
{
	address_space *space;
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
		CHEAT_GREATEROF,
		CHEAT_CHANGEDBY
	};

	if (cheat.cpu[0] == 0)
	{
		debug_console_printf(machine, "Use cheatinit before cheatnext\n");
		return;
	}

	if (!debug_command_parameter_cpu_space(machine, cheat.cpu, AS_PROGRAM, space))
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
	else if (!strcmp(param[0], "changedby") || !strcmp(param[0], "ch") || !strcmp(param[0], "~"))
		condition = CHEAT_CHANGEDBY;
	else
	{
		debug_console_printf(machine, "Invalid condition type\n");
		return;
	}

	cheat.undo++;

	/* execute the search */
	for (cheatindex = 0; cheatindex < cheat.cheatmap.size(); cheatindex += 1)
		if (cheat.cheatmap[cheatindex].state == 1)
		{
			UINT64 cheat_value = cheat_read_extended(&cheat, *space, cheat.cheatmap[cheatindex].offset);
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
				case CHEAT_CHANGEDBY:
					if (cheat_value > comp_byte)
						disable_byte = (cheat_value != comp_byte + comp_value);
					else
						disable_byte = (cheat_value != comp_byte - comp_value);
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
		execute_cheatlist(machine, 0, 0, nullptr);

	debug_console_printf(machine, "%u cheats found\n", active_cheat);
}


/*-------------------------------------------------
    execute_cheatlist - show a list of active cheat
-------------------------------------------------*/

static void execute_cheatlist(running_machine &machine, int ref, int params, const char *param[])
{
	char spaceletter, sizeletter;
	address_space *space;
	device_t *cpu;
	UINT32 active_cheat = 0;
	UINT64 cheatindex;
	UINT64 sizemask;
	FILE *f = nullptr;

	if (!debug_command_parameter_cpu_space(machine, cheat.cpu, AS_PROGRAM, space))
		return;

	if (!debug_command_parameter_cpu(machine, cheat.cpu, &cpu))
		return;

	if (params > 0)
		f = fopen(param[0], "w");

	switch (space->spacenum())
	{
		default:
		case AS_PROGRAM:    spaceletter = 'p';  break;
		case AS_DATA:   spaceletter = 'd';  break;
		case AS_IO:     spaceletter = 'i';  break;
	}

	switch (cheat.width)
	{
		default:
		case 1:                     sizeletter = 'b';   sizemask = 0xff;                    break;
		case 2:                     sizeletter = 'w';   sizemask = 0xffff;                  break;
		case 4:                     sizeletter = 'd';   sizemask = 0xffffffff;              break;
		case 8:                     sizeletter = 'q';   sizemask = U64(0xffffffffffffffff); break;
	}

	/* write the cheat list */
	for (cheatindex = 0; cheatindex < cheat.cheatmap.size(); cheatindex += 1)
	{
		if (cheat.cheatmap[cheatindex].state == 1)
		{
			UINT64 value = cheat_byte_swap(&cheat, cheat_read_extended(&cheat, *space, cheat.cheatmap[cheatindex].offset)) & sizemask;
			offs_t address = space->byte_to_address(cheat.cheatmap[cheatindex].offset);

			if (params > 0)
			{
				active_cheat++;
				fprintf(f, "  <cheat desc=\"Possibility %d : %s (%s)\">\n", active_cheat, core_i64_hex_format(address, space->logaddrchars()), core_i64_hex_format(value, cheat.width * 2));
				fprintf(f, "    <script state=\"run\">\n");
				fprintf(f, "      <action>%s.p%c%c@%s=%s</action>\n", cpu->tag(), spaceletter, sizeletter, core_i64_hex_format(address, space->logaddrchars()), core_i64_hex_format(cheat_byte_swap(&cheat, cheat.cheatmap[cheatindex].first_value) & sizemask, cheat.width * 2));
				fprintf(f, "    </script>\n");
				fprintf(f, "  </cheat>\n\n");
			}
			else
				debug_console_printf(machine, "Address=%s Start=%s Current=%s\n", core_i64_hex_format(address, space->logaddrchars()), core_i64_hex_format(cheat_byte_swap(&cheat, cheat.cheatmap[cheatindex].first_value) & sizemask, cheat.width * 2), core_i64_hex_format(value, cheat.width * 2));
		}
	}
	if (params > 0)
		fclose(f);
}


/*-------------------------------------------------
    execute_cheatundo - undo the last search
-------------------------------------------------*/

static void execute_cheatundo(running_machine &machine, int ref, int params, const char *param[])
{
	UINT64 cheatindex;
	UINT32 undo_count = 0;

	if (cheat.undo > 0)
	{
		for (cheatindex = 0; cheatindex < cheat.cheatmap.size(); cheatindex += 1)
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

static void execute_find(running_machine &machine, int ref, int params, const char *param[])
{
	UINT64 offset, endoffset, length;
	address_space *space;
	UINT64 data_to_find[256];
	UINT8 data_size[256];
	int cur_data_size;
	int data_count = 0;
	int found = 0;
	int j;

	/* validate parameters */
	if (!debug_command_parameter_number(machine, param[0], &offset))
		return;
	if (!debug_command_parameter_number(machine, param[1], &length))
		return;
	if (!debug_command_parameter_cpu_space(machine, nullptr, ref, space))
		return;

	/* further validation */
	endoffset = space->address_to_byte(offset + length - 1) & space->bytemask();
	offset = space->address_to_byte(offset) & space->bytemask();
	cur_data_size = space->address_to_byte(1);
	if (cur_data_size == 0)
		cur_data_size = 1;

	/* parse the data parameters */
	for (int i = 2; i < params; i++)
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
	for (UINT64 i = offset; i <= endoffset; i += data_size[0])
	{
		int suboffset = 0;
		int match = 1;

		/* find the entire string */
		for (j = 0; j < data_count && match; j++)
		{
			switch (data_size[j])
			{
				case 1: match = ((UINT8)debug_read_byte(*space, i + suboffset, TRUE) == (UINT8)data_to_find[j]);    break;
				case 2: match = ((UINT16)debug_read_word(*space, i + suboffset, TRUE) == (UINT16)data_to_find[j]);  break;
				case 4: match = ((UINT32)debug_read_dword(*space, i + suboffset, TRUE) == (UINT32)data_to_find[j]); break;
				case 8: match = ((UINT64)debug_read_qword(*space, i + suboffset, TRUE) == (UINT64)data_to_find[j]); break;
				default:    /* all other cases are wildcards */     break;
			}
			suboffset += data_size[j] & 0x0f;
		}

		/* did we find it? */
		if (match)
		{
			found++;
			debug_console_printf(machine, "Found at %s\n", core_i64_hex_format((UINT32)space->byte_to_address(i), space->addrchars()));
		}
	}

	/* print something if not found */
	if (found == 0)
		debug_console_printf(machine, "Not found\n");
}


/*-------------------------------------------------
    execute_dasm - execute the dasm command
-------------------------------------------------*/

static void execute_dasm(running_machine &machine, int ref, int params, const char *param[])
{
	UINT64 offset, length, bytes = 1;
	int minbytes, maxbytes, byteswidth;
	address_space *space, *decrypted_space;
	FILE *f;
	int j;

	/* validate parameters */
	if (!debug_command_parameter_number(machine, param[1], &offset))
		return;
	if (!debug_command_parameter_number(machine, param[2], &length))
		return;
	if (!debug_command_parameter_number(machine, param[3], &bytes))
		return;
	if (!debug_command_parameter_cpu_space(machine, (params > 4) ? param[4] : nullptr, AS_PROGRAM, space))
		return;
	if (!debug_command_parameter_cpu_space(machine, (params > 4) ? param[4] : nullptr, AS_DECRYPTED_OPCODES, decrypted_space))
		decrypted_space = space;

	/* determine the width of the bytes */
	cpu_device *cpudevice = downcast<cpu_device *>(&space->device());
	minbytes = cpudevice->min_opcode_bytes();
	maxbytes = cpudevice->max_opcode_bytes();
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
	for (UINT64 i = 0; i < length; )
	{
		int pcbyte = space->address_to_byte(offset + i) & space->bytemask();
		char output[512], disasm[200];
		const char *comment;
		offs_t tempaddr;
		int outdex = 0;
		int numbytes = 0;

		/* print the address */
		outdex += sprintf(&output[outdex], "%s: ", core_i64_hex_format((UINT32)space->byte_to_address(pcbyte), space->logaddrchars()));

		/* make sure we can translate the address */
		tempaddr = pcbyte;
		if (debug_cpu_translate(*space, TRANSLATE_FETCH_DEBUG, &tempaddr))
		{
			UINT8 opbuf[64], argbuf[64];

			/* fetch the bytes up to the maximum */
			for (numbytes = 0; numbytes < maxbytes; numbytes++)
			{
				opbuf[numbytes] = debug_read_opcode(*decrypted_space, pcbyte + numbytes, 1);
				argbuf[numbytes] = debug_read_opcode(*space, pcbyte + numbytes, 1);
			}

			/* disassemble the result */
			i += numbytes = space->device().debug()->disassemble(disasm, offset + i, opbuf, argbuf) & DASMFLAG_LENGTHMASK;
		}

		/* print the bytes */
		if (bytes)
		{
			int startdex = outdex;
			numbytes = space->address_to_byte(numbytes);
			for (j = 0; j < numbytes; j += minbytes)
				outdex += sprintf(&output[outdex], "%s ", core_i64_hex_format(debug_read_opcode(*decrypted_space, pcbyte + j, minbytes), minbytes * 2));
			if (outdex - startdex < byteswidth)
				outdex += sprintf(&output[outdex], "%*s", byteswidth - (outdex - startdex), "");
			outdex += sprintf(&output[outdex], "  ");
		}

		/* add the disassembly */
		sprintf(&output[outdex], "%s", disasm);

		/* attempt to add the comment */
		comment = space->device().debug()->comment_text(tempaddr);
		if (comment != nullptr)
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

static void execute_trace_internal(running_machine &machine, int ref, int params, const char *param[], int trace_over)
{
	const char *action = nullptr;
	device_t *cpu;
	FILE *f = nullptr;
	const char *mode;
	std::string filename = param[0];

	/* replace macros */
	strreplace(filename, "{game}", machine.basename());

	/* validate parameters */
	if (!debug_command_parameter_cpu(machine, (params > 1) ? param[1] : nullptr, &cpu))
		return;
	if (!debug_command_parameter_command(machine, action = param[2]))
		return;

	/* open the file */
	if (core_stricmp(filename.c_str(), "off") != 0)
	{
		mode = "w";

		/* opening for append? */
		if ((filename[0] == '>') && (filename[1] == '>'))
		{
			mode = "a";
			filename = filename.substr(2);
		}

		f = fopen(filename.c_str(), mode);
		if (!f)
		{
			debug_console_printf(machine, "Error opening file '%s'\n", param[0]);
			return;
		}
	}

	/* do it */
	cpu->debug()->trace(f, trace_over, action);
	if (f)
		debug_console_printf(machine, "Tracing CPU '%s' to file %s\n", cpu->tag(), filename.c_str());
	else
		debug_console_printf(machine, "Stopped tracing on CPU '%s'\n", cpu->tag());
}


/*-------------------------------------------------
    execute_trace - execute the trace command
-------------------------------------------------*/

static void execute_trace(running_machine &machine, int ref, int params, const char *param[])
{
	execute_trace_internal(machine, ref, params, param, 0);
}


/*-------------------------------------------------
    execute_traceover - execute the trace over command
-------------------------------------------------*/

static void execute_traceover(running_machine &machine, int ref, int params, const char *param[])
{
	execute_trace_internal(machine, ref, params, param, 1);
}


/*-------------------------------------------------
    execute_traceflush - execute the trace flush command
-------------------------------------------------*/

static void execute_traceflush(running_machine &machine, int ref, int params, const char *param[])
{
	debug_cpu_flush_traces(machine);
}


/*-------------------------------------------------
    execute_history - execute the history command
-------------------------------------------------*/

static void execute_history(running_machine &machine, int ref, int params, const char *param[])
{
	/* validate parameters */
	address_space *space, *decrypted_space;
	if (!debug_command_parameter_cpu_space(machine, (params > 0) ? param[0] : nullptr, AS_PROGRAM, space))
		return;
	if (!debug_command_parameter_cpu_space(machine, (params > 0) ? param[0] : nullptr, AS_DECRYPTED_OPCODES, decrypted_space))
		decrypted_space = space;

	UINT64 count = device_debug::HISTORY_SIZE;
	if (!debug_command_parameter_number(machine, param[1], &count))
		return;

	/* further validation */
	if (count > device_debug::HISTORY_SIZE)
		count = device_debug::HISTORY_SIZE;

	device_debug *debug = space->device().debug();

	/* loop over lines */
	int maxbytes = debug->max_opcode_bytes();
	for (int index = 0; index < (int) count; index++)
	{
		offs_t pc = debug->history_pc(-index);

		/* fetch the bytes up to the maximum */
		offs_t pcbyte = space->address_to_byte(pc) & space->bytemask();
		UINT8 opbuf[64], argbuf[64];
		for (int numbytes = 0; numbytes < maxbytes; numbytes++)
		{
			opbuf[numbytes] = debug_read_opcode(*decrypted_space, pcbyte + numbytes, 1);
			argbuf[numbytes] = debug_read_opcode(*space, pcbyte + numbytes, 1);
		}

		char buffer[200];
		debug->disassemble(buffer, pc, opbuf, argbuf);

		debug_console_printf(machine, "%s: %s\n", core_i64_hex_format(pc, space->logaddrchars()), buffer);
	}
}


/*-------------------------------------------------
    execute_trackpc - execute the trackpc command
-------------------------------------------------*/

static void execute_trackpc(running_machine &machine, int ref, int params, const char *param[])
{
	// Gather the on/off switch (if present)
	UINT64 turnOn = true;
	if (!debug_command_parameter_number(machine, param[0], &turnOn))
		return;

	// Gather the cpu id (if present)
	device_t *cpu = nullptr;
	if (!debug_command_parameter_cpu(machine, (params > 1) ? param[1] : nullptr, &cpu))
		return;

	// Should we clear the existing data?
	UINT64 clear = false;
	if (!debug_command_parameter_number(machine, param[2], &clear))
		return;

	cpu->debug()->set_track_pc((bool)turnOn);
	if (turnOn)
	{
		// Insert current pc
		if (debug_cpu_get_visible_cpu(machine) == cpu)
		{
			const offs_t pc = cpu->debug()->pc();
			cpu->debug()->set_track_pc_visited(pc);
		}
		debug_console_printf(machine, "PC tracking enabled\n");
	}
	else
	{
		debug_console_printf(machine, "PC tracking disabled\n");
	}

	if (clear)
		cpu->debug()->track_pc_data_clear();
}


/*-------------------------------------------------
    execute_trackmem - execute the trackmem command
-------------------------------------------------*/

static void execute_trackmem(running_machine &machine, int ref, int params, const char *param[])
{
	// Gather the on/off switch (if present)
	UINT64 turnOn = true;
	if (!debug_command_parameter_number(machine, param[0], &turnOn))
		return;

	// Gather the cpu id (if present)
	device_t *cpu = nullptr;
	if (!debug_command_parameter_cpu(machine, (params > 1) ? param[1] : nullptr, &cpu))
		return;

	// Should we clear the existing data?
	UINT64 clear = false;
	if (!debug_command_parameter_number(machine, param[2], &clear))
		return;

	// Get the address space for the given cpu
	address_space *space;
	if (!debug_command_parameter_cpu_space(machine, (params > 1) ? param[1] : nullptr, AS_PROGRAM, space))
		return;

	// Inform the CPU it's time to start tracking memory writes
	cpu->debug()->set_track_mem(turnOn);

	// Use the watchpoint system to catch memory writes
	space->enable_write_watchpoints(true);

	// Clear out the existing data if requested
	if (clear)
		space->device().debug()->track_mem_data_clear();
}


/*-------------------------------------------------
    execute_pcatmem - execute the pcatmem command
-------------------------------------------------*/

static void execute_pcatmem(running_machine &machine, int ref, int params, const char *param[])
{
	// Gather the required address parameter
	UINT64 address;
	if (!debug_command_parameter_number(machine, param[0], &address))
		return;

	// Gather the cpu id (if present)
	device_t *cpu = nullptr;
	if (!debug_command_parameter_cpu(machine, (params > 1) ? param[1] : nullptr, &cpu))
		return;

	// Get the address space for the given cpu
	address_space *space;
	if (!debug_command_parameter_cpu_space(machine, (params > 1) ? param[1] : nullptr, ref, space))
		return;

	// Get the value of memory at the address
	const int nativeDataWidth = space->data_width() / 8;
	const UINT64 data = debug_read_memory(*space,
											space->address_to_byte(address),
											nativeDataWidth,
											true);

	// Recover the pc & print
	const address_spacenum spaceNum = (address_spacenum)ref;
	const offs_t result = space->device().debug()->track_mem_pc_from_space_address_data(spaceNum, address, data);
	if (result != (offs_t)(-1))
		debug_console_printf(machine, "%02x\n", result);
	else
		debug_console_printf(machine, "UNKNOWN PC\n");
}


/*-------------------------------------------------
    execute_snap - execute the snapshot command
-------------------------------------------------*/

static void execute_snap(running_machine &machine, int ref, int params, const char *param[])
{
	/* if no params, use the default behavior */
	if (params == 0)
	{
		machine.video().save_active_screen_snapshots();
		debug_console_printf(machine, "Saved snapshot\n");
	}

	/* otherwise, we have to open the file ourselves */
	else
	{
		const char *filename = param[0];
		int scrnum = (params > 1) ? atoi(param[1]) : 0;

		screen_device_iterator iter(machine.root_device());
		screen_device *screen = iter.byindex(scrnum);

		if ((screen == nullptr) || !machine.render().is_live(*screen))
		{
			debug_console_printf(machine, "Invalid screen number '%d'\n", scrnum);
			return;
		}

		std::string fname(filename);
		if (fname.find(".png") == -1)
			fname.append(".png");
		emu_file file(machine.options().snapshot_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
		file_error filerr = file.open(fname.c_str());

		if (filerr != FILERR_NONE)
		{
			debug_console_printf(machine, "Error creating file '%s'\n", filename);
			return;
		}

		screen->machine().video().save_snapshot(screen, file);
		debug_console_printf(machine, "Saved screen #%d snapshot as '%s'\n", scrnum, filename);
	}
}


/*-------------------------------------------------
    execute_source - execute the source command
-------------------------------------------------*/

static void execute_source(running_machine &machine, int ref, int params, const char *param[])
{
	debug_cpu_source_script(machine, param[0]);
}


/*-------------------------------------------------
    execute_map - execute the map command
-------------------------------------------------*/

static void execute_map(running_machine &machine, int ref, int params, const char *param[])
{
	address_space *space;
	offs_t taddress;
	UINT64 address;
	int intention;

	/* validate parameters */
	if (!debug_command_parameter_number(machine, param[0], &address))
		return;

	/* CPU is implicit */
	if (!debug_command_parameter_cpu_space(machine, nullptr, ref, space))
		return;

	/* do the translation first */
	for (intention = TRANSLATE_READ_DEBUG; intention <= TRANSLATE_FETCH_DEBUG; intention++)
	{
		static const char *const intnames[] = { "Read", "Write", "Fetch" };
		taddress = space->address_to_byte(address) & space->bytemask();
		if (debug_cpu_translate(*space, intention, &taddress))
		{
			const char *mapname = space->get_handler_string((intention == TRANSLATE_WRITE_DEBUG) ? ROW_WRITE : ROW_READ, taddress);
			debug_console_printf(machine, "%7s: %s logical == %s physical -> %s\n", intnames[intention & 3], core_i64_hex_format(address, space->logaddrchars()), core_i64_hex_format(space->byte_to_address(taddress), space->addrchars()), mapname);
		}
		else
			debug_console_printf(machine, "%7s: %s logical is unmapped\n", intnames[intention & 3], core_i64_hex_format(address, space->logaddrchars()));
	}
}


/*-------------------------------------------------
    execute_memdump - execute the memdump command
-------------------------------------------------*/

static void execute_memdump(running_machine &machine, int ref, int params, const char **param)
{
	FILE *file;
	const char *filename;

	filename = (params == 0) ? "memdump.log" : param[0];

	debug_console_printf(machine, "Dumping memory to %s\n", filename);

	file = fopen(filename, "w");
	if (file)
	{
		machine.memory().dump(file);
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

static void execute_symlist(running_machine &machine, int ref, int params, const char **param)
{
	device_t *cpu = nullptr;
	const char *namelist[1000];
	symbol_table *symtable;
	int symnum, count = 0;


	if (param[0] != nullptr)
	{
		/* validate parameters */
		if (!debug_command_parameter_cpu(machine, param[0], &cpu))
			return;
		symtable = &cpu->debug()->symtable();
		debug_console_printf(machine, "CPU '%s' symbols:\n", cpu->tag());
	}
	else
	{
		symtable = debug_cpu_get_global_symtable(machine);
		debug_console_printf(machine, "Global symbols:\n");
	}

	/* gather names for all symbols */
	for (symbol_entry *entry = symtable->first(); entry != nullptr; entry = entry->next())
	{
		/* only display "register" type symbols */
		if (!entry->is_function())
		{
			namelist[count++] = entry->name();
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
		const symbol_entry *entry = symtable->find(namelist[symnum]);
		assert(entry != nullptr);
		UINT64 value = entry->value();

		/* only display "register" type symbols */
		debug_console_printf(machine, "%s = %s", namelist[symnum], core_i64_hex_format(value, 0));
		if (!entry->is_lval())
			debug_console_printf(machine, "  (read-only)");
		debug_console_printf(machine, "\n");
	}
}


/*-------------------------------------------------
    execute_softreset - execute the softreset command
-------------------------------------------------*/

static void execute_softreset(running_machine &machine, int ref, int params, const char **param)
{
	machine.schedule_soft_reset();
}


/*-------------------------------------------------
    execute_hardreset - execute the hardreset command
-------------------------------------------------*/

static void execute_hardreset(running_machine &machine, int ref, int params, const char **param)
{
	machine.schedule_hard_reset();
}

/*-------------------------------------------------
    execute_images - lists all image devices with
    mounted files
-------------------------------------------------*/

static void execute_images(running_machine &machine, int ref, int params, const char **param)
{
	image_interface_iterator iter(machine.root_device());
	for (device_image_interface *img = iter.first(); img != nullptr; img = iter.next())
	{
		debug_console_printf(machine, "%s: %s\n",img->brief_instance_name(),img->exists() ? img->filename() : "[empty slot]");
	}
	if (iter.first() == nullptr) {
		debug_console_printf(machine, "No image devices in this driver\n");
	}
}

/*-------------------------------------------------
    execute_mount - execute the image mount command
-------------------------------------------------*/

static void execute_mount(running_machine &machine, int ref, int params, const char **param)
{
	image_interface_iterator iter(machine.root_device());
	bool done = false;
	for (device_image_interface *img = iter.first(); img != nullptr; img = iter.next())
	{
		if (strcmp(img->brief_instance_name(),param[0])==0) {
			if (img->load(param[1])==IMAGE_INIT_FAIL) {
				debug_console_printf(machine, "Unable to mount file %s on %s\n",param[1],param[0]);
			} else {
				debug_console_printf(machine, "File %s mounted on %s\n",param[1],param[0]);
			}
			done = true;
			break;
		}
	}
	if (!done)
		debug_console_printf(machine, "There is no image device :%s\n",param[0]);
}

/*-------------------------------------------------
    execute_unmount - execute the image unmount command
-------------------------------------------------*/

static void execute_unmount(running_machine &machine, int ref, int params, const char **param)
{
	image_interface_iterator iter(machine.root_device());
	bool done = false;
	for (device_image_interface *img = iter.first(); img != nullptr; img = iter.next())
	{
		if (strcmp(img->brief_instance_name(),param[0])==0) {
			img->unload();
			debug_console_printf(machine, "Unmounted file from : %s\n",param[0]);
			done = true;
			break;
		}
	}
	if (!done)
		debug_console_printf(machine, "There is no image device :%s\n",param[0]);
}


/*-------------------------------------------------
    execute_input - debugger command to enter
    natural keyboard input
-------------------------------------------------*/

static void execute_input(running_machine &machine, int ref, int params, const char **param)
{
	machine.ioport().natkeyboard().post_coded(param[0]);
}


/*-------------------------------------------------
    execute_dumpkbd - debugger command to natural
    keyboard codes
-------------------------------------------------*/

static void execute_dumpkbd(running_machine &machine, int ref, int params, const char **param)
{
	// was there a file specified?
	const char *filename = (params > 0) ? param[0] : nullptr;
	FILE *file = nullptr;
	if (filename != nullptr)
	{
		// if so, open it
		file = fopen(filename, "w");
		if (file == nullptr)
		{
			debug_console_printf(machine, "Cannot open \"%s\"\n", filename);
			return;
		}
	}

	// loop through all codes
	std::string buffer = machine.ioport().natkeyboard().dump();

	// and output it as appropriate
	if (file != nullptr)
		fprintf(file, "%s\n", buffer.c_str());
	else
		debug_console_printf(machine, "%s\n", buffer.c_str());

	// cleanup
	if (file != nullptr)
		fclose(file);
}
