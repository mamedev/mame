// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    debugcmd.cpp

    Debugger command interface engine.

*********************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "debugger.h"
#include "debugcmd.h"
#include "debugcon.h"
#include "debugcpu.h"
#include "debugbuf.h"
#include "express.h"
#include "debughlp.h"
#include "debugvw.h"
#include "points.h"
#include "natkeyboard.h"
#include "render.h"
#include "corestr.h"
#include <cctype>
#include <algorithm>
#include <fstream>



/***************************************************************************
    CONSTANTS
***************************************************************************/

const size_t debugger_commands::MAX_GLOBALS = 1000;

/***************************************************************************
    FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    cheat_address_is_valid - return true if the
    given address is valid for cheating
-------------------------------------------------*/

bool debugger_commands::cheat_address_is_valid(address_space &space, offs_t address)
{
	return space.device().memory().translate(space.spacenum(), TRANSLATE_READ, address) && (space.get_write_ptr(address) != nullptr);
}


/*-------------------------------------------------
    cheat_sign_extend - sign-extend a value to
    the current cheat width, if signed
-------------------------------------------------*/

u64 debugger_commands::cheat_sign_extend(const cheat_system *cheatsys, u64 value)
{
	if (cheatsys->signed_cheat)
	{
		switch (cheatsys->width)
		{
		case 1: value = s8(value);  break;
		case 2: value = s16(value); break;
		case 4: value = s32(value); break;
		}
	}
	return value;
}

/*-------------------------------------------------
    cheat_byte_swap - swap a value
-------------------------------------------------*/

u64 debugger_commands::cheat_byte_swap(const cheat_system *cheatsys, u64 value)
{
	if (cheatsys->swapped_cheat)
	{
		switch (cheatsys->width)
		{
		case 2: value = swapendian_int16(value);    break;
		case 4: value = swapendian_int32(value);    break;
		case 8: value = swapendian_int64(value);    break;
		}
	}
	return value;
}

/*-------------------------------------------------
    cheat_read_extended - read a value from memory
    in the given address space, sign-extending
    and swapping if necessary
-------------------------------------------------*/

u64 debugger_commands::cheat_read_extended(const cheat_system *cheatsys, address_space &space, offs_t address)
{
	address &= space.logaddrmask();
	u64 value = space.unmap();
	if (space.device().memory().translate(space.spacenum(), TRANSLATE_READ_DEBUG, address))
	{
		switch (cheatsys->width)
		{
		case 1: value = space.read_byte(address);               break;
		case 2: value = space.read_word_unaligned(address);     break;
		case 4: value = space.read_dword_unaligned(address);    break;
		case 8: value = space.read_qword_unaligned(address);    break;
		}
	}
	return cheat_sign_extend(cheatsys, cheat_byte_swap(cheatsys, value));
}

debugger_commands::debugger_commands(running_machine& machine, debugger_cpu& cpu, debugger_console& console)
	: m_machine(machine)
	, m_console(console)
{
	m_global_array = std::make_unique<global_entry []>(MAX_GLOBALS);

	symbol_table &symtable = cpu.global_symtable();

	/* add a few simple global functions */
	using namespace std::placeholders;
	symtable.add("min", 2, 2, std::bind(&debugger_commands::execute_min, this, _1, _2));
	symtable.add("max", 2, 2, std::bind(&debugger_commands::execute_max, this, _1, _2));
	symtable.add("if", 3, 3, std::bind(&debugger_commands::execute_if, this, _1, _2));
	symtable.add("abs", 1, 1, std::bind(&debugger_commands::execute_abs, this, _1, _2));
	symtable.add("bit", 2, 3, std::bind(&debugger_commands::execute_bit, this, _1, _2));
	symtable.add("s8", 1, 1, std::bind(&debugger_commands::execute_s8, this, _1, _2));
	symtable.add("s16", 1, 1, std::bind(&debugger_commands::execute_s16, this, _1, _2));
	symtable.add("s32", 1, 1, std::bind(&debugger_commands::execute_s32, this, _1, _2));
	symtable.add("cpunum", std::bind(&debugger_commands::get_cpunum, this));

	/* add all single-entry save state globals */
	for (int itemnum = 0; itemnum < MAX_GLOBALS; itemnum++)
	{
		void *base;
		u32 valsize, valcount, blockcount, stride;

		/* stop when we run out of items */
		const char* name = m_machine.save().indexed_item(itemnum, base, valsize, valcount, blockcount, stride);
		if (!name)
			break;

		/* if this is a single-entry global, add it */
		if ((valcount == 1) && (blockcount == 1) && strstr(name, "/globals/"))
		{
			char symname[100];
			sprintf(symname, ".%s", strrchr(name, '/') + 1);
			m_global_array[itemnum].base = base;
			m_global_array[itemnum].size = valsize;
			symtable.add(
					symname,
					std::bind(&debugger_commands::global_get, this, &m_global_array[itemnum]),
					std::bind(&debugger_commands::global_set, this, &m_global_array[itemnum], _1));
		}
	}

	/* add all the commands */
	m_console.register_command("help",      CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_help, this, _1, _2));
	m_console.register_command("print",     CMDFLAG_NONE, 0, 1, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_print, this, _1, _2));
	m_console.register_command("printf",    CMDFLAG_NONE, 0, 1, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_printf, this, _1, _2));
	m_console.register_command("logerror",  CMDFLAG_NONE, 0, 1, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_logerror, this, _1, _2));
	m_console.register_command("tracelog",  CMDFLAG_NONE, 0, 1, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_tracelog, this, _1, _2));
	m_console.register_command("tracesym",  CMDFLAG_NONE, 0, 1, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_tracesym, this, _1, _2));
	m_console.register_command("cls",       CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_cls, this, _1, _2));
	m_console.register_command("quit",      CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_quit, this, _1, _2));
	m_console.register_command("exit",      CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_quit, this, _1, _2));
	m_console.register_command("do",        CMDFLAG_NONE, 0, 1, 1, std::bind(&debugger_commands::execute_do, this, _1, _2));
	m_console.register_command("step",      CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_step, this, _1, _2));
	m_console.register_command("s",         CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_step, this, _1, _2));
	m_console.register_command("over",      CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_over, this, _1, _2));
	m_console.register_command("o",         CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_over, this, _1, _2));
	m_console.register_command("out" ,      CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_out, this, _1, _2));
	m_console.register_command("go",        CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_go, this, _1, _2));
	m_console.register_command("g",         CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_go, this, _1, _2));
	m_console.register_command("gvblank",   CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_go_vblank, this, _1, _2));
	m_console.register_command("gv",        CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_go_vblank, this, _1, _2));
	m_console.register_command("gint",      CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_go_interrupt, this, _1, _2));
	m_console.register_command("gi",        CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_go_interrupt, this, _1, _2));
	m_console.register_command("gex",       CMDFLAG_NONE, 0, 0, 2, std::bind(&debugger_commands::execute_go_exception, this, _1, _2));
	m_console.register_command("ge",        CMDFLAG_NONE, 0, 0, 2, std::bind(&debugger_commands::execute_go_exception, this, _1, _2));
	m_console.register_command("gtime",     CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_go_time, this, _1, _2));
	m_console.register_command("gt",        CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_go_time, this, _1, _2));
	m_console.register_command("gp",        CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_go_privilege, this, _1, _2));
	m_console.register_command("next",      CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_next, this, _1, _2));
	m_console.register_command("n",         CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_next, this, _1, _2));
	m_console.register_command("focus",     CMDFLAG_NONE, 0, 1, 1, std::bind(&debugger_commands::execute_focus, this, _1, _2));
	m_console.register_command("ignore",    CMDFLAG_NONE, 0, 0, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_ignore, this, _1, _2));
	m_console.register_command("observe",   CMDFLAG_NONE, 0, 0, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_observe, this, _1, _2));
	m_console.register_command("suspend",   CMDFLAG_NONE, 0, 0, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_suspend, this, _1, _2));
	m_console.register_command("resume",    CMDFLAG_NONE, 0, 0, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_resume, this, _1, _2));
	m_console.register_command("cpulist",   CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_cpulist, this, _1, _2));

	m_console.register_command("comadd",    CMDFLAG_NONE, 0, 1, 2, std::bind(&debugger_commands::execute_comment_add, this, _1, _2));
	m_console.register_command("//",        CMDFLAG_NONE, 0, 1, 2, std::bind(&debugger_commands::execute_comment_add, this, _1, _2));
	m_console.register_command("comdelete", CMDFLAG_NONE, 0, 1, 1, std::bind(&debugger_commands::execute_comment_del, this, _1, _2));
	m_console.register_command("comsave",   CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_comment_save, this, _1, _2));
	m_console.register_command("comlist",   CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_comment_list, this, _1, _2));
	m_console.register_command("commit",    CMDFLAG_NONE, 0, 1, 2, std::bind(&debugger_commands::execute_comment_commit, this, _1, _2));
	m_console.register_command("/*",        CMDFLAG_NONE, 0, 1, 2, std::bind(&debugger_commands::execute_comment_commit, this, _1, _2));

	m_console.register_command("bpset",     CMDFLAG_NONE, 0, 1, 3, std::bind(&debugger_commands::execute_bpset, this, _1, _2));
	m_console.register_command("bp",        CMDFLAG_NONE, 0, 1, 3, std::bind(&debugger_commands::execute_bpset, this, _1, _2));
	m_console.register_command("bpclear",   CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_bpclear, this, _1, _2));
	m_console.register_command("bpdisable", CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_bpdisenable, this, _1, _2));
	m_console.register_command("bpenable",  CMDFLAG_NONE, 1, 0, 1, std::bind(&debugger_commands::execute_bpdisenable, this, _1, _2));
	m_console.register_command("bplist",    CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_bplist, this, _1, _2));

	m_console.register_command("wpset",     CMDFLAG_NONE, AS_PROGRAM, 3, 5, std::bind(&debugger_commands::execute_wpset, this, _1, _2));
	m_console.register_command("wp",        CMDFLAG_NONE, AS_PROGRAM, 3, 5, std::bind(&debugger_commands::execute_wpset, this, _1, _2));
	m_console.register_command("wpdset",    CMDFLAG_NONE, AS_DATA, 3, 5, std::bind(&debugger_commands::execute_wpset, this, _1, _2));
	m_console.register_command("wpd",       CMDFLAG_NONE, AS_DATA, 3, 5, std::bind(&debugger_commands::execute_wpset, this, _1, _2));
	m_console.register_command("wpiset",    CMDFLAG_NONE, AS_IO, 3, 5, std::bind(&debugger_commands::execute_wpset, this, _1, _2));
	m_console.register_command("wpi",       CMDFLAG_NONE, AS_IO, 3, 5, std::bind(&debugger_commands::execute_wpset, this, _1, _2));
	m_console.register_command("wposet",    CMDFLAG_NONE, AS_OPCODES, 3, 5, std::bind(&debugger_commands::execute_wpset, this, _1, _2));
	m_console.register_command("wpo",       CMDFLAG_NONE, AS_OPCODES, 3, 5, std::bind(&debugger_commands::execute_wpset, this, _1, _2));
	m_console.register_command("wpclear",   CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_wpclear, this, _1, _2));
	m_console.register_command("wpdisable", CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_wpdisenable, this, _1, _2));
	m_console.register_command("wpenable",  CMDFLAG_NONE, 1, 0, 1, std::bind(&debugger_commands::execute_wpdisenable, this, _1, _2));
	m_console.register_command("wplist",    CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_wplist, this, _1, _2));

	m_console.register_command("rpset",     CMDFLAG_NONE, 0, 1, 2, std::bind(&debugger_commands::execute_rpset, this, _1, _2));
	m_console.register_command("rp",        CMDFLAG_NONE, 0, 1, 2, std::bind(&debugger_commands::execute_rpset, this, _1, _2));
	m_console.register_command("rpclear",   CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_rpclear, this, _1, _2));
	m_console.register_command("rpdisable", CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_rpdisenable, this, _1, _2));
	m_console.register_command("rpenable",  CMDFLAG_NONE, 1, 0, 1, std::bind(&debugger_commands::execute_rpdisenable, this, _1, _2));
	m_console.register_command("rplist",    CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_rplist, this, _1, _2));

	m_console.register_command("statesave", CMDFLAG_NONE, 0, 1, 1, std::bind(&debugger_commands::execute_statesave, this, _1, _2));
	m_console.register_command("ss",        CMDFLAG_NONE, 0, 1, 1, std::bind(&debugger_commands::execute_statesave, this, _1, _2));
	m_console.register_command("stateload", CMDFLAG_NONE, 0, 1, 1, std::bind(&debugger_commands::execute_stateload, this, _1, _2));
	m_console.register_command("sl",        CMDFLAG_NONE, 0, 1, 1, std::bind(&debugger_commands::execute_stateload, this, _1, _2));

	m_console.register_command("rewind",    CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_rewind, this, _1, _2));
	m_console.register_command("rw",        CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_rewind, this, _1, _2));

	m_console.register_command("save",      CMDFLAG_NONE, AS_PROGRAM, 3, 4, std::bind(&debugger_commands::execute_save, this, _1, _2));
	m_console.register_command("saved",     CMDFLAG_NONE, AS_DATA, 3, 4, std::bind(&debugger_commands::execute_save, this, _1, _2));
	m_console.register_command("savei",     CMDFLAG_NONE, AS_IO, 3, 4, std::bind(&debugger_commands::execute_save, this, _1, _2));
	m_console.register_command("saveo",     CMDFLAG_NONE, AS_OPCODES, 3, 4, std::bind(&debugger_commands::execute_save, this, _1, _2));
	m_console.register_command("saver",     CMDFLAG_NONE, 0, 4, 4, std::bind(&debugger_commands::execute_saveregion, this, _1, _2));

	m_console.register_command("load",      CMDFLAG_NONE, AS_PROGRAM, 2, 4, std::bind(&debugger_commands::execute_load, this, _1, _2));
	m_console.register_command("loadd",     CMDFLAG_NONE, AS_DATA, 2, 4, std::bind(&debugger_commands::execute_load, this, _1, _2));
	m_console.register_command("loadi",     CMDFLAG_NONE, AS_IO, 2, 4, std::bind(&debugger_commands::execute_load, this, _1, _2));
	m_console.register_command("loado",     CMDFLAG_NONE, AS_OPCODES, 2, 4, std::bind(&debugger_commands::execute_load, this, _1, _2));
	m_console.register_command("loadr",     CMDFLAG_NONE, 0, 4, 4, std::bind(&debugger_commands::execute_loadregion, this, _1, _2));

	m_console.register_command("dump",      CMDFLAG_NONE, AS_PROGRAM, 3, 7, std::bind(&debugger_commands::execute_dump, this, _1, _2));
	m_console.register_command("dumpd",     CMDFLAG_NONE, AS_DATA, 3, 7, std::bind(&debugger_commands::execute_dump, this, _1, _2));
	m_console.register_command("dumpi",     CMDFLAG_NONE, AS_IO, 3, 7, std::bind(&debugger_commands::execute_dump, this, _1, _2));
	m_console.register_command("dumpo",     CMDFLAG_NONE, AS_OPCODES, 3, 7, std::bind(&debugger_commands::execute_dump, this, _1, _2));

	m_console.register_command("strdump",   CMDFLAG_NONE, AS_PROGRAM, 3, 5, std::bind(&debugger_commands::execute_strdump, this, _1, _2));
	m_console.register_command("strdumpd",  CMDFLAG_NONE, AS_DATA, 3, 5, std::bind(&debugger_commands::execute_strdump, this, _1, _2));
	m_console.register_command("strdumpi",  CMDFLAG_NONE, AS_IO, 3, 5, std::bind(&debugger_commands::execute_strdump, this, _1, _2));
	m_console.register_command("strdumpo",  CMDFLAG_NONE, AS_OPCODES, 3, 5, std::bind(&debugger_commands::execute_strdump, this, _1, _2));

	m_console.register_command("cheatinit", CMDFLAG_NONE, 0, 0, 4, std::bind(&debugger_commands::execute_cheatinit, this, _1, _2));
	m_console.register_command("ci",        CMDFLAG_NONE, 0, 0, 4, std::bind(&debugger_commands::execute_cheatinit, this, _1, _2));

	m_console.register_command("cheatrange",CMDFLAG_NONE, 1, 2, 2, std::bind(&debugger_commands::execute_cheatinit, this, _1, _2));
	m_console.register_command("cr",        CMDFLAG_NONE, 1, 2, 2, std::bind(&debugger_commands::execute_cheatinit, this, _1, _2));

	m_console.register_command("cheatnext", CMDFLAG_NONE, 0, 1, 2, std::bind(&debugger_commands::execute_cheatnext, this, _1, _2));
	m_console.register_command("cn",        CMDFLAG_NONE, 0, 1, 2, std::bind(&debugger_commands::execute_cheatnext, this, _1, _2));
	m_console.register_command("cheatnextf",CMDFLAG_NONE, 1, 1, 2, std::bind(&debugger_commands::execute_cheatnext, this, _1, _2));
	m_console.register_command("cnf",       CMDFLAG_NONE, 1, 1, 2, std::bind(&debugger_commands::execute_cheatnext, this, _1, _2));

	m_console.register_command("cheatlist", CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_cheatlist, this, _1, _2));
	m_console.register_command("cl",        CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_cheatlist, this, _1, _2));

	m_console.register_command("cheatundo", CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_cheatundo, this, _1, _2));
	m_console.register_command("cu",        CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_cheatundo, this, _1, _2));

	m_console.register_command("f",         CMDFLAG_KEEP_QUOTES, AS_PROGRAM, 3, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_find, this, _1, _2));
	m_console.register_command("find",      CMDFLAG_KEEP_QUOTES, AS_PROGRAM, 3, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_find, this, _1, _2));
	m_console.register_command("fd",        CMDFLAG_KEEP_QUOTES, AS_DATA, 3, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_find, this, _1, _2));
	m_console.register_command("findd",     CMDFLAG_KEEP_QUOTES, AS_DATA, 3, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_find, this, _1, _2));
	m_console.register_command("fi",        CMDFLAG_KEEP_QUOTES, AS_IO, 3, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_find, this, _1, _2));
	m_console.register_command("findi",     CMDFLAG_KEEP_QUOTES, AS_IO, 3, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_find, this, _1, _2));
	m_console.register_command("fo",        CMDFLAG_KEEP_QUOTES, AS_OPCODES, 3, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_find, this, _1, _2));
	m_console.register_command("findo",     CMDFLAG_KEEP_QUOTES, AS_OPCODES, 3, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_find, this, _1, _2));

	m_console.register_command("fill",      CMDFLAG_KEEP_QUOTES, AS_PROGRAM, 3, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_fill, this, _1, _2));
	m_console.register_command("filld",     CMDFLAG_KEEP_QUOTES, AS_DATA, 3, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_fill, this, _1, _2));
	m_console.register_command("filli",     CMDFLAG_KEEP_QUOTES, AS_IO, 3, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_fill, this, _1, _2));
	m_console.register_command("fillo",     CMDFLAG_KEEP_QUOTES, AS_OPCODES, 3, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_fill, this, _1, _2));

	m_console.register_command("dasm",      CMDFLAG_NONE, 0, 3, 5, std::bind(&debugger_commands::execute_dasm, this, _1, _2));

	m_console.register_command("trace",     CMDFLAG_NONE, 0, 1, 4, std::bind(&debugger_commands::execute_trace, this, _1, _2));
	m_console.register_command("traceover", CMDFLAG_NONE, 0, 1, 4, std::bind(&debugger_commands::execute_traceover, this, _1, _2));
	m_console.register_command("traceflush",CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_traceflush, this, _1, _2));

	m_console.register_command("history",   CMDFLAG_NONE, 0, 0, 2, std::bind(&debugger_commands::execute_history, this, _1, _2));
	m_console.register_command("trackpc",   CMDFLAG_NONE, 0, 0, 3, std::bind(&debugger_commands::execute_trackpc, this, _1, _2));

	m_console.register_command("trackmem",  CMDFLAG_NONE, 0, 0, 3, std::bind(&debugger_commands::execute_trackmem, this, _1, _2));
	m_console.register_command("pcatmemp",  CMDFLAG_NONE, AS_PROGRAM, 1, 2, std::bind(&debugger_commands::execute_pcatmem, this, _1, _2));
	m_console.register_command("pcatmemd",  CMDFLAG_NONE, AS_DATA,    1, 2, std::bind(&debugger_commands::execute_pcatmem, this, _1, _2));
	m_console.register_command("pcatmemi",  CMDFLAG_NONE, AS_IO,      1, 2, std::bind(&debugger_commands::execute_pcatmem, this, _1, _2));
	m_console.register_command("pcatmemo",  CMDFLAG_NONE, AS_OPCODES, 1, 2, std::bind(&debugger_commands::execute_pcatmem, this, _1, _2));

	m_console.register_command("snap",      CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_snap, this, _1, _2));

	m_console.register_command("source",    CMDFLAG_NONE, 0, 1, 1, std::bind(&debugger_commands::execute_source, this, _1, _2));

	m_console.register_command("map",       CMDFLAG_NONE, AS_PROGRAM, 1, 1, std::bind(&debugger_commands::execute_map, this, _1, _2));
	m_console.register_command("mapd",      CMDFLAG_NONE, AS_DATA, 1, 1, std::bind(&debugger_commands::execute_map, this, _1, _2));
	m_console.register_command("mapi",      CMDFLAG_NONE, AS_IO, 1, 1, std::bind(&debugger_commands::execute_map, this, _1, _2));
	m_console.register_command("mapo",      CMDFLAG_NONE, AS_OPCODES, 1, 1, std::bind(&debugger_commands::execute_map, this, _1, _2));
	m_console.register_command("memdump",   CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_memdump, this, _1, _2));

	m_console.register_command("symlist",   CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_symlist, this, _1, _2));

	m_console.register_command("softreset", CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_softreset, this, _1, _2));
	m_console.register_command("hardreset", CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_hardreset, this, _1, _2));

	m_console.register_command("images",    CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_commands::execute_images, this, _1, _2));
	m_console.register_command("mount",     CMDFLAG_NONE, 0, 2, 2, std::bind(&debugger_commands::execute_mount, this, _1, _2));
	m_console.register_command("unmount",   CMDFLAG_NONE, 0, 1, 1, std::bind(&debugger_commands::execute_unmount, this, _1, _2));

	m_console.register_command("input",     CMDFLAG_NONE, 0, 1, 1, std::bind(&debugger_commands::execute_input, this, _1, _2));
	m_console.register_command("dumpkbd",   CMDFLAG_NONE, 0, 0, 1, std::bind(&debugger_commands::execute_dumpkbd, this, _1, _2));

	/* set up the initial debugscript if specified */
	const char* name = m_machine.options().debug_script();
	if (name[0] != 0)
		m_console.source_script(name);

	m_cheat.cpu[0] = m_cheat.cpu[1] = 0;
}

//-------------------------------------------------
//  execute_min - return the minimum of two values
//-------------------------------------------------

u64 debugger_commands::execute_min(int params, const u64 *param)
{
	return (param[0] < param[1]) ? param[0] : param[1];
}


//-------------------------------------------------
//  execute_max - return the maximum of two values
//-------------------------------------------------

u64 debugger_commands::execute_max(int params, const u64 *param)
{
	return (param[0] > param[1]) ? param[0] : param[1];
}


//-------------------------------------------------
//  execute_if - if (a) return b; else return c;
//-------------------------------------------------

u64 debugger_commands::execute_if(int params, const u64 *param)
{
	return param[0] ? param[1] : param[2];
}


//-------------------------------------------------
//  execute_abs - return the absolute value
//-------------------------------------------------

u64 debugger_commands::execute_abs(int params, const u64 *param)
{
	return std::abs(s64(param[0]));
}


//-------------------------------------------------
//  execute_bit - extract bit field from value
//-------------------------------------------------

u64 debugger_commands::execute_bit(int params, const u64 *param)
{
	if (params == 2)
		return BIT(param[0], param[1]);
	else
		return BIT(param[0], param[1], param[2]);
}


//-------------------------------------------------
//  execute_s8 - sign-extend from 8 bits
//-------------------------------------------------

u64 debugger_commands::execute_s8(int params, const u64 *param)
{
	return s8(param[0]);
}


//-------------------------------------------------
//  execute_s16 - sign-extend from 16 bits
//-------------------------------------------------

u64 debugger_commands::execute_s16(int params, const u64 *param)
{
	return s16(param[0]);
}


//-------------------------------------------------
//  execute_s32 - sign-extend from 32 bits
//-------------------------------------------------

u64 debugger_commands::execute_s32(int params, const u64 *param)
{
	return s32(param[0]);
}


//-------------------------------------------------
//  get_cpunum - getter callback for the
//  'cpunum' symbol
//-------------------------------------------------

u64 debugger_commands::get_cpunum()
{
	execute_interface_enumerator iter(m_machine.root_device());
	return iter.indexof(m_console.get_visible_cpu()->execute());
}


/***************************************************************************
    GLOBAL ACCESSORS
***************************************************************************/

/*-------------------------------------------------
    global_get - symbol table getter for globals
-------------------------------------------------*/

u64 debugger_commands::global_get(global_entry *global)
{
	switch (global->size)
	{
		case 1:     return *(u8 *)global->base;
		case 2:     return *(u16 *)global->base;
		case 4:     return *(u32 *)global->base;
		case 8:     return *(u64 *)global->base;
	}
	return ~0;
}


/*-------------------------------------------------
    global_set - symbol table setter for globals
-------------------------------------------------*/

void debugger_commands::global_set(global_entry *global, u64 value)
{
	switch (global->size)
	{
		case 1:     *(u8 *)global->base = value; break;
		case 2:     *(u16 *)global->base = value;    break;
		case 4:     *(u32 *)global->base = value;    break;
		case 8:     *(u64 *)global->base = value;    break;
	}
}



/***************************************************************************
    PARAMETER VALIDATION HELPERS
***************************************************************************/

/*-------------------------------------------------
    validate_number_parameter - validates a
    number parameter
-------------------------------------------------*/

bool debugger_commands::validate_number_parameter(const std::string &param, u64 &result)
{
	/* evaluate the expression; success if no error */
	try
	{
		result = parsed_expression(m_console.visible_symtable(), param.c_str()).execute();
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
    validate_boolean_parameter - validates a
    boolean parameter
-------------------------------------------------*/

bool debugger_commands::validate_boolean_parameter(const std::string &param, bool &result)
{
	/* nullptr parameter does nothing and returns no error */
	if (param.empty())
		return true;

	/* evaluate the expression; success if no error */
	bool is_true = core_stricmp(param.c_str(), "true") == 0 || param == "1";
	bool is_false = core_stricmp(param.c_str(), "false") == 0 || param == "0";

	if (!is_true && !is_false)
	{
		m_console.printf("Invalid boolean '%s'\n", param);
		return false;
	}

	result = is_true;

	return true;
}


/*-------------------------------------------------
    validate_cpu_parameter - validates a
    parameter as a cpu
-------------------------------------------------*/

bool debugger_commands::validate_cpu_parameter(const char *param, device_t *&result)
{
	/* if no parameter, use the visible CPU */
	if (param == nullptr)
	{
		result = m_console.get_visible_cpu();
		if (!result)
		{
			m_console.printf("No valid CPU is currently selected\n");
			return false;
		}
		return true;
	}

	/* first look for a tag match */
	result = m_machine.root_device().subdevice(strmakelower(param));
	if (result)
		return true;

	/* then evaluate as an expression; on an error assume it was a tag */
	u64 cpunum;
	try
	{
		cpunum = parsed_expression(m_console.visible_symtable(), param).execute();
	}
	catch (expression_error &)
	{
		m_console.printf("Unable to find CPU '%s'\n", param);
		return false;
	}

	// attempt to find by numerical index
	int index = 0;
	for (device_execute_interface &exec : execute_interface_enumerator(m_machine.root_device()))
	{
		// real CPUs should have pcbase
		const device_state_interface *state;
		if (exec.device().interface(state) && state->state_find_entry(STATE_GENPCBASE) != nullptr && index++ == cpunum)
		{
			result = &exec.device();
			return true;
		}
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
	if (!validate_cpu_parameter(param, cpu))
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
    validate_memory_region_parameter - validates
    a parameter as a memory region name and
    retrieves the given memory region
-------------------------------------------------*/

bool debugger_commands::validate_memory_region_parameter(const std::string &param, memory_region *&result)
{
	auto &regions = m_machine.memory().regions();
	auto iter = regions.find(strmakelower(param));
	if(iter == regions.end()) {
		m_console.printf("No matching memory region found for '%s'\n", param);
		return false;
	}
	result = iter->second.get();
	return true;
}


/*-------------------------------------------------
    debug_command_parameter_expression - validates
    an expression parameter
-------------------------------------------------*/

bool debugger_commands::debug_command_parameter_expression(const std::string &param, parsed_expression &result)
{
	/* parse the expression; success if no error */
	try
	{
		result.parse(param.c_str());
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
	if (err.error_class() == CMDERR::NONE)
		return true;

	/* output an error */
	m_console.printf("Error in command: %s\n", param);
	m_console.printf("                  %*s^", err.error_offset(), "");
	m_console.printf("%s\n", debugger_console::cmderr_to_string(err));
	return 0;
}

/*-------------------------------------------------
    execute_help - execute the help command
-------------------------------------------------*/

void debugger_commands::execute_help(int ref, const std::vector<std::string> &params)
{
	if (params.size() == 0)
		m_console.printf_wrap(80, "%s\n", debug_get_help(""));
	else
		m_console.printf_wrap(80, "%s\n", debug_get_help(params[0].c_str()));
}


/*-------------------------------------------------
    execute_print - execute the print command
-------------------------------------------------*/

void debugger_commands::execute_print(int ref, const std::vector<std::string> &params)
{
	/* validate the other parameters */
	u64 values[MAX_COMMAND_PARAMS];
	for (int i = 0; i < params.size(); i++)
		if (!validate_number_parameter(params[i], values[i]))
			return;

	/* then print each one */
	for (int i = 0; i < params.size(); i++)
		m_console.printf("%X", values[i]);
	m_console.printf("\n");
}


/*-------------------------------------------------
    mini_printf - safe printf to a buffer
-------------------------------------------------*/

int debugger_commands::mini_printf(char *buffer, const char *format, int params, u64 *param)
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
					if (u32(*param >> 32) != 0)
						p += sprintf(p, zerofill ? "%0*X" : "%*X", (width <= 8) ? 1 : width - 8, u32(*param >> 32));
					else if (width > 8)
						p += sprintf(p, zerofill ? "%0*X" : "%*X", width - 8, 0);
					p += sprintf(p, zerofill ? "%0*X" : "%*X", (width < 8) ? width : 8, u32(*param));
					param++;
					params--;
					break;

				case 'O':
				case 'o':
					if (params == 0)
					{
						m_console.printf("Not enough parameters for format!\n");
						return 0;
					}
					if (u32(*param >> 60) != 0)
					{
						p += sprintf(p, zerofill ? "%0*o" : "%*o", (width <= 20) ? 1 : width - 20, u32(*param >> 60));
						p += sprintf(p, "%0*o", 10, u32(BIT(*param, 30, 30)));
					}
					else
					{
						if (width > 20)
							p += sprintf(p, zerofill ? "%0*o" : "%*o", width - 20, 0);
						if (u32(BIT(*param, 30, 30)) != 0)
							p += sprintf(p, zerofill ? "%0*o" : "%*o", (width <= 10) ? 1 : width - 10, u32(BIT(*param, 30, 30)));
						else if (width > 10)
							p += sprintf(p, zerofill ? "%0*o" : "%*o", width - 10, 0);
					}
					p += sprintf(p, zerofill ? "%0*o" : "%*o", (width < 10) ? width : 10, u32(BIT(*param, 0, 30)));
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
					p += sprintf(p, zerofill ? "%0*d" : "%*d", width, u32(*param));
					param++;
					params--;
					break;
				case 'C':
				case 'c':
					if (params == 0)
					{
						m_console.printf("Not enough parameters for format!\n");
						return 0;
					}
					p += sprintf(p, "%c", char(*param));
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

void debugger_commands::execute_printf(int ref, const std::vector<std::string> &params)
{
	/* validate the other parameters */
	u64 values[MAX_COMMAND_PARAMS];
	for (int i = 1; i < params.size(); i++)
		if (!validate_number_parameter(params[i], values[i]))
			return;

	/* then do a printf */
	char buffer[1024];
	if (mini_printf(buffer, params[0].c_str(), params.size() - 1, &values[1]))
		m_console.printf("%s\n", buffer);
}


/*-------------------------------------------------
    execute_logerror - execute the logerror command
-------------------------------------------------*/

void debugger_commands::execute_logerror(int ref, const std::vector<std::string> &params)
{
	/* validate the other parameters */
	u64 values[MAX_COMMAND_PARAMS];
	for (int i = 1; i < params.size(); i++)
		if (!validate_number_parameter(params[i], values[i]))
			return;

	/* then do a printf */
	char buffer[1024];
	if (mini_printf(buffer, params[0].c_str(), params.size() - 1, &values[1]))
		m_machine.logerror("%s", buffer);
}


/*-------------------------------------------------
    execute_tracelog - execute the tracelog command
-------------------------------------------------*/

void debugger_commands::execute_tracelog(int ref, const std::vector<std::string> &params)
{
	/* validate the other parameters */
	u64 values[MAX_COMMAND_PARAMS];
	for (int i = 1; i < params.size(); i++)
		if (!validate_number_parameter(params[i], values[i]))
			return;

	/* then do a printf */
	char buffer[1024];
	if (mini_printf(buffer, params[0].c_str(), params.size() - 1, &values[1]))
		m_console.get_visible_cpu()->debug()->trace_printf("%s", buffer);
}


/*-------------------------------------------------
    execute_tracesym - execute the tracesym command
-------------------------------------------------*/

void debugger_commands::execute_tracesym(int ref, const std::vector<std::string> &params)
{
	// build a format string appropriate for the parameters and validate them
	std::stringstream format;
	u64 values[MAX_COMMAND_PARAMS];
	for (int i = 0; i < params.size(); i++)
	{
		// find this symbol
		symbol_entry *sym = m_console.visible_symtable().find(strmakelower(params[i]).c_str());
		if (!sym)
		{
			m_console.printf("Unknown symbol: %s\n", params[i]);
			return;
		}

		// build the format string
		util::stream_format(format, "%s=%s ",
			params[i],
			sym->format().empty() ? "%16X" : sym->format());

		// validate the parameter
		if (!validate_number_parameter(params[i], values[i]))
			return;
	}

	// then do a printf
	char buffer[1024];
	if (mini_printf(buffer, format.str().c_str(), params.size(), values))
		m_console.get_visible_cpu()->debug()->trace_printf("%s", buffer);
}


/*-------------------------------------------------
    execute_cls - execute the cls command
-------------------------------------------------*/

void debugger_commands::execute_cls(int ref, const std::vector<std::string> &params)
{
	text_buffer_clear(m_console.get_console_textbuf());
}


/*-------------------------------------------------
    execute_quit - execute the quit command
-------------------------------------------------*/

void debugger_commands::execute_quit(int ref, const std::vector<std::string> &params)
{
	osd_printf_warning("Exited via the debugger\n");
	m_machine.schedule_exit();
}


/*-------------------------------------------------
    execute_do - execute the do command
-------------------------------------------------*/

void debugger_commands::execute_do(int ref, const std::vector<std::string> &params)
{
	u64 dummy;
	validate_number_parameter(params[0], dummy);
}


/*-------------------------------------------------
    execute_step - execute the step command
-------------------------------------------------*/

void debugger_commands::execute_step(int ref, const std::vector<std::string> &params)
{
	/* if we have a parameter, use it */
	u64 steps = 1;
	if (params.size() > 0 && !validate_number_parameter(params[0], steps))
		return;

	m_console.get_visible_cpu()->debug()->single_step(steps);
}


/*-------------------------------------------------
    execute_over - execute the over command
-------------------------------------------------*/

void debugger_commands::execute_over(int ref, const std::vector<std::string> &params)
{
	/* if we have a parameter, use it */
	u64 steps = 1;
	if (params.size() > 0 && !validate_number_parameter(params[0], steps))
		return;

	m_console.get_visible_cpu()->debug()->single_step_over(steps);
}


/*-------------------------------------------------
    execute_out - execute the out command
-------------------------------------------------*/

void debugger_commands::execute_out(int ref, const std::vector<std::string> &params)
{
	m_console.get_visible_cpu()->debug()->single_step_out();
}


/*-------------------------------------------------
    execute_go - execute the go command
-------------------------------------------------*/

void debugger_commands::execute_go(int ref, const std::vector<std::string> &params)
{
	u64 addr = ~0;

	/* if we have a parameter, use it instead */
	if (params.size() > 0 && !validate_number_parameter(params[0], addr))
		return;

	m_console.get_visible_cpu()->debug()->go(addr);
}


/*-------------------------------------------------
    execute_go_vblank - execute the govblank
    command
-------------------------------------------------*/

void debugger_commands::execute_go_vblank(int ref, const std::vector<std::string> &params)
{
	m_console.get_visible_cpu()->debug()->go_vblank();
}


/*-------------------------------------------------
    execute_go_interrupt - execute the goint command
-------------------------------------------------*/

void debugger_commands::execute_go_interrupt(int ref, const std::vector<std::string> &params)
{
	u64 irqline = -1;

	/* if we have a parameter, use it instead */
	if (params.size() > 0 && !validate_number_parameter(params[0], irqline))
		return;

	m_console.get_visible_cpu()->debug()->go_interrupt(irqline);
}

/*-------------------------------------------------
    execute_go_exception - execute the goex command
-------------------------------------------------*/

void debugger_commands::execute_go_exception(int ref, const std::vector<std::string> &params)
{
	u64 exception = -1;

	/* if we have a parameter, use it instead */
	if (params.size() > 0 && !validate_number_parameter(params[0], exception))
		return;

	parsed_expression condition(m_console.visible_symtable());
	if (params.size() > 1 && !debug_command_parameter_expression(params[1], condition))
		return;

	m_console.get_visible_cpu()->debug()->go_exception(exception, (condition.is_empty()) ? "1" : condition.original_string());
}


/*-------------------------------------------------
    execute_go_time - execute the gtime command
-------------------------------------------------*/

void debugger_commands::execute_go_time(int ref, const std::vector<std::string> &params)
{
	u64 milliseconds = -1;

	/* if we have a parameter, use it instead */
	if (params.size() > 0 && !validate_number_parameter(params[0], milliseconds))
		return;

	m_console.get_visible_cpu()->debug()->go_milliseconds(milliseconds);
}



/*-------------------------------------------------
    execute_go_privilege - execute the gp command
-------------------------------------------------*/
void debugger_commands::execute_go_privilege(int ref, const std::vector<std::string> &params)
{
	parsed_expression condition(m_console.visible_symtable());
	if (params.size() > 0 && !debug_command_parameter_expression(params[0], condition))
		return;

	m_console.get_visible_cpu()->debug()->go_privilege((condition.is_empty()) ? "1" : condition.original_string());
}

/*-------------------------------------------------
    execute_next - execute the next command
-------------------------------------------------*/

void debugger_commands::execute_next(int ref, const std::vector<std::string> &params)
{
	m_console.get_visible_cpu()->debug()->go_next_device();
}


/*-------------------------------------------------
    execute_focus - execute the focus command
-------------------------------------------------*/

void debugger_commands::execute_focus(int ref, const std::vector<std::string> &params)
{
	/* validate params */
	device_t *cpu;
	if (!validate_cpu_parameter(params[0].c_str(), cpu))
		return;

	/* first clear the ignore flag on the focused CPU */
	cpu->debug()->ignore(false);

	/* then loop over CPUs and set the ignore flags on all other CPUs */
	for (device_execute_interface &exec : execute_interface_enumerator(m_machine.root_device()))
		if (&exec.device() != cpu)
			exec.device().debug()->ignore(true);
	m_console.printf("Now focused on CPU '%s'\n", cpu->tag());
}


/*-------------------------------------------------
    execute_ignore - execute the ignore command
-------------------------------------------------*/

void debugger_commands::execute_ignore(int ref, const std::vector<std::string> &params)
{
	/* if there are no parameters, dump the ignore list */
	if (params.empty())
	{
		std::string buffer;

		/* loop over all executable devices */
		for (device_execute_interface &exec : execute_interface_enumerator(m_machine.root_device()))

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
		m_console.printf("%s\n", buffer);
	}

	/* otherwise clear the ignore flag on all requested CPUs */
	else
	{
		device_t *devicelist[MAX_COMMAND_PARAMS];

		/* validate parameters */
		for (int paramnum = 0; paramnum < params.size(); paramnum++)
			if (!validate_cpu_parameter(params[paramnum].c_str(), devicelist[paramnum]))
				return;

		/* set the ignore flags */
		for (int paramnum = 0; paramnum < params.size(); paramnum++)
		{
			/* make sure this isn't the last live CPU */
			bool gotone = false;
			for (device_execute_interface &exec : execute_interface_enumerator(m_machine.root_device()))
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

void debugger_commands::execute_observe(int ref, const std::vector<std::string> &params)
{
	/* if there are no parameters, dump the ignore list */
	if (params.empty())
	{
		std::string buffer;

		/* loop over all executable devices */
		for (device_execute_interface &exec : execute_interface_enumerator(m_machine.root_device()))

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
		m_console.printf("%s\n", buffer);
	}

	/* otherwise set the ignore flag on all requested CPUs */
	else
	{
		device_t *devicelist[MAX_COMMAND_PARAMS];

		/* validate parameters */
		for (int paramnum = 0; paramnum < params.size(); paramnum++)
			if (!validate_cpu_parameter(params[paramnum].c_str(), devicelist[paramnum]))
				return;

		/* clear the ignore flags */
		for (int paramnum = 0; paramnum < params.size(); paramnum++)
		{
			devicelist[paramnum]->debug()->ignore(false);
			m_console.printf("Now observing device '%s'\n", devicelist[paramnum]->tag());
		}
	}
}

/*-------------------------------------------------
    execute_suspend - suspend execution on cpu
-------------------------------------------------*/

void debugger_commands::execute_suspend(int ref, const std::vector<std::string> &params)
{
	/* if there are no parameters, dump the ignore list */
	if (params.empty())
	{
		std::string buffer;

		/* loop over all executable devices */
		for (device_execute_interface &exec : execute_interface_enumerator(m_machine.root_device()))

			/* build up a comma-separated list */
			if (exec.device().debug()->suspended())
			{
				if (buffer.empty())
					buffer = string_format("Currently suspended device '%s'", exec.device().tag());
				else
					buffer.append(string_format(", '%s'", exec.device().tag()));
			}

		/* special message for none */
		if (buffer.empty())
			buffer = string_format("No currently suspended devices");
		m_console.printf("%s\n", buffer);
	}
	else
	{
		device_t *devicelist[MAX_COMMAND_PARAMS];

		/* validate parameters */
		for (int paramnum = 0; paramnum < params.size(); paramnum++)
			if (!validate_cpu_parameter(params[paramnum].c_str(), devicelist[paramnum]))
				return;

		for (int paramnum = 0; paramnum < params.size(); paramnum++)
		{
			/* make sure this isn't the last live CPU */
			bool gotone = false;
			for (device_execute_interface &exec : execute_interface_enumerator(m_machine.root_device()))
				if (&exec.device() != devicelist[paramnum] && !exec.device().debug()->suspended())
				{
					gotone = true;
					break;
				}
			if (!gotone)
			{
				m_console.printf("Can't suspend all devices!\n");
				return;
			}

			devicelist[paramnum]->debug()->suspend(true);
			m_console.printf("Suspended device '%s'\n", devicelist[paramnum]->tag());
		}
	}
}

/*-------------------------------------------------
    execute_resume - Resume execution on CPU
-------------------------------------------------*/

void debugger_commands::execute_resume(int ref, const std::vector<std::string> &params)
{
	/* if there are no parameters, dump the ignore list */
	if (params.empty())
	{
		std::string buffer;

		/* loop over all executable devices */
		for (device_execute_interface &exec : execute_interface_enumerator(m_machine.root_device()))

			/* build up a comma-separated list */
			if (exec.device().debug()->suspended())
			{
				if (buffer.empty())
					buffer = string_format("Currently suspended device '%s'", exec.device().tag());
				else
					buffer.append(string_format(", '%s'", exec.device().tag()));
			}

		/* special message for none */
		if (buffer.empty())
			buffer = string_format("No currently suspended devices");
		m_console.printf("%s\n", buffer);
	}
	else
	{
		device_t *devicelist[MAX_COMMAND_PARAMS];

		/* validate parameters */
		for (int paramnum = 0; paramnum < params.size(); paramnum++)
			if (!validate_cpu_parameter(params[paramnum].c_str(), devicelist[paramnum]))
				return;

		for (int paramnum = 0; paramnum < params.size(); paramnum++)
		{
			devicelist[paramnum]->debug()->suspend(false);
			m_console.printf("Resumed device '%s'\n", devicelist[paramnum]->tag());
		}
	}
}

//-------------------------------------------------
//  execute_cpulist - list all CPUs
//-------------------------------------------------

void debugger_commands::execute_cpulist(int ref, const std::vector<std::string> &params)
{
	int index = 0;
	for (device_execute_interface &exec : execute_interface_enumerator(m_machine.root_device()))
	{
		const device_state_interface *state;
		if (exec.device().interface(state) && state->state_find_entry(STATE_GENPCBASE) != nullptr)
			m_console.printf("[%s%d] %s\n", &exec.device() == m_console.get_visible_cpu() ? "*" : "", index++, exec.device().tag());
	}
}

/*-------------------------------------------------
    execute_comment - add a comment to a line
-------------------------------------------------*/

void debugger_commands::execute_comment_add(int ref, const std::vector<std::string> &params)
{
	device_t *cpu;
	u64 address;

	/* param 1 is the address for the comment */
	if (!validate_number_parameter(params[0], address))
		return;

	/* CPU parameter is implicit */
	if (!validate_cpu_parameter(nullptr, cpu))
		return;

	/* make sure param 2 exists */
	if (params[1].empty())
	{
		m_console.printf("Error : comment text empty\n");
		return;
	}

	/* Now try adding the comment */
	cpu->debug()->comment_add(address, params[1].c_str(), 0x00ff0000);
	cpu->machine().debug_view().update_all(DVT_DISASSEMBLY);
}


/*------------------------------------------------------
    execute_comment_del - remove a comment from an addr
--------------------------------------------------------*/

void debugger_commands::execute_comment_del(int ref, const std::vector<std::string> &params)
{
	device_t *cpu;
	u64 address;

	/* param 1 can either be a command or the address for the comment */
	if (!validate_number_parameter(params[0], address))
		return;

	/* CPU parameter is implicit */
	if (!validate_cpu_parameter(nullptr, cpu))
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

void debugger_commands::execute_comment_list(int ref, const std::vector<std::string> &params)
{
	if (!m_machine.debugger().cpu().comment_load(false))
		m_console.printf("Error while parsing XML file\n");
}

/**
 * @fn void execute_comment_commit(running_machine &machine, int ref, int params, const char *param[])
 * @brief Add and Save current list of comments in debugger
 *
 */

void debugger_commands::execute_comment_commit(int ref, const std::vector<std::string> &params)
{
	execute_comment_add(ref, params);
	execute_comment_save(ref, params);
}

/*-------------------------------------------------
    execute_comment - add a comment to a line
-------------------------------------------------*/

void debugger_commands::execute_comment_save(int ref, const std::vector<std::string> &params)
{
	if (m_machine.debugger().cpu().comment_save())
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

void debugger_commands::execute_bpset(int ref, const std::vector<std::string> &params)
{
	device_t *cpu;
	u64 address;
	int bpnum;
	const char *action = nullptr;

	/* CPU is implicit */
	if (!validate_cpu_parameter(nullptr, cpu))
		return;

	/* param 1 is the address */
	if (!validate_number_parameter(params[0], address))
		return;

	/* param 2 is the condition */
	parsed_expression condition(cpu->debug()->symtable());
	if (params.size() > 1 && !debug_command_parameter_expression(params[1], condition))
		return;

	/* param 3 is the action */
	if (params.size() > 2 && !debug_command_parameter_command(action = params[2].c_str()))
		return;

	/* set the breakpoint */
	bpnum = cpu->debug()->breakpoint_set(address, (condition.is_empty()) ? nullptr : condition.original_string(), action);
	m_console.printf("Breakpoint %X set\n", bpnum);
}


/*-------------------------------------------------
    execute_bpclear - execute the breakpoint
    clear command
-------------------------------------------------*/

void debugger_commands::execute_bpclear(int ref, const std::vector<std::string> &params)
{
	u64 bpindex;

	/* if 0 parameters, clear all */
	if (params.empty())
	{
		for (device_t &device : device_enumerator(m_machine.root_device()))
			device.debug()->breakpoint_clear_all();
		m_console.printf("Cleared all breakpoints\n");
	}

	/* otherwise, clear the specific one */
	else if (!validate_number_parameter(params[0], bpindex))
		return;
	else
	{
		bool found = false;
		for (device_t &device : device_enumerator(m_machine.root_device()))
			if (device.debug()->breakpoint_clear(bpindex))
				found = true;
		if (found)
			m_console.printf("Breakpoint %X cleared\n", u32(bpindex));
		else
			m_console.printf("Invalid breakpoint number %X\n", u32(bpindex));
	}
}


/*-------------------------------------------------
    execute_bpdisenable - execute the breakpoint
    disable/enable commands
-------------------------------------------------*/

void debugger_commands::execute_bpdisenable(int ref, const std::vector<std::string> &params)
{
	u64 bpindex;

	/* if 0 parameters, clear all */
	if (params.empty())
	{
		for (device_t &device : device_enumerator(m_machine.root_device()))
			device.debug()->breakpoint_enable_all(ref);
		if (ref == 0)
			m_console.printf("Disabled all breakpoints\n");
		else
			m_console.printf("Enabled all breakpoints\n");
	}

	/* otherwise, clear the specific one */
	else if (!validate_number_parameter(params[0], bpindex))
		return;
	else
	{
		bool found = false;
		for (device_t &device : device_enumerator(m_machine.root_device()))
			if (device.debug()->breakpoint_enable(bpindex, ref))
				found = true;
		if (found)
			m_console.printf("Breakpoint %X %s\n", u32(bpindex), ref ? "enabled" : "disabled");
		else
			m_console.printf("Invalid breakpoint number %X\n", u32(bpindex));
	}
}


/*-------------------------------------------------
    execute_bplist - execute the breakpoint list
    command
-------------------------------------------------*/

void debugger_commands::execute_bplist(int ref, const std::vector<std::string> &params)
{
	int printed = 0;
	std::string buffer;

	/* loop over all CPUs */
	for (device_t &device : device_enumerator(m_machine.root_device()))
		if (!device.debug()->breakpoint_list().empty())
		{
			m_console.printf("Device '%s' breakpoints:\n", device.tag());

			/* loop over the breakpoints */
			for (const auto &bpp : device.debug()->breakpoint_list())
			{
				debug_breakpoint &bp = *bpp.second;
				buffer = string_format("%c%4X @ %0*X", bp.enabled() ? ' ' : 'D', bp.index(), device.debug()->logaddrchars(), bp.address());
				if (std::string(bp.condition()).compare("1") != 0)
					buffer.append(string_format(" if %s", bp.condition()));
				if (std::string(bp.action()).compare("") != 0)
					buffer.append(string_format(" do %s", bp.action()));
				m_console.printf("%s\n", buffer);
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

void debugger_commands::execute_wpset(int ref, const std::vector<std::string> &params)
{
	address_space *space;
	const char *action = nullptr;
	u64 address, length;
	read_or_write type;
	int wpnum;

	/* CPU is implicit */
	if (!validate_cpu_space_parameter(nullptr, ref, space))
		return;

	/* param 1 is the address */
	if (!validate_number_parameter(params[0], address))
		return;

	/* param 2 is the length */
	if (!validate_number_parameter(params[1], length))
		return;

	/* param 3 is the type */
	if (!core_stricmp(params[2].c_str(), "r"))
		type = read_or_write::READ;
	else if (!core_stricmp(params[2].c_str(), "w"))
		type = read_or_write::WRITE;
	else if (!core_stricmp(params[2].c_str(), "rw") || !core_stricmp(params[2].c_str(), "wr"))
		type = read_or_write::READWRITE;
	else
	{
		m_console.printf("Invalid watchpoint type: expected r, w, or rw\n");
		return;
	}

	/* param 4 is the condition */
	parsed_expression condition(space->device().debug()->symtable());
	if (params.size() > 3 && !debug_command_parameter_expression(params[3], condition))
		return;

	/* param 5 is the action */
	if (params.size() > 4 && !debug_command_parameter_command(action = params[4].c_str()))
		return;

	/* set the watchpoint */
	wpnum = space->device().debug()->watchpoint_set(*space, type, address, length, (condition.is_empty()) ? nullptr : condition.original_string(), action);
	m_console.printf("Watchpoint %X set\n", wpnum);
}


/*-------------------------------------------------
    execute_wpclear - execute the watchpoint
    clear command
-------------------------------------------------*/

void debugger_commands::execute_wpclear(int ref, const std::vector<std::string> &params)
{
	u64 wpindex;

	/* if 0 parameters, clear all */
	if (params.empty())
	{
		for (device_t &device : device_enumerator(m_machine.root_device()))
			device.debug()->watchpoint_clear_all();
		m_console.printf("Cleared all watchpoints\n");
	}

	/* otherwise, clear the specific one */
	else if (!validate_number_parameter(params[0], wpindex))
		return;
	else
	{
		bool found = false;
		for (device_t &device : device_enumerator(m_machine.root_device()))
			if (device.debug()->watchpoint_clear(wpindex))
				found = true;
		if (found)
			m_console.printf("Watchpoint %X cleared\n", u32(wpindex));
		else
			m_console.printf("Invalid watchpoint number %X\n", u32(wpindex));
	}
}


/*-------------------------------------------------
    execute_wpdisenable - execute the watchpoint
    disable/enable commands
-------------------------------------------------*/

void debugger_commands::execute_wpdisenable(int ref, const std::vector<std::string> &params)
{
	u64 wpindex;

	/* if 0 parameters, clear all */
	if (params.empty())
	{
		for (device_t &device : device_enumerator(m_machine.root_device()))
			device.debug()->watchpoint_enable_all(ref);
		if (ref == 0)
			m_console.printf("Disabled all watchpoints\n");
		else
			m_console.printf("Enabled all watchpoints\n");
	}

	/* otherwise, clear the specific one */
	else if (!validate_number_parameter(params[0], wpindex))
		return;
	else
	{
		bool found = false;
		for (device_t &device : device_enumerator(m_machine.root_device()))
			if (device.debug()->watchpoint_enable(wpindex, ref))
				found = true;
		if (found)
			m_console.printf("Watchpoint %X %s\n", u32(wpindex), ref ? "enabled" : "disabled");
		else
			m_console.printf("Invalid watchpoint number %X\n", u32(wpindex));
	}
}


/*-------------------------------------------------
    execute_wplist - execute the watchpoint list
    command
-------------------------------------------------*/

void debugger_commands::execute_wplist(int ref, const std::vector<std::string> &params)
{
	int printed = 0;
	std::string buffer;

	/* loop over all CPUs */
	for (device_t &device : device_enumerator(m_machine.root_device()))
		for (int spacenum = 0; spacenum < device.debug()->watchpoint_space_count(); ++spacenum)
			if (!device.debug()->watchpoint_vector(spacenum).empty())
			{
				static const char *const types[] = { "unkn ", "read ", "write", "r/w  " };

				m_console.printf("Device '%s' %s space watchpoints:\n", device.tag(),
								 device.debug()->watchpoint_vector(spacenum).front()->space().name());

				/* loop over the watchpoints */
				for (const auto &wp : device.debug()->watchpoint_vector(spacenum))
				{
					buffer = string_format("%c%4X @ %0*X-%0*X %s", wp->enabled() ? ' ' : 'D', wp->index(),
							wp->space().addrchars(), wp->address(),
							wp->space().addrchars(), wp->address() + wp->length() - 1,
							types[int(wp->type())]);
					if (std::string(wp->condition()).compare("1") != 0)
						buffer.append(string_format(" if %s", wp->condition()));
					if (std::string(wp->action()).compare("") != 0)
						buffer.append(string_format(" do %s", wp->action()));
					m_console.printf("%s\n", buffer);
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

void debugger_commands::execute_rpset(int ref, const std::vector<std::string> &params)
{
	device_t *cpu;
	const char *action = nullptr;
	int bpnum;

	/* CPU is implicit */
	if (!validate_cpu_parameter(nullptr, cpu))
		return;

	/* param 1 is the condition */
	parsed_expression condition(cpu->debug()->symtable());
	if (params.size() > 0 && !debug_command_parameter_expression(params[0], condition))
		return;

	/* param 2 is the action */
	if (params.size() > 1 && !debug_command_parameter_command(action = params[1].c_str()))
		return;

	/* set the breakpoint */
	bpnum = cpu->debug()->registerpoint_set(condition.original_string(), action);
	m_console.printf("Registerpoint %X set\n", bpnum);
}


/*-------------------------------------------------
    execute_rpclear - execute the registerpoint
    clear command
-------------------------------------------------*/

void debugger_commands::execute_rpclear(int ref, const std::vector<std::string> &params)
{
	u64 rpindex;

	/* if 0 parameters, clear all */
	if (params.empty())
	{
		for (device_t &device : device_enumerator(m_machine.root_device()))
			device.debug()->registerpoint_clear_all();
		m_console.printf("Cleared all registerpoints\n");
	}

	/* otherwise, clear the specific one */
	else if (!validate_number_parameter(params[0], rpindex))
		return;
	else
	{
		bool found = false;
		for (device_t &device : device_enumerator(m_machine.root_device()))
			if (device.debug()->registerpoint_clear(rpindex))
				found = true;
		if (found)
			m_console.printf("Registerpoint %X cleared\n", u32(rpindex));
		else
			m_console.printf("Invalid registerpoint number %X\n", u32(rpindex));
	}
}


/*-------------------------------------------------
    execute_rpdisenable - execute the registerpoint
    disable/enable commands
-------------------------------------------------*/

void debugger_commands::execute_rpdisenable(int ref, const std::vector<std::string> &params)
{
	u64 rpindex;

	/* if 0 parameters, clear all */
	if (params.empty())
	{
		for (device_t &device : device_enumerator(m_machine.root_device()))
			device.debug()->registerpoint_enable_all(ref);
		if (ref == 0)
			m_console.printf("Disabled all registerpoints\n");
		else
			m_console.printf("Enabled all registeroints\n");
	}

	/* otherwise, clear the specific one */
	else if (!validate_number_parameter(params[0], rpindex))
		return;
	else
	{
		bool found = false;
		for (device_t &device : device_enumerator(m_machine.root_device()))
			if (device.debug()->registerpoint_enable(rpindex, ref))
				found = true;
		if (found)
			m_console.printf("Registerpoint %X %s\n", u32(rpindex), ref ? "enabled" : "disabled");
		else
			m_console.printf("Invalid registerpoint number %X\n", u32(rpindex));
	}
}


/*-------------------------------------------------
    execute_rplist - execute the registerpoint list
    command
-------------------------------------------------*/

void debugger_commands::execute_rplist(int ref, const std::vector<std::string> &params)
{
	int printed = 0;
	std::string buffer;

	/* loop over all CPUs */
	for (device_t &device : device_enumerator(m_machine.root_device()))
		if (!device.debug()->registerpoint_list().empty())
		{
			m_console.printf("Device '%s' registerpoints:\n", device.tag());

			/* loop over the breakpoints */
			for (const debug_registerpoint &rp : device.debug()->registerpoint_list())
			{
				buffer = string_format("%c%4X if %s", rp.enabled() ? ' ' : 'D', rp.index(), rp.condition());
				if (rp.action() != nullptr)
					buffer.append(string_format(" do %s", rp.action()));
				m_console.printf("%s\n", buffer);
				printed++;
			}
		}

	if (printed == 0)
		m_console.printf("No registerpoints currently installed\n");
}


/*-------------------------------------------------
    execute_statesave - execute the statesave command
-------------------------------------------------*/

void debugger_commands::execute_statesave(int ref, const std::vector<std::string> &params)
{
	const std::string &filename(params[0]);
	m_machine.immediate_save(filename.c_str());
	m_console.printf("State save attempted.  Please refer to window message popup for results.\n");
}


/*-------------------------------------------------
    execute_stateload - execute the stateload command
-------------------------------------------------*/

void debugger_commands::execute_stateload(int ref, const std::vector<std::string> &params)
{
	const std::string &filename(params[0]);
	m_machine.immediate_load(filename.c_str());

	// clear all PC & memory tracks
	for (device_t &device : device_enumerator(m_machine.root_device()))
	{
		device.debug()->track_pc_data_clear();
		device.debug()->track_mem_data_clear();
	}
	m_console.printf("State load attempted.  Please refer to window message popup for results.\n");
}


/*-------------------------------------------------
    execute_rewind - execute the rewind command
-------------------------------------------------*/

void debugger_commands::execute_rewind(int ref, const std::vector<std::string> &params)
{
	bool success = m_machine.rewind_step();
	if (success)
		// clear all PC & memory tracks
		for (device_t &device : device_enumerator(m_machine.root_device()))
		{
			device.debug()->track_pc_data_clear();
			device.debug()->track_mem_data_clear();
		}
	else
		m_console.printf("Rewind error occured.  See error.log for details.\n");
}


/*-------------------------------------------------
    execute_save - execute the save command
-------------------------------------------------*/

void debugger_commands::execute_save(int ref, const std::vector<std::string> &params)
{
	u64 offset, endoffset, length;
	address_space *space;
	FILE *f;

	/* validate parameters */
	if (!validate_number_parameter(params[1], offset))
		return;
	if (!validate_number_parameter(params[2], length))
		return;
	if (!validate_cpu_space_parameter(params.size() > 3 ? params[3].c_str() : nullptr, ref, space))
		return;

	/* determine the addresses to write */
	endoffset = (offset + length - 1) & space->addrmask();
	offset = offset & space->addrmask();
	endoffset ++;

	/* open the file */
	f = fopen(params[0].c_str(), "wb");
	if (!f)
	{
		m_console.printf("Error opening file '%s'\n", params[0]);
		return;
	}

	/* now write the data out */
	auto dis = space->device().machine().disable_side_effects();
	switch (space->addr_shift())
	{
	case -3:
		for (u64 i = offset; i != endoffset; i++)
		{
			offs_t curaddr = i;
			u64 data = space->device().memory().translate(space->spacenum(), TRANSLATE_READ_DEBUG, curaddr) ?
				space->read_qword(curaddr) : space->unmap();
			fwrite(&data, 8, 1, f);
		}
		break;
	case -2:
		for (u64 i = offset; i != endoffset; i++)
		{
			offs_t curaddr = i;
			u32 data = space->device().memory().translate(space->spacenum(), TRANSLATE_READ_DEBUG, curaddr) ?
				space->read_dword(curaddr) : space->unmap();
			fwrite(&data, 4, 1, f);
		}
		break;
	case -1:
		for (u64 i = offset; i != endoffset; i++)
		{
			offs_t curaddr = i;
			u16 data = space->device().memory().translate(space->spacenum(), TRANSLATE_READ_DEBUG, curaddr) ?
				space->read_word(curaddr) : space->unmap();
			fwrite(&data, 2, 1, f);
		}
		break;
	case  0:
		for (u64 i = offset; i != endoffset; i++)
		{
			offs_t curaddr = i;
			u8 data = space->device().memory().translate(space->spacenum(), TRANSLATE_READ_DEBUG, curaddr) ?
				space->read_byte(curaddr) : space->unmap();
			fwrite(&data, 1, 1, f);
		}
		break;
	case  3:
		offset &= ~15;
		endoffset &= ~15;
		for (u64 i = offset; i != endoffset; i+=16)
		{
			offs_t curaddr = i;
			u16 data = space->device().memory().translate(space->spacenum(), TRANSLATE_READ_DEBUG, curaddr) ?
				space->read_word(curaddr) : space->unmap();
			fwrite(&data, 2, 1, f);
		}
		break;
	}

	/* close the file */
	fclose(f);
	m_console.printf("Data saved successfully\n");
}


/*-------------------------------------------------
    execute_saveregion - execute the save command on region memory
-------------------------------------------------*/

void debugger_commands::execute_saveregion(int ref, const std::vector<std::string> &params)
{
	u64 offset, length;
	memory_region *region;

	/* validate parameters */
	if (!validate_number_parameter(params[1], offset))
		return;
	if (!validate_number_parameter(params[2], length))
		return;
	if (!validate_memory_region_parameter(params[3], region))
		return;

	if (offset >= region->bytes())
	{
		m_console.printf("Invalid offset\n");
		return;
	}
	if ((length <= 0) || ((length + offset) >= region->bytes()))
		length = region->bytes() - offset;

	/* open the file */
	FILE *f = fopen(params[0].c_str(), "wb");
	if (!f)
	{
		m_console.printf("Error opening file '%s'\n", params[0]);
		return;
	}
	fwrite(region->base() + offset, 1, length, f);

	fclose(f);
	m_console.printf("Data saved successfully\n");
}


/*-------------------------------------------------
    execute_load - execute the load command
-------------------------------------------------*/

void debugger_commands::execute_load(int ref, const std::vector<std::string> &params)
{
	u64 offset, endoffset, length = 0;
	address_space *space;

	// validate parameters
	if (!validate_number_parameter(params[1], offset))
		return;
	if (params.size() > 2 && !validate_number_parameter(params[2], length))
		return;
	if (!validate_cpu_space_parameter((params.size() > 3) ? params[3].c_str() : nullptr, ref, space))
		return;

	// open the file
	std::ifstream f;
	f.open(params[0], std::ifstream::in | std::ifstream::binary);
	if (f.fail())
	{
		m_console.printf("Error opening file '%s'\n", params[0]);
		return;
	}

	// determine the file size, if not specified
	if (params.size() <= 2)
	{
		f.seekg(0, std::ios::end);
		length = f.tellg();
		f.seekg(0);
		if (space->addr_shift() < 0)
			length >>= -space->addr_shift();
		else if (space->addr_shift() > 0)
			length <<= space->addr_shift();
	}

	// determine the addresses to read
	endoffset = (offset + length - 1) & space->addrmask();
	offset = offset & space->addrmask();
	u64 i = 0;
	// now read the data in, ignore endoffset and load entire file if length has been set to zero (offset-1)
	switch (space->addr_shift())
	{
	case -3:
		for (i = offset; f.good() && (i <= endoffset || endoffset == offset - 1); i++)
		{
			offs_t curaddr = i;
			u64 data;
			f.read((char *)&data, 8);
			if (f && space->device().memory().translate(space->spacenum(), TRANSLATE_WRITE_DEBUG, curaddr))
				space->write_qword(curaddr, data);
		}
		break;
	case -2:
		for (i = offset; f.good() && (i <= endoffset || endoffset == offset - 1); i++)
		{
			offs_t curaddr = i;
			u32 data;
			f.read((char *)&data, 4);
			if (f && space->device().memory().translate(space->spacenum(), TRANSLATE_WRITE_DEBUG, curaddr))
				space->write_dword(curaddr, data);
		}
		break;
	case -1:
		for (i = offset; f.good() && (i <= endoffset || endoffset == offset - 1); i++)
		{
			offs_t curaddr = i;
			u16 data;
			f.read((char *)&data, 2);
			if (f && space->device().memory().translate(space->spacenum(), TRANSLATE_WRITE_DEBUG, curaddr))
				space->write_word(curaddr, data);
		}
		break;
	case  0:
		for (i = offset; f.good() && (i <= endoffset || endoffset == offset - 1); i++)
		{
			offs_t curaddr = i;
			u8 data;
			f.read((char *)&data, 1);
			if (f && space->device().memory().translate(space->spacenum(), TRANSLATE_WRITE_DEBUG, curaddr))
				space->write_byte(curaddr, data);
		}
		break;
	case  3:
		offset &= ~15;
		endoffset &= ~15;
		for (i = offset; f.good() && (i <= endoffset || endoffset == offset - 16); i+=16)
		{
			offs_t curaddr = i;
			u16 data;
			f.read((char *)&data, 2);
			if (f && space->device().memory().translate(space->spacenum(), TRANSLATE_WRITE_DEBUG, curaddr))
				space->write_word(curaddr, data);
		}
		break;
	}

	if (!f.good())
		m_console.printf("I/O error, load failed\n");
	else if (i == offset)
		m_console.printf("Length specified too large, load failed\n");
	else
		m_console.printf("Data loaded successfully to memory : 0x%X to 0x%X\n", offset, i-1);
}


/*-------------------------------------------------
    execute_loadregion - execute the load command on region memory
-------------------------------------------------*/

void debugger_commands::execute_loadregion(int ref, const std::vector<std::string> &params)
{
	u64 offset, length;
	memory_region *region;

	/* validate parameters */
	if (!validate_number_parameter(params[1], offset))
		return;
	if (!validate_number_parameter(params[2], length))
		return;
	if (!validate_memory_region_parameter(params[3], region))
		return;

	if (offset >= region->bytes())
	{
		m_console.printf("Invalid offset\n");
		return;
	}
	if ((length <= 0) || ((length + offset) >= region->bytes()))
		length = region->bytes() - offset;

	/* open the file */
	FILE *f = fopen(params[0].c_str(), "rb");
	if (!f)
	{
		m_console.printf("Error opening file '%s'\n", params[0]);
		return;
	}

	fseek(f, 0L, SEEK_END);
	u64 size = ftell(f);
	rewind(f);

	// check file size
	if (length >= size)
		length = size;

	fread(region->base() + offset, 1, length, f);

	fclose(f);
	m_console.printf("Data loaded successfully to memory : 0x%X to 0x%X\n", offset, offset + length - 1);
}


/*-------------------------------------------------
    execute_dump - execute the dump command
-------------------------------------------------*/

void debugger_commands::execute_dump(int ref, const std::vector<std::string> &params)
{
	/* validate parameters */
	u64 offset;
	if (!validate_number_parameter(params[1], offset))
		return;

	u64 length;
	if (!validate_number_parameter(params[2], length))
		return;

	u64 width = 0;
	if (params.size() > 3 && !validate_number_parameter(params[3], width))
		return;

	u64 ascii = 1;
	if (params.size() > 4 && !validate_number_parameter(params[4], ascii))
		return;

	address_space *space;
	if (!validate_cpu_space_parameter((params.size() > 6) ? params[6].c_str() : nullptr, ref, space))
		return;

	u64 rowsize = space->byte_to_address(16);
	if (params.size() > 5 && !validate_number_parameter(params[5], rowsize))
		return;

	int shift = space->addr_shift();
	u64 granularity = shift >= 0 ? 1 : 1 << -shift;

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
	if (width < granularity)
	{
		m_console.printf("Invalid width! (must be at least %d)\n", granularity);
		return;
	}
	if (rowsize == 0 || (rowsize % space->byte_to_address(width)) != 0)
	{
		m_console.printf("Invalid row size! (must be a positive multiple of %d)\n", space->byte_to_address(width));
		return;
	}

	u64 endoffset = (offset + length - 1) & space->addrmask();
	offset = offset & space->addrmask();

	/* open the file */
	FILE* f = fopen(params[0].c_str(), "w");
	if (!f)
	{
		m_console.printf("Error opening file '%s'\n", params[0]);
		return;
	}

	/* now write the data out */
	util::ovectorstream output;
	output.reserve(200);

	const unsigned delta = (shift >= 0) ? (width << shift) : (width >> -shift);

	auto dis = space->device().machine().disable_side_effects();
	bool be = space->endianness() == ENDIANNESS_BIG;

	for (u64 i = offset; i <= endoffset; i += rowsize)
	{
		output.clear();
		output.rdbuf()->clear();

		/* print the address */
		util::stream_format(output, "%0*X: ", space->logaddrchars(), i);

		/* print the bytes */
		for (u64 j = 0; j < rowsize; j += delta)
		{
			if (i + j <= endoffset)
			{
				offs_t curaddr = i + j;
				if (space->device().memory().translate(space->spacenum(), TRANSLATE_READ_DEBUG, curaddr))
				{
					switch (width)
					{
					case 8:
						util::stream_format(output, " %016X", space->read_qword_unaligned(i+j));
						break;
					case 4:
						util::stream_format(output, " %08X", space->read_dword_unaligned(i+j));
						break;
					case 2:
						util::stream_format(output, " %04X", space->read_word_unaligned(i+j));
						break;
					case 1:
						util::stream_format(output, " %02X", space->read_byte(i+j));
						break;
					}
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
			for (u64 j = 0; j < rowsize && (i + j) <= endoffset; j += delta)
			{
				offs_t curaddr = i + j;
				if (space->device().memory().translate(space->spacenum(), TRANSLATE_READ_DEBUG, curaddr))
				{
					u64 data = 0;
					switch (width)
					{
					case 8:
						data = space->read_qword_unaligned(i+j);
						break;
					case 4:
						data = space->read_dword_unaligned(i+j);
						break;
					case 2:
						data = space->read_word_unaligned(i+j);
						break;
					case 1:
						data = space->read_byte(i+j);
						break;
					}
					for (unsigned int b = 0; b != width; b++) {
						u8 byte = data >> (8 * (be ? (width-1-b) : b));
						util::stream_format(output, "%c", (byte >= 32 && byte < 127) ? byte : '.');
					}
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


//-------------------------------------------------
//  execute_strdump - execute the strdump command
//-------------------------------------------------

void debugger_commands::execute_strdump(int ref, const std::vector<std::string> &params)
{
	// validate parameters
	u64 offset;
	if (!validate_number_parameter(params[1], offset))
		return;

	u64 length;
	if (!validate_number_parameter(params[2], length))
		return;

	u64 term = 0;
	if (params.size() > 3 && !validate_number_parameter(params[3], term))
		return;

	address_space *space;
	if (!validate_cpu_space_parameter((params.size() > 4) ? params[4].c_str() : nullptr, ref, space))
		return;

	// further validation
	if (term >= 0x100 && term != u64(-0x80))
	{
		m_console.printf("Invalid termination character\n");
		return;
	}

	// open the file
	FILE *f = fopen(params[0].c_str(), "w");
	if (!f)
	{
		m_console.printf("Error opening file '%s'\n", params[0]);
		return;
	}

	const int shift = space->addr_shift();
	const unsigned delta = (shift >= 0) ? (1 << shift) : 1;
	const unsigned width = (shift >= 0) ? 1 : (1 << -shift);
	const bool be = space->endianness() == ENDIANNESS_BIG;

	offset = offset & space->addrmask();
	if (shift > 0)
		length >>= shift;

	// now write the data out
	util::ovectorstream output;
	output.reserve(200);

	auto dis = space->device().machine().disable_side_effects();

	bool terminated = true;
	while (length-- != 0)
	{
		if (terminated)
		{
			terminated = false;
			output.clear();
			output.rdbuf()->clear();

			// print the address
			util::stream_format(output, "%0*X: \"", space->logaddrchars(), offset);
		}

		// get the character data
		u64 data = 0;
		offs_t curaddr = offset;
		if (space->device().memory().translate(space->spacenum(), TRANSLATE_READ_DEBUG, curaddr))
		{
			switch (width)
			{
			case 1:
				data = space->read_byte(curaddr);
				break;

			case 2:
				data = space->read_word(curaddr);
				if (be)
					data = swapendian_int16(data);
				break;

			case 4:
				data = space->read_dword(curaddr);
				if (be)
					data = swapendian_int32(data);
				break;

			case 8:
				data = space->read_qword(curaddr);
				if (be)
					data = swapendian_int64(data);
				break;
			}
		}

		// print the characters
		for (int n = 0; n < width; n++)
		{
			// check for termination within word
			if (terminated)
			{
				terminated = false;

				// output the result
				auto const &text = output.vec();
				fprintf(f, "%.*s\"\n", int(unsigned(text.size())), &text[0]);
				output.clear();
				output.rdbuf()->clear();

				// print the address
				util::stream_format(output, "%0*X.%d: \"", space->logaddrchars(), offset, n);
			}

			u8 ch = data & 0xff;
			data >>= 8;

			// check for termination
			if (term == u64(-0x80))
			{
				if (BIT(ch, 7))
				{
					terminated = true;
					ch &= 0x7f;
				}
			}
			else if (ch == term)
			{
				terminated = true;
				continue;
			}

			// check for non-ASCII characters
			if (ch < 0x20 || ch >= 0x7f)
			{
				// use special or octal escape
				if (ch >= 0x07 && ch <= 0x0d)
					util::stream_format(output, "\\%c", "abtnvfr"[ch - 0x07]);
				else
					util::stream_format(output, "\\%03o", ch);
			}
			else
			{
				if (ch == '"' || ch == '\\')
					output << '\\';
				output << char(ch);
			}
		}

		if (terminated)
		{
			// output the result
			auto const &text = output.vec();
			fprintf(f, "%.*s\"\n", int(unsigned(text.size())), &text[0]);
			output.clear();
			output.rdbuf()->clear();
		}

		offset += delta;
	}

	if (!terminated)
	{
		// output the result
		auto const &text = output.vec();
		fprintf(f, "%.*s\"\\\n", int(unsigned(text.size())), &text[0]);
	}

	// close the file
	fclose(f);
	m_console.printf("Data dumped successfully\n");
}


/*-------------------------------------------------
   execute_cheatinit - initialize the cheat system
-------------------------------------------------*/

void debugger_commands::execute_cheatinit(int ref, const std::vector<std::string> &params)
{
	u64 offset, length = 0, real_length = 0;
	address_space *space;
	u32 active_cheat = 0;
	u64 curaddr;
	u8 i, region_count = 0;

	cheat_region_map cheat_region[100];

	memset(cheat_region, 0, sizeof(cheat_region));

	/* validate parameters */
	if (!validate_cpu_space_parameter((params.size() > 3) ? params[3].c_str() : nullptr, AS_PROGRAM, space))
		return;

	if (ref == 0)
	{
		m_cheat.width = 1;
		m_cheat.signed_cheat = false;
		m_cheat.swapped_cheat = false;
		if (!params.empty())
		{
			char *srtpnt = (char*)params[0].c_str();

			char sspec = std::tolower((unsigned char)*srtpnt);
			if (sspec == 's')
				m_cheat.signed_cheat = true;
			else if (sspec == 'u')
				m_cheat.signed_cheat = false;
			else
			{
				m_console.printf("Invalid sign: expected s or u\n");
				return;
			}

			char wspec = std::tolower((unsigned char)*(++srtpnt));
			if (wspec == 'b')
				m_cheat.width = 1;
			else if (wspec == 'w')
				m_cheat.width = 2;
			else if (wspec == 'd')
				m_cheat.width = 4;
			else if (wspec == 'q')
				m_cheat.width = 8;
			else
			{
				m_console.printf("Invalid width: expected b, w, d or q\n");
				return;
			}

			if (std::tolower((unsigned char)*(++srtpnt)) == 's')
				m_cheat.swapped_cheat = true;
			else
				m_cheat.swapped_cheat = false;
		}
	}

	/* initialize entire memory by default */
	if (params.size() <= 1)
	{
		for (address_map_entry &entry : space->map()->m_entrylist)
		{
			cheat_region[region_count].offset = entry.m_addrstart & space->addrmask();
			cheat_region[region_count].endoffset = entry.m_addrend & space->addrmask();
			cheat_region[region_count].share = entry.m_share;
			cheat_region[region_count].disabled = (entry.m_write.m_type == AMH_RAM) ? false : true;

			/* disable double share regions */
			if (entry.m_share != nullptr)
				for (i = 0; i < region_count; i++)
					if (cheat_region[i].share != nullptr)
						if (strcmp(cheat_region[i].share, entry.m_share) == 0)
							cheat_region[region_count].disabled = true;

			region_count++;
		}
	}
	else
	{
		/* validate parameters */
		if (!validate_number_parameter(params[(ref == 0) ? 1 : 0], offset))
			return;
		if (!validate_number_parameter(params[(ref == 0) ? 2 : 1], length))
			return;

		/* force region to the specified range */
		cheat_region[region_count].offset = offset & space->addrmask();
		cheat_region[region_count].endoffset = (offset + length - 1) & space->addrmask();
		cheat_region[region_count].share = nullptr;
		cheat_region[region_count].disabled = false;
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
		m_cheat.cpu[0] = params.size() > 3 ? params[3][0] : '0';
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
	validate_cpu_parameter(m_cheat.cpu, cpu);
	m_console.printf("%u cheat initialized for CPU index %s ( aka %s )\n", active_cheat, m_cheat.cpu, cpu->tag());
}


/*-------------------------------------------------
    execute_cheatnext - execute the search
-------------------------------------------------*/

void debugger_commands::execute_cheatnext(int ref, const std::vector<std::string> &params)
{
	address_space *space;
	u64 cheatindex;
	u32 active_cheat = 0;
	u8 condition;
	u64 comp_value = 0;

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

	if (params.size() > 1 && !validate_number_parameter(params[1], comp_value))
		return;
	comp_value = cheat_sign_extend(&m_cheat, comp_value);

	/* decode condition */
	if (!core_stricmp(params[0].c_str(), "all"))
		condition = CHEAT_ALL;
	else if (!core_stricmp(params[0].c_str(), "equal") || !core_stricmp(params[0].c_str(), "eq"))
		condition = (params.size() > 1) ? CHEAT_EQUALTO : CHEAT_EQUAL;
	else if (!core_stricmp(params[0].c_str(), "notequal") || !core_stricmp(params[0].c_str(), "ne"))
		condition = (params.size() > 1) ? CHEAT_NOTEQUALTO : CHEAT_NOTEQUAL;
	else if (!core_stricmp(params[0].c_str(), "decrease") || !core_stricmp(params[0].c_str(), "de") || params[0] == "-")
		condition = (params.size() > 1) ? CHEAT_DECREASEOF : CHEAT_DECREASE;
	else if (!core_stricmp(params[0].c_str(), "increase") || !core_stricmp(params[0].c_str(), "in") || params[0] == "+")
		condition = (params.size() > 1) ? CHEAT_INCREASEOF : CHEAT_INCREASE;
	else if (!core_stricmp(params[0].c_str(), "decreaseorequal") || !core_stricmp(params[0].c_str(), "deeq"))
		condition = CHEAT_DECREASE_OR_EQUAL;
	else if (!core_stricmp(params[0].c_str(), "increaseorequal") || !core_stricmp(params[0].c_str(), "ineq"))
		condition = CHEAT_INCREASE_OR_EQUAL;
	else if (!core_stricmp(params[0].c_str(), "smallerof") || !core_stricmp(params[0].c_str(), "lt") || params[0] == "<")
		condition = CHEAT_SMALLEROF;
	else if (!core_stricmp(params[0].c_str(), "greaterof") || !core_stricmp(params[0].c_str(), "gt") || params[0] == ">")
		condition = CHEAT_GREATEROF;
	else if (!core_stricmp(params[0].c_str(), "changedby") || !core_stricmp(params[0].c_str(), "ch") || params[0] == "~")
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
			u64 cheat_value = cheat_read_extended(&m_cheat, *space, m_cheat.cheatmap[cheatindex].offset);
			u64 comp_byte = (ref == 0) ? m_cheat.cheatmap[cheatindex].previous_value : m_cheat.cheatmap[cheatindex].first_value;
			u8 disable_byte = false;

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
						disable_byte = (s64(cheat_value) >= s64(comp_byte));
					else
						disable_byte = (u64(cheat_value) >= u64(comp_byte));
					break;

				case CHEAT_INCREASE:
					if (m_cheat.signed_cheat)
						disable_byte = (s64(cheat_value) <= s64(comp_byte));
					else
						disable_byte = (u64(cheat_value) <= u64(comp_byte));
					break;

				case CHEAT_DECREASE_OR_EQUAL:
					if (m_cheat.signed_cheat)
						disable_byte = (s64(cheat_value) > s64(comp_byte));
					else
						disable_byte = (u64(cheat_value) > u64(comp_byte));
					break;

				case CHEAT_INCREASE_OR_EQUAL:
					if (m_cheat.signed_cheat)
						disable_byte = (s64(cheat_value) < s64(comp_byte));
					else
						disable_byte = (u64(cheat_value) < u64(comp_byte));
					break;

				case CHEAT_DECREASEOF:
					disable_byte = (cheat_value != comp_byte - comp_value);
					break;

				case CHEAT_INCREASEOF:
					disable_byte = (cheat_value != comp_byte + comp_value);
					break;

				case CHEAT_SMALLEROF:
					if (m_cheat.signed_cheat)
						disable_byte = (s64(cheat_value) >= s64(comp_value));
					else
						disable_byte = (u64(cheat_value) >= u64(comp_value));
					break;

				case CHEAT_GREATEROF:
					if (m_cheat.signed_cheat)
						disable_byte = (s64(cheat_value) <= s64(comp_value));
					else
						disable_byte = (u64(cheat_value) <= u64(comp_value));
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
		execute_cheatlist(0, std::vector<std::string>());

	m_console.printf("%u cheats found\n", active_cheat);
}


/*-------------------------------------------------
    execute_cheatlist - show a list of active cheat
-------------------------------------------------*/

void debugger_commands::execute_cheatlist(int ref, const std::vector<std::string> &params)
{
	char spaceletter, sizeletter;
	address_space *space;
	device_t *cpu;
	u32 active_cheat = 0;
	u64 cheatindex;
	u64 sizemask;
	FILE *f = nullptr;

	if (m_cheat.cpu[0] == 0)
	{
		m_console.printf("Use cheatinit before cheatlist\n");
		return;
	}

	if (!validate_cpu_space_parameter(m_cheat.cpu, AS_PROGRAM, space))
		return;

	if (!validate_cpu_parameter(m_cheat.cpu, cpu))
		return;

	if (params.size() > 0)
		f = fopen(params[0].c_str(), "w");

	switch (space->spacenum())
	{
		default:
		case AS_PROGRAM:    spaceletter = 'p';  break;
		case AS_DATA:   spaceletter = 'd';  break;
		case AS_IO:     spaceletter = 'i';  break;
		case AS_OPCODES: spaceletter = 'o'; break;
	}

	switch (m_cheat.width)
	{
		default:
		case 1:                     sizeletter = 'b';   sizemask = 0xffU;               break;
		case 2:                     sizeletter = 'w';   sizemask = 0xffffU;             break;
		case 4:                     sizeletter = 'd';   sizemask = 0xffffffffU;         break;
		case 8:                     sizeletter = 'q';   sizemask = 0xffffffffffffffffU; break;
	}

	/* write the cheat list */
	util::ovectorstream output;
	for (cheatindex = 0; cheatindex < m_cheat.cheatmap.size(); cheatindex += 1)
	{
		if (m_cheat.cheatmap[cheatindex].state == 1)
		{
			u64 value = cheat_byte_swap(&m_cheat, cheat_read_extended(&m_cheat, *space, m_cheat.cheatmap[cheatindex].offset)) & sizemask;
			offs_t address = space->byte_to_address(m_cheat.cheatmap[cheatindex].offset);

			if (!params.empty())
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
	if (params.size() > 0)
		fclose(f);
}


/*-------------------------------------------------
    execute_cheatundo - undo the last search
-------------------------------------------------*/

void debugger_commands::execute_cheatundo(int ref, const std::vector<std::string> &params)
{
	u64 cheatindex;
	u32 undo_count = 0;

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

void debugger_commands::execute_find(int ref, const std::vector<std::string> &params)
{
	u64 offset, endoffset, length;
	address_space *space;
	u64 data_to_find[256];
	u8 data_size[256];
	int cur_data_size;
	int data_count = 0;
	int found = 0;

	/* validate parameters */
	if (!validate_number_parameter(params[0], offset))
		return;
	if (!validate_number_parameter(params[1], length))
		return;
	if (!validate_cpu_space_parameter(nullptr, ref, space))
		return;

	/* further validation */
	endoffset = space->address_to_byte_end((offset + length - 1) & space->addrmask());
	offset = space->address_to_byte(offset & space->addrmask());
	cur_data_size = space->addr_shift() > 0 ? 2 : 1 << -space->addr_shift();
	if (cur_data_size == 0)
		cur_data_size = 1;

	/* parse the data parameters */
	for (int i = 2; i < params.size(); i++)
	{
		const char *pdata = params[i].c_str();
		size_t pdatalen = strlen(pdata) - 1;

		/* check for a string */
		if (pdata[0] == '"' && pdata[pdatalen] == '"')
		{
			for (int j = 1; j < pdatalen; j++)
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
			if (tolower(u8(pdata[0])) == 'b' && pdata[1] == '.') { data_size[data_count] = cur_data_size = 1; pdata += 2; }
			if (tolower(u8(pdata[0])) == 'w' && pdata[1] == '.') { data_size[data_count] = cur_data_size = 2; pdata += 2; }
			if (tolower(u8(pdata[0])) == 'd' && pdata[1] == '.') { data_size[data_count] = cur_data_size = 4; pdata += 2; }
			if (tolower(u8(pdata[0])) == 'q' && pdata[1] == '.') { data_size[data_count] = cur_data_size = 8; pdata += 2; }

			/* look for a wildcard */
			if (!strcmp(pdata, "?"))
				data_size[data_count++] |= 0x10;

			/* otherwise, validate as a number */
			else if (!validate_number_parameter(pdata, data_to_find[data_count++]))
				return;
		}
	}

	/* now search */
	device_memory_interface &memory = space->device().memory();
	auto dis = space->device().machine().disable_side_effects();
	for (u64 i = offset; i <= endoffset; i += data_size[0])
	{
		int suboffset = 0;
		bool match = true;

		/* find the entire string */
		for (int j = 0; j < data_count && match; j++)
		{
			offs_t address = space->byte_to_address(i + suboffset);
			switch (data_size[j])
			{
			case 1:
				address &= space->logaddrmask();
				if (memory.translate(space->spacenum(), TRANSLATE_READ_DEBUG, address))
					match = space->read_byte(address) == u8(data_to_find[j]);
				else
					match = false;
				break;

			case 2:
				address &= space->logaddrmask();
				if (memory.translate(space->spacenum(), TRANSLATE_READ_DEBUG, address))
					match = space->read_word_unaligned(address) == u16(data_to_find[j]);
				else
					match = false;
				break;

			case 4:
				address &= space->logaddrmask();
				if (memory.translate(space->spacenum(), TRANSLATE_READ_DEBUG, address))
					match = space->read_dword_unaligned(address) == u32(data_to_find[j]);
				else
					match = false;
				break;

			case 8:
				address &= space->logaddrmask();
				if (memory.translate(space->spacenum(), TRANSLATE_READ_DEBUG, address))
					match = space->read_qword_unaligned(address) == u64(data_to_find[j]);
				else
					match = false;
				break;

			default:
				/* all other cases are wildcards */
				break;
			}
			suboffset += data_size[j] & 0x0f;
		}

		/* did we find it? */
		if (match)
		{
			found++;
			m_console.printf("Found at %0*X\n", space->addrchars(), u32(space->byte_to_address(i)));
		}
	}

	/* print something if not found */
	if (found == 0)
		m_console.printf("Not found\n");
}


//-------------------------------------------------
//  execute_fill - execute the fill command
//-------------------------------------------------

void debugger_commands::execute_fill(int ref, const std::vector<std::string> &params)
{
	u64 offset, length;
	address_space *space;

	// validate parameters
	if (!validate_number_parameter(params[0], offset))
		return;
	if (!validate_number_parameter(params[1], length))
		return;
	if (!validate_cpu_space_parameter(nullptr, ref, space))
		return;

	// further validation
	offset = space->address_to_byte(offset & space->addrmask());
	int cur_data_size = space->addr_shift() > 0 ? 2 : 1 << -space->addr_shift();
	if (cur_data_size == 0)
		cur_data_size = 1;

	// parse the data parameters
	u64 fill_data[256];
	u8 fill_data_size[256];
	int data_count = 0;
	for (int i = 2; i < params.size(); i++)
	{
		const char *pdata = params[i].c_str();
		size_t pdatalen = strlen(pdata) - 1;

		// check for a string
		if (pdata[0] == '"' && pdata[pdatalen] == '"')
		{
			for (int j = 1; j < pdatalen; j++)
			{
				fill_data[data_count] = pdata[j];
				fill_data_size[data_count++] = 1;
			}
		}

		// otherwise, validate as a number
		else
		{
			// check for a 'b','w','d',or 'q' prefix
			fill_data_size[data_count] = cur_data_size;
			if (tolower(u8(pdata[0])) == 'b' && pdata[1] == '.') { fill_data_size[data_count] = cur_data_size = 1; pdata += 2; }
			if (tolower(u8(pdata[0])) == 'w' && pdata[1] == '.') { fill_data_size[data_count] = cur_data_size = 2; pdata += 2; }
			if (tolower(u8(pdata[0])) == 'd' && pdata[1] == '.') { fill_data_size[data_count] = cur_data_size = 4; pdata += 2; }
			if (tolower(u8(pdata[0])) == 'q' && pdata[1] == '.') { fill_data_size[data_count] = cur_data_size = 8; pdata += 2; }

			// validate as a number
			if (!validate_number_parameter(pdata, fill_data[data_count++]))
				return;
		}
	}
	if (data_count == 0)
		return;

	// now fill memory
	device_memory_interface &memory = space->device().memory();
	auto dis = space->device().machine().disable_side_effects();
	u64 count = space->address_to_byte(length);
	while (count != 0)
	{
		// write the entire string
		for (int j = 0; j < data_count; j++)
		{
			offs_t address = space->byte_to_address(offset) & space->logaddrmask();
			if (!memory.translate(space->spacenum(), TRANSLATE_WRITE_DEBUG, address))
			{
				m_console.printf("Fill aborted due to page fault at %0*X\n", space->logaddrchars(), space->byte_to_address(offset) & space->logaddrmask());
				length = 0;
				break;
			}
			switch (fill_data_size[j])
			{
			case 1:
				space->write_byte(address, fill_data[j]);
				break;

			case 2:
				space->write_word_unaligned(address, fill_data[j]);
				break;

			case 4:
				space->write_dword_unaligned(address, fill_data[j]);
				break;

			case 8:
				space->read_qword_unaligned(address, fill_data[j]);
				break;
			}
			offset += fill_data_size[j];
			if (count <= fill_data_size[j])
			{
				count = 0;
				break;
			}
			else
				count -= fill_data_size[j];
		}
	}
}


/*-------------------------------------------------
    execute_dasm - execute the dasm command
-------------------------------------------------*/

void debugger_commands::execute_dasm(int ref, const std::vector<std::string> &params)
{
	u64 offset, length, bytes = 1;
	address_space *space;

	/* validate parameters */
	if (!validate_number_parameter(params[1], offset))
		return;
	if (!validate_number_parameter(params[2], length))
		return;
	if (params.size() > 3 && !validate_number_parameter(params[3], bytes))
		return;
	if (!validate_cpu_space_parameter(params.size() > 4 ? params[4].c_str() : nullptr, AS_PROGRAM, space))
		return;

	/* determine the width of the bytes */
	device_disasm_interface *dasmintf;
	if (!space->device().interface(dasmintf))
	{
		m_console.printf("No disassembler available for %s\n", space->device().name());
		return;
	}

	/* build the data, check the maximum size of the opcodes and disasm */
	std::vector<offs_t> pcs;
	std::vector<std::string> instructions;
	std::vector<std::string> tpc;
	std::vector<std::string> topcodes;
	int max_opcodes_size = 0;
	int max_disasm_size = 0;

	debug_disasm_buffer buffer(space->device());

	for (u64 i = 0; i < length; )
	{
		std::string instruction;
		offs_t next_offset;
		offs_t size;
		u32 info;
		buffer.disassemble(offset, instruction, next_offset, size, info);
		pcs.push_back(offset);
		instructions.emplace_back(instruction);
		tpc.emplace_back(buffer.pc_to_string(offset));
		topcodes.emplace_back(buffer.data_to_string(offset, size, true));

		int osize = topcodes.back().size();
		if(osize > max_opcodes_size)
			max_opcodes_size = osize;

		int dsize = instructions.back().size();
		if(dsize > max_disasm_size)
			max_disasm_size = dsize;

		i += size;
		offset = next_offset;
	}

	/* write the data */
	std::ofstream f(params[0]);
	if (!f.good())
	{
		m_console.printf("Error opening file '%s'\n", params[0]);
		return;
	}

	if (bytes)
	{
		for(unsigned int i=0; i != pcs.size(); i++)
		{
			const char *comment = space->device().debug()->comment_text(pcs[i]);
			if (comment)
				util::stream_format(f, "%s: %-*s %-*s // %s\n", tpc[i], max_opcodes_size, topcodes[i], max_disasm_size, instructions[i], comment);
			else
				util::stream_format(f, "%s: %-*s %s\n", tpc[i], max_opcodes_size, topcodes[i], instructions[i]);
		}
	}
	else
	{
		for(unsigned int i=0; i != pcs.size(); i++)
		{
			const char *comment = space->device().debug()->comment_text(pcs[i]);
			if (comment)
				util::stream_format(f, "%s: %-*s // %s\n", tpc[i], max_disasm_size, instructions[i], comment);
			else
				util::stream_format(f, "%s: %s\n", tpc[i], instructions[i]);
		}
	}

	m_console.printf("Data dumped successfully\n");
}


/*-------------------------------------------------
    execute_trace_internal - functionality for
    trace over and trace info
-------------------------------------------------*/

void debugger_commands::execute_trace_internal(int ref, const std::vector<std::string> &params, bool trace_over)
{
	const char *action = nullptr;
	bool detect_loops = true;
	bool logerror = false;
	device_t *cpu;
	FILE *f = nullptr;
	const char *mode;
	std::string filename = params[0];

	/* replace macros */
	strreplace(filename, "{game}", m_machine.basename());

	/* validate parameters */
	if (!validate_cpu_parameter(params.size() > 1 ? params[1].c_str() : nullptr, cpu))
		return;
	if (params.size() > 2)
	{
		std::stringstream stream;
		stream.str(params[2]);

		std::string flag;
		while (std::getline(stream, flag, '|'))
		{
			if (!core_stricmp(flag.c_str(), "noloop"))
				detect_loops = false;
			else if (!core_stricmp(flag.c_str(), "logerror"))
				logerror = true;
			else
			{
				m_console.printf("Invalid flag '%s'\n", flag);
				return;
			}
		}
	}
	if (!debug_command_parameter_command(action = (params.size() > 3) ? params[3].c_str() : nullptr))
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
			m_console.printf("Error opening file '%s'\n", params[0]);
			return;
		}
	}

	/* do it */
	cpu->debug()->trace(f, trace_over, detect_loops, logerror, action);
	if (f)
		m_console.printf("Tracing CPU '%s' to file %s\n", cpu->tag(), filename);
	else
		m_console.printf("Stopped tracing on CPU '%s'\n", cpu->tag());
}


/*-------------------------------------------------
    execute_trace - execute the trace command
-------------------------------------------------*/

void debugger_commands::execute_trace(int ref, const std::vector<std::string> &params)
{
	execute_trace_internal(ref, params, false);
}


/*-------------------------------------------------
    execute_traceover - execute the trace over command
-------------------------------------------------*/

void debugger_commands::execute_traceover(int ref, const std::vector<std::string> &params)
{
	execute_trace_internal(ref, params, true);
}


/*-------------------------------------------------
    execute_traceflush - execute the trace flush command
-------------------------------------------------*/

void debugger_commands::execute_traceflush(int ref, const std::vector<std::string> &params)
{
	m_machine.debugger().cpu().flush_traces();
}


/*-------------------------------------------------
    execute_history - execute the history command
-------------------------------------------------*/

void debugger_commands::execute_history(int ref, const std::vector<std::string> &params)
{
	/* validate parameters */
	address_space *space;
	if (!validate_cpu_space_parameter(!params.empty() ? params[0].c_str() : nullptr, AS_PROGRAM, space))
		return;

	u64 count = device_debug::HISTORY_SIZE;
	if (params.size() > 1 && !validate_number_parameter(params[1], count))
		return;

	/* further validation */
	if (count > device_debug::HISTORY_SIZE)
		count = device_debug::HISTORY_SIZE;

	device_debug *debug = space->device().debug();

	/* loop over lines */
	device_disasm_interface *dasmintf;
	if (!space->device().interface(dasmintf))
	{
		m_console.printf("No disassembler available for %s\n", space->device().name());
		return;
	}

	debug_disasm_buffer buffer(space->device());

	for (int index = 0; index < (int) count; index++)
	{
		offs_t pc = debug->history_pc(-index);
		std::string instruction;
		offs_t next_offset;
		offs_t size;
		u32 info;
		buffer.disassemble(pc, instruction, next_offset, size, info);

		m_console.printf("%s: %s\n", buffer.pc_to_string(pc), instruction);
	}
}


/*-------------------------------------------------
    execute_trackpc - execute the trackpc command
-------------------------------------------------*/

void debugger_commands::execute_trackpc(int ref, const std::vector<std::string> &params)
{
	// Gather the on/off switch (if present)
	bool turnOn = true;
	if (params.size() > 0 && !validate_boolean_parameter(params[0], turnOn))
		return;

	// Gather the cpu id (if present)
	device_t *cpu = nullptr;
	if (!validate_cpu_parameter((params.size() > 1) ? params[1].c_str() : nullptr, cpu))
		return;

	const device_state_interface *state;
	if (!cpu->interface(state))
	{
		m_console.printf("Device has no PC to be tracked\n");
		return;
	}

	// Should we clear the existing data?
	bool clear = false;
	if (params.size() > 2 && !validate_boolean_parameter(params[2], clear))
		return;

	cpu->debug()->set_track_pc((bool)turnOn);
	if (turnOn)
	{
		// Insert current pc
		if (m_console.get_visible_cpu() == cpu)
		{
			const offs_t pc = state->pcbase();
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

void debugger_commands::execute_trackmem(int ref, const std::vector<std::string> &params)
{
	// Gather the on/off switch (if present)
	bool turnOn = true;
	if (params.size() > 0 && !validate_boolean_parameter(params[0], turnOn))
		return;

	// Gather the cpu id (if present)
	device_t *cpu = nullptr;
	if (!validate_cpu_parameter((params.size() > 1) ? params[1].c_str() : nullptr, cpu))
		return;

	// Should we clear the existing data?
	bool clear = false;
	if (params.size() > 2 && !validate_boolean_parameter(params[2], clear))
		return;

	// Get the address space for the given cpu
	address_space *space;
	if (!validate_cpu_space_parameter((params.size() > 1) ? params[1].c_str() : nullptr, AS_PROGRAM, space))
		return;

	// Inform the CPU it's time to start tracking memory writes
	cpu->debug()->set_track_mem(turnOn);

	// Clear out the existing data if requested
	if (clear)
		space->device().debug()->track_mem_data_clear();
}


/*-------------------------------------------------
    execute_pcatmem - execute the pcatmem command
-------------------------------------------------*/

void debugger_commands::execute_pcatmem(int ref, const std::vector<std::string> &params)
{
	// Gather the required address parameter
	u64 address;
	if (!validate_number_parameter(params[0], address))
		return;

	// Gather the cpu id (if present)
	device_t *cpu = nullptr;
	if (!validate_cpu_parameter((params.size() > 1) ? params[1].c_str() : nullptr, cpu))
		return;

	// Get the address space for the given cpu
	address_space *space;
	if (!validate_cpu_space_parameter((params.size() > 1) ? params[1].c_str() : nullptr, ref, space))
		return;

	// Translate the address
	offs_t a = address & space->logaddrmask();
	if (!space->device().memory().translate(space->spacenum(), TRANSLATE_READ_DEBUG, a))
	{
		m_console.printf("Bad address\n");
		return;
	}

	// Get the value of memory at the address
	u64 data = space->unmap();
	auto dis = space->device().machine().disable_side_effects();
	switch (space->data_width())
	{
	case 8:
		data = space->read_byte(a);
		break;

	case 16:
		data = space->read_word_unaligned(a);
		break;

	case 32:
		data = space->read_dword_unaligned(a);
		break;

	case 64:
		data = space->read_qword_unaligned(a);
		break;
	}

	// Recover the pc & print
	const int space_num = (int)ref;
	const offs_t result = space->device().debug()->track_mem_pc_from_space_address_data(space_num, address, data);
	if (result != (offs_t)(-1))
		m_console.printf("%02x\n", result);
	else
		m_console.printf("UNKNOWN PC\n");
}


/*-------------------------------------------------
    execute_snap - execute the snapshot command
-------------------------------------------------*/

void debugger_commands::execute_snap(int ref, const std::vector<std::string> &params)
{
	/* if no params, use the default behavior */
	if (params.empty())
	{
		m_machine.video().save_active_screen_snapshots();
		m_console.printf("Saved snapshot\n");
	}

	/* otherwise, we have to open the file ourselves */
	else
	{
		const char *filename = params[0].c_str();
		int scrnum = (params.size() > 1) ? atoi(params[1].c_str()) : 0;

		screen_device_enumerator iter(m_machine.root_device());
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
		osd_file::error filerr = file.open(std::move(fname));

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

void debugger_commands::execute_source(int ref, const std::vector<std::string> &params)
{
	m_console.source_script(params[0].c_str());
}


/*-------------------------------------------------
    execute_map - execute the map command
-------------------------------------------------*/

void debugger_commands::execute_map(int ref, const std::vector<std::string> &params)
{
	address_space *space;
	offs_t taddress;
	u64 address;
	int intention;

	/* validate parameters */
	if (!validate_number_parameter(params[0], address))
		return;

	/* CPU is implicit */
	if (!validate_cpu_space_parameter(nullptr, ref, space))
		return;

	/* do the translation first */
	for (intention = TRANSLATE_READ_DEBUG; intention <= TRANSLATE_FETCH_DEBUG; intention++)
	{
		static const char *const intnames[] = { "Read", "Write", "Fetch" };
		taddress = address & space->addrmask();
		if (space->device().memory().translate(space->spacenum(), intention, taddress))
		{
			std::string mapname = space->get_handler_string((intention == TRANSLATE_WRITE_DEBUG) ? read_or_write::WRITE : read_or_write::READ, taddress);
			m_console.printf(
					"%7s: %0*X logical == %0*X physical -> %s\n",
					intnames[intention & 3],
					space->logaddrchars(), address,
					space->addrchars(), taddress,
					mapname);
		}
		else
			m_console.printf("%7s: %0*X logical is unmapped\n", intnames[intention & 3], space->logaddrchars(), address);
	}
}


/*-------------------------------------------------
    execute_memdump - execute the memdump command
-------------------------------------------------*/

void debugger_commands::execute_memdump(int ref, const std::vector<std::string> &params)
{
	FILE *file;
	const char *filename;

	filename = params.empty() ? "memdump.log" : params[0].c_str();

	m_console.printf("Dumping memory to %s\n", filename);

	file = fopen(filename, "w");
	if (file)
	{
		memory_interface_enumerator iter(m_machine.root_device());
		for (device_memory_interface &memory : iter) {
			for (int space = 0; space != memory.max_space_count(); space++)
				if (memory.has_space(space))
				{
					address_space &sp = memory.space(space);
					bool octal = sp.is_octal();
					int nc = octal ? (sp.addr_width() + 2) / 3 : (sp.addr_width() + 3) / 4;

					std::vector<memory_entry> entries[2];
					sp.dump_maps(entries[0], entries[1]);
					for (int mode = 0; mode < 2; mode ++)
					{
						fprintf(file, "  device %s space %s %s:\n", memory.device().tag(), sp.name(), mode ? "write" : "read");
						for (memory_entry &entry : entries[mode])
						{
							if (octal)
								fprintf(file, "%0*o - %0*o:", nc, entry.start, nc, entry.end);
							else
								fprintf(file, "%0*x - %0*x:", nc, entry.start, nc, entry.end);
							for(const auto &c : entry.context)
								if(c.disabled)
									fprintf(file, " %s[off]", c.view->name().c_str());
								else
									fprintf(file, " %s[%d]", c.view->name().c_str(), c.slot);
							fprintf(file, " %s\n", entry.entry->name().c_str());
						}
						fprintf(file, "\n");
					}
				}
		}
		fclose(file);
	}
}


/*-------------------------------------------------
    execute_symlist - execute the symlist command
-------------------------------------------------*/

void debugger_commands::execute_symlist(int ref, const std::vector<std::string> &params)
{
	device_t *cpu = nullptr;
	const char *namelist[1000];
	symbol_table *symtable;
	int symnum, count = 0;

	if (!params.empty())
	{
		/* validate parameters */
		if (!validate_cpu_parameter(params[0].c_str(), cpu))
			return;
		symtable = &cpu->debug()->symtable();
		m_console.printf("CPU '%s' symbols:\n", cpu->tag());
	}
	else
	{
		symtable = &m_machine.debugger().cpu().global_symtable();
		m_console.printf("Global symbols:\n");
	}

	/* gather names for all symbols */
	for (auto &entry : symtable->entries())
	{
		/* only display "register" type symbols */
		if (!entry.second->is_function())
		{
			namelist[count++] = entry.second->name();
			if (count >= std::size(namelist))
				break;
		}
	}

	/* sort the symbols */
	if (count > 1)
		std::sort(&namelist[0], &namelist[count], [](const char *item1, const char *item2) {
			return strcmp(item1, item2) < 0;
		});

	/* iterate over symbols and print out relevant ones */
	for (symnum = 0; symnum < count; symnum++)
	{
		const symbol_entry *entry = symtable->find(namelist[symnum]);
		assert(entry != nullptr);
		u64 value = entry->value();

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

void debugger_commands::execute_softreset(int ref, const std::vector<std::string> &params)
{
	m_machine.schedule_soft_reset();
}


/*-------------------------------------------------
    execute_hardreset - execute the hardreset command
-------------------------------------------------*/

void debugger_commands::execute_hardreset(int ref, const std::vector<std::string> &params)
{
	m_machine.schedule_hard_reset();
}

/*-------------------------------------------------
    execute_images - lists all image devices with
    mounted files
-------------------------------------------------*/

void debugger_commands::execute_images(int ref, const std::vector<std::string> &params)
{
	image_interface_enumerator iter(m_machine.root_device());
	for (device_image_interface &img : iter)
		m_console.printf("%s: %s\n", img.brief_instance_name(), img.exists() ? img.filename() : "[empty slot]");
	if (iter.first() == nullptr)
		m_console.printf("No image devices in this driver\n");
}

/*-------------------------------------------------
    execute_mount - execute the image mount command
-------------------------------------------------*/

void debugger_commands::execute_mount(int ref, const std::vector<std::string> &params)
{
	bool done = false;
	for (device_image_interface &img : image_interface_enumerator(m_machine.root_device()))
	{
		if (img.brief_instance_name() == params[0])
		{
			if (img.load(params[1]) != image_init_result::PASS)
				m_console.printf("Unable to mount file %s on %s\n", params[1], params[0]);
			else
				m_console.printf("File %s mounted on %s\n", params[1], params[0]);
			done = true;
			break;
		}
	}
	if (!done)
		m_console.printf("There is no image device :%s\n", params[0]);
}

/*-------------------------------------------------
    execute_unmount - execute the image unmount command
-------------------------------------------------*/

void debugger_commands::execute_unmount(int ref, const std::vector<std::string> &params)
{
	bool done = false;
	for (device_image_interface &img : image_interface_enumerator(m_machine.root_device()))
	{
		if (img.brief_instance_name() == params[0])
		{
			img.unload();
			m_console.printf("Unmounted file from : %s\n", params[0]);
			done = true;
			break;
		}
	}
	if (!done)
		m_console.printf("There is no image device :%s\n", params[0]);
}


/*-------------------------------------------------
    execute_input - debugger command to enter
    natural keyboard input
-------------------------------------------------*/

void debugger_commands::execute_input(int ref, const std::vector<std::string> &params)
{
	m_machine.natkeyboard().post_coded(params[0].c_str());
}


/*-------------------------------------------------
    execute_dumpkbd - debugger command to natural
    keyboard codes
-------------------------------------------------*/

void debugger_commands::execute_dumpkbd(int ref, const std::vector<std::string> &params)
{
	// was there a file specified?
	const char *filename = !params.empty() ? params[0].c_str() : nullptr;
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
	std::string buffer = m_machine.natkeyboard().dump();

	// and output it as appropriate
	if (file != nullptr)
		fprintf(file, "%s\n", buffer.c_str());
	else
		m_console.printf("%s\n", buffer);

	// cleanup
	if (file != nullptr)
		fclose(file);
}
