// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    debugcmd.c

    Debugger command interface engine.

*********************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "debugger.h"
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

const size_t debugger_commands::MAX_GLOBALS = 1000;

/***************************************************************************
    FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    cheat_address_is_valid - return TRUE if the
    given address is valid for cheating
-------------------------------------------------*/

bool debugger_commands::cheat_address_is_valid(address_space &space, offs_t address)
{
	return m_cpu.translate(space, TRANSLATE_READ, &address) && (space.get_write_ptr(address) != nullptr);
}


/*-------------------------------------------------
    cheat_sign_extend - sign-extend a value to
    the current cheat width, if signed
-------------------------------------------------*/

UINT64 debugger_commands::cheat_sign_extend(const cheat_system *cheatsys, UINT64 value)
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

UINT64 debugger_commands::cheat_byte_swap(const cheat_system *cheatsys, UINT64 value)
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

UINT64 debugger_commands::cheat_read_extended(const cheat_system *cheatsys, address_space &space, offs_t address)
{
	return cheat_sign_extend(cheatsys, cheat_byte_swap(cheatsys, m_cpu.read_memory(space, address, cheatsys->width, TRUE)));
}

debugger_commands::debugger_commands(running_machine& machine, debugger_cpu& cpu, debugger_console& console)
	: m_machine(machine)
	, m_cpu(cpu)
	, m_console(console)
{
	m_global_array = auto_alloc_array_clear(m_machine, global_entry, MAX_GLOBALS);

	symbol_table *symtable = m_cpu.get_global_symtable();

	/* add a few simple global functions */
	using namespace std::placeholders;
	symtable->add("min", nullptr, 2, 2, std::bind(&debugger_commands::execute_min, this, _1, _2, _3, _4));
	symtable->add("max", nullptr, 2, 2, std::bind(&debugger_commands::execute_max, this, _1, _2, _3, _4));
	symtable->add("if", nullptr, 3, 3, std::bind(&debugger_commands::execute_if, this, _1, _2, _3, _4));

	/* add all single-entry save state globals */
	for (int itemnum = 0; itemnum < MAX_GLOBALS; itemnum++)
	{
		UINT32 valsize, valcount;
		void *base;

		/* stop when we run out of items */
		const char* name = m_machine.save().indexed_item(itemnum, base, valsize, valcount);
		if (name == nullptr)
			break;

		/* if this is a single-entry global, add it */
		if (valcount == 1 && strstr(name, "/globals/"))
		{
			char symname[100];
			sprintf(symname, ".%s", strrchr(name, '/') + 1);
			m_global_array[itemnum].base = base;
			m_global_array[itemnum].size = valsize;
			symtable->add(symname, &m_global_array, std::bind(&debugger_commands::global_get, this, _1, _2), std::bind(&debugger_commands::global_set, this, _1, _2, _3));
		}
	}

	/* add all the commands */
	m_console.register_command("help",      CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_help, this, _1, _2, _3));
	m_console.register_command("print",     CMDFLAG_NONE, 0, 1, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_print, this, _1, _2, _3));
	m_console.register_command("printf",    CMDFLAG_NONE, 0, 1, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_printf, this, _1, _2, _3));
	m_console.register_command("logerror",  CMDFLAG_NONE, 0, 1, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_logerror, this, _1, _2, _3));
	m_console.register_command("tracelog",  CMDFLAG_NONE, 0, 1, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_tracelog, this, _1, _2, _3));
	m_console.register_command("quit",      CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_quit, this, _1, _2, _3));
	m_console.register_command("exit",      CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_quit, this, _1, _2, _3));
	m_console.register_command("do",        CMDFLAG_NONE, 0, 1, 1, std::bind(&debugger_commands::execute_do, this, _1, _2, _3));
	m_console.register_command("step",      CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_step, this, _1, _2, _3));
	m_console.register_command("s",         CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_step, this, _1, _2, _3));
	m_console.register_command("over",      CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_over, this, _1, _2, _3));
	m_console.register_command("o",         CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_over, this, _1, _2, _3));
	m_console.register_command("out" ,      CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_out, this, _1, _2, _3));
	m_console.register_command("go",        CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_go, this, _1, _2, _3));
	m_console.register_command("g",         CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_go, this, _1, _2, _3));
	m_console.register_command("gvblank",   CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_go_vblank, this, _1, _2, _3));
	m_console.register_command("gv",        CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_go_vblank, this, _1, _2, _3));
	m_console.register_command("gint",      CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_go_interrupt, this, _1, _2, _3));
	m_console.register_command("gi",        CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_go_interrupt, this, _1, _2, _3));
	m_console.register_command("gtime",     CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_go_time, this, _1, _2, _3));
	m_console.register_command("gt",        CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_go_time, this, _1, _2, _3));
	m_console.register_command("next",      CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_next, this, _1, _2, _3));
	m_console.register_command("n",         CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_next, this, _1, _2, _3));
	m_console.register_command("focus",     CMDFLAG_NONE, 0, 1, 1, std::bind(&debugger_commands::execute_focus, this, _1, _2, _3));
	m_console.register_command("ignore",    CMDFLAG_NONE, 0, 0, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_ignore, this, _1, _2, _3));
	m_console.register_command("observe",   CMDFLAG_NONE, 0, 0, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_observe, this, _1, _2, _3));

	m_console.register_command("comadd",    CMDFLAG_NONE, 0, 1, 2, std::bind(&debugger_commands::execute_comment_add, this, _1, _2, _3));
	m_console.register_command("//",        CMDFLAG_NONE, 0, 1, 2, std::bind(&debugger_commands::execute_comment_add, this, _1, _2, _3));
	m_console.register_command("comdelete", CMDFLAG_NONE, 0, 1, 1, std::bind(&debugger_commands::execute_comment_del, this, _1, _2, _3));
	m_console.register_command("comsave",   CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_comment_save, this, _1, _2, _3));
	m_console.register_command("comlist",   CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_comment_list, this, _1, _2, _3));
	m_console.register_command("commit",    CMDFLAG_NONE, 0, 1, 2, std::bind(&debugger_commands::execute_comment_commit, this, _1, _2, _3));
	m_console.register_command("/*",        CMDFLAG_NONE, 0, 1, 2, std::bind(&debugger_commands::execute_comment_commit, this, _1, _2, _3));

	m_console.register_command("bpset",     CMDFLAG_NONE, 0, 1, 3, std::bind(&debugger_commands::execute_bpset, this, _1, _2, _3));
	m_console.register_command("bp",        CMDFLAG_NONE, 0, 1, 3, std::bind(&debugger_commands::execute_bpset, this, _1, _2, _3));
	m_console.register_command("bpclear",   CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_bpclear, this, _1, _2, _3));
	m_console.register_command("bpdisable", CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_bpdisenable, this, _1, _2, _3));
	m_console.register_command("bpenable",  CMDFLAG_NONE, 1, 0, 1, std::bind(&debugger_commands::execute_bpdisenable, this, _1, _2, _3));
	m_console.register_command("bplist",    CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_bplist, this, _1, _2, _3));

	m_console.register_command("wpset",     CMDFLAG_NONE, AS_PROGRAM, 3, 5, std::bind(&debugger_commands::execute_wpset, this, _1, _2, _3));
	m_console.register_command("wp",        CMDFLAG_NONE, AS_PROGRAM, 3, 5, std::bind(&debugger_commands::execute_wpset, this, _1, _2, _3));
	m_console.register_command("wpdset",    CMDFLAG_NONE, AS_DATA, 3, 5, std::bind(&debugger_commands::execute_wpset, this, _1, _2, _3));
	m_console.register_command("wpd",       CMDFLAG_NONE, AS_DATA, 3, 5, std::bind(&debugger_commands::execute_wpset, this, _1, _2, _3));
	m_console.register_command("wpiset",    CMDFLAG_NONE, AS_IO, 3, 5, std::bind(&debugger_commands::execute_wpset, this, _1, _2, _3));
	m_console.register_command("wpi",       CMDFLAG_NONE, AS_IO, 3, 5, std::bind(&debugger_commands::execute_wpset, this, _1, _2, _3));
	m_console.register_command("wpclear",   CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_wpclear, this, _1, _2, _3));
	m_console.register_command("wpdisable", CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_wpdisenable, this, _1, _2, _3));
	m_console.register_command("wpenable",  CMDFLAG_NONE, 1, 0, 1, std::bind(&debugger_commands::execute_wpdisenable, this, _1, _2, _3));
	m_console.register_command("wplist",    CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_wplist, this, _1, _2, _3));

	m_console.register_command("rpset",     CMDFLAG_NONE, 0, 1, 2, std::bind(&debugger_commands::execute_rpset, this, _1, _2, _3));
	m_console.register_command("rp",        CMDFLAG_NONE, 0, 1, 2, std::bind(&debugger_commands::execute_rpset, this, _1, _2, _3));
	m_console.register_command("rpclear",   CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_rpclear, this, _1, _2, _3));
	m_console.register_command("rpdisable", CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_rpdisenable, this, _1, _2, _3));
	m_console.register_command("rpenable",  CMDFLAG_NONE, 1, 0, 1, std::bind(&debugger_commands::execute_rpdisenable, this, _1, _2, _3));
	m_console.register_command("rplist",    CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_rplist, this, _1, _2, _3));

	m_console.register_command("hotspot",   CMDFLAG_NONE, 0, 0, 3, std::bind(&debugger_commands::execute_hotspot, this, _1, _2, _3));

	m_console.register_command("statesave", CMDFLAG_NONE, 0, 1, 1, std::bind(&debugger_commands::execute_statesave, this, _1, _2, _3));
	m_console.register_command("ss",        CMDFLAG_NONE, 0, 1, 1, std::bind(&debugger_commands::execute_statesave, this, _1, _2, _3));
	m_console.register_command("stateload", CMDFLAG_NONE, 0, 1, 1, std::bind(&debugger_commands::execute_stateload, this, _1, _2, _3));
	m_console.register_command("sl",        CMDFLAG_NONE, 0, 1, 1, std::bind(&debugger_commands::execute_stateload, this, _1, _2, _3));

	m_console.register_command("save",      CMDFLAG_NONE, AS_PROGRAM, 3, 4, std::bind(&debugger_commands::execute_save, this, _1, _2, _3));
	m_console.register_command("saved",     CMDFLAG_NONE, AS_DATA, 3, 4, std::bind(&debugger_commands::execute_save, this, _1, _2, _3));
	m_console.register_command("savei",     CMDFLAG_NONE, AS_IO, 3, 4, std::bind(&debugger_commands::execute_save, this, _1, _2, _3));

	m_console.register_command("load",      CMDFLAG_NONE, AS_PROGRAM, 3, 4, std::bind(&debugger_commands::execute_load, this, _1, _2, _3));
	m_console.register_command("loadd",     CMDFLAG_NONE, AS_DATA, 3, 4, std::bind(&debugger_commands::execute_load, this, _1, _2, _3));
	m_console.register_command("loadi",     CMDFLAG_NONE, AS_IO, 3, 4, std::bind(&debugger_commands::execute_load, this, _1, _2, _3));

	m_console.register_command("dump",      CMDFLAG_NONE, AS_PROGRAM, 3, 6, std::bind(&debugger_commands::execute_dump, this, _1, _2, _3));
	m_console.register_command("dumpd",     CMDFLAG_NONE, AS_DATA, 3, 6, std::bind(&debugger_commands::execute_dump, this, _1, _2, _3));
	m_console.register_command("dumpi",     CMDFLAG_NONE, AS_IO, 3, 6, std::bind(&debugger_commands::execute_dump, this, _1, _2, _3));

	m_console.register_command("cheatinit", CMDFLAG_NONE, 0, 0, 4, std::bind(&debugger_commands::execute_cheatinit, this, _1, _2, _3));
	m_console.register_command("ci",        CMDFLAG_NONE, 0, 0, 4, std::bind(&debugger_commands::execute_cheatinit, this, _1, _2, _3));

	m_console.register_command("cheatrange",CMDFLAG_NONE, 1, 2, 2, std::bind(&debugger_commands::execute_cheatinit, this, _1, _2, _3));
	m_console.register_command("cr",        CMDFLAG_NONE, 1, 2, 2, std::bind(&debugger_commands::execute_cheatinit, this, _1, _2, _3));

	m_console.register_command("cheatnext", CMDFLAG_NONE, 0, 1, 2, std::bind(&debugger_commands::execute_cheatnext, this, _1, _2, _3));
	m_console.register_command("cn",        CMDFLAG_NONE, 0, 1, 2, std::bind(&debugger_commands::execute_cheatnext, this, _1, _2, _3));
	m_console.register_command("cheatnextf",CMDFLAG_NONE, 1, 1, 2, std::bind(&debugger_commands::execute_cheatnext, this, _1, _2, _3));
	m_console.register_command("cnf",       CMDFLAG_NONE, 1, 1, 2, std::bind(&debugger_commands::execute_cheatnext, this, _1, _2, _3));

	m_console.register_command("cheatlist", CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_cheatlist, this, _1, _2, _3));
	m_console.register_command("cl",        CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_cheatlist, this, _1, _2, _3));

	m_console.register_command("cheatundo", CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_cheatundo, this, _1, _2, _3));
	m_console.register_command("cu",        CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_cheatundo, this, _1, _2, _3));

	m_console.register_command("f",         CMDFLAG_KEEP_QUOTES, AS_PROGRAM, 3, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_find, this, _1, _2, _3));
	m_console.register_command("find",      CMDFLAG_KEEP_QUOTES, AS_PROGRAM, 3, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_find, this, _1, _2, _3));
	m_console.register_command("fd",        CMDFLAG_KEEP_QUOTES, AS_DATA, 3, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_find, this, _1, _2, _3));
	m_console.register_command("findd",     CMDFLAG_KEEP_QUOTES, AS_DATA, 3, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_find, this, _1, _2, _3));
	m_console.register_command("fi",        CMDFLAG_KEEP_QUOTES, AS_IO, 3, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_find, this, _1, _2, _3));
	m_console.register_command("findi",     CMDFLAG_KEEP_QUOTES, AS_IO, 3, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_find, this, _1, _2, _3));

	m_console.register_command("dasm",      CMDFLAG_NONE, 0, 3, 5, std::bind(&debugger_commands::execute_dasm, this, _1, _2, _3));

	m_console.register_command("trace",     CMDFLAG_NONE, 0, 1, 3, std::bind(&debugger_commands::execute_trace, this, _1, _2, _3));
	m_console.register_command("traceover", CMDFLAG_NONE, 0, 1, 3, std::bind(&debugger_commands::execute_traceover, this, _1, _2, _3));
	m_console.register_command("traceflush",CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_traceflush, this, _1, _2, _3));

	m_console.register_command("history",   CMDFLAG_NONE, 0, 0, 2, std::bind(&debugger_commands::execute_history, this, _1, _2, _3));
	m_console.register_command("trackpc",   CMDFLAG_NONE, 0, 0, 3, std::bind(&debugger_commands::execute_trackpc, this, _1, _2, _3));

	m_console.register_command("trackmem",  CMDFLAG_NONE, 0, 0, 3, std::bind(&debugger_commands::execute_trackmem, this, _1, _2, _3));
	m_console.register_command("pcatmemp",  CMDFLAG_NONE, AS_PROGRAM, 1, 2, std::bind(&debugger_commands::execute_pcatmem, this, _1, _2, _3));
	m_console.register_command("pcatmemd",  CMDFLAG_NONE, AS_DATA,    1, 2, std::bind(&debugger_commands::execute_pcatmem, this, _1, _2, _3));
	m_console.register_command("pcatmemi",  CMDFLAG_NONE, AS_IO,      1, 2, std::bind(&debugger_commands::execute_pcatmem, this, _1, _2, _3));

	m_console.register_command("snap",      CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_snap, this, _1, _2, _3));

	m_console.register_command("source",    CMDFLAG_NONE, 0, 1, 1, std::bind(&debugger_commands::execute_source, this, _1, _2, _3));

	m_console.register_command("map",       CMDFLAG_NONE, AS_PROGRAM, 1, 1, std::bind(&debugger_commands::execute_map, this, _1, _2, _3));
	m_console.register_command("mapd",      CMDFLAG_NONE, AS_DATA, 1, 1, std::bind(&debugger_commands::execute_map, this, _1, _2, _3));
	m_console.register_command("mapi",      CMDFLAG_NONE, AS_IO, 1, 1, std::bind(&debugger_commands::execute_map, this, _1, _2, _3));
	m_console.register_command("memdump",   CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_memdump, this, _1, _2, _3));

	m_console.register_command("symlist",   CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_symlist, this, _1, _2, _3));

	m_console.register_command("softreset", CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_softreset, this, _1, _2, _3));
	m_console.register_command("hardreset", CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_hardreset, this, _1, _2, _3));

	m_console.register_command("images",    CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_images, this, _1, _2, _3));
	m_console.register_command("mount",     CMDFLAG_NONE, 0, 2, 2, std::bind(&debugger_commands::execute_mount, this, _1, _2, _3));
	m_console.register_command("unmount",   CMDFLAG_NONE, 0, 1, 1, std::bind(&debugger_commands::execute_unmount, this, _1, _2, _3));

	m_console.register_command("input",     CMDFLAG_NONE, 0, 1, 1, std::bind(&debugger_commands::execute_input, this, _1, _2, _3));
	m_console.register_command("dumpkbd",   CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_dumpkbd, this, _1, _2, _3));

	/* set up the initial debugscript if specified */
	const char* name = m_machine.options().debug_script();
	if (name[0] != 0)
		m_cpu.source_script(name);

	m_cheat.cpu[0] = m_cheat.cpu[1] = 0;
}

/*-------------------------------------------------
    execute_min - return the minimum of two values
-------------------------------------------------*/

UINT64 debugger_commands::execute_min(symbol_table &table, void *ref, int params, const UINT64 *param)
{
	return (param[0] < param[1]) ? param[0] : param[1];
}


/*-------------------------------------------------
    execute_max - return the maximum of two values
-------------------------------------------------*/

UINT64 debugger_commands::execute_max(symbol_table &table, void *ref, int params, const UINT64 *param)
{
	return (param[0] > param[1]) ? param[0] : param[1];
}


/*-------------------------------------------------
    execute_if - if (a) return b; else return c;
-------------------------------------------------*/

UINT64 debugger_commands::execute_if(symbol_table &table, void *ref, int params, const UINT64 *param)
{
	return param[0] ? param[1] : param[2];
}



/***************************************************************************
    GLOBAL ACCESSORS
***************************************************************************/

/*-------------------------------------------------
    global_get - symbol table getter for globals
-------------------------------------------------*/

UINT64 debugger_commands::global_get(symbol_table &table, void *ref)
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

void debugger_commands::global_set(symbol_table &table, void *ref, UINT64 value)
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
    validate_number_parameter - validates a
    number parameter
-------------------------------------------------*/

bool debugger_commands::validate_number_parameter(const char *param, UINT64 *result)
{
	/* nullptr parameter does nothing and returns no error */
	if (param == nullptr)
		return true;

	/* evaluate the expression; success if no error */
	try
	{
		parsed_expression expression(m_cpu.get_visible_symtable(), param, result);
		return true;
	}
	catch (expression_error &error)
	{
		/* print an error pointing to the character that caused it */
		m_console.printf("Error in expression: %s\n", param);
		m_console.printf("                     %*s^", error.offset(), "");
		m_console.printf("%s\n", error.code_string());
		return false;
	}
}


/*-------------------------------------------------
    validate_cpu_parameter - validates a
    parameter as a cpu
-------------------------------------------------*/

bool debugger_commands::validate_cpu_parameter(const char *param, device_t **result)
{
	/* if no parameter, use the visible CPU */
	if (param == nullptr)
	{
		*result = m_cpu.get_visible_cpu();
		if (*result == nullptr)
		{
			m_console.printf("No valid CPU is currently selected\n");
			return false;
		}
		return true;
	}

	/* first look for a tag match */
	*result = m_machine.device(param);
	if (*result != nullptr)
		return true;

	/* then evaluate as an expression; on an error assume it was a tag */
	UINT64 cpunum;
	try
	{
		parsed_expression expression(m_cpu.get_visible_symtable(), param, &cpunum);
	}
	catch (expression_error &)
	{
		m_console.printf("Unable to find CPU '%s'\n", param);
		return false;
	}

	/* if we got a valid one, return */
	device_execute_interface *exec = execute_interface_iterator(m_machine.root_device()).byindex(cpunum);
	if (exec != nullptr)
	{
		*result = &exec->device();
		return true;
	}

	/* if out of range, complain */
	m_console.printf("Invalid CPU index %d\n", (int)cpunum);
	return false;
}


/*-------------------------------------------------
    validate_cpu_space_parameter - validates
    a parameter as a cpu and retrieves the given
    address space
-------------------------------------------------*/

bool debugger_commands::validate_cpu_space_parameter(const char *param, int spacenum, address_space *&result)
{
	/* first do the standard CPU thing */
	device_t *cpu;
	if (!validate_cpu_parameter(param, &cpu))
		return false;

	/* fetch the space pointer */
	if (!cpu->memory().has_space(spacenum))
	{
		m_console.printf("No matching memory space found for CPU '%s'\n", cpu->tag());
		return false;
	}
	result = &cpu->memory().space(spacenum);
	return true;
}


/*-------------------------------------------------
    debug_command_parameter_expression - validates
    an expression parameter
-------------------------------------------------*/

bool debugger_commands::debug_command_parameter_expression(const char *param, parsed_expression &result)
{
	/* nullptr parameter does nothing and returns no error */
	if (param == nullptr)
		return true;

	/* parse the expression; success if no error */
	try
	{
		result.parse(param);
		return true;
	}
	catch (expression_error &err)
	{
		/* output an error */
		m_console.printf("Error in expression: %s\n", param);
		m_console.printf("                     %*s^", err.offset(), "");
		m_console.printf("%s\n", err.code_string());
		return false;
	}
}


/*-------------------------------------------------
    debug_command_parameter_command - validates a
    command parameter
-------------------------------------------------*/

bool debugger_commands::debug_command_parameter_command(const char *param)
{
	/* nullptr parameter does nothing and returns no error */
	if (param == nullptr)
		return true;

	/* validate the comment; success if no error */
	CMDERR err = m_console.validate_command(param);
	if (err == CMDERR_NONE)
		return true;

	/* output an error */
	m_console.printf("Error in command: %s\n", param);
	m_console.printf("                  %*s^", CMDERR_ERROR_OFFSET(err), "");
	m_console.printf("%s\n", debugger_console::cmderr_to_string(err));
	return 0;
}

/*-------------------------------------------------
    execute_help - execute the help command
-------------------------------------------------*/

void debugger_commands::execute_help(int ref, int params, const char *param[])
{
	if (params == 0)
		m_console.printf_wrap(80, "%s\n", debug_get_help(""));
	else
		m_console.printf_wrap(80, "%s\n", debug_get_help(param[0]));
}


/*-------------------------------------------------
    execute_print - execute the print command
-------------------------------------------------*/

void debugger_commands::execute_print(int ref, int params, const char *param[])
{
	/* validate the other parameters */
	UINT64 values[MAX_COMMAND_PARAMS];
	for (int i = 0; i < params; i++)
		if (!validate_number_parameter(param[i], &values[i]))
			return;

	/* then print each one */
	for (int i = 0; i < params; i++)
		m_console.printf("%X", values[i]);
	m_console.printf("\n");
}


/*-------------------------------------------------
    mini_printf - safe printf to a buffer
-------------------------------------------------*/

int debugger_commands::mini_printf(char *buffer, const char *format, int params, UINT64 *param)
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
						m_console.printf("Not enough parameters for format!\n");
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
						m_console.printf("Not enough parameters for format!\n");
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

void debugger_commands::execute_printf(int ref, int params, const char *param[])
{
	/* validate the other parameters */
	UINT64 values[MAX_COMMAND_PARAMS];
	for (int i = 1; i < params; i++)
		if (!validate_number_parameter(param[i], &values[i]))
			return;

	/* then do a printf */
	char buffer[1024];
	if (mini_printf(buffer, param[0], params - 1, &values[1]))
		m_console.printf("%s\n", buffer);
}


/*-------------------------------------------------
    execute_logerror - execute the logerror command
-------------------------------------------------*/

void debugger_commands::execute_logerror(int ref, int params, const char *param[])
{
	/* validate the other parameters */
	UINT64 values[MAX_COMMAND_PARAMS];
	for (int i = 1; i < params; i++)
		if (!validate_number_parameter(param[i], &values[i]))
			return;

	/* then do a printf */
	char buffer[1024];
	if (mini_printf(buffer, param[0], params - 1, &values[1]))
		m_machine.logerror("%s", buffer);
}


/*-------------------------------------------------
    execute_tracelog - execute the tracelog command
-------------------------------------------------*/

void debugger_commands::execute_tracelog(int ref, int params, const char *param[])
{
	/* validate the other parameters */
	UINT64 values[MAX_COMMAND_PARAMS];
	for (int i = 1; i < params; i++)
		if (!validate_number_parameter(param[i], &values[i]))
			return;

	/* then do a printf */
	char buffer[1024];
	if (mini_printf(buffer, param[0], params - 1, &values[1]))
		m_cpu.get_visible_cpu()->debug()->trace_printf("%s", buffer);
}


/*-------------------------------------------------
    execute_quit - execute the quit command
-------------------------------------------------*/

void debugger_commands::execute_quit(int ref, int params, const char *param[])
{
	osd_printf_error("Exited via the debugger\n");
	m_machine.schedule_exit();
}


/*-------------------------------------------------
    execute_do - execute the do command
-------------------------------------------------*/

void debugger_commands::execute_do(int ref, int params, const char *param[])
{
	UINT64 dummy;
	validate_number_parameter(param[0], &dummy);
}


/*-------------------------------------------------
    execute_step - execute the step command
-------------------------------------------------*/

void debugger_commands::execute_step(int ref, int params, const char *param[])
{
	/* if we have a parameter, use it */
	UINT64 steps = 1;
	if (!validate_number_parameter(param[0], &steps))
		return;

	m_cpu.get_visible_cpu()->debug()->single_step(steps);
}


/*-------------------------------------------------
    execute_over - execute the over command
-------------------------------------------------*/

void debugger_commands::execute_over(int ref, int params, const char *param[])
{
	/* if we have a parameter, use it */
	UINT64 steps = 1;
	if (!validate_number_parameter(param[0], &steps))
		return;

	m_cpu.get_visible_cpu()->debug()->single_step_over(steps);
}


/*-------------------------------------------------
    execute_out - execute the out command
-------------------------------------------------*/

void debugger_commands::execute_out(int ref, int params, const char *param[])
{
	m_cpu.get_visible_cpu()->debug()->single_step_out();
}


/*-------------------------------------------------
    execute_go - execute the go command
-------------------------------------------------*/

void debugger_commands::execute_go(int ref, int params, const char *param[])
{
	UINT64 addr = ~0;

	/* if we have a parameter, use it instead */
	if (!validate_number_parameter(param[0], &addr))
		return;

	m_cpu.get_visible_cpu()->debug()->go(addr);
}


/*-------------------------------------------------
    execute_go_vblank - execute the govblank
    command
-------------------------------------------------*/

void debugger_commands::execute_go_vblank(int ref, int params, const char *param[])
{
	m_cpu.get_visible_cpu()->debug()->go_vblank();
}


/*-------------------------------------------------
    execute_go_interrupt - execute the goint command
-------------------------------------------------*/

void debugger_commands::execute_go_interrupt(int ref, int params, const char *param[])
{
	UINT64 irqline = -1;

	/* if we have a parameter, use it instead */
	if (!validate_number_parameter(param[0], &irqline))
		return;

	m_cpu.get_visible_cpu()->debug()->go_interrupt(irqline);
}


/*-------------------------------------------------
    execute_go_time - execute the gtime command
-------------------------------------------------*/

void debugger_commands::execute_go_time(int ref, int params, const char *param[])
{
	UINT64 milliseconds = -1;

	/* if we have a parameter, use it instead */
	if (!validate_number_parameter(param[0], &milliseconds))
		return;

	m_cpu.get_visible_cpu()->debug()->go_milliseconds(milliseconds);
}


/*-------------------------------------------------
    execute_next - execute the next command
-------------------------------------------------*/

void debugger_commands::execute_next(int ref, int params, const char *param[])
{
	m_cpu.get_visible_cpu()->debug()->go_next_device();
}


/*-------------------------------------------------
    execute_focus - execute the focus command
-------------------------------------------------*/

void debugger_commands::execute_focus(int ref, int params, const char *param[])
{
	/* validate params */
	device_t *cpu;
	if (!validate_cpu_parameter(param[0], &cpu))
		return;

	/* first clear the ignore flag on the focused CPU */
	cpu->debug()->ignore(false);

	/* then loop over CPUs and set the ignore flags on all other CPUs */
	for (device_execute_interface &exec : execute_interface_iterator(m_machine.root_device()))
		if (&exec.device() != cpu)
			exec.device().debug()->ignore(true);
	m_console.printf("Now focused on CPU '%s'\n", cpu->tag());
}


/*-------------------------------------------------
    execute_ignore - execute the ignore command
-------------------------------------------------*/

void debugger_commands::execute_ignore(int ref, int params, const char *param[])
{
	/* if there are no parameters, dump the ignore list */
	if (params == 0)
	{
		std::string buffer;

		/* loop over all executable devices */
		for (device_execute_interface &exec : execute_interface_iterator(m_machine.root_device()))

			/* build up a comma-separated list */
			if (!exec.device().debug()->observing())
			{
				if (buffer.empty())
					buffer = string_format("Currently ignoring device '%s'", exec.device().tag());
				else
					buffer.append(string_format(", '%s'", exec.device().tag()));
			}

		/* special message for none */
		if (buffer.empty())
			buffer = string_format("Not currently ignoring any devices");
		m_console.printf("%s\n", buffer.c_str());
	}

	/* otherwise clear the ignore flag on all requested CPUs */
	else
	{
		device_t *devicelist[MAX_COMMAND_PARAMS];

		/* validate parameters */
		for (int paramnum = 0; paramnum < params; paramnum++)
			if (!validate_cpu_parameter(param[paramnum], &devicelist[paramnum]))
				return;

		/* set the ignore flags */
		for (int paramnum = 0; paramnum < params; paramnum++)
		{
			/* make sure this isn't the last live CPU */
			bool gotone = false;
			for (device_execute_interface &exec : execute_interface_iterator(m_machine.root_device()))
				if (&exec.device() != devicelist[paramnum] && exec.device().debug()->observing())
				{
					gotone = true;
					break;
				}
			if (!gotone)
			{
				m_console.printf("Can't ignore all devices!\n");
				return;
			}

			devicelist[paramnum]->debug()->ignore(true);
			m_console.printf("Now ignoring device '%s'\n", devicelist[paramnum]->tag());
		}
	}
}


/*-------------------------------------------------
    execute_observe - execute the observe command
-------------------------------------------------*/

void debugger_commands::execute_observe(int ref, int params, const char *param[])
{
	/* if there are no parameters, dump the ignore list */
	if (params == 0)
	{
		std::string buffer;

		/* loop over all executable devices */
		for (device_execute_interface &exec : execute_interface_iterator(m_machine.root_device()))

			/* build up a comma-separated list */
			if (exec.device().debug()->observing())
			{
				if (buffer.empty())
					buffer = string_format("Currently observing CPU '%s'", exec.device().tag());
				else
					buffer.append(string_format(", '%s'", exec.device().tag()));
			}

		/* special message for none */
		if (buffer.empty())
			buffer = string_format("Not currently observing any devices");
		m_console.printf("%s\n", buffer.c_str());
	}

	/* otherwise set the ignore flag on all requested CPUs */
	else
	{
		device_t *devicelist[MAX_COMMAND_PARAMS];

		/* validate parameters */
		for (int paramnum = 0; paramnum < params; paramnum++)
			if (!validate_cpu_parameter(param[paramnum], &devicelist[paramnum]))
				return;

		/* clear the ignore flags */
		for (int paramnum = 0; paramnum < params; paramnum++)
		{
			devicelist[paramnum]->debug()->ignore(false);
			m_console.printf("Now observing device '%s'\n", devicelist[paramnum]->tag());
		}
	}
}


/*-------------------------------------------------
    execute_comment - add a comment to a line
-------------------------------------------------*/

void debugger_commands::execute_comment_add(int ref, int params, const char *param[])
{
	device_t *cpu;
	UINT64 address;

	/* param 1 is the address for the comment */
	if (!validate_number_parameter(param[0], &address))
		return;

	/* CPU parameter is implicit */
	if (!validate_cpu_parameter(nullptr, &cpu))
		return;

	/* make sure param 2 exists */
	if (strlen(param[1]) == 0)
	{
		m_console.printf("Error : comment text empty\n");
		return;
	}

	/* Now try adding the comment */
	cpu->debug()->comment_add(address, param[1], 0x00ff0000);
	cpu->machine().debug_view().update_all(DVT_DISASSEMBLY);
}


/*------------------------------------------------------
    execute_comment_del - remove a comment from an addr
--------------------------------------------------------*/

void debugger_commands::execute_comment_del(int ref, int params, const char *param[])
{
	device_t *cpu;
	UINT64 address;

	/* param 1 can either be a command or the address for the comment */
	if (!validate_number_parameter(param[0], &address))
		return;

	/* CPU parameter is implicit */
	if (!validate_cpu_parameter(nullptr, &cpu))
		return;

	/* If it's a number, it must be an address */
	/* The bankoff and cbn will be pulled from what's currently active */
	cpu->debug()->comment_remove(address);
	cpu->machine().debug_view().update_all(DVT_DISASSEMBLY);
}

/**
 * @fn void execute_comment_list(running_machine &machine, int ref, int params, const char *param[])
 * @brief Print current list of comments in debugger
 *
 *
 */

void debugger_commands::execute_comment_list(int ref, int params, const char *param[])
{
	if (!m_machine.debugger().cpu().comment_load(false))
		m_console.printf("Error while parsing XML file\n");
}

/**
 * @fn void execute_comment_commit(running_machine &machine, int ref, int params, const char *param[])
 * @brief Add and Save current list of comments in debugger
 *
 */

void debugger_commands::execute_comment_commit(int ref, int params, const char *param[])
{
	execute_comment_add(ref, params, param);
	execute_comment_save(ref, params, param);
}

/*-------------------------------------------------
    execute_comment - add a comment to a line
-------------------------------------------------*/

void debugger_commands::execute_comment_save(int ref, int params, const char *param[])
{
	if (m_cpu.comment_save())
		m_console.printf("Comment successfully saved\n");
	else
		m_console.printf("Comment not saved\n");
}

// TODO: add color hex editing capabilities for comments, see below for more info
/**
 * @fn void execute_comment_color(running_machine &machine, int ref, int params, const char *param[])
 * @brief Modifies comment given at address $xx with given color
 * Useful for marking comment with a different color scheme (for example by marking start and end of a given function visually).
 * @param[in] "address,color" First is the comment address in the current context, color can be hexadecimal or shorthanded to common 1bpp RGB names.
 *
 * @todo check if the comment exists in the first place, bail out with error if not.
 * @todo add shorthand for color modify and save
 *
 */



/*-------------------------------------------------
    execute_bpset - execute the breakpoint set
    command
-------------------------------------------------*/

void debugger_commands::execute_bpset(int ref, int params, const char *param[])
{
	device_t *cpu;
	const char *action = nullptr;
	UINT64 address;
	int bpnum;

	/* CPU is implicit */
	if (!validate_cpu_parameter(nullptr, &cpu))
		return;

	/* param 1 is the address */
	if (!validate_number_parameter(param[0], &address))
		return;

	/* param 2 is the condition */
	parsed_expression condition(&cpu->debug()->symtable());
	if (!debug_command_parameter_expression(param[1], condition))
		return;

	/* param 3 is the action */
	if (!debug_command_parameter_command(action = param[2]))
		return;

	/* set the breakpoint */
	bpnum = cpu->debug()->breakpoint_set(address, (condition.is_empty()) ? nullptr : condition.original_string(), action);
	m_console.printf("Breakpoint %X set\n", bpnum);
}


/*-------------------------------------------------
    execute_bpclear - execute the breakpoint
    clear command
-------------------------------------------------*/

void debugger_commands::execute_bpclear(int ref, int params, const char *param[])
{
	UINT64 bpindex;

	/* if 0 parameters, clear all */
	if (params == 0)
	{
		for (device_t &device : device_iterator(m_machine.root_device()))
			device.debug()->breakpoint_clear_all();
		m_console.printf("Cleared all breakpoints\n");
	}

	/* otherwise, clear the specific one */
	else if (!validate_number_parameter(param[0], &bpindex))
		return;
	else
	{
		bool found = false;
		for (device_t &device : device_iterator(m_machine.root_device()))
			if (device.debug()->breakpoint_clear(bpindex))
				found = true;
		if (found)
			m_console.printf("Breakpoint %X cleared\n", (UINT32)bpindex);
		else
			m_console.printf("Invalid breakpoint number %X\n", (UINT32)bpindex);
	}
}


/*-------------------------------------------------
    execute_bpdisenable - execute the breakpoint
    disable/enable commands
-------------------------------------------------*/

void debugger_commands::execute_bpdisenable(int ref, int params, const char *param[])
{
	UINT64 bpindex;

	/* if 0 parameters, clear all */
	if (params == 0)
	{
		for (device_t &device : device_iterator(m_machine.root_device()))
			device.debug()->breakpoint_enable_all(ref);
		if (ref == 0)
			m_console.printf("Disabled all breakpoints\n");
		else
			m_console.printf("Enabled all breakpoints\n");
	}

	/* otherwise, clear the specific one */
	else if (!validate_number_parameter(param[0], &bpindex))
		return;
	else
	{
		bool found = false;
		for (device_t &device : device_iterator(m_machine.root_device()))
			if (device.debug()->breakpoint_enable(bpindex, ref))
				found = true;
		if (found)
			m_console.printf("Breakpoint %X %s\n", (UINT32)bpindex, ref ? "enabled" : "disabled");
		else
			m_console.printf("Invalid breakpoint number %X\n", (UINT32)bpindex);
	}
}


/*-------------------------------------------------
    execute_bplist - execute the breakpoint list
    command
-------------------------------------------------*/

void debugger_commands::execute_bplist(int ref, int params, const char *param[])
{
	int printed = 0;
	std::string buffer;

	/* loop over all CPUs */
	for (device_t &device : device_iterator(m_machine.root_device()))
		if (device.debug()->breakpoint_first() != nullptr)
		{
			m_console.printf("Device '%s' breakpoints:\n", device.tag());

			/* loop over the breakpoints */
			for (device_debug::breakpoint *bp = device.debug()->breakpoint_first(); bp != nullptr; bp = bp->next())
			{
				buffer = string_format("%c%4X @ %0*X", bp->enabled() ? ' ' : 'D', bp->index(), device.debug()->logaddrchars(), bp->address());
				if (std::string(bp->condition()).compare("1") != 0)
					buffer.append(string_format(" if %s", bp->condition()));
				if (std::string(bp->action()).compare("") != 0)
					buffer.append(string_format(" do %s", bp->action()));
				m_console.printf("%s\n", buffer.c_str());
				printed++;
			}
		}

	if (printed == 0)
		m_console.printf("No breakpoints currently installed\n");
}


/*-------------------------------------------------
    execute_wpset - execute the watchpoint set
    command
-------------------------------------------------*/

void debugger_commands::execute_wpset(int ref, int params, const char *param[])
{
	address_space *space;
	const char *action = nullptr;
	UINT64 address, length;
	int type;
	int wpnum;

	/* CPU is implicit */
	if (!validate_cpu_space_parameter(nullptr, ref, space))
		return;

	/* param 1 is the address */
	if (!validate_number_parameter(param[0], &address))
		return;

	/* param 2 is the length */
	if (!validate_number_parameter(param[1], &length))
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
		m_console.printf("Invalid watchpoint type: expected r, w, or rw\n");
		return;
	}

	/* param 4 is the condition */
	parsed_expression condition(&space->device().debug()->symtable());
	if (!debug_command_parameter_expression(param[3], condition))
		return;

	/* param 5 is the action */
	if (!debug_command_parameter_command(action = param[4]))
		return;

	/* set the watchpoint */
	wpnum = space->device().debug()->watchpoint_set(*space, type, address, length, (condition.is_empty()) ? nullptr : condition.original_string(), action);
	m_console.printf("Watchpoint %X set\n", wpnum);
}


/*-------------------------------------------------
    execute_wpclear - execute the watchpoint
    clear command
-------------------------------------------------*/

void debugger_commands::execute_wpclear(int ref, int params, const char *param[])
{
	UINT64 wpindex;

	/* if 0 parameters, clear all */
	if (params == 0)
	{
		for (device_t &device : device_iterator(m_machine.root_device()))
			device.debug()->watchpoint_clear_all();
		m_console.printf("Cleared all watchpoints\n");
	}

	/* otherwise, clear the specific one */
	else if (!validate_number_parameter(param[0], &wpindex))
		return;
	else
	{
		bool found = false;
		for (device_t &device : device_iterator(m_machine.root_device()))
			if (device.debug()->watchpoint_clear(wpindex))
				found = true;
		if (found)
			m_console.printf("Watchpoint %X cleared\n", (UINT32)wpindex);
		else
			m_console.printf("Invalid watchpoint number %X\n", (UINT32)wpindex);
	}
}


/*-------------------------------------------------
    execute_wpdisenable - execute the watchpoint
    disable/enable commands
-------------------------------------------------*/

void debugger_commands::execute_wpdisenable(int ref, int params, const char *param[])
{
	UINT64 wpindex;

	/* if 0 parameters, clear all */
	if (params == 0)
	{
		for (device_t &device : device_iterator(m_machine.root_device()))
			device.debug()->watchpoint_enable_all(ref);
		if (ref == 0)
			m_console.printf("Disabled all watchpoints\n");
		else
			m_console.printf("Enabled all watchpoints\n");
	}

	/* otherwise, clear the specific one */
	else if (!validate_number_parameter(param[0], &wpindex))
		return;
	else
	{
		bool found = false;
		for (device_t &device : device_iterator(m_machine.root_device()))
			if (device.debug()->watchpoint_enable(wpindex, ref))
				found = true;
		if (found)
			m_console.printf("Watchpoint %X %s\n", (UINT32)wpindex, ref ? "enabled" : "disabled");
		else
			m_console.printf("Invalid watchpoint number %X\n", (UINT32)wpindex);
	}
}


/*-------------------------------------------------
    execute_wplist - execute the watchpoint list
    command
-------------------------------------------------*/

void debugger_commands::execute_wplist(int ref, int params, const char *param[])
{
	int printed = 0;
	std::string buffer;

	/* loop over all CPUs */
	for (device_t &device : device_iterator(m_machine.root_device()))
		for (address_spacenum spacenum = AS_0; spacenum < ADDRESS_SPACES; ++spacenum)
			if (device.debug()->watchpoint_first(spacenum) != nullptr)
			{
				static const char *const types[] = { "unkn ", "read ", "write", "r/w  " };

				m_console.printf("Device '%s' %s space watchpoints:\n", device.tag(),
																						device.debug()->watchpoint_first(spacenum)->space().name());

				/* loop over the watchpoints */
				for (device_debug::watchpoint *wp = device.debug()->watchpoint_first(spacenum); wp != nullptr; wp = wp->next())
				{
					buffer = string_format("%c%4X @ %0*X-%0*X %s", wp->enabled() ? ' ' : 'D', wp->index(),
							wp->space().addrchars(), wp->space().byte_to_address(wp->address()),
							wp->space().addrchars(), wp->space().byte_to_address_end(wp->address() + wp->length()) - 1,
							types[wp->type() & 3]);
					if (std::string(wp->condition()).compare("1") != 0)
						buffer.append(string_format(" if %s", wp->condition()));
					if (std::string(wp->action()).compare("") != 0)
						buffer.append(string_format(" do %s", wp->action()));
					m_console.printf("%s\n", buffer.c_str());
					printed++;
				}
			}

	if (printed == 0)
		m_console.printf("No watchpoints currently installed\n");
}


/*-------------------------------------------------
    execute_rpset - execute the registerpoint set
    command
-------------------------------------------------*/

void debugger_commands::execute_rpset(int ref, int params, const char *param[])
{
	device_t *cpu;
	const char *action = nullptr;
	int bpnum;

	/* CPU is implicit */
	if (!validate_cpu_parameter(nullptr, &cpu))
		return;

	/* param 1 is the condition */
	parsed_expression condition(&cpu->debug()->symtable());
	if (!debug_command_parameter_expression(param[0], condition))
		return;

	/* param 2 is the action */
	if (!debug_command_parameter_command(action = param[1]))
		return;

	/* set the breakpoint */
	bpnum = cpu->debug()->registerpoint_set(condition.original_string(), action);
	m_console.printf("Registerpoint %X set\n", bpnum);
}


/*-------------------------------------------------
    execute_rpclear - execute the registerpoint
    clear command
-------------------------------------------------*/

void debugger_commands::execute_rpclear(int ref, int params, const char *param[])
{
	UINT64 rpindex;

	/* if 0 parameters, clear all */
	if (params == 0)
	{
		for (device_t &device : device_iterator(m_machine.root_device()))
			device.debug()->registerpoint_clear_all();
		m_console.printf("Cleared all registerpoints\n");
	}

	/* otherwise, clear the specific one */
	else if (!validate_number_parameter(param[0], &rpindex))
		return;
	else
	{
		bool found = false;
		for (device_t &device : device_iterator(m_machine.root_device()))
			if (device.debug()->registerpoint_clear(rpindex))
				found = true;
		if (found)
			m_console.printf("Registerpoint %X cleared\n", (UINT32)rpindex);
		else
			m_console.printf("Invalid registerpoint number %X\n", (UINT32)rpindex);
	}
}


/*-------------------------------------------------
    execute_rpdisenable - execute the registerpoint
    disable/enable commands
-------------------------------------------------*/

void debugger_commands::execute_rpdisenable(int ref, int params, const char *param[])
{
	UINT64 rpindex;

	/* if 0 parameters, clear all */
	if (params == 0)
	{
		for (device_t &device : device_iterator(m_machine.root_device()))
			device.debug()->registerpoint_enable_all(ref);
		if (ref == 0)
			m_console.printf("Disabled all registerpoints\n");
		else
			m_console.printf("Enabled all registeroints\n");
	}

	/* otherwise, clear the specific one */
	else if (!validate_number_parameter(param[0], &rpindex))
		return;
	else
	{
		bool found = false;
		for (device_t &device : device_iterator(m_machine.root_device()))
			if (device.debug()->registerpoint_enable(rpindex, ref))
				found = true;
		if (found)
			m_console.printf("Registerpoint %X %s\n", (UINT32)rpindex, ref ? "enabled" : "disabled");
		else
			m_console.printf("Invalid registerpoint number %X\n", (UINT32)rpindex);
	}
}


/*-------------------------------------------------
    execute_rplist - execute the registerpoint list
    command
-------------------------------------------------*/

void debugger_commands::execute_rplist(int ref, int params, const char *param[])
{
	int printed = 0;
	std::string buffer;

	/* loop over all CPUs */
	for (device_t &device : device_iterator(m_machine.root_device()))
		if (device.debug()->registerpoint_first() != nullptr)
		{
			m_console.printf("Device '%s' registerpoints:\n", device.tag());

			/* loop over the breakpoints */
			for (device_debug::registerpoint *rp = device.debug()->registerpoint_first(); rp != nullptr; rp = rp->next())
			{
				buffer = string_format("%c%4X if %s", rp->enabled() ? ' ' : 'D', rp->index(), rp->condition());
				if (rp->action() != nullptr)
					buffer.append(string_format(" do %s", rp->action()));
				m_console.printf("%s\n", buffer.c_str());
				printed++;
			}
		}

	if (printed == 0)
		m_console.printf("No registerpoints currently installed\n");
}


/*-------------------------------------------------
    execute_hotspot - execute the hotspot
    command
-------------------------------------------------*/

void debugger_commands::execute_hotspot(int ref, int params, const char *param[])
{
	/* if no params, and there are live hotspots, clear them */
	if (params == 0)
	{
		bool cleared = false;

		/* loop over CPUs and find live spots */
		for (device_t &device : device_iterator(m_machine.root_device()))
			if (device.debug()->hotspot_tracking_enabled())
			{
				device.debug()->hotspot_track(0, 0);
				m_console.printf("Cleared hotspot tracking on CPU '%s'\n", device.tag());
				cleared = true;
			}

		/* if we cleared, we're done */
		if (cleared)
			return;
	}

	/* extract parameters */
	device_t *device = nullptr;
	if (!validate_cpu_parameter((params > 0) ? param[0] : nullptr, &device))
		return;
	UINT64 count = 64;
	if (!validate_number_parameter(param[1], &count))
		return;
	UINT64 threshhold = 250;
	if (!validate_number_parameter(param[2], &threshhold))
		return;

	/* attempt to install */
	device->debug()->hotspot_track(count, threshhold);
	m_console.printf("Now tracking hotspots on CPU '%s' using %d slots with a threshold of %d\n", device->tag(), (int)count, (int)threshhold);
}


/*-------------------------------------------------
    execute_statesave - execute the statesave command
-------------------------------------------------*/

void debugger_commands::execute_statesave(int ref, int params, const char *param[])
{
	std::string filename(param[0]);
	m_machine.immediate_save(filename.c_str());
	m_console.printf("State save attempted.  Please refer to window message popup for results.\n");
}


/*-------------------------------------------------
    execute_stateload - execute the stateload command
-------------------------------------------------*/

void debugger_commands::execute_stateload(int ref, int params, const char *param[])
{
	std::string filename(param[0]);
	m_machine.immediate_load(filename.c_str());

	// Clear all PC & memory tracks
	for (device_t &device : device_iterator(m_machine.root_device()))
	{
		device.debug()->track_pc_data_clear();
		device.debug()->track_mem_data_clear();
	}
	m_console.printf("State load attempted.  Please refer to window message popup for results.\n");
}


/*-------------------------------------------------
    execute_save - execute the save command
-------------------------------------------------*/

void debugger_commands::execute_save(int ref, int params, const char *param[])
{
	UINT64 offset, endoffset, length;
	address_space *space;
	FILE *f;
	UINT64 i;

	/* validate parameters */
	if (!validate_number_parameter(param[1], &offset))
		return;
	if (!validate_number_parameter(param[2], &length))
		return;
	if (!validate_cpu_space_parameter((params > 3) ? param[3] : nullptr, ref, space))
		return;

	/* determine the addresses to write */
	endoffset = space->address_to_byte(offset + length - 1) & space->bytemask();
	offset = space->address_to_byte(offset) & space->bytemask();

	/* open the file */
	f = fopen(param[0], "wb");
	if (!f)
	{
		m_console.printf("Error opening file '%s'\n", param[0]);
		return;
	}

	/* now write the data out */
	for (i = offset; i <= endoffset; i++)
	{
		UINT8 byte = m_cpu.read_byte(*space, i, TRUE);
		fwrite(&byte, 1, 1, f);
	}

	/* close the file */
	fclose(f);
	m_console.printf("Data saved successfully\n");
}


/*-------------------------------------------------
    execute_load - execute the load command
-------------------------------------------------*/

void debugger_commands::execute_load(int ref, int params, const char *param[])
{
	UINT64 offset, endoffset, length;
	address_space *space;
	FILE *f;
	UINT64 i;

	/* validate parameters */
	if (!validate_number_parameter(param[1], &offset))
		return;
	if (!validate_number_parameter(param[2], &length))
		return;
	if (!validate_cpu_space_parameter((params > 3) ? param[3] : nullptr, ref, space))
		return;

	/* determine the addresses to read */
	endoffset = space->address_to_byte(offset + length - 1) & space->bytemask();
	offset = space->address_to_byte(offset) & space->bytemask();

	/* open the file */
	f = fopen(param[0], "rb");
	if (!f)
	{
		m_console.printf("Error opening file '%s'\n", param[0]);
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
		m_cpu.write_byte(*space, i, byte, true);
	}
	/* close the file */
	fclose(f);
	if ( i == offset)
		m_console.printf("Length specified too large, load failed\n");
	else
		m_console.printf("Data loaded successfully to memory : 0x%X to 0x%X\n", offset, i-1);
}


/*-------------------------------------------------
    execute_dump - execute the dump command
-------------------------------------------------*/

void debugger_commands::execute_dump(int ref, int params, const char *param[])
{
	/* validate parameters */
	UINT64 offset;
	if (!validate_number_parameter(param[1], &offset))
		return;

	UINT64 length;
	if (!validate_number_parameter(param[2], &length))
		return;

	UINT64 width = 0;
	if (!validate_number_parameter(param[3], &width))
		return;

	UINT64 ascii = 1;
	if (!validate_number_parameter(param[4], &ascii))
		return;

	address_space *space;
	if (!validate_cpu_space_parameter((params > 5) ? param[5] : nullptr, ref, space))
		return;

	/* further validation */
	if (width == 0)
		width = space->data_width() / 8;
	if (width < space->address_to_byte(1))
		width = space->address_to_byte(1);
	if (width != 1 && width != 2 && width != 4 && width != 8)
	{
		m_console.printf("Invalid width! (must be 1,2,4 or 8)\n");
		return;
	}
	UINT64 endoffset = space->address_to_byte(offset + length - 1) & space->bytemask();
	offset = space->address_to_byte(offset) & space->bytemask();

	/* open the file */
	FILE* f = fopen(param[0], "w");
	if (!f)
	{
		m_console.printf("Error opening file '%s'\n", param[0]);
		return;
	}

	/* now write the data out */
	util::ovectorstream output;
	output.reserve(200);
	for (UINT64 i = offset; i <= endoffset; i += 16)
	{
		output.clear();
		output.rdbuf()->clear();

		/* print the address */
		util::stream_format(output, "%0*X: ", space->logaddrchars(), (UINT32)space->byte_to_address(i));

		/* print the bytes */
		for (UINT64 j = 0; j < 16; j += width)
		{
			if (i + j <= endoffset)
			{
				offs_t curaddr = i + j;
				if (m_cpu.translate(*space, TRANSLATE_READ_DEBUG, &curaddr))
				{
					UINT64 value = m_cpu.read_memory(*space, i + j, width, TRUE);
					util::stream_format(output, " %0*X", width * 2, value);
				}
				else
				{
					util::stream_format(output, " %.*s", width * 2, "****************");
				}
			}
			else
				util::stream_format(output, " %*s", width * 2, "");
		}

		/* print the ASCII */
		if (ascii)
		{
			util::stream_format(output, "  ");
			for (UINT64 j = 0; j < 16 && (i + j) <= endoffset; j++)
			{
				offs_t curaddr = i + j;
				if (m_cpu.translate(*space, TRANSLATE_READ_DEBUG, &curaddr))
				{
					UINT8 byte = m_cpu.read_byte(*space, i + j, TRUE);
					util::stream_format(output, "%c", (byte >= 32 && byte < 127) ? byte : '.');
				}
				else
				{
					util::stream_format(output, " ");
				}
			}
		}

		/* output the result */
		auto const &text = output.vec();
		fprintf(f, "%.*s\n", int(unsigned(text.size())), &text[0]);
	}

	/* close the file */
	fclose(f);
	m_console.printf("Data dumped successfully\n");
}


/*-------------------------------------------------
   execute_cheatinit - initialize the cheat system
-------------------------------------------------*/

void debugger_commands::execute_cheatinit(int ref, int params, const char *param[])
{
	UINT64 offset, length = 0, real_length = 0;
	address_space *space;
	UINT32 active_cheat = 0;
	UINT64 curaddr;
	UINT8 i, region_count = 0;

	cheat_region_map cheat_region[100];

	memset(cheat_region, 0, sizeof(cheat_region));

	/* validate parameters */
	if (!validate_cpu_space_parameter((params > 3) ? param[3] : nullptr, AS_PROGRAM, space))
		return;

	if (ref == 0)
	{
		m_cheat.width = 1;
		m_cheat.signed_cheat = FALSE;
		m_cheat.swapped_cheat = FALSE;
		if (params > 0)
		{
			char *srtpnt = (char*)param[0];

			if (*srtpnt == 's')
				m_cheat.signed_cheat = TRUE;
			else if (*srtpnt == 'u')
				m_cheat.signed_cheat = FALSE;
			else
			{
				m_console.printf("Invalid sign: expected s or u\n");
				return;
			}

			if (*(++srtpnt) == 'b')
				m_cheat.width = 1;
			else if (*srtpnt == 'w')
				m_cheat.width = 2;
			else if (*srtpnt == 'd')
				m_cheat.width = 4;
			else if (*srtpnt == 'q')
				m_cheat.width = 8;
			else
			{
				m_console.printf("Invalid width: expected b, w, d or q\n");
				return;
			}

			if (*(++srtpnt) == 's')
				m_cheat.swapped_cheat = TRUE;
			else
				m_cheat.swapped_cheat = FALSE;
		}
	}

	/* initialize entire memory by default */
	if (params <= 1)
	{
		for (address_map_entry &entry : space->map()->m_entrylist)
		{
			cheat_region[region_count].offset = space->address_to_byte(entry.m_addrstart) & space->bytemask();
			cheat_region[region_count].endoffset = space->address_to_byte(entry.m_addrend) & space->bytemask();
			cheat_region[region_count].share = entry.m_share;
			cheat_region[region_count].disabled = (entry.m_write.m_type == AMH_RAM) ? FALSE : TRUE;

			/* disable double share regions */
			if (entry.m_share != nullptr)
				for (i = 0; i < region_count; i++)
					if (cheat_region[i].share != nullptr)
						if (strcmp(cheat_region[i].share, entry.m_share) == 0)
							cheat_region[region_count].disabled = TRUE;

			region_count++;
		}
	}
	else
	{
		/* validate parameters */
		if (!validate_number_parameter(param[(ref == 0) ? 1 : 0], &offset))
			return;
		if (!validate_number_parameter(param[(ref == 0) ? 2 : 1], &length))
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
			for (curaddr = cheat_region[i].offset; curaddr <= cheat_region[i].endoffset; curaddr += m_cheat.width)
				if (cheat_address_is_valid(*space, curaddr))
					real_length++;

	if (real_length == 0)
	{
		m_console.printf("No writable bytes found in this area\n");
		return;
	}

	if (ref == 0)
	{
		/* initialize new cheat system */
		m_cheat.cheatmap.resize(real_length);
		m_cheat.undo = 0;
		m_cheat.cpu[0] = (params > 3) ? *param[3] : '0';
	}
	else
	{
		/* add range to cheat system */
		if (m_cheat.cpu[0] == 0)
		{
			m_console.printf("Use cheatinit before cheatrange\n");
			return;
		}

		if (!validate_cpu_space_parameter(m_cheat.cpu, AS_PROGRAM, space))
			return;

		active_cheat = m_cheat.cheatmap.size();
		m_cheat.cheatmap.resize(m_cheat.cheatmap.size() + real_length);
	}

	/* initialize cheatmap in the selected space */
	for (i = 0; i < region_count; i++)
		if (!cheat_region[i].disabled)
			for (curaddr = cheat_region[i].offset; curaddr <= cheat_region[i].endoffset; curaddr += m_cheat.width)
				if (cheat_address_is_valid(*space, curaddr))
				{
					m_cheat.cheatmap[active_cheat].previous_value = cheat_read_extended(&m_cheat, *space, curaddr);
					m_cheat.cheatmap[active_cheat].first_value = m_cheat.cheatmap[active_cheat].previous_value;
					m_cheat.cheatmap[active_cheat].offset = curaddr;
					m_cheat.cheatmap[active_cheat].state = 1;
					m_cheat.cheatmap[active_cheat].undo = 0;
					active_cheat++;
				}

	/* give a detailed init message to avoid searches being mistakingly carried out on the wrong CPU */
	device_t *cpu = nullptr;
	validate_cpu_parameter(m_cheat.cpu, &cpu);
	m_console.printf("%u cheat initialized for CPU index %s ( aka %s )\n", active_cheat, m_cheat.cpu, cpu->tag());
}


/*-------------------------------------------------
    execute_cheatnext - execute the search
-------------------------------------------------*/

void debugger_commands::execute_cheatnext(int ref, int params, const char *param[])
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

	if (m_cheat.cpu[0] == 0)
	{
		m_console.printf("Use cheatinit before cheatnext\n");
		return;
	}

	if (!validate_cpu_space_parameter(m_cheat.cpu, AS_PROGRAM, space))
		return;

	if (params > 1 && !validate_number_parameter(param[1], &comp_value))
		return;
	comp_value = cheat_sign_extend(&m_cheat, comp_value);

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
		m_console.printf("Invalid condition type\n");
		return;
	}

	m_cheat.undo++;

	/* execute the search */
	for (cheatindex = 0; cheatindex < m_cheat.cheatmap.size(); cheatindex += 1)
		if (m_cheat.cheatmap[cheatindex].state == 1)
		{
			UINT64 cheat_value = cheat_read_extended(&m_cheat, *space, m_cheat.cheatmap[cheatindex].offset);
			UINT64 comp_byte = (ref == 0) ? m_cheat.cheatmap[cheatindex].previous_value : m_cheat.cheatmap[cheatindex].first_value;
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
					if (m_cheat.signed_cheat)
						disable_byte = ((INT64)cheat_value >= (INT64)comp_byte);
					else
						disable_byte = ((UINT64)cheat_value >= (UINT64)comp_byte);
					break;

				case CHEAT_INCREASE:
					if (m_cheat.signed_cheat)
						disable_byte = ((INT64)cheat_value <= (INT64)comp_byte);
					else
						disable_byte = ((UINT64)cheat_value <= (UINT64)comp_byte);
					break;

				case CHEAT_DECREASE_OR_EQUAL:
					if (m_cheat.signed_cheat)
						disable_byte = ((INT64)cheat_value > (INT64)comp_byte);
					else
						disable_byte = ((UINT64)cheat_value > (UINT64)comp_byte);
					break;

				case CHEAT_INCREASE_OR_EQUAL:
					if (m_cheat.signed_cheat)
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
					if (m_cheat.signed_cheat)
						disable_byte = ((INT64)cheat_value >= (INT64)comp_value);
					else
						disable_byte = ((UINT64)cheat_value >= (UINT64)comp_value);
					break;

				case CHEAT_GREATEROF:
					if (m_cheat.signed_cheat)
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
				m_cheat.cheatmap[cheatindex].state = 0;
				m_cheat.cheatmap[cheatindex].undo = m_cheat.undo;
			}
			else
				active_cheat++;

			/* update previous value */
			m_cheat.cheatmap[cheatindex].previous_value = cheat_value;
		}

	if (active_cheat <= 5)
		execute_cheatlist(0, 0, nullptr);

	m_console.printf("%u cheats found\n", active_cheat);
}


/*-------------------------------------------------
    execute_cheatlist - show a list of active cheat
-------------------------------------------------*/

void debugger_commands::execute_cheatlist(int ref, int params, const char *param[])
{
	char spaceletter, sizeletter;
	address_space *space;
	device_t *cpu;
	UINT32 active_cheat = 0;
	UINT64 cheatindex;
	UINT64 sizemask;
	FILE *f = nullptr;

	if (!validate_cpu_space_parameter(m_cheat.cpu, AS_PROGRAM, space))
		return;

	if (!validate_cpu_parameter(m_cheat.cpu, &cpu))
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

	switch (m_cheat.width)
	{
		default:
		case 1:                     sizeletter = 'b';   sizemask = 0xff;                    break;
		case 2:                     sizeletter = 'w';   sizemask = 0xffff;                  break;
		case 4:                     sizeletter = 'd';   sizemask = 0xffffffff;              break;
		case 8:                     sizeletter = 'q';   sizemask = U64(0xffffffffffffffff); break;
	}

	/* write the cheat list */
	util::ovectorstream output;
	for (cheatindex = 0; cheatindex < m_cheat.cheatmap.size(); cheatindex += 1)
	{
		if (m_cheat.cheatmap[cheatindex].state == 1)
		{
			UINT64 value = cheat_byte_swap(&m_cheat, cheat_read_extended(&m_cheat, *space, m_cheat.cheatmap[cheatindex].offset)) & sizemask;
			offs_t address = space->byte_to_address(m_cheat.cheatmap[cheatindex].offset);

			if (params > 0)
			{
				active_cheat++;
				output.clear();
				output.rdbuf()->clear();
				stream_format(
						output,
						"  <cheat desc=\"Possibility %d : %0*X (%0*X)\">\n"
						"    <script state=\"run\">\n"
						"      <action>%s.p%c%c@%0*X=%0*X</action>\n"
						"    </script>\n"
						"  </cheat>\n\n",
						active_cheat, space->logaddrchars(), address, m_cheat.width * 2, value,
						cpu->tag(), spaceletter, sizeletter, space->logaddrchars(), address, m_cheat.width * 2, cheat_byte_swap(&m_cheat, m_cheat.cheatmap[cheatindex].first_value) & sizemask);
				auto const &text(output.vec());
				fprintf(f, "%.*s", int(unsigned(text.size())), &text[0]);
			}
			else
			{
				m_console.printf(
						"Address=%0*X Start=%0*X Current=%0*X\n",
						space->logaddrchars(), address,
						m_cheat.width * 2, cheat_byte_swap(&m_cheat, m_cheat.cheatmap[cheatindex].first_value) & sizemask,
						m_cheat.width * 2, value);
			}
		}
	}
	if (params > 0)
		fclose(f);
}


/*-------------------------------------------------
    execute_cheatundo - undo the last search
-------------------------------------------------*/

void debugger_commands::execute_cheatundo(int ref, int params, const char *param[])
{
	UINT64 cheatindex;
	UINT32 undo_count = 0;

	if (m_cheat.undo > 0)
	{
		for (cheatindex = 0; cheatindex < m_cheat.cheatmap.size(); cheatindex += 1)
		{
			if (m_cheat.cheatmap[cheatindex].undo == m_cheat.undo)
			{
				m_cheat.cheatmap[cheatindex].state = 1;
				m_cheat.cheatmap[cheatindex].undo = 0;
				undo_count++;
			}
		}

		m_cheat.undo--;
		m_console.printf("%u cheat reactivated\n", undo_count);
	}
	else
		m_console.printf("Maximum undo reached\n");
}


/*-------------------------------------------------
    execute_find - execute the find command
-------------------------------------------------*/

void debugger_commands::execute_find(int ref, int params, const char *param[])
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
	if (!validate_number_parameter(param[0], &offset))
		return;
	if (!validate_number_parameter(param[1], &length))
		return;
	if (!validate_cpu_space_parameter(nullptr, ref, space))
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
		size_t pdatalen = strlen(pdata) - 1;

		/* check for a string */
		if (pdata[0] == '"' && pdata[pdatalen] == '"')
		{
			for (j = 1; j < pdatalen; j++)
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
			else if (!validate_number_parameter(pdata, &data_to_find[data_count++]))
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
				case 1: match = ((UINT8)m_cpu.read_byte(*space, i + suboffset, TRUE) == (UINT8)data_to_find[j]);    break;
				case 2: match = ((UINT16)m_cpu.read_word(*space, i + suboffset, TRUE) == (UINT16)data_to_find[j]);  break;
				case 4: match = ((UINT32)m_cpu.read_dword(*space, i + suboffset, TRUE) == (UINT32)data_to_find[j]); break;
				case 8: match = ((UINT64)m_cpu.read_qword(*space, i + suboffset, TRUE) == (UINT64)data_to_find[j]); break;
				default:    /* all other cases are wildcards */     break;
			}
			suboffset += data_size[j] & 0x0f;
		}

		/* did we find it? */
		if (match)
		{
			found++;
			m_console.printf("Found at %0*X\n", space->addrchars(), (UINT32)space->byte_to_address(i));
		}
	}

	/* print something if not found */
	if (found == 0)
		m_console.printf("Not found\n");
}


/*-------------------------------------------------
    execute_dasm - execute the dasm command
-------------------------------------------------*/

void debugger_commands::execute_dasm(int ref, int params, const char *param[])
{
	UINT64 offset, length, bytes = 1;
	int minbytes, maxbytes, byteswidth;
	address_space *space, *decrypted_space;
	FILE *f;
	int j;

	/* validate parameters */
	if (!validate_number_parameter(param[1], &offset))
		return;
	if (!validate_number_parameter(param[2], &length))
		return;
	if (!validate_number_parameter(param[3], &bytes))
		return;
	if (!validate_cpu_space_parameter((params > 4) ? param[4] : nullptr, AS_PROGRAM, space))
		return;
	if (space->device().memory().has_space(AS_DECRYPTED_OPCODES))
		decrypted_space = &space->device().memory().space(AS_DECRYPTED_OPCODES);
	else
		decrypted_space = space;

	/* determine the width of the bytes */
	device_disasm_interface *dasmintf;
	if (!space->device().interface(dasmintf))
	{
		m_console.printf("No disassembler available for %s\n", space->device().name());
		return;
	}
	minbytes = dasmintf->min_opcode_bytes();
	maxbytes = dasmintf->max_opcode_bytes();
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
		m_console.printf("Error opening file '%s'\n", param[0]);
		return;
	}

	/* now write the data out */
	util::ovectorstream output;
	output.reserve(512);
	for (UINT64 i = 0; i < length; )
	{
		int pcbyte = space->address_to_byte(offset + i) & space->bytemask();
		char disasm[200];
		const char *comment;
		offs_t tempaddr;
		int numbytes = 0;
		output.clear();
		output.rdbuf()->clear();

		/* print the address */
		stream_format(output, "%0*X: ", space->logaddrchars(), (UINT32)space->byte_to_address(pcbyte));

		/* make sure we can translate the address */
		tempaddr = pcbyte;
		if (m_cpu.translate(*space, TRANSLATE_FETCH_DEBUG, &tempaddr))
		{
			UINT8 opbuf[64], argbuf[64];

			/* fetch the bytes up to the maximum */
			for (numbytes = 0; numbytes < maxbytes; numbytes++)
			{
				opbuf[numbytes] = m_cpu.read_opcode(*decrypted_space, pcbyte + numbytes, 1);
				argbuf[numbytes] = m_cpu.read_opcode(*space, pcbyte + numbytes, 1);
			}

			/* disassemble the result */
			i += numbytes = space->device().debug()->disassemble(disasm, offset + i, opbuf, argbuf) & DASMFLAG_LENGTHMASK;
		}

		/* print the bytes */
		if (bytes)
		{
			auto const startdex = output.tellp();
			numbytes = space->address_to_byte(numbytes);
			for (j = 0; j < numbytes; j += minbytes)
				stream_format(output, "%0*X ", minbytes * 2, m_cpu.read_opcode(*decrypted_space, pcbyte + j, minbytes));
			if ((output.tellp() - startdex) < byteswidth)
				stream_format(output, "%*s", byteswidth - (output.tellp() - startdex), "");
			stream_format(output, "  ");
		}

		/* add the disassembly */
		stream_format(output, "%s", disasm);

		/* attempt to add the comment */
		comment = space->device().debug()->comment_text(tempaddr);
		if (comment != nullptr)
		{
			/* somewhat arbitrary guess as to how long most disassembly lines will be [column 60] */
			if (output.tellp() < 60)
			{
				/* pad the comment space out to 60 characters and null-terminate */
				while (output.tellp() < 60) output.put(' ');

				stream_format(output, "// %s", comment);
			}
			else
				stream_format(output, "\t// %s", comment);
		}

		/* output the result */
		auto const &text(output.vec());
		fprintf(f, "%.*s\n", int(unsigned(text.size())), &text[0]);
	}

	/* close the file */
	fclose(f);
	m_console.printf("Data dumped successfully\n");
}


/*-------------------------------------------------
    execute_trace_internal - functionality for
    trace over and trace info
-------------------------------------------------*/

void debugger_commands::execute_trace_internal(int ref, int params, const char *param[], bool trace_over)
{
	const char *action = nullptr;
	device_t *cpu;
	FILE *f = nullptr;
	const char *mode;
	std::string filename = param[0];

	/* replace macros */
	strreplace(filename, "{game}", m_machine.basename());

	/* validate parameters */
	if (!validate_cpu_parameter((params > 1) ? param[1] : nullptr, &cpu))
		return;
	if (!debug_command_parameter_command(action = param[2]))
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
			m_console.printf("Error opening file '%s'\n", param[0]);
			return;
		}
	}

	/* do it */
	cpu->debug()->trace(f, trace_over, action);
	if (f)
		m_console.printf("Tracing CPU '%s' to file %s\n", cpu->tag(), filename.c_str());
	else
		m_console.printf("Stopped tracing on CPU '%s'\n", cpu->tag());
}


/*-------------------------------------------------
    execute_trace - execute the trace command
-------------------------------------------------*/

void debugger_commands::execute_trace(int ref, int params, const char *param[])
{
	execute_trace_internal(ref, params, param, false);
}


/*-------------------------------------------------
    execute_traceover - execute the trace over command
-------------------------------------------------*/

void debugger_commands::execute_traceover(int ref, int params, const char *param[])
{
	execute_trace_internal(ref, params, param, true);
}


/*-------------------------------------------------
    execute_traceflush - execute the trace flush command
-------------------------------------------------*/

void debugger_commands::execute_traceflush(int ref, int params, const char *param[])
{
	m_cpu.flush_traces();
}


/*-------------------------------------------------
    execute_history - execute the history command
-------------------------------------------------*/

void debugger_commands::execute_history(int ref, int params, const char *param[])
{
	/* validate parameters */
	address_space *space, *decrypted_space;
	if (!validate_cpu_space_parameter((params > 0) ? param[0] : nullptr, AS_PROGRAM, space))
		return;
	if (space->device().memory().has_space(AS_DECRYPTED_OPCODES))
		decrypted_space = &space->device().memory().space(AS_DECRYPTED_OPCODES);
	else
		decrypted_space = space;

	UINT64 count = device_debug::HISTORY_SIZE;
	if (!validate_number_parameter(param[1], &count))
		return;

	/* further validation */
	if (count > device_debug::HISTORY_SIZE)
		count = device_debug::HISTORY_SIZE;

	device_debug *debug = space->device().debug();

	/* loop over lines */
	device_disasm_interface *dasmintf;
	int maxbytes = space->device().interface(dasmintf) ? dasmintf->max_opcode_bytes() : 1;
	for (int index = 0; index < (int) count; index++)
	{
		offs_t pc = debug->history_pc(-index);

		/* fetch the bytes up to the maximum */
		offs_t pcbyte = space->address_to_byte(pc) & space->bytemask();
		UINT8 opbuf[64], argbuf[64];
		for (int numbytes = 0; numbytes < maxbytes; numbytes++)
		{
			opbuf[numbytes] = m_cpu.read_opcode(*decrypted_space, pcbyte + numbytes, 1);
			argbuf[numbytes] = m_cpu.read_opcode(*space, pcbyte + numbytes, 1);
		}

		char buffer[200];
		debug->disassemble(buffer, pc, opbuf, argbuf);

		m_console.printf("%0*X: %s\n", space->logaddrchars(), pc, buffer);
	}
}


/*-------------------------------------------------
    execute_trackpc - execute the trackpc command
-------------------------------------------------*/

void debugger_commands::execute_trackpc(int ref, int params, const char *param[])
{
	// Gather the on/off switch (if present)
	UINT64 turnOn = true;
	if (!validate_number_parameter(param[0], &turnOn))
		return;

	// Gather the cpu id (if present)
	device_t *cpu = nullptr;
	if (!validate_cpu_parameter((params > 1) ? param[1] : nullptr, &cpu))
		return;

	// Should we clear the existing data?
	UINT64 clear = false;
	if (!validate_number_parameter(param[2], &clear))
		return;

	cpu->debug()->set_track_pc((bool)turnOn);
	if (turnOn)
	{
		// Insert current pc
		if (m_cpu.get_visible_cpu() == cpu)
		{
			const offs_t pc = cpu->safe_pc();
			cpu->debug()->set_track_pc_visited(pc);
		}
		m_console.printf("PC tracking enabled\n");
	}
	else
	{
		m_console.printf("PC tracking disabled\n");
	}

	if (clear)
		cpu->debug()->track_pc_data_clear();
}


/*-------------------------------------------------
    execute_trackmem - execute the trackmem command
-------------------------------------------------*/

void debugger_commands::execute_trackmem(int ref, int params, const char *param[])
{
	// Gather the on/off switch (if present)
	UINT64 turnOn = true;
	if (!validate_number_parameter(param[0], &turnOn))
		return;

	// Gather the cpu id (if present)
	device_t *cpu = nullptr;
	if (!validate_cpu_parameter((params > 1) ? param[1] : nullptr, &cpu))
		return;

	// Should we clear the existing data?
	UINT64 clear = false;
	if (!validate_number_parameter(param[2], &clear))
		return;

	// Get the address space for the given cpu
	address_space *space;
	if (!validate_cpu_space_parameter((params > 1) ? param[1] : nullptr, AS_PROGRAM, space))
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

void debugger_commands::execute_pcatmem(int ref, int params, const char *param[])
{
	// Gather the required address parameter
	UINT64 address;
	if (!validate_number_parameter(param[0], &address))
		return;

	// Gather the cpu id (if present)
	device_t *cpu = nullptr;
	if (!validate_cpu_parameter((params > 1) ? param[1] : nullptr, &cpu))
		return;

	// Get the address space for the given cpu
	address_space *space;
	if (!validate_cpu_space_parameter((params > 1) ? param[1] : nullptr, ref, space))
		return;

	// Get the value of memory at the address
	const int native_data_width = space->data_width() / 8;
	const UINT64 data = m_cpu.read_memory(*space, space->address_to_byte(address), native_data_width, true);

	// Recover the pc & print
	const address_spacenum space_num = (address_spacenum)ref;
	const offs_t result = space->device().debug()->track_mem_pc_from_space_address_data(space_num, address, data);
	if (result != (offs_t)(-1))
		m_console.printf("%02x\n", result);
	else
		m_console.printf("UNKNOWN PC\n");
}


/*-------------------------------------------------
    execute_snap - execute the snapshot command
-------------------------------------------------*/

void debugger_commands::execute_snap(int ref, int params, const char *param[])
{
	/* if no params, use the default behavior */
	if (params == 0)
	{
		m_machine.video().save_active_screen_snapshots();
		m_console.printf("Saved snapshot\n");
	}

	/* otherwise, we have to open the file ourselves */
	else
	{
		const char *filename = param[0];
		int scrnum = (params > 1) ? atoi(param[1]) : 0;

		screen_device_iterator iter(m_machine.root_device());
		screen_device *screen = iter.byindex(scrnum);

		if ((screen == nullptr) || !m_machine.render().is_live(*screen))
		{
			m_console.printf("Invalid screen number '%d'\n", scrnum);
			return;
		}

		std::string fname(filename);
		if (fname.find(".png") == -1)
			fname.append(".png");
		emu_file file(m_machine.options().snapshot_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
		osd_file::error filerr = file.open(fname.c_str());

		if (filerr != osd_file::error::NONE)
		{
			m_console.printf("Error creating file '%s'\n", filename);
			return;
		}

		screen->machine().video().save_snapshot(screen, file);
		m_console.printf("Saved screen #%d snapshot as '%s'\n", scrnum, filename);
	}
}


/*-------------------------------------------------
    execute_source - execute the source command
-------------------------------------------------*/

void debugger_commands::execute_source(int ref, int params, const char *param[])
{
	m_cpu.source_script(param[0]);
}


/*-------------------------------------------------
    execute_map - execute the map command
-------------------------------------------------*/

void debugger_commands::execute_map(int ref, int params, const char *param[])
{
	address_space *space;
	offs_t taddress;
	UINT64 address;
	int intention;

	/* validate parameters */
	if (!validate_number_parameter(param[0], &address))
		return;

	/* CPU is implicit */
	if (!validate_cpu_space_parameter(nullptr, ref, space))
		return;

	/* do the translation first */
	for (intention = TRANSLATE_READ_DEBUG; intention <= TRANSLATE_FETCH_DEBUG; intention++)
	{
		static const char *const intnames[] = { "Read", "Write", "Fetch" };
		taddress = space->address_to_byte(address) & space->bytemask();
		if (m_cpu.translate(*space, intention, &taddress))
		{
			const char *mapname = space->get_handler_string((intention == TRANSLATE_WRITE_DEBUG) ? ROW_WRITE : ROW_READ, taddress);
			m_console.printf(
					"%7s: %0*X logical == %0*X physical -> %s\n",
					intnames[intention & 3],
					space->logaddrchars(), address,
					space->addrchars(), space->byte_to_address(taddress),
					mapname);
		}
		else
			m_console.printf("%7s: %0*X logical is unmapped\n", intnames[intention & 3], space->logaddrchars(), address);
	}
}


/*-------------------------------------------------
    execute_memdump - execute the memdump command
-------------------------------------------------*/

void debugger_commands::execute_memdump(int ref, int params, const char **param)
{
	FILE *file;
	const char *filename;

	filename = (params == 0) ? "memdump.log" : param[0];

	m_console.printf("Dumping memory to %s\n", filename);

	file = fopen(filename, "w");
	if (file)
	{
		m_machine.memory().dump(file);
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

void debugger_commands::execute_symlist(int ref, int params, const char **param)
{
	device_t *cpu = nullptr;
	const char *namelist[1000];
	symbol_table *symtable;
	int symnum, count = 0;


	if (param[0] != nullptr)
	{
		/* validate parameters */
		if (!validate_cpu_parameter(param[0], &cpu))
			return;
		symtable = &cpu->debug()->symtable();
		m_console.printf("CPU '%s' symbols:\n", cpu->tag());
	}
	else
	{
		symtable = m_cpu.get_global_symtable();
		m_console.printf("Global symbols:\n");
	}

	/* gather names for all symbols */
	for (auto &entry : symtable->entries())
	{
		/* only display "register" type symbols */
		if (!entry.second->is_function())
		{
			namelist[count++] = entry.second->name();
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
		m_console.printf("%s = %X", namelist[symnum], value);
		if (!entry->is_lval())
			m_console.printf("  (read-only)");
		m_console.printf("\n");
	}
}


/*-------------------------------------------------
    execute_softreset - execute the softreset command
-------------------------------------------------*/

void debugger_commands::execute_softreset(int ref, int params, const char **param)
{
	m_machine.schedule_soft_reset();
}


/*-------------------------------------------------
    execute_hardreset - execute the hardreset command
-------------------------------------------------*/

void debugger_commands::execute_hardreset(int ref, int params, const char **param)
{
	m_machine.schedule_hard_reset();
}

/*-------------------------------------------------
    execute_images - lists all image devices with
    mounted files
-------------------------------------------------*/

void debugger_commands::execute_images(int ref, int params, const char **param)
{
	image_interface_iterator iter(m_machine.root_device());
	for (device_image_interface &img : iter)
		m_console.printf("%s: %s\n", img.brief_instance_name(), img.exists() ? img.filename() : "[empty slot]");
	if (iter.first() == nullptr)
		m_console.printf("No image devices in this driver\n");
}

/*-------------------------------------------------
    execute_mount - execute the image mount command
-------------------------------------------------*/

void debugger_commands::execute_mount(int ref, int params, const char **param)
{
	bool done = false;
	for (device_image_interface &img : image_interface_iterator(m_machine.root_device()))
	{
		if (strcmp(img.brief_instance_name(),param[0]) == 0)
		{
			if (img.load(param[1])==IMAGE_INIT_FAIL)
				m_console.printf("Unable to mount file %s on %s\n",param[1],param[0]);
			else
				m_console.printf("File %s mounted on %s\n",param[1],param[0]);
			done = true;
			break;
		}
	}
	if (!done)
		m_console.printf("There is no image device :%s\n",param[0]);
}

/*-------------------------------------------------
    execute_unmount - execute the image unmount command
-------------------------------------------------*/

void debugger_commands::execute_unmount(int ref, int params, const char **param)
{
	bool done = false;
	for (device_image_interface &img : image_interface_iterator(m_machine.root_device()))
	{
		if (strcmp(img.brief_instance_name(),param[0]) == 0)
		{
			img.unload();
			m_console.printf("Unmounted file from : %s\n",param[0]);
			done = true;
			break;
		}
	}
	if (!done)
		m_console.printf("There is no image device :%s\n",param[0]);
}


/*-------------------------------------------------
    execute_input - debugger command to enter
    natural keyboard input
-------------------------------------------------*/

void debugger_commands::execute_input(int ref, int params, const char **param)
{
	m_machine.ioport().natkeyboard().post_coded(param[0]);
}


/*-------------------------------------------------
    execute_dumpkbd - debugger command to natural
    keyboard codes
-------------------------------------------------*/

void debugger_commands::execute_dumpkbd(int ref, int params, const char **param)
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
			m_console.printf("Cannot open \"%s\"\n", filename);
			return;
		}
	}

	// loop through all codes
	std::string buffer = m_machine.ioport().natkeyboard().dump();

	// and output it as appropriate
	if (file != nullptr)
		fprintf(file, "%s\n", buffer.c_str());
	else
		m_console.printf("%s\n", buffer.c_str());

	// cleanup
	if (file != nullptr)
		fclose(file);
}
