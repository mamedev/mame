// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    debugcmd.cpp

    Debugger command interface engine.

*********************************************************************/

#include "emu.h"
#include "debugcmd.h"

#include "debugbuf.h"
#include "debugcon.h"
#include "debugcpu.h"
#include "debughlp.h"
#include "debugvw.h"
#include "express.h"
#include "points.h"

#include "debugger.h"
#include "emuopts.h"
#include "fileio.h"
#include "natkeyboard.h"
#include "render.h"
#include "screen.h"
#include "softlist.h"

#include "corestr.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>



namespace {

template <typename T>
inline std::string_view::size_type find_delimiter(std::string_view str, T &&is_delim)
{
	unsigned parens = 0;
	for (std::string_view::size_type i = 0; str.length() > i; ++i)
	{
		if (str[i] == '(')
		{
			++parens;
		}
		else if (parens)
		{
			if (str[i] == ')')
				--parens;
		}
		else if (is_delim(str[i]))
		{
			return i;
		}
	}
	return std::string_view::npos;
}

} // anonymous namespace



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

inline u64 debugger_commands::cheat_system::sign_extend(u64 value) const
{
	if (signed_cheat)
	{
		switch (width)
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

inline u64 debugger_commands::cheat_system::byte_swap(u64 value) const
{
	if (swapped_cheat)
	{
		switch (width)
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

u64 debugger_commands::cheat_system::read_extended(offs_t address) const
{
	address &= space->logaddrmask();
	u64 value = space->unmap();
	if (space->device().memory().translate(space->spacenum(), TRANSLATE_READ_DEBUG, address))
	{
		switch (width)
		{
		case 1: value = space->read_byte(address);              break;
		case 2: value = space->read_word_unaligned(address);    break;
		case 4: value = space->read_dword_unaligned(address);   break;
		case 8: value = space->read_qword_unaligned(address);   break;
		}
	}
	return sign_extend(byte_swap(value));
}

debugger_commands::debugger_commands(running_machine& machine, debugger_cpu& cpu, debugger_console& console)
	: m_machine(machine)
	, m_console(console)
{
	using namespace std::placeholders;
	m_global_array = std::make_unique<global_entry []>(MAX_GLOBALS);

	symbol_table &symtable = cpu.global_symtable();

	// add a few simple global functions
	symtable.add("min", 2, 2, // lower of two values
			[] (int params, const u64 *param) -> u64
			{ return (std::min)(param[0], param[1]); });
	symtable.add("max", 2, 2, // higher of two values
			[] (int params, const u64 *param) -> u64
			{ return (std::max)(param[0], param[1]); });
	symtable.add("if", 3, 3, // a ? b : c
			[] (int params, const u64 *param) -> u64
			{ return param[0] ? param[1] : param[2]; });
	symtable.add("abs", 1, 1, // absolute value of signed number
			[] (int params, const u64 *param) -> u64
			{ return std::abs(s64(param[0])); });
	symtable.add("bit", 2, 3, // extract bit field
			[] (int params, const u64 *param) -> u64
			{ return (params == 2) ? BIT(param[0], param[1]) : BIT(param[0], param[1], param[2]); });
	symtable.add("s8", 1, 1, // sign-extend from 8 bits
			[] (int params, const u64 *param) -> u64
			{ return s64(s8(u8(param[0]))); });
	symtable.add("s16", 1, 1, // sign-extend from 16 bits
			[] (int params, const u64 *param) -> u64
			{ return s64(s16(u16(param[0]))); });
	symtable.add("s32", 1, 1, // sign-extend from 32 bits
			[] (int params, const u64 *param) -> u64
			{ return s64(s32(u32(param[0]))); });
	symtable.add("cpunum", std::bind(&debugger_commands::get_cpunum, this));

	// add all single-entry save state globals
	for (int itemnum = 0; itemnum < MAX_GLOBALS; itemnum++)
	{
		void *base;
		u32 valsize, valcount, blockcount, stride;

		// stop when we run out of items
		const char* name = m_machine.save().indexed_item(itemnum, base, valsize, valcount, blockcount, stride);
		if (!name)
			break;

		// if this is a single-entry global, add it
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

	// add all the commands
	m_console.register_command("help",      CMDFLAG_NONE, 0, 1, std::bind(&debugger_commands::execute_help, this, _1));
	m_console.register_command("print",     CMDFLAG_NONE, 1, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_print, this, _1));
	m_console.register_command("printf",    CMDFLAG_NONE, 1, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_printf, this, _1));
	m_console.register_command("logerror",  CMDFLAG_NONE, 1, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_logerror, this, _1));
	m_console.register_command("tracelog",  CMDFLAG_NONE, 1, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_tracelog, this, _1));
	m_console.register_command("tracesym",  CMDFLAG_NONE, 1, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_tracesym, this, _1));
	m_console.register_command("cls",       CMDFLAG_NONE, 0, 0, std::bind(&debugger_commands::execute_cls, this, _1));
	m_console.register_command("quit",      CMDFLAG_NONE, 0, 0, std::bind(&debugger_commands::execute_quit, this, _1));
	m_console.register_command("exit",      CMDFLAG_NONE, 0, 0, std::bind(&debugger_commands::execute_quit, this, _1));
	m_console.register_command("do",        CMDFLAG_NONE, 1, 1, std::bind(&debugger_commands::execute_do, this, _1));
	m_console.register_command("step",      CMDFLAG_NONE, 0, 1, std::bind(&debugger_commands::execute_step, this, _1));
	m_console.register_command("s",         CMDFLAG_NONE, 0, 1, std::bind(&debugger_commands::execute_step, this, _1));
	m_console.register_command("over",      CMDFLAG_NONE, 0, 1, std::bind(&debugger_commands::execute_over, this, _1));
	m_console.register_command("o",         CMDFLAG_NONE, 0, 1, std::bind(&debugger_commands::execute_over, this, _1));
	m_console.register_command("out" ,      CMDFLAG_NONE, 0, 0, std::bind(&debugger_commands::execute_out, this, _1));
	m_console.register_command("go",        CMDFLAG_NONE, 0, 1, std::bind(&debugger_commands::execute_go, this, _1));
	m_console.register_command("g",         CMDFLAG_NONE, 0, 1, std::bind(&debugger_commands::execute_go, this, _1));
	m_console.register_command("gvblank",   CMDFLAG_NONE, 0, 0, std::bind(&debugger_commands::execute_go_vblank, this, _1));
	m_console.register_command("gv",        CMDFLAG_NONE, 0, 0, std::bind(&debugger_commands::execute_go_vblank, this, _1));
	m_console.register_command("gint",      CMDFLAG_NONE, 0, 1, std::bind(&debugger_commands::execute_go_interrupt, this, _1));
	m_console.register_command("gi",        CMDFLAG_NONE, 0, 1, std::bind(&debugger_commands::execute_go_interrupt, this, _1));
	m_console.register_command("gex",       CMDFLAG_NONE, 0, 2, std::bind(&debugger_commands::execute_go_exception, this, _1));
	m_console.register_command("ge",        CMDFLAG_NONE, 0, 2, std::bind(&debugger_commands::execute_go_exception, this, _1));
	m_console.register_command("gtime",     CMDFLAG_NONE, 0, 1, std::bind(&debugger_commands::execute_go_time, this, _1));
	m_console.register_command("gt",        CMDFLAG_NONE, 0, 1, std::bind(&debugger_commands::execute_go_time, this, _1));
	m_console.register_command("gp",        CMDFLAG_NONE, 0, 1, std::bind(&debugger_commands::execute_go_privilege, this, _1));
	m_console.register_command("next",      CMDFLAG_NONE, 0, 0, std::bind(&debugger_commands::execute_next, this, _1));
	m_console.register_command("n",         CMDFLAG_NONE, 0, 0, std::bind(&debugger_commands::execute_next, this, _1));
	m_console.register_command("focus",     CMDFLAG_NONE, 1, 1, std::bind(&debugger_commands::execute_focus, this, _1));
	m_console.register_command("ignore",    CMDFLAG_NONE, 0, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_ignore, this, _1));
	m_console.register_command("observe",   CMDFLAG_NONE, 0, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_observe, this, _1));
	m_console.register_command("suspend",   CMDFLAG_NONE, 0, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_suspend, this, _1));
	m_console.register_command("resume",    CMDFLAG_NONE, 0, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_resume, this, _1));
	m_console.register_command("cpulist",   CMDFLAG_NONE, 0, 0, std::bind(&debugger_commands::execute_cpulist, this, _1));

	m_console.register_command("comadd",    CMDFLAG_NONE, 1, 2, std::bind(&debugger_commands::execute_comment_add, this, _1));
	m_console.register_command("//",        CMDFLAG_NONE, 1, 2, std::bind(&debugger_commands::execute_comment_add, this, _1));
	m_console.register_command("comdelete", CMDFLAG_NONE, 1, 1, std::bind(&debugger_commands::execute_comment_del, this, _1));
	m_console.register_command("comsave",   CMDFLAG_NONE, 0, 0, std::bind(&debugger_commands::execute_comment_save, this, _1));
	m_console.register_command("comlist",   CMDFLAG_NONE, 0, 0, std::bind(&debugger_commands::execute_comment_list, this, _1));
	m_console.register_command("commit",    CMDFLAG_NONE, 1, 2, std::bind(&debugger_commands::execute_comment_commit, this, _1));
	m_console.register_command("/*",        CMDFLAG_NONE, 1, 2, std::bind(&debugger_commands::execute_comment_commit, this, _1));

	m_console.register_command("bpset",     CMDFLAG_NONE, 1, 3, std::bind(&debugger_commands::execute_bpset, this, _1));
	m_console.register_command("bp",        CMDFLAG_NONE, 1, 3, std::bind(&debugger_commands::execute_bpset, this, _1));
	m_console.register_command("bpclear",   CMDFLAG_NONE, 0, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_bpclear, this, _1));
	m_console.register_command("bpdisable", CMDFLAG_NONE, 0, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_bpdisenable, this, false, _1));
	m_console.register_command("bpenable",  CMDFLAG_NONE, 0, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_bpdisenable, this, true, _1));
	m_console.register_command("bplist",    CMDFLAG_NONE, 0, 1, std::bind(&debugger_commands::execute_bplist, this, _1));

	m_console.register_command("wpset",     CMDFLAG_NONE, 3, 5, std::bind(&debugger_commands::execute_wpset, this, -1, _1));
	m_console.register_command("wp",        CMDFLAG_NONE, 3, 5, std::bind(&debugger_commands::execute_wpset, this, -1, _1));
	m_console.register_command("wpdset",    CMDFLAG_NONE, 3, 5, std::bind(&debugger_commands::execute_wpset, this, AS_DATA, _1));
	m_console.register_command("wpd",       CMDFLAG_NONE, 3, 5, std::bind(&debugger_commands::execute_wpset, this, AS_DATA, _1));
	m_console.register_command("wpiset",    CMDFLAG_NONE, 3, 5, std::bind(&debugger_commands::execute_wpset, this, AS_IO, _1));
	m_console.register_command("wpi",       CMDFLAG_NONE, 3, 5, std::bind(&debugger_commands::execute_wpset, this, AS_IO, _1));
	m_console.register_command("wposet",    CMDFLAG_NONE, 3, 5, std::bind(&debugger_commands::execute_wpset, this, AS_OPCODES, _1));
	m_console.register_command("wpo",       CMDFLAG_NONE, 3, 5, std::bind(&debugger_commands::execute_wpset, this, AS_OPCODES, _1));
	m_console.register_command("wpclear",   CMDFLAG_NONE, 0, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_wpclear, this, _1));
	m_console.register_command("wpdisable", CMDFLAG_NONE, 0, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_wpdisenable, this, false, _1));
	m_console.register_command("wpenable",  CMDFLAG_NONE, 0, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_wpdisenable, this, true, _1));
	m_console.register_command("wplist",    CMDFLAG_NONE, 0, 1, std::bind(&debugger_commands::execute_wplist, this, _1));

	m_console.register_command("rpset",     CMDFLAG_NONE, 1, 2, std::bind(&debugger_commands::execute_rpset, this, _1));
	m_console.register_command("rp",        CMDFLAG_NONE, 1, 2, std::bind(&debugger_commands::execute_rpset, this, _1));
	m_console.register_command("rpclear",   CMDFLAG_NONE, 0, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_rpclear, this, _1));
	m_console.register_command("rpdisable", CMDFLAG_NONE, 0, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_rpdisenable, this, false, _1));
	m_console.register_command("rpenable",  CMDFLAG_NONE, 0, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_rpdisenable, this, true, _1));
	m_console.register_command("rplist",    CMDFLAG_NONE, 0, 1, std::bind(&debugger_commands::execute_rplist, this, _1));

	m_console.register_command("statesave", CMDFLAG_NONE, 1, 1, std::bind(&debugger_commands::execute_statesave, this, _1));
	m_console.register_command("ss",        CMDFLAG_NONE, 1, 1, std::bind(&debugger_commands::execute_statesave, this, _1));
	m_console.register_command("stateload", CMDFLAG_NONE, 1, 1, std::bind(&debugger_commands::execute_stateload, this, _1));
	m_console.register_command("sl",        CMDFLAG_NONE, 1, 1, std::bind(&debugger_commands::execute_stateload, this, _1));

	m_console.register_command("rewind",    CMDFLAG_NONE, 0, 0, std::bind(&debugger_commands::execute_rewind, this, _1));
	m_console.register_command("rw",        CMDFLAG_NONE, 0, 0, std::bind(&debugger_commands::execute_rewind, this, _1));

	m_console.register_command("save",      CMDFLAG_NONE, 3, 3, std::bind(&debugger_commands::execute_save, this, -1, _1));
	m_console.register_command("saved",     CMDFLAG_NONE, 3, 3, std::bind(&debugger_commands::execute_save, this, AS_DATA, _1));
	m_console.register_command("savei",     CMDFLAG_NONE, 3, 3, std::bind(&debugger_commands::execute_save, this, AS_IO, _1));
	m_console.register_command("saveo",     CMDFLAG_NONE, 3, 3, std::bind(&debugger_commands::execute_save, this, AS_OPCODES, _1));
	m_console.register_command("saver",     CMDFLAG_NONE, 4, 4, std::bind(&debugger_commands::execute_saveregion, this, _1));

	m_console.register_command("load",      CMDFLAG_NONE, 2, 3, std::bind(&debugger_commands::execute_load, this, -1, _1));
	m_console.register_command("loadd",     CMDFLAG_NONE, 2, 3, std::bind(&debugger_commands::execute_load, this, AS_DATA, _1));
	m_console.register_command("loadi",     CMDFLAG_NONE, 2, 3, std::bind(&debugger_commands::execute_load, this, AS_IO, _1));
	m_console.register_command("loado",     CMDFLAG_NONE, 2, 3, std::bind(&debugger_commands::execute_load, this, AS_OPCODES, _1));
	m_console.register_command("loadr",     CMDFLAG_NONE, 4, 4, std::bind(&debugger_commands::execute_loadregion, this, _1));

	m_console.register_command("dump",      CMDFLAG_NONE, 3, 6, std::bind(&debugger_commands::execute_dump, this, -1, _1));
	m_console.register_command("dumpd",     CMDFLAG_NONE, 3, 6, std::bind(&debugger_commands::execute_dump, this, AS_DATA, _1));
	m_console.register_command("dumpi",     CMDFLAG_NONE, 3, 6, std::bind(&debugger_commands::execute_dump, this, AS_IO, _1));
	m_console.register_command("dumpo",     CMDFLAG_NONE, 3, 6, std::bind(&debugger_commands::execute_dump, this, AS_OPCODES, _1));

	m_console.register_command("strdump",   CMDFLAG_NONE, 3, 4, std::bind(&debugger_commands::execute_strdump, this, -1, _1));
	m_console.register_command("strdumpd",  CMDFLAG_NONE, 3, 4, std::bind(&debugger_commands::execute_strdump, this, AS_DATA, _1));
	m_console.register_command("strdumpi",  CMDFLAG_NONE, 3, 4, std::bind(&debugger_commands::execute_strdump, this, AS_IO, _1));
	m_console.register_command("strdumpo",  CMDFLAG_NONE, 3, 4, std::bind(&debugger_commands::execute_strdump, this, AS_OPCODES, _1));

	m_console.register_command("cheatinit", CMDFLAG_NONE, 0, 4, std::bind(&debugger_commands::execute_cheatrange, this, true, _1));
	m_console.register_command("ci",        CMDFLAG_NONE, 0, 4, std::bind(&debugger_commands::execute_cheatrange, this, true, _1));

	m_console.register_command("cheatrange",CMDFLAG_NONE, 2, 2, std::bind(&debugger_commands::execute_cheatrange, this, false, _1));
	m_console.register_command("cr",        CMDFLAG_NONE, 2, 2, std::bind(&debugger_commands::execute_cheatrange, this, false, _1));

	m_console.register_command("cheatnext", CMDFLAG_NONE, 1, 2, std::bind(&debugger_commands::execute_cheatnext, this, false, _1));
	m_console.register_command("cn",        CMDFLAG_NONE, 1, 2, std::bind(&debugger_commands::execute_cheatnext, this, false, _1));
	m_console.register_command("cheatnextf",CMDFLAG_NONE, 1, 2, std::bind(&debugger_commands::execute_cheatnext, this, true, _1));
	m_console.register_command("cnf",       CMDFLAG_NONE, 1, 2, std::bind(&debugger_commands::execute_cheatnext, this, true, _1));

	m_console.register_command("cheatlist", CMDFLAG_NONE, 0, 1, std::bind(&debugger_commands::execute_cheatlist, this, _1));
	m_console.register_command("cl",        CMDFLAG_NONE, 0, 1, std::bind(&debugger_commands::execute_cheatlist, this, _1));

	m_console.register_command("cheatundo", CMDFLAG_NONE, 0, 0, std::bind(&debugger_commands::execute_cheatundo, this, _1));
	m_console.register_command("cu",        CMDFLAG_NONE, 0, 0, std::bind(&debugger_commands::execute_cheatundo, this, _1));

	m_console.register_command("f",         CMDFLAG_KEEP_QUOTES, 3, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_find, this, -1, _1));
	m_console.register_command("find",      CMDFLAG_KEEP_QUOTES, 3, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_find, this, -1, _1));
	m_console.register_command("fd",        CMDFLAG_KEEP_QUOTES, 3, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_find, this, AS_DATA, _1));
	m_console.register_command("findd",     CMDFLAG_KEEP_QUOTES, 3, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_find, this, AS_DATA, _1));
	m_console.register_command("fi",        CMDFLAG_KEEP_QUOTES, 3, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_find, this, AS_IO, _1));
	m_console.register_command("findi",     CMDFLAG_KEEP_QUOTES, 3, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_find, this, AS_IO, _1));
	m_console.register_command("fo",        CMDFLAG_KEEP_QUOTES, 3, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_find, this, AS_OPCODES, _1));
	m_console.register_command("findo",     CMDFLAG_KEEP_QUOTES, 3, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_find, this, AS_OPCODES, _1));

	m_console.register_command("fill",      CMDFLAG_KEEP_QUOTES, 3, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_fill, this, -1, _1));
	m_console.register_command("filld",     CMDFLAG_KEEP_QUOTES, 3, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_fill, this, AS_DATA, _1));
	m_console.register_command("filli",     CMDFLAG_KEEP_QUOTES, 3, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_fill, this, AS_IO, _1));
	m_console.register_command("fillo",     CMDFLAG_KEEP_QUOTES, 3, MAX_COMMAND_PARAMS, std::bind(&debugger_commands::execute_fill, this, AS_OPCODES, _1));

	m_console.register_command("dasm",      CMDFLAG_NONE, 3, 5, std::bind(&debugger_commands::execute_dasm, this, _1));

	m_console.register_command("trace",     CMDFLAG_NONE, 1, 4, std::bind(&debugger_commands::execute_trace, this, _1, false));
	m_console.register_command("traceover", CMDFLAG_NONE, 1, 4, std::bind(&debugger_commands::execute_trace, this, _1, true));
	m_console.register_command("traceflush",CMDFLAG_NONE, 0, 0, std::bind(&debugger_commands::execute_traceflush, this, _1));

	m_console.register_command("history",   CMDFLAG_NONE, 0, 2, std::bind(&debugger_commands::execute_history, this, _1));
	m_console.register_command("trackpc",   CMDFLAG_NONE, 0, 3, std::bind(&debugger_commands::execute_trackpc, this, _1));

	m_console.register_command("trackmem",  CMDFLAG_NONE, 0, 3, std::bind(&debugger_commands::execute_trackmem, this, _1));
	m_console.register_command("pcatmem",   CMDFLAG_NONE, 1, 1, std::bind(&debugger_commands::execute_pcatmem, this, -1, _1));
	m_console.register_command("pcatmemd",  CMDFLAG_NONE, 1, 1, std::bind(&debugger_commands::execute_pcatmem, this, AS_DATA, _1));
	m_console.register_command("pcatmemi",  CMDFLAG_NONE, 1, 1, std::bind(&debugger_commands::execute_pcatmem, this, AS_IO, _1));
	m_console.register_command("pcatmemo",  CMDFLAG_NONE, 1, 1, std::bind(&debugger_commands::execute_pcatmem, this, AS_OPCODES, _1));

	m_console.register_command("snap",      CMDFLAG_NONE, 0, 1, std::bind(&debugger_commands::execute_snap, this, _1));

	m_console.register_command("source",    CMDFLAG_NONE, 1, 1, std::bind(&debugger_commands::execute_source, this, _1));

	m_console.register_command("map",       CMDFLAG_NONE, 1, 1, std::bind(&debugger_commands::execute_map, this, -1, _1));
	m_console.register_command("mapd",      CMDFLAG_NONE, 1, 1, std::bind(&debugger_commands::execute_map, this, AS_DATA, _1));
	m_console.register_command("mapi",      CMDFLAG_NONE, 1, 1, std::bind(&debugger_commands::execute_map, this, AS_IO, _1));
	m_console.register_command("mapo",      CMDFLAG_NONE, 1, 1, std::bind(&debugger_commands::execute_map, this, AS_OPCODES, _1));
	m_console.register_command("memdump",   CMDFLAG_NONE, 0, 2, std::bind(&debugger_commands::execute_memdump, this, _1));

	m_console.register_command("symlist",   CMDFLAG_NONE, 0, 1, std::bind(&debugger_commands::execute_symlist, this, _1));

	m_console.register_command("softreset", CMDFLAG_NONE, 0, 1, std::bind(&debugger_commands::execute_softreset, this, _1));
	m_console.register_command("hardreset", CMDFLAG_NONE, 0, 1, std::bind(&debugger_commands::execute_hardreset, this, _1));

	m_console.register_command("images",    CMDFLAG_NONE, 0, 0, std::bind(&debugger_commands::execute_images, this, _1));
	m_console.register_command("mount",     CMDFLAG_NONE, 2, 2, std::bind(&debugger_commands::execute_mount, this, _1));
	m_console.register_command("unmount",   CMDFLAG_NONE, 1, 1, std::bind(&debugger_commands::execute_unmount, this, _1));

	m_console.register_command("input",     CMDFLAG_NONE, 1, 1, std::bind(&debugger_commands::execute_input, this, _1));
	m_console.register_command("dumpkbd",   CMDFLAG_NONE, 0, 1, std::bind(&debugger_commands::execute_dumpkbd, this, _1));

	// set up the initial debugscript if specified
	const char* name = m_machine.options().debug_script();
	if (name[0] != 0)
		m_console.source_script(name);

	m_cheat.space = nullptr;
}


//-------------------------------------------------
//  get_cpunum - getter callback for the
//  'cpunum' symbol
//-------------------------------------------------

u64 debugger_commands::get_cpunum()
{
	unsigned index = 0;
	for (device_execute_interface &exec : execute_interface_enumerator(m_machine.root_device()))
	{
		if (m_console.get_visible_cpu() == &exec.device())
			return index;

		// real CPUs should have pcbase
		device_state_interface const *state;
		if (exec.device().interface(state) && state->state_find_entry(STATE_GENPCBASE))
			++index;
	}
	return u64(s64(-1));
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
	case 1: return *(u8 *)global->base;
	case 2: return *(u16 *)global->base;
	case 4: return *(u32 *)global->base;
	case 8: return *(u64 *)global->base;
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
	case 1: *(u8 *)global->base = value; break;
	case 2: *(u16 *)global->base = value; break;
	case 4: *(u32 *)global->base = value; break;
	case 8: *(u64 *)global->base = value; break;
	}
}



/***************************************************************************
    PARAMETER VALIDATION HELPERS
***************************************************************************/

/// \brief Validate parameter as a Boolean value
///
/// Validates a parameter as a Boolean value.  Fixed strings and
/// expressions evaluating to numeric values are recognised.  The result
/// is unchanged for an empty string.
/// \param [in] param The parameter string.
/// \param [in,out] result The default value on entry, and the value of
///   the parameter interpreted as a Boolean on success.  Unchanged if
///   the parameter is an empty string.
/// \return true if the parameter is a valid Boolean value or an empty
///   string, or false otherwise.
bool debugger_commands::validate_boolean_parameter(const std::string &param, bool &result)
{
	// nullptr parameter does nothing and returns no error
	if (param.empty())
		return true;

	// evaluate the expression; success if no error
	bool const is_true = !core_stricmp(param.c_str(), "true");
	bool const is_false = !core_stricmp(param.c_str(), "false");

	if (is_true || is_false)
	{
		result = is_true;
		return true;
	}

	// try to evaluate as a number
	u64 val;
	if (!validate_number_parameter(param, val))
		return false;

	result = val != 0;
	return true;
}


/// \brief Validate parameter as a numeric value
///
/// Parses the parameter as an expression and evaluates it as a number.
/// \param [in] param The parameter string.
/// \param [out] result The numeric value of the expression on success.
///   Unchanged on failure.
/// \return true if the parameter is a valid expression that evaluates
///   to a numeric value, or false otherwise.
bool debugger_commands::validate_number_parameter(std::string_view param, u64 &result)
{
	// evaluate the expression; success if no error
	try
	{
		result = parsed_expression(m_console.visible_symtable(), param).execute();
		return true;
	}
	catch (expression_error const &error)
	{
		// print an error pointing to the character that caused it
		m_console.printf("Error in expression: %s\n", param);
		m_console.printf("                     %*s^", error.offset(), "");
		m_console.printf("%s\n", error.code_string());
		return false;
	}
}


/// \brief Validate parameter as a device
///
/// Validates a parameter as a device identifier and retrieves the
/// device on success.  A string corresponding to the tag of a device
/// refers to that device; an empty string refers to the current CPU
/// with debugger focus; any other string is parsed as an expression
/// and treated as an index of a device implementing
/// #device_execute_interface and #device_state_interface, and exposing
/// a generic PC base value.
/// \param [in] param The parameter string.
/// \param [out] result A pointer to the device on success, or unchanged
///   on failure.
/// \return true if the parameter refers to a device in the current
///   system, or false otherwise.
bool debugger_commands::validate_device_parameter(std::string_view param, device_t *&result)
{
	// if no parameter, use the visible CPU
	if (param.empty())
	{
		device_t *const current = m_console.get_visible_cpu();
		if (current)
		{
			result = current;
			return true;
		}
		else
		{
			m_console.printf("No valid CPU is currently selected\n");
			return false;
		}
	}

	// next look for a tag match
	std::string_view relative = param;
	device_t &base = get_device_search_base(relative);
	device_t *device = base.subdevice(strmakelower(relative));
	if (device)
	{
		result = device;
		return true;
	}

	// then evaluate as an expression; on an error assume it was a tag
	u64 cpunum;
	try
	{
		cpunum = parsed_expression(m_console.visible_symtable(), param).execute();
	}
	catch (expression_error &)
	{
		m_console.printf("Unable to find device '%s'\n", param);
		return false;
	}

	// attempt to find by numerical index
	device = get_cpu_by_index(cpunum);
	if (device)
	{
		result = device;
		return true;
	}
	else
	{
		// if out of range, complain
		m_console.printf("Invalid CPU index %u\n", cpunum);
		return false;
	}
}


/// \brief Validate a parameter as a CPU
///
/// Validates a parameter as a CPU identifier.  Uses the same rules as
/// #validate_device_parameter to identify devices, but additionally
/// checks that the device is a "CPU" for the debugger's purposes.
/// \param [in] The parameter string.
/// \param [out] result The device on success, or unchanged on failure.
/// \return true if the parameter refers to a CPU-like device in the
///   current system, or false otherwise.
bool debugger_commands::validate_cpu_parameter(std::string_view param, device_t *&result)
{
	// first do the standard device thing
	device_t *device;
	if (!validate_device_parameter(param, device))
		return false;

	// check that it's a "CPU" for the debugger's purposes
	device_execute_interface const *execute;
	if (device->interface(execute))
	{
		result = device;
		return true;
	}

	m_console.printf("Device %s is not a CPU\n", device->name());
	return false;
}


/// \brief Validate a parameter as an address space identifier
///
/// Validates a parameter as an address space identifier.  Uses the same
/// rules as #validate_device_parameter to identify devices.  If the
/// default address space number is negative, the first address space
/// exposed by the device will be used as the default.
/// \param [in] The parameter string.
/// \param [in] spacenum The default address space index.  If negative,
///   the first address space exposed by the device (i.e. the address
///   space with the lowest index) will be used as the default.
/// \param [out] result The address space on success, or unchanged on
///   failure.
/// \return true if the parameter refers to an address space in the
///   current system, or false otherwise.
bool debugger_commands::validate_device_space_parameter(std::string_view param, int spacenum, address_space *&result)
{
	device_t *device;
	std::string spacename;
	if (param.empty())
	{
		// if no parameter, use the visible CPU
		device = m_console.get_visible_cpu();
		if (!device)
		{
			m_console.printf("No valid CPU is currently selected\n");
			return false;
		}
	}
	else
	{
		// look for a tag match on the whole parameter value
		std::string_view relative = param;
		device_t &base = get_device_search_base(relative);
		device = base.subdevice(strmakelower(relative));

		// if that failed, treat the last component as an address space
		if (!device)
		{
			auto const delimiter = relative.find_last_of(":^");
			bool const found = std::string_view::npos != delimiter;
			if (!found || (':' == relative[delimiter]))
			{
				spacename = strmakelower(relative.substr(found ? (delimiter + 1) : 0));
				relative = relative.substr(0, !found ? 0 : !delimiter ? 1 : delimiter);
				if (!relative.empty())
					device = base.subdevice(strmakelower(relative));
				else if (m_console.get_visible_cpu())
					device = m_console.get_visible_cpu();
				else
					device = &m_machine.root_device();
			}
		}
	}

	// if still no device found, evaluate as an expression
	if (!device)
	{
		u64 cpunum;
		try
		{
			cpunum = parsed_expression(m_console.visible_symtable(), param).execute();
		}
		catch (expression_error const &)
		{
			// parsing failed - assume it was a tag
			m_console.printf("Unable to find device '%s'\n", param);
			return false;
		}

		// attempt to find by numerical index
		device = get_cpu_by_index(cpunum);
		if (!device)
		{
			// if out of range, complain
			m_console.printf("Invalid CPU index %u\n", cpunum);
			return false;
		}
	}

	// ensure the device implements the memory interface
	device_memory_interface *memory;
	if (!device->interface(memory))
	{
		m_console.printf("No memory interface found for device %s\n", device->name());
		return false;
	}

	// fall back to supplied default space if appropriate
	if (spacename.empty() && (0 <= spacenum))
	{
		if (memory->has_space(spacenum))
		{
			result = &memory->space(spacenum);
			return true;
		}
		else
		{
			m_console.printf("No matching memory space found for device '%s'\n", device->tag());
			return false;
		}
	}

	// otherwise find the specified space or fall back to the first populated space
	for (int i = 0; memory->max_space_count() > i; ++i)
	{
		if (memory->has_space(i) && (spacename.empty() || (memory->space(i).name() == spacename)))
		{
			result = &memory->space(i);
			return true;
		}
	}

	// report appropriate error message
	if (spacename.empty())
		m_console.printf("No memory spaces found for device '%s'\n", device->tag());
	else
		m_console.printf("Memory space '%s' not found found for device '%s'\n", spacename, device->tag());
	return false;
}


/// \brief Validate a parameter as a target address
///
/// Validates a parameter as an numeric expression to use as an address
/// optionally followed by a colon and a device identifier.  If the
/// device identifier is not presnt, the current CPU with debugger focus
/// is assumed.  See #validate_device_parameter for information on how
/// device parametersare interpreted.
/// \param [in] The parameter string.
/// \param [in] spacenum The default address space index.  If negative,
///   the first address space exposed by the device (i.e. the address
///   space with the lowest index) will be used as the default.
/// \param [out] space The address space on success, or unchanged on
///   failure.
/// \param [out] addr The address on success, or unchanged on failure.
/// \return true if the address is a valid expression evaluating to a
///   number and the address space is found, or false otherwise.
bool debugger_commands::validate_target_address_parameter(std::string_view param, int spacenum, address_space *&space, u64 &addr)
{
	// check for the device delimiter
	std::string_view::size_type const devdelim = find_delimiter(param, [] (char ch) { return ':' == ch; });
	std::string_view device;
	if (devdelim != std::string::npos)
		device = param.substr(devdelim + 1);

	// parse the address first
	u64 addrval;
	if (!validate_number_parameter(param.substr(0, devdelim), addrval))
		return false;

	// find the address space
	if (!validate_device_space_parameter(device, spacenum, space))
		return false;

	// set the address now that we have the space
	addr = addrval;
	return true;
}


/// \brief Validate a parameter as a memory region
///
/// Validates a parameter as a memory region tag and retrieves the
/// specified memory region.
/// \param [in] The parameter string.
/// \param [out] result The memory region on success, or unchanged on
///   failure.
/// \return true if the parameter refers to a memory region in the
///   current system, or false otherwise.
bool debugger_commands::validate_memory_region_parameter(std::string_view param, memory_region *&result)
{
	auto const &regions = m_machine.memory().regions();
	std::string_view relative = param;
	device_t &base = get_device_search_base(relative);
	auto const iter = regions.find(base.subtag(strmakelower(relative)));
	if (regions.end() != iter)
	{
		result = iter->second.get();
		return true;
	}
	else
	{
		m_console.printf("No matching memory region found for '%s'\n", param);
		return false;
	}
}


/// \brief Get search base for device or address space parameter
///
/// Handles prefix prefixes used to indicate that a device tag should be
/// interpreted relative to the selected CPU.  Removes the recognised
/// prefixes from the parameter value.
/// \param [in,out] param The parameter string.  Recognised prefixes
///   affecting the search base are removed, leaving a tag relative to
///   the base device.
/// \return A reference to the base device that the tag should be
///   interpreted relative to.
device_t &debugger_commands::get_device_search_base(std::string_view &param)
{
	if (!param.empty())
	{
		// handle ".:" or ".^" prefix for tag relative to current CPU if any
		if (('.' == param[0]) && ((param.size() == 1) || (':' == param[1]) || ('^' == param[1])))
		{
			param.remove_prefix(((param.size() > 1) && (':' == param[1])) ? 2 : 1);
			device_t *const current = m_console.get_visible_cpu();
			return current ? *current : m_machine.root_device();
		}

		// a sibling path makes most sense relative to current CPU
		if ('^' == param[0])
		{
			device_t *const current = m_console.get_visible_cpu();
			return current ? *current : m_machine.root_device();
		}
	}


	// default to root device
	return m_machine.root_device();
}


/// \brief Get CPU by index
///
/// Looks up a CPU by the number the debugger assigns it based on its
/// position in the device tree relative to other CPUs.
/// \param [in] cpunum Zero-based index of the CPU to find.
/// \return A pointer to the CPU if found, or \c nullptr if no CPU has
///   the specified index.
device_t *debugger_commands::get_cpu_by_index(u64 cpunum)
{
	unsigned index = 0;
	for (device_execute_interface &exec : execute_interface_enumerator(m_machine.root_device()))
	{
		// real CPUs should have pcbase
		device_state_interface const *state;
		if (exec.device().interface(state) && state->state_find_entry(STATE_GENPCBASE))
		{
			if (index++ == cpunum)
			{
				return &exec.device();
			}
		}
	}
	return nullptr;
}


/*-------------------------------------------------
    debug_command_parameter_expression - validates
    an expression parameter
-------------------------------------------------*/

bool debugger_commands::debug_command_parameter_expression(std::string_view param, parsed_expression &result)
{
	try
	{
		// parse the expression; success if no error
		result.parse(param);
		return true;
	}
	catch (expression_error const &err)
	{
		// output an error
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

void debugger_commands::execute_help(const std::vector<std::string> &params)
{
	if (params.size() == 0)
		m_console.printf_wrap(80, "%s\n", debug_get_help(std::string_view()));
	else
		m_console.printf_wrap(80, "%s\n", debug_get_help(params[0]));
}


/*-------------------------------------------------
    execute_print - execute the print command
-------------------------------------------------*/

void debugger_commands::execute_print(const std::vector<std::string> &params)
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

bool debugger_commands::mini_printf(std::ostream &stream, std::string_view format, int params, u64 *param)
{
	auto f = format.begin();

	// parse the string looking for % signs
	while (f != format.end())
	{
		char c = *f++;

		// escape sequences
		if (c == '\\')
		{
			if (f == format.end()) break;
			c = *f++;
			switch (c)
			{
				case '\\':  stream << c;    break;
				case 'n':   stream << '\n'; break;
				default:                    break;
			}
			continue;
		}

		// formatting
		else if (c == '%')
		{
			int width = 0;
			int zerofill = 0;

			// parse out the width
			while (f != format.end() && *f >= '0' && *f <= '9')
			{
				c = *f++;
				if (c == '0' && width == 0)
					zerofill = 1;
				width = width * 10 + (c - '0');
			}
			if (f == format.end()) break;

			// get the format
			c = *f++;
			switch (c)
			{
				case '%':
					stream << c;
					break;

				case 'X':
				case 'x':
					if (params == 0)
					{
						m_console.printf("Not enough parameters for format!\n");
						return false;
					}
					if (u32(*param >> 32) != 0)
						util::stream_format(stream, zerofill ? "%0*X" : "%*X", (width <= 8) ? 1 : width - 8, u32(*param >> 32));
					else if (width > 8)
						util::stream_format(stream, zerofill ? "%0*X" : "%*X", width - 8, 0);
					util::stream_format(stream, zerofill ? "%0*X" : "%*X", (width < 8) ? width : 8, u32(*param));
					param++;
					params--;
					break;

				case 'O':
				case 'o':
					if (params == 0)
					{
						m_console.printf("Not enough parameters for format!\n");
						return false;
					}
					if (u32(*param >> 60) != 0)
					{
						util::stream_format(stream, zerofill ? "%0*o" : "%*o", (width <= 20) ? 1 : width - 20, u32(*param >> 60));
						util::stream_format(stream, "%0*o", 10, u32(BIT(*param, 30, 30)));
					}
					else
					{
						if (width > 20)
							util::stream_format(stream, zerofill ? "%0*o" : "%*o", width - 20, 0);
						if (u32(BIT(*param, 30, 30)) != 0)
							util::stream_format(stream, zerofill ? "%0*o" : "%*o", (width <= 10) ? 1 : width - 10, u32(BIT(*param, 30, 30)));
						else if (width > 10)
							util::stream_format(stream, zerofill ? "%0*o" : "%*o", width - 10, 0);
					}
					util::stream_format(stream, zerofill ? "%0*o" : "%*o", (width < 10) ? width : 10, u32(BIT(*param, 0, 30)));
					param++;
					params--;
					break;

				case 'D':
				case 'd':
					if (params == 0)
					{
						m_console.printf("Not enough parameters for format!\n");
						return false;
					}
					util::stream_format(stream, zerofill ? "%0*d" : "%*d", width, u32(*param));
					param++;
					params--;
					break;
				case 'C':
				case 'c':
					if (params == 0)
					{
						m_console.printf("Not enough parameters for format!\n");
						return false;
					}
					stream << char(*param);
					param++;
					params--;
					break;

			}
		}

		// normal stuff
		else
			stream << c;
	}

	return true;
}


/*-------------------------------------------------
    execute_index_command - helper for commands
    that take multiple indices as arguments
-------------------------------------------------*/

template <typename T>
void debugger_commands::execute_index_command(std::vector<std::string> const &params, T &&apply, char const *unused_message)
{
	std::vector<u64> index(params.size());
	for (int paramnum = 0; paramnum < params.size(); paramnum++)
	{
		if (!validate_number_parameter(params[paramnum], index[paramnum]))
			return;
	}

	for (device_t &device : device_enumerator(m_machine.root_device()))
	{
		for (auto param = index.begin(); index.end() != param; )
		{
			if (apply(device, *param))
				param = index.erase(param);
			else
				++param;
		}
	}

	for (auto const &param : index)
		m_console.printf(unused_message, param);
}


/*-------------------------------------------------
    execute_printf - execute the printf command
-------------------------------------------------*/

void debugger_commands::execute_printf(const std::vector<std::string> &params)
{
	/* validate the other parameters */
	u64 values[MAX_COMMAND_PARAMS];
	for (int i = 1; i < params.size(); i++)
		if (!validate_number_parameter(params[i], values[i]))
			return;

	/* then do a printf */
	std::ostringstream buffer;
	if (mini_printf(buffer, params[0], params.size() - 1, &values[1]))
		m_console.printf("%s\n", std::move(buffer).str());
}


/*-------------------------------------------------
    execute_logerror - execute the logerror command
-------------------------------------------------*/

void debugger_commands::execute_logerror(const std::vector<std::string> &params)
{
	/* validate the other parameters */
	u64 values[MAX_COMMAND_PARAMS];
	for (int i = 1; i < params.size(); i++)
		if (!validate_number_parameter(params[i], values[i]))
			return;

	/* then do a printf */
	std::ostringstream buffer;
	if (mini_printf(buffer, params[0], params.size() - 1, &values[1]))
		m_machine.logerror("%s", std::move(buffer).str());
}


/*-------------------------------------------------
    execute_tracelog - execute the tracelog command
-------------------------------------------------*/

void debugger_commands::execute_tracelog(const std::vector<std::string> &params)
{
	/* validate the other parameters */
	u64 values[MAX_COMMAND_PARAMS];
	for (int i = 1; i < params.size(); i++)
		if (!validate_number_parameter(params[i], values[i]))
			return;

	/* then do a printf */
	std::ostringstream buffer;
	if (mini_printf(buffer, params[0], params.size() - 1, &values[1]))
		m_console.get_visible_cpu()->debug()->trace_printf("%s", std::move(buffer).str().c_str());
}


/*-------------------------------------------------
    execute_tracesym - execute the tracesym command
-------------------------------------------------*/

void debugger_commands::execute_tracesym(const std::vector<std::string> &params)
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
	std::ostringstream buffer;
	if (mini_printf(buffer, format.str(), params.size(), values))
		m_console.get_visible_cpu()->debug()->trace_printf("%s", std::move(buffer).str().c_str());
}


/*-------------------------------------------------
    execute_cls - execute the cls command
-------------------------------------------------*/

void debugger_commands::execute_cls(const std::vector<std::string> &params)
{
	text_buffer_clear(m_console.get_console_textbuf());
}


/*-------------------------------------------------
    execute_quit - execute the quit command
-------------------------------------------------*/

void debugger_commands::execute_quit(const std::vector<std::string> &params)
{
	osd_printf_warning("Exited via the debugger\n");
	m_machine.schedule_exit();
}


/*-------------------------------------------------
    execute_do - execute the do command
-------------------------------------------------*/

void debugger_commands::execute_do(const std::vector<std::string> &params)
{
	u64 dummy;
	validate_number_parameter(params[0], dummy);
}


/*-------------------------------------------------
    execute_step - execute the step command
-------------------------------------------------*/

void debugger_commands::execute_step(const std::vector<std::string> &params)
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

void debugger_commands::execute_over(const std::vector<std::string> &params)
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

void debugger_commands::execute_out(const std::vector<std::string> &params)
{
	m_console.get_visible_cpu()->debug()->single_step_out();
}


/*-------------------------------------------------
    execute_go - execute the go command
-------------------------------------------------*/

void debugger_commands::execute_go(const std::vector<std::string> &params)
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

void debugger_commands::execute_go_vblank(const std::vector<std::string> &params)
{
	m_console.get_visible_cpu()->debug()->go_vblank();
}


/*-------------------------------------------------
    execute_go_interrupt - execute the goint command
-------------------------------------------------*/

void debugger_commands::execute_go_interrupt(const std::vector<std::string> &params)
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

void debugger_commands::execute_go_exception(const std::vector<std::string> &params)
{
	u64 exception = -1;

	/* if we have a parameter, use it instead */
	if (params.size() > 0 && !validate_number_parameter(params[0], exception))
		return;

	parsed_expression condition(m_console.visible_symtable());
	if (params.size() > 1 && !debug_command_parameter_expression(params[1], condition))
		return;

	m_console.get_visible_cpu()->debug()->go_exception(exception, condition.is_empty() ? "1" : condition.original_string());
}


/*-------------------------------------------------
    execute_go_time - execute the gtime command
-------------------------------------------------*/

void debugger_commands::execute_go_time(const std::vector<std::string> &params)
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
void debugger_commands::execute_go_privilege(const std::vector<std::string> &params)
{
	parsed_expression condition(m_console.visible_symtable());
	if (params.size() > 0 && !debug_command_parameter_expression(params[0], condition))
		return;

	m_console.get_visible_cpu()->debug()->go_privilege((condition.is_empty()) ? "1" : condition.original_string());
}

/*-------------------------------------------------
    execute_next - execute the next command
-------------------------------------------------*/

void debugger_commands::execute_next(const std::vector<std::string> &params)
{
	m_console.get_visible_cpu()->debug()->go_next_device();
}


/*-------------------------------------------------
    execute_focus - execute the focus command
-------------------------------------------------*/

void debugger_commands::execute_focus(const std::vector<std::string> &params)
{
	// validate params
	device_t *cpu;
	if (!validate_cpu_parameter(params[0], cpu))
		return;

	// first clear the ignore flag on the focused CPU
	cpu->debug()->ignore(false);

	// then loop over CPUs and set the ignore flags on all other CPUs
	for (device_execute_interface &exec : execute_interface_enumerator(m_machine.root_device()))
		if (&exec.device() != cpu)
			exec.device().debug()->ignore(true);
	m_console.printf("Now focused on CPU '%s'\n", cpu->tag());
}


/*-------------------------------------------------
    execute_ignore - execute the ignore command
-------------------------------------------------*/

void debugger_commands::execute_ignore(const std::vector<std::string> &params)
{
	if (params.empty())
	{
		// if there are no parameters, dump the ignore list
		std::string buffer;

		// loop over all executable devices
		for (device_execute_interface &exec : execute_interface_enumerator(m_machine.root_device()))
		{
			// build up a comma-separated list
			if (!exec.device().debug()->observing())
			{
				if (buffer.empty())
					buffer = string_format("Currently ignoring device '%s'", exec.device().tag());
				else
					buffer.append(string_format(", '%s'", exec.device().tag()));
			}
		}

		// special message for none
		if (buffer.empty())
			buffer = string_format("Not currently ignoring any devices");
		m_console.printf("%s\n", buffer);
	}
	else
	{
		// otherwise clear the ignore flag on all requested CPUs
		device_t *devicelist[MAX_COMMAND_PARAMS];

		// validate parameters
		for (int paramnum = 0; paramnum < params.size(); paramnum++)
			if (!validate_cpu_parameter(params[paramnum], devicelist[paramnum]))
				return;

		// set the ignore flags
		for (int paramnum = 0; paramnum < params.size(); paramnum++)
		{
			// make sure this isn't the last live CPU
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

void debugger_commands::execute_observe(const std::vector<std::string> &params)
{
	if (params.empty())
	{
		// if there are no parameters, dump the ignore list
		std::string buffer;

		// loop over all executable devices
		for (device_execute_interface &exec : execute_interface_enumerator(m_machine.root_device()))
		{
			// build up a comma-separated list
			if (exec.device().debug()->observing())
			{
				if (buffer.empty())
					buffer = string_format("Currently observing CPU '%s'", exec.device().tag());
				else
					buffer.append(string_format(", '%s'", exec.device().tag()));
			}
		}

		// special message for none
		if (buffer.empty())
			buffer = string_format("Not currently observing any devices");
		m_console.printf("%s\n", buffer);
	}
	else
	{
		// otherwise set the ignore flag on all requested CPUs
		device_t *devicelist[MAX_COMMAND_PARAMS];

		// validate parameters
		for (int paramnum = 0; paramnum < params.size(); paramnum++)
			if (!validate_cpu_parameter(params[paramnum], devicelist[paramnum]))
				return;

		// clear the ignore flags
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

void debugger_commands::execute_suspend(const std::vector<std::string> &params)
{
	// if there are no parameters, dump the ignore list
	if (params.empty())
	{
		std::string buffer;

		// loop over all executable devices
		for (device_execute_interface &exec : execute_interface_enumerator(m_machine.root_device()))

			// build up a comma-separated list
			if (exec.device().debug()->suspended())
			{
				if (buffer.empty())
					buffer = string_format("Currently suspended device '%s'", exec.device().tag());
				else
					buffer.append(string_format(", '%s'", exec.device().tag()));
			}

		// special message for none
		if (buffer.empty())
			buffer = string_format("No currently suspended devices");
		m_console.printf("%s\n", buffer);
	}
	else
	{
		device_t *devicelist[MAX_COMMAND_PARAMS];

		// validate parameters
		for (int paramnum = 0; paramnum < params.size(); paramnum++)
			if (!validate_cpu_parameter(params[paramnum], devicelist[paramnum]))
				return;

		for (int paramnum = 0; paramnum < params.size(); paramnum++)
		{
			// make sure this isn't the last live CPU
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

void debugger_commands::execute_resume(const std::vector<std::string> &params)
{
	// if there are no parameters, dump the ignore list
	if (params.empty())
	{
		std::string buffer;

		// loop over all executable devices
		for (device_execute_interface &exec : execute_interface_enumerator(m_machine.root_device()))

			// build up a comma-separated list
			if (exec.device().debug()->suspended())
			{
				if (buffer.empty())
					buffer = string_format("Currently suspended device '%s'", exec.device().tag());
				else
					buffer.append(string_format(", '%s'", exec.device().tag()));
			}

		// special message for none
		if (buffer.empty())
			buffer = string_format("No currently suspended devices");
		m_console.printf("%s\n", buffer);
	}
	else
	{
		device_t *devicelist[MAX_COMMAND_PARAMS];

		// validate parameters
		for (int paramnum = 0; paramnum < params.size(); paramnum++)
			if (!validate_cpu_parameter(params[paramnum], devicelist[paramnum]))
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

void debugger_commands::execute_cpulist(const std::vector<std::string> &params)
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

void debugger_commands::execute_comment_add(const std::vector<std::string> &params)
{
	// param 1 is the address for the comment
	u64 address;
	if (!validate_number_parameter(params[0], address))
		return;

	// CPU parameter is implicit
	device_t *cpu;
	if (!validate_cpu_parameter(std::string_view(), cpu))
		return;

	// make sure param 2 exists
	if (params[1].empty())
	{
		m_console.printf("Error : comment text empty\n");
		return;
	}

	// Now try adding the comment
	cpu->debug()->comment_add(address, params[1].c_str(), 0x00ff0000);
	cpu->machine().debug_view().update_all(DVT_DISASSEMBLY);
}


/*------------------------------------------------------
    execute_comment_del - remove a comment from an addr
--------------------------------------------------------*/

void debugger_commands::execute_comment_del(const std::vector<std::string> &params)
{
	// param 1 can either be a command or the address for the comment
	u64 address;
	if (!validate_number_parameter(params[0], address))
		return;

	// CPU parameter is implicit
	device_t *cpu;
	if (!validate_cpu_parameter(std::string_view(), cpu))
		return;

	// If it's a number, it must be an address
	// The bankoff and cbn will be pulled from what's currently active
	cpu->debug()->comment_remove(address);
	cpu->machine().debug_view().update_all(DVT_DISASSEMBLY);
}

/**
 * @fn void execute_comment_list(const std::vector<std::string> &params)
 * @brief Print current list of comments in debugger
 *
 *
 */

void debugger_commands::execute_comment_list(const std::vector<std::string> &params)
{
	if (!m_machine.debugger().cpu().comment_load(false))
		m_console.printf("Error while parsing XML file\n");
}

/**
 * @fn void execute_comment_commit(const std::vector<std::string> &params)
 * @brief Add and Save current list of comments in debugger
 *
 */

void debugger_commands::execute_comment_commit(const std::vector<std::string> &params)
{
	execute_comment_add(params);
	execute_comment_save(params);
}

/*-------------------------------------------------
    execute_comment - add a comment to a line
-------------------------------------------------*/

void debugger_commands::execute_comment_save(const std::vector<std::string> &params)
{
	if (m_machine.debugger().cpu().comment_save())
		m_console.printf("Comment successfully saved\n");
	else
		m_console.printf("Comment not saved\n");
}

// TODO: add color hex editing capabilities for comments, see below for more info
/**
 * @fn void execute_comment_color(const std::vector<std::string> &params)
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

void debugger_commands::execute_bpset(const std::vector<std::string> &params)
{
	// param 1 is the address/CPU
	u64 address;
	address_space *space;
	if (!validate_target_address_parameter(params[0], AS_PROGRAM, space, address))
		return;

	device_execute_interface const *execute;
	if (!space->device().interface(execute))
	{
		m_console.printf("Device %s is not a CPU\n", space->device().name());
		return;
	}
	device_debug *const debug = space->device().debug();

	if (space->spacenum() != AS_PROGRAM)
	{
		m_console.printf("Only program space breakpoints are supported\n");
		return;
	}

	// param 2 is the condition
	parsed_expression condition(debug->symtable());
	if (params.size() > 1 && !debug_command_parameter_expression(params[1], condition))
		return;

	// param 3 is the action
	const char *action = nullptr;
	if (params.size() > 2 && !debug_command_parameter_command(action = params[2].c_str()))
		return;

	// set the breakpoint
	int const bpnum = debug->breakpoint_set(address, condition.is_empty() ? nullptr : condition.original_string(), action);
	m_console.printf("Breakpoint %X set\n", bpnum);
}


/*-------------------------------------------------
    execute_bpclear - execute the breakpoint
    clear command
-------------------------------------------------*/

void debugger_commands::execute_bpclear(const std::vector<std::string> &params)
{
	if (params.empty()) // if no parameters, clear all
	{
		for (device_t &device : device_enumerator(m_machine.root_device()))
			device.debug()->breakpoint_clear_all();
		m_console.printf("Cleared all breakpoints\n");
	}
	else // otherwise, clear the specific ones
	{
		execute_index_command(
				params,
				[this] (device_t &device, u64 param) -> bool
				{
					if (!device.debug()->breakpoint_clear(param))
						return false;
					m_console.printf("Breakpoint %X cleared\n", param);
					return true;
				},
				"Invalid breakpoint number %X\n");
	}
}


/*-------------------------------------------------
    execute_bpdisenable - execute the breakpoint
    disable/enable commands
-------------------------------------------------*/

void debugger_commands::execute_bpdisenable(bool enable, const std::vector<std::string> &params)
{
	if (params.empty()) // if no parameters, disable/enable all
	{
		for (device_t &device : device_enumerator(m_machine.root_device()))
			device.debug()->breakpoint_enable_all(enable);
		m_console.printf(enable ? "Enabled all breakpoints\n" : "Disabled all breakpoints\n");
	}
	else // otherwise, disable/enable the specific ones
	{
		execute_index_command(
				params,
				[this, enable] (device_t &device, u64 param) -> bool
				{
					if (!device.debug()->breakpoint_enable(param, enable))
						return false;
					m_console.printf(enable ? "Breakpoint %X enabled\n" : "Breakpoint %X disabled\n", param);
					return true;
				},
				"Invalid breakpoint number %X\n");
	}
}


/*-------------------------------------------------
    execute_bplist - execute the breakpoint list
    command
-------------------------------------------------*/

void debugger_commands::execute_bplist(const std::vector<std::string> &params)
{
	int printed = 0;
	std::string buffer;
	auto const apply =
			[this, &printed, &buffer] (device_t &device)
			{
				if (!device.debug()->breakpoint_list().empty())
				{
					m_console.printf("Device '%s' breakpoints:\n", device.tag());

					// loop over the breakpoints
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
			};

	if (!params.empty())
	{
		device_t *cpu;
		if (!validate_cpu_parameter(params[0], cpu))
			return;
		apply(*cpu);
		if (!printed)
			m_console.printf("No breakpoints currently installed for CPU %s\n", cpu->tag());
	}
	else
	{
		// loop over all CPUs
		for (device_t &device : device_enumerator(m_machine.root_device()))
			apply(device);
		if (!printed)
			m_console.printf("No breakpoints currently installed\n");
	}
}


/*-------------------------------------------------
    execute_wpset - execute the watchpoint set
    command
-------------------------------------------------*/

void debugger_commands::execute_wpset(int spacenum, const std::vector<std::string> &params)
{
	u64 address, length;
	address_space *space;

	// param 1 is the address/CPU
	if (!validate_target_address_parameter(params[0], spacenum, space, address))
		return;

	device_execute_interface const *execute;
	if (!space->device().interface(execute))
	{
		m_console.printf("Device %s is not a CPU\n", space->device().name());
		return;
	}
	device_debug *const debug = space->device().debug();

	// param 2 is the length
	if (!validate_number_parameter(params[1], length))
		return;

	// param 3 is the type
	read_or_write type;
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

	// param 4 is the condition
	parsed_expression condition(debug->symtable());
	if (params.size() > 3 && !debug_command_parameter_expression(params[3], condition))
		return;

	// param 5 is the action
	const char *action = nullptr;
	if (params.size() > 4 && !debug_command_parameter_command(action = params[4].c_str()))
		return;

	// set the watchpoint
	int const wpnum = debug->watchpoint_set(*space, type, address, length, (condition.is_empty()) ? nullptr : condition.original_string(), action);
	m_console.printf("Watchpoint %X set\n", wpnum);
}


/*-------------------------------------------------
    execute_wpclear - execute the watchpoint
    clear command
-------------------------------------------------*/

void debugger_commands::execute_wpclear(const std::vector<std::string> &params)
{
	if (params.empty()) // if no parameters, clear all
	{
		for (device_t &device : device_enumerator(m_machine.root_device()))
			device.debug()->watchpoint_clear_all();
		m_console.printf("Cleared all watchpoints\n");
	}
	else // otherwise, clear the specific ones
	{
		execute_index_command(
				params,
				[this] (device_t &device, u64 param) -> bool
				{
					if (!device.debug()->watchpoint_clear(param))
						return false;
					m_console.printf("Watchpoint %X cleared\n", param);
					return true;
				},
				"Invalid watchpoint number %X\n");
	}
}


/*-------------------------------------------------
    execute_wpdisenable - execute the watchpoint
    disable/enable commands
-------------------------------------------------*/

void debugger_commands::execute_wpdisenable(bool enable, const std::vector<std::string> &params)
{
	if (params.empty()) // if no parameters, disable/enable all
	{
		for (device_t &device : device_enumerator(m_machine.root_device()))
			device.debug()->watchpoint_enable_all(enable);
		m_console.printf(enable ? "Enabled all watchpoints\n" : "Disabled all watchpoints\n");
	}
	else // otherwise, disable/enable the specific ones
	{
		execute_index_command(
				params,
				[this, enable] (device_t &device, u64 param) -> bool
				{
					if (!device.debug()->watchpoint_enable(param, enable))
						return false;
					m_console.printf(enable ? "Watchpoint %X enabled\n" : "Watchpoint %X disabled\n", param);
					return true;
				},
				"Invalid watchpoint number %X\n");
	}
}


/*-------------------------------------------------
    execute_wplist - execute the watchpoint list
    command
-------------------------------------------------*/

void debugger_commands::execute_wplist(const std::vector<std::string> &params)
{
	int printed = 0;
	std::string buffer;
	auto const apply =
			[this, &printed, &buffer] (device_t &device)
			{
				for (int spacenum = 0; spacenum < device.debug()->watchpoint_space_count(); ++spacenum)
				{
					if (!device.debug()->watchpoint_vector(spacenum).empty())
					{
						static const char *const types[] = { "unkn ", "read ", "write", "r/w  " };

						m_console.printf(
								"Device '%s' %s space watchpoints:\n",
								device.tag(),
								device.debug()->watchpoint_vector(spacenum).front()->space().name());

						// loop over the watchpoints
						for (const auto &wp : device.debug()->watchpoint_vector(spacenum))
						{
							buffer = string_format(
									"%c%4X @ %0*X-%0*X %s",
									wp->enabled() ? ' ' : 'D', wp->index(),
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
				}
			};

	if (!params.empty())
	{
		device_t *cpu;
		if (!validate_cpu_parameter(params[0], cpu))
			return;
		apply(*cpu);
		if (!printed)
			m_console.printf("No watchpoints currently installed for CPU %s\n", cpu->tag());
	}
	else
	{
		// loop over all CPUs
		for (device_t &device : device_enumerator(m_machine.root_device()))
			apply(device);
		if (!printed)
			m_console.printf("No watchpoints currently installed\n");
	}
}


/*-------------------------------------------------
    execute_rpset - execute the registerpoint set
    command
-------------------------------------------------*/

void debugger_commands::execute_rpset(const std::vector<std::string> &params)
{
	// CPU is implicit
	device_t *cpu;
	if (!validate_cpu_parameter(std::string_view(), cpu))
		return;

	// param 1 is the condition
	parsed_expression condition(cpu->debug()->symtable());
	if (params.size() > 0 && !debug_command_parameter_expression(params[0], condition))
		return;

	// param 2 is the action
	const char *action = nullptr;
	if (params.size() > 1 && !debug_command_parameter_command(action = params[1].c_str()))
		return;

	// set the registerpoint
	int const rpnum = cpu->debug()->registerpoint_set(condition.original_string(), action);
	m_console.printf("Registerpoint %X set\n", rpnum);
}


/*-------------------------------------------------
    execute_rpclear - execute the registerpoint
    clear command
-------------------------------------------------*/

void debugger_commands::execute_rpclear(const std::vector<std::string> &params)
{
	if (params.empty()) // if no parameters, clear all
	{
		for (device_t &device : device_enumerator(m_machine.root_device()))
			device.debug()->registerpoint_clear_all();
		m_console.printf("Cleared all registerpoints\n");
	}
	else // otherwise, clear the specific ones
	{
		execute_index_command(
				params,
				[this] (device_t &device, u64 param) -> bool
				{
					if (!device.debug()->registerpoint_clear(param))
						return false;
					m_console.printf("Registerpoint %X cleared\n", param);
					return true;
				},
				"Invalid registerpoint number %X\n");
	}
}


/*-------------------------------------------------
    execute_rpdisenable - execute the registerpoint
    disable/enable commands
-------------------------------------------------*/

void debugger_commands::execute_rpdisenable(bool enable, const std::vector<std::string> &params)
{
	if (params.empty()) // if no parameters, disable/enable all
	{
		for (device_t &device : device_enumerator(m_machine.root_device()))
			device.debug()->registerpoint_enable_all(enable);
		m_console.printf(enable ? "Enabled all registerpoints\n" : "Disabled all registerpoints\n");
	}
	else // otherwise, disable/enable the specific ones
	{
		execute_index_command(
				params,
				[this, enable] (device_t &device, u64 param) -> bool
				{
					if (!device.debug()->registerpoint_enable(param, enable))
						return false;
					m_console.printf(enable ? "Registerpoint %X enabled\n" : "Breakpoint %X disabled\n", param);
					return true;
				},
				"Invalid registerpoint number %X\n");
	}
}


/*-------------------------------------------------
    execute_rplist - execute the registerpoint list
    command
-------------------------------------------------*/

void debugger_commands::execute_rplist(const std::vector<std::string> &params)
{
	int printed = 0;
	std::string buffer;
	auto const apply =
			[this, &printed, &buffer] (device_t &device)
			{
				if (!device.debug()->registerpoint_list().empty())
				{
					m_console.printf("Device '%s' registerpoints:\n", device.tag());

					// loop over the registerpoints
					for (const auto &rp : device.debug()->registerpoint_list())
					{
						buffer = string_format("%c%4X if %s", rp.enabled() ? ' ' : 'D', rp.index(), rp.condition());
						if (rp.action() && *rp.action())
							buffer.append(string_format(" do %s", rp.action()));
						m_console.printf("%s\n", buffer);
						printed++;
					}
				}
			};

	if (!params.empty())
	{
		device_t *cpu;
		if (!validate_cpu_parameter(params[0], cpu))
			return;
		apply(*cpu);
		if (!printed)
			m_console.printf("No registerpoints currently installed for CPU %s\n", cpu->tag());
	}
	else
	{
		// loop over all CPUs
		for (device_t &device : device_enumerator(m_machine.root_device()))
			apply(device);
		if (!printed)
			m_console.printf("No registerpoints currently installed\n");
	}
}


/*-------------------------------------------------
    execute_statesave - execute the statesave command
-------------------------------------------------*/

void debugger_commands::execute_statesave(const std::vector<std::string> &params)
{
	m_machine.immediate_save(params[0]);
	m_console.printf("State save attempted.  Please refer to window message popup for results.\n");
}


/*-------------------------------------------------
    execute_stateload - execute the stateload command
-------------------------------------------------*/

void debugger_commands::execute_stateload(const std::vector<std::string> &params)
{
	m_machine.immediate_load(params[0]);

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

void debugger_commands::execute_rewind(const std::vector<std::string> &params)
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

void debugger_commands::execute_save(int spacenum, const std::vector<std::string> &params)
{
	u64 offset, endoffset, length;
	address_space *space;

	// validate parameters
	if (!validate_target_address_parameter(params[1], spacenum, space, offset))
		return;
	if (!validate_number_parameter(params[2], length))
		return;

	// determine the addresses to write
	endoffset = (offset + length - 1) & space->addrmask();
	offset = offset & space->addrmask();
	endoffset++;

	// open the file
	FILE *const f = fopen(params[0].c_str(), "wb");
	if (!f)
	{
		m_console.printf("Error opening file '%s'\n", params[0]);
		return;
	}

	// now write the data out
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

	// close the file
	fclose(f);
	m_console.printf("Data saved successfully\n");
}


/*-------------------------------------------------
    execute_saveregion - execute the save command on region memory
-------------------------------------------------*/

void debugger_commands::execute_saveregion(const std::vector<std::string> &params)
{
	u64 offset, length;
	memory_region *region;

	// validate parameters
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

void debugger_commands::execute_load(int spacenum, const std::vector<std::string> &params)
{
	u64 offset, endoffset, length = 0;
	address_space *space;

	// validate parameters
	if (!validate_target_address_parameter(params[1], spacenum, space, offset))
		return;
	if (params.size() > 2 && !validate_number_parameter(params[2], length))
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
	auto dis = space->device().machine().disable_side_effects();
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

void debugger_commands::execute_loadregion(const std::vector<std::string> &params)
{
	u64 offset, length;
	memory_region *region;

	// validate parameters
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

	// open the file
	FILE *const f = fopen(params[0].c_str(), "rb");
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

void debugger_commands::execute_dump(int spacenum, const std::vector<std::string> &params)
{
	// validate parameters
	address_space *space;
	u64 offset;
	if (!validate_target_address_parameter(params[1], spacenum, space, offset))
		return;

	u64 length;
	if (!validate_number_parameter(params[2], length))
		return;

	u64 width = 0;
	if (params.size() > 3 && !validate_number_parameter(params[3], width))
		return;

	bool ascii = true;
	if (params.size() > 4 && !validate_boolean_parameter(params[4], ascii))
		return;

	u64 rowsize = space->byte_to_address(16);
	if (params.size() > 5 && !validate_number_parameter(params[5], rowsize))
		return;

	int shift = space->addr_shift();
	u64 granularity = shift >= 0 ? 1 : 1 << -shift;

	// further validation
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

	// open the file
	FILE *const f = fopen(params[0].c_str(), "w");
	if (!f)
	{
		m_console.printf("Error opening file '%s'\n", params[0]);
		return;
	}

	// now write the data out
	util::ovectorstream output;
	output.reserve(200);

	const unsigned delta = (shift >= 0) ? (width << shift) : (width >> -shift);

	auto dis = space->device().machine().disable_side_effects();
	bool be = space->endianness() == ENDIANNESS_BIG;

	for (u64 i = offset; i <= endoffset; i += rowsize)
	{
		output.clear();
		output.rdbuf()->clear();

		// print the address
		util::stream_format(output, "%0*X: ", space->logaddrchars(), i);

		// print the bytes
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

		// print the ASCII
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

		// output the result
		auto const &text = output.vec();
		fprintf(f, "%.*s\n", int(unsigned(text.size())), &text[0]);
	}

	// close the file
	fclose(f);
	m_console.printf("Data dumped successfully\n");
}


//-------------------------------------------------
//  execute_strdump - execute the strdump command
//-------------------------------------------------

void debugger_commands::execute_strdump(int spacenum, const std::vector<std::string> &params)
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
	if (!validate_device_space_parameter((params.size() > 4) ? params[4] : std::string_view(), spacenum, space))
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
   execute_cheatrange - add a range to search for
   cheats
-------------------------------------------------*/

void debugger_commands::execute_cheatrange(bool init, const std::vector<std::string> &params)
{
	address_space *space = m_cheat.space;
	if (!space && !init)
	{
		m_console.printf("Use cheatinit before cheatrange\n");
		return;
	}

	u8 width = (space || !init) ? m_cheat.width : 1;
	bool signed_cheat = (space || !init) ? m_cheat.signed_cheat : false;
	bool swapped_cheat = (space || !init) ? m_cheat.swapped_cheat : false;
	if (init)
	{
		// first argument is sign/size/swap flags
		if (!params.empty())
		{
			std::string const &srtpnt = params[0];
			if (!srtpnt.empty())
			{
				width = 1;
				signed_cheat = false;
				swapped_cheat = false;
			}

			if (srtpnt.length() >= 1)
			{
				char const sspec = std::tolower((unsigned char)srtpnt[0]);
				if (sspec == 's')
					signed_cheat = true;
				else if (sspec == 'u')
					signed_cheat = false;
				else
				{
					m_console.printf("Invalid sign: expected s or u\n");
					return;
				}
			}

			if (srtpnt.length() >= 2)
			{
				char const wspec = std::tolower((unsigned char)srtpnt[1]);
				if (wspec == 'b')
					width = 1;
				else if (wspec == 'w')
					width = 2;
				else if (wspec == 'd')
					width = 4;
				else if (wspec == 'q')
					width = 8;
				else
				{
					m_console.printf("Invalid width: expected b, w, d or q\n");
					return;
				}
			}

			if (srtpnt.length() >= 3)
			{
				if (std::tolower((unsigned char)srtpnt[2]) == 's')
					swapped_cheat = true;
				else
				{
					m_console.printf("Invalid swap: expected s\n");
					return;
				}
			}
		}

		// fourth argument is device/space
		if (!validate_device_space_parameter((params.size() > 3) ? params[3] : std::string_view(), -1, space))
			return;
	}

	cheat_region_map cheat_region[100]; // FIXME: magic number
	unsigned region_count = 0;
	if (params.size() >= (init ? 3 : 2))
	{
		// validate parameters
		u64 offset, length;
		if (!validate_number_parameter(params[init ? 1 : 0], offset))
			return;
		if (!validate_number_parameter(params[init ? 2 : 1], length))
			return;

		// force region to the specified range
		cheat_region[region_count].offset = offset & space->addrmask();
		cheat_region[region_count].endoffset = (offset + length - 1) & space->addrmask();
		cheat_region[region_count].share = nullptr;
		cheat_region[region_count].disabled = false;
		region_count++;
	}
	else
	{
		// initialize to entire memory by default
		for (address_map_entry &entry : space->map()->m_entrylist)
		{
			cheat_region[region_count].offset = entry.m_addrstart & space->addrmask();
			cheat_region[region_count].endoffset = entry.m_addrend & space->addrmask();
			cheat_region[region_count].share = entry.m_share;
			cheat_region[region_count].disabled = entry.m_write.m_type != AMH_RAM;

			// disable duplicate share regions
			if (entry.m_share)
				for (unsigned i = 0; i < region_count; i++)
					if (cheat_region[i].share && !strcmp(cheat_region[i].share, entry.m_share))
						cheat_region[region_count].disabled = true;

			if (!cheat_region[region_count].disabled)
				region_count++;
		}
	}

	// determine the writable extent of each region in total
	u64 real_length = 0;
	for (unsigned i = 0; i < region_count; i++)
		for (u64 curaddr = cheat_region[i].offset; curaddr <= cheat_region[i].endoffset; curaddr += width)
			if (cheat_address_is_valid(*space, curaddr))
				real_length++;

	if (!real_length)
	{
		m_console.printf("No writable bytes found in this area\n");
		return;
	}

	size_t active_cheat = 0;
	if (init)
	{
		// initialize new cheat system
		m_cheat.space = space;
		m_cheat.width = width;
		m_cheat.undo = 0;
		m_cheat.signed_cheat = signed_cheat;
		m_cheat.swapped_cheat = swapped_cheat;
	}
	else
	{
		active_cheat = m_cheat.cheatmap.size();
	}
	m_cheat.cheatmap.resize(active_cheat + real_length);

	// initialize cheatmap in the selected space
	for (unsigned i = 0; i < region_count; i++)
		for (u64 curaddr = cheat_region[i].offset; curaddr <= cheat_region[i].endoffset; curaddr += width)
			if (cheat_address_is_valid(*space, curaddr))
			{
				m_cheat.cheatmap[active_cheat].previous_value = m_cheat.read_extended(curaddr);
				m_cheat.cheatmap[active_cheat].first_value = m_cheat.cheatmap[active_cheat].previous_value;
				m_cheat.cheatmap[active_cheat].offset = curaddr;
				m_cheat.cheatmap[active_cheat].state = 1;
				m_cheat.cheatmap[active_cheat].undo = 0;
				active_cheat++;
			}

	// give a detailed init message to avoid searches being mistakenly carried out on the wrong CPU
	m_console.printf(
			"%u cheat locations initialized for %s '%s' %s space\n",
			active_cheat,
			space->device().type().fullname(),
			space->device().tag(),
			space->name());
}


/*-------------------------------------------------
    execute_cheatnext - execute the search
-------------------------------------------------*/

void debugger_commands::execute_cheatnext(bool initial, const std::vector<std::string> &params)
{
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

	address_space *const space = m_cheat.space;
	if (!space)
	{
		m_console.printf("Use cheatinit before cheatnext\n");
		return;
	}

	u64 comp_value = 0;
	if (params.size() > 1 && !validate_number_parameter(params[1], comp_value))
		return;
	comp_value = m_cheat.sign_extend(comp_value);

	// decode condition
	u8 condition;
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

	// execute the search
	u32 active_cheat = 0;
	for (u64 cheatindex = 0; cheatindex < m_cheat.cheatmap.size(); cheatindex += 1)
		if (m_cheat.cheatmap[cheatindex].state == 1)
		{
			u64 cheat_value = m_cheat.read_extended(m_cheat.cheatmap[cheatindex].offset);
			u64 comp_byte = initial
					? m_cheat.cheatmap[cheatindex].first_value
					: m_cheat.cheatmap[cheatindex].previous_value;
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

			// update previous value
			m_cheat.cheatmap[cheatindex].previous_value = cheat_value;
		}

	if (active_cheat <= 5)
		execute_cheatlist(std::vector<std::string>());

	m_console.printf("%u cheats found\n", active_cheat);
}


/*-------------------------------------------------
    execute_cheatlist - show a list of active cheat
-------------------------------------------------*/

void debugger_commands::execute_cheatlist(const std::vector<std::string> &params)
{
	address_space *const space = m_cheat.space;
	if (!space)
	{
		m_console.printf("Use cheatinit before cheatlist\n");
		return;
	}

	FILE *f = nullptr;
	if (params.size() > 0)
	{
		f = fopen(params[0].c_str(), "w");
		if (!f)
		{
			m_console.printf("Error opening file '%s'\n", params[0]);
			return;
		}
	}

	// get device/space syntax for memory access
	std::string tag(space->device().tag());
	std::string spaceletter;
	switch (space->spacenum())
	{
	default:
		tag.append(1, ':');
		tag.append(space->name());
		break;
	case AS_PROGRAM:
		spaceletter = "p";
		break;
	case AS_DATA:
		spaceletter = "d";
		break;
	case AS_IO:
		spaceletter = "i";
		break;
	case AS_OPCODES:
		spaceletter = "3";
		break;
	}

	// get size syntax for memory access and formatting values
	bool const octal = space->is_octal();
	int const addrchars = octal
			? ((2 + space->logaddr_width()) / 3)
			: ((3 + space->logaddr_width()) / 4);
	int const datachars = octal
			? ((2 + (m_cheat.width * 8)) / 3)
			: ((3 + (m_cheat.width * 8)) / 4);
	u64 const sizemask = util::make_bitmask<u64>(m_cheat.width * 8);
	char sizeletter;
	switch (m_cheat.width)
	{
	default:
	case 1: sizeletter = 'b';   break;
	case 2: sizeletter = 'w';   break;
	case 4: sizeletter = 'd';   break;
	case 8: sizeletter = 'q';   break;
	}

	// write the cheat list
	u32 active_cheat = 0;
	util::ovectorstream output;
	for (u64 cheatindex = 0; cheatindex < m_cheat.cheatmap.size(); cheatindex += 1)
	{
		if (m_cheat.cheatmap[cheatindex].state == 1)
		{
			u64 const value = m_cheat.byte_swap(m_cheat.read_extended(m_cheat.cheatmap[cheatindex].offset)) & sizemask;
			u64 const first_value = m_cheat.byte_swap(m_cheat.cheatmap[cheatindex].first_value) & sizemask;
			offs_t const address = space->byte_to_address(m_cheat.cheatmap[cheatindex].offset);

			if (!params.empty())
			{
				active_cheat++;
				output.clear();
				output.rdbuf()->clear();
				stream_format(
						output,
						octal ?
							"  <cheat desc=\"Possibility %d: 0%0*o (0%0*o)\">\n"
							"    <script state=\"run\">\n"
							"      <action>%s.%s%c@0o%0*o=0o%0*o</action>\n"
							"    </script>\n"
							"  </cheat>\n\n" :
							"  <cheat desc=\"Possibility %d: %0*X (%0*X)\">\n"
							"    <script state=\"run\">\n"
							"      <action>%s.%s%c@0x%0*X=0x%0*X</action>\n"
							"    </script>\n"
							"  </cheat>\n\n",
						active_cheat, addrchars, address, datachars, value,
						tag, spaceletter, sizeletter, addrchars, address, datachars, first_value);
				auto const &text(output.vec());
				fprintf(f, "%.*s", int(unsigned(text.size())), &text[0]);
			}
			else
			{
				m_console.printf(
						octal
							? "Address=0%0*o Start=0%0*o Current=0%0*o\n"
							: "Address=%0*X Start=%0*X Current=%0*X\n",
						addrchars, address,
						datachars, first_value,
						datachars, value);
			}
		}
	}
	if (params.size() > 0)
		fclose(f);
}


/*-------------------------------------------------
    execute_cheatundo - undo the last search
-------------------------------------------------*/

void debugger_commands::execute_cheatundo(const std::vector<std::string> &params)
{
	if (m_cheat.undo > 0)
	{
		u64 undo_count = 0;
		for (u64 cheatindex = 0; cheatindex < m_cheat.cheatmap.size(); cheatindex += 1)
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
	{
		m_console.printf("Maximum undo reached\n");
	}
}


/*-------------------------------------------------
    execute_find - execute the find command
-------------------------------------------------*/

void debugger_commands::execute_find(int spacenum, const std::vector<std::string> &params)
{
	u64 offset, length;
	address_space *space;

	// validate parameters
	if (!validate_target_address_parameter(params[0], spacenum, space, offset))
		return;
	if (!validate_number_parameter(params[1], length))
		return;

	// further validation
	u64 const endoffset = space->address_to_byte_end((offset + length - 1) & space->addrmask());
	offset = space->address_to_byte(offset & space->addrmask());
	int cur_data_size = (space->addr_shift() > 0) ? 2 : (1 << -space->addr_shift());
	if (cur_data_size == 0)
		cur_data_size = 1;

	// parse the data parameters
	u64 data_to_find[256];
	u8 data_size[256];
	int data_count = 0;
	for (int i = 2; i < params.size(); i++)
	{
		char const *pdata = params[i].c_str();
		auto const pdatalen = params[i].length() - 1;

		if (pdata[0] == '"' && pdata[pdatalen] == '"') // check for a string
		{
			for (int j = 1; j < pdatalen; j++)
			{
				data_to_find[data_count] = pdata[j];
				data_size[data_count++] = 1;
			}
		}
		else // otherwise, validate as a number
		{
			// check for a 'b','w','d',or 'q' prefix
			data_size[data_count] = cur_data_size;
			if (tolower(u8(pdata[0])) == 'b' && pdata[1] == '.') { data_size[data_count] = cur_data_size = 1; pdata += 2; }
			if (tolower(u8(pdata[0])) == 'w' && pdata[1] == '.') { data_size[data_count] = cur_data_size = 2; pdata += 2; }
			if (tolower(u8(pdata[0])) == 'd' && pdata[1] == '.') { data_size[data_count] = cur_data_size = 4; pdata += 2; }
			if (tolower(u8(pdata[0])) == 'q' && pdata[1] == '.') { data_size[data_count] = cur_data_size = 8; pdata += 2; }

			// look for a wildcard
			if (!strcmp(pdata, "?"))
				data_size[data_count++] |= 0x10;

			// otherwise, validate as a number
			else if (!validate_number_parameter(pdata, data_to_find[data_count++]))
				return;
		}
	}

	// now search
	device_memory_interface &memory = space->device().memory();
	auto dis = space->device().machine().disable_side_effects();
	int found = 0;
	for (u64 i = offset; i <= endoffset; i += data_size[0])
	{
		int suboffset = 0;
		bool match = true;

		// find the entire string
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
				// all other cases are wildcards
				break;
			}
			suboffset += data_size[j] & 0x0f;
		}

		// did we find it?
		if (match)
		{
			found++;
			m_console.printf("Found at %0*X\n", space->addrchars(), u32(space->byte_to_address(i)));
		}
	}

	// print something if not found
	if (found == 0)
		m_console.printf("Not found\n");
}


//-------------------------------------------------
//  execute_fill - execute the fill command
//-------------------------------------------------

void debugger_commands::execute_fill(int spacenum, const std::vector<std::string> &params)
{
	u64 offset, length;
	address_space *space;

	// validate parameters
	if (!validate_target_address_parameter(params[0], spacenum, space, offset))
		return;
	if (!validate_number_parameter(params[1], length))
		return;

	// further validation
	offset = space->address_to_byte(offset & space->addrmask());
	int cur_data_size = (space->addr_shift() > 0) ? 2 : (1 << -space->addr_shift());
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

void debugger_commands::execute_dasm(const std::vector<std::string> &params)
{
	u64 offset, length;
	bool bytes = true;
	address_space *space;

	// validate parameters
	if (!validate_number_parameter(params[1], offset))
		return;
	if (!validate_number_parameter(params[2], length))
		return;
	if (params.size() > 3 && !validate_boolean_parameter(params[3], bytes))
		return;
	if (!validate_device_space_parameter(params.size() > 4 ? params[4] : std::string_view(), AS_PROGRAM, space))
		return;

	// determine the width of the bytes
	device_disasm_interface *dasmintf;
	if (!space->device().interface(dasmintf))
	{
		m_console.printf("No disassembler available for %s\n", space->device().name());
		return;
	}

	// build the data, check the maximum size of the opcodes and disasm
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
		if (osize > max_opcodes_size)
			max_opcodes_size = osize;

		int dsize = instructions.back().size();
		if (dsize > max_disasm_size)
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
		for (unsigned int i=0; i != pcs.size(); i++)
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
		for (unsigned int i=0; i != pcs.size(); i++)
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
    execute_trace - functionality for
    trace over and trace info
-------------------------------------------------*/

void debugger_commands::execute_trace(const std::vector<std::string> &params, bool trace_over)
{
	const char *action = nullptr;
	bool detect_loops = true;
	bool logerror = false;
	device_t *cpu;
	const char *mode;
	std::string filename = params[0];

	// replace macros
	strreplace(filename, "{game}", m_machine.basename());

	// validate parameters
	if (!validate_cpu_parameter(params.size() > 1 ? params[1] : std::string_view(), cpu))
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

	// open the file
	FILE *f = nullptr;
	if (core_stricmp(filename.c_str(), "off") != 0)
	{
		mode = "w";

		// opening for append?
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

	// do it
	cpu->debug()->trace(f, trace_over, detect_loops, logerror, action);
	if (f)
		m_console.printf("Tracing CPU '%s' to file %s\n", cpu->tag(), filename);
	else
		m_console.printf("Stopped tracing on CPU '%s'\n", cpu->tag());
}


/*-------------------------------------------------
    execute_traceflush - execute the trace flush command
-------------------------------------------------*/

void debugger_commands::execute_traceflush(const std::vector<std::string> &params)
{
	m_machine.debugger().cpu().flush_traces();
}


/*-------------------------------------------------
    execute_history - execute the history command
-------------------------------------------------*/

void debugger_commands::execute_history(const std::vector<std::string> &params)
{
	// validate parameters
	device_t *device;
	if (!validate_cpu_parameter(!params.empty() ? params[0] : std::string_view(), device))
		return;

	u64 count = device_debug::HISTORY_SIZE;
	if (params.size() > 1 && !validate_number_parameter(params[1], count))
		return;

	// further validation
	if (count > device_debug::HISTORY_SIZE)
		count = device_debug::HISTORY_SIZE;

	device_debug *const debug = device->debug();

	device_disasm_interface *dasmintf;
	if (!device->interface(dasmintf))
	{
		m_console.printf("No disassembler available for device %s\n", device->name());
		return;
	}

	// loop over lines
	debug_disasm_buffer buffer(*device);
	std::string instruction;
	for (int index = int(unsigned(count)); index > 0; index--)
	{
		auto const pc = debug->history_pc(1 - index);
		if (pc.second)
		{
			offs_t next_offset;
			offs_t size;
			u32 info;
			instruction.clear();
			buffer.disassemble(pc.first, instruction, next_offset, size, info);

			m_console.printf("%s: %s\n", buffer.pc_to_string(pc.first), instruction);
		}
	}
}


/*-------------------------------------------------
    execute_trackpc - execute the trackpc command
-------------------------------------------------*/

void debugger_commands::execute_trackpc(const std::vector<std::string> &params)
{
	// Gather the on/off switch (if present)
	bool turnOn = true;
	if (params.size() > 0 && !validate_boolean_parameter(params[0], turnOn))
		return;

	// Gather the cpu id (if present)
	device_t *cpu = nullptr;
	if (!validate_cpu_parameter((params.size() > 1) ? params[1] : std::string_view(), cpu))
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

void debugger_commands::execute_trackmem(const std::vector<std::string> &params)
{
	// Gather the on/off switch (if present)
	bool turnOn = true;
	if (params.size() > 0 && !validate_boolean_parameter(params[0], turnOn))
		return;

	// Gather the cpu id (if present)
	std::string_view cpuparam;
	if (params.size() > 1)
		cpuparam = params[1];
	device_t *cpu = nullptr;
	if (!validate_cpu_parameter(cpuparam, cpu))
		return;

	// Should we clear the existing data?
	bool clear = false;
	if (params.size() > 2 && !validate_boolean_parameter(params[2], clear))
		return;

	// Get the address space for the given cpu
	address_space *space;
	if (!validate_device_space_parameter(cpuparam, AS_PROGRAM, space))
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

void debugger_commands::execute_pcatmem(int spacenum, const std::vector<std::string> &params)
{
	// Gather the required target address/space parameter
	u64 address;
	address_space *space;
	if (!validate_target_address_parameter(params[0], spacenum, space, address))
		return;

	// Translate the address
	offs_t a = address & space->logaddrmask();
	if (!space->device().memory().translate(space->spacenum(), TRANSLATE_READ_DEBUG, a))
	{
		m_console.printf("Address translation failed\n");
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
	const offs_t result = space->device().debug()->track_mem_pc_from_space_address_data(space->spacenum(), address, data);
	if (result != (offs_t)(-1))
		m_console.printf("%02x\n", result);
	else
		m_console.printf("UNKNOWN PC\n");
}


/*-------------------------------------------------
    execute_snap - execute the snapshot command
-------------------------------------------------*/

void debugger_commands::execute_snap(const std::vector<std::string> &params)
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
		std::error_condition filerr = file.open(std::move(fname));

		if (filerr)
		{
			m_console.printf("Error creating file '%s' (%s:%d %s)\n", filename, filerr.category().name(), filerr.value(), filerr.message());
			return;
		}

		screen->machine().video().save_snapshot(screen, file);
		m_console.printf("Saved screen #%d snapshot as '%s'\n", scrnum, filename);
	}
}


/*-------------------------------------------------
    execute_source - execute the source command
-------------------------------------------------*/

void debugger_commands::execute_source(const std::vector<std::string> &params)
{
	m_console.source_script(params[0].c_str());
}


/*-------------------------------------------------
    execute_map - execute the map command
-------------------------------------------------*/

void debugger_commands::execute_map(int spacenum, const std::vector<std::string> &params)
{
	// validate parameters
	u64 address;
	address_space *space;
	if (!validate_target_address_parameter(params[0], spacenum, space, address))
		return;

	// do the translation first
	for (int intention = TRANSLATE_READ_DEBUG; intention <= TRANSLATE_FETCH_DEBUG; intention++)
	{
		static const char *const intnames[] = { "Read", "Write", "Fetch" };
		offs_t taddress = address & space->addrmask();
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

void debugger_commands::execute_memdump(const std::vector<std::string> &params)
{
	device_t *root = &m_machine.root_device();
	if ((params.size() >= 2) && !validate_device_parameter(params[1], root))
		return;

	char const *const filename = params.empty() ? "memdump.log" : params[0].c_str();
	FILE *const file = fopen(filename, "w");
	if (!file)
	{
		m_console.printf("Error opening file %s\n", filename);
		return;
	}

	m_console.printf("Dumping memory maps to %s\n", filename);

	try
	{
		memory_interface_enumerator iter(*root);
		std::vector<memory_entry> entries[2];
		for (device_memory_interface &memory : iter)
		{
			for (int space = 0; space != memory.max_space_count(); space++)
				if (memory.has_space(space))
				{
					address_space &sp = memory.space(space);
					bool octal = sp.is_octal();
					int nc = octal ? (sp.addr_width() + 2) / 3 : (sp.addr_width() + 3) / 4;

					sp.dump_maps(entries[0], entries[1]);
					for (int mode = 0; mode < 2; mode ++)
					{
						fprintf(file, "  %s '%s' space %s %s:\n", memory.device().type().fullname(), memory.device().tag(), sp.name(), mode ? "write" : "read");
						for (memory_entry &entry : entries[mode])
						{
							if (octal)
								fprintf(file, "%0*o - %0*o:", nc, entry.start, nc, entry.end);
							else
								fprintf(file, "%0*x - %0*x:", nc, entry.start, nc, entry.end);
							for (const auto &c : entry.context)
								if (c.disabled)
									fprintf(file, " %s[off]", c.view->name().c_str());
								else
									fprintf(file, " %s[%d]", c.view->name().c_str(), c.slot);
							fprintf(file, " %s\n", entry.entry->name().c_str());
						}
						fprintf(file, "\n");
					}
					entries[0].clear();
					entries[1].clear();
				}
		}
		fclose(file);
	}
	catch (...)
	{
		fclose(file);
		throw;
	}
}


/*-------------------------------------------------
    execute_symlist - execute the symlist command
-------------------------------------------------*/

void debugger_commands::execute_symlist(const std::vector<std::string> &params)
{
	const char *namelist[1000];
	symbol_table *symtable;
	int count = 0;

	if (!params.empty())
	{
		// validate parameters
		device_t *cpu;
		if (!validate_cpu_parameter(params[0], cpu))
			return;
		symtable = &cpu->debug()->symtable();
		m_console.printf("CPU '%s' symbols:\n", cpu->tag());
	}
	else
	{
		symtable = &m_machine.debugger().cpu().global_symtable();
		m_console.printf("Global symbols:\n");
	}

	// gather names for all symbols
	for (auto &entry : symtable->entries())
	{
		// only display "register" type symbols
		if (!entry.second->is_function())
		{
			namelist[count++] = entry.second->name();
			if (count >= std::size(namelist))
				break;
		}
	}

	// sort the symbols
	if (count > 1)
	{
		std::sort(
				&namelist[0],
				&namelist[count],
				[] (const char *item1, const char *item2) { return strcmp(item1, item2) < 0; });
	}

	// iterate over symbols and print out relevant ones
	for (int symnum = 0; symnum < count; symnum++)
	{
		symbol_entry const *const entry = symtable->find(namelist[symnum]);
		assert(entry != nullptr);
		u64 value = entry->value();

		// only display "register" type symbols
		m_console.printf("%s = %X", namelist[symnum], value);
		if (!entry->is_lval())
			m_console.printf("  (read-only)");
		m_console.printf("\n");
	}
}


/*-------------------------------------------------
    execute_softreset - execute the softreset command
-------------------------------------------------*/

void debugger_commands::execute_softreset(const std::vector<std::string> &params)
{
	m_machine.schedule_soft_reset();
}


/*-------------------------------------------------
    execute_hardreset - execute the hardreset command
-------------------------------------------------*/

void debugger_commands::execute_hardreset(const std::vector<std::string> &params)
{
	m_machine.schedule_hard_reset();
}

/*-------------------------------------------------
    execute_images - lists all image devices with
    mounted files
-------------------------------------------------*/

void debugger_commands::execute_images(const std::vector<std::string> &params)
{
	image_interface_enumerator iter(m_machine.root_device());
	for (device_image_interface &img : iter)
	{
		if (!img.exists())
		{
			m_console.printf("%s: [no media]\n", img.brief_instance_name());
		}
		else if (img.loaded_through_softlist())
		{
			m_console.printf("%s: %s:%s:%s\n",
					img.brief_instance_name(),
					img.software_list_name(),
					img.software_entry()->shortname(),
					img.part_entry()->name());
		}
		else
		{
			m_console.printf("%s: %s\n", img.brief_instance_name(), img.filename());
		}
	}
	if (!iter.first())
		m_console.printf("No image devices present\n");
}

/*-------------------------------------------------
    execute_mount - execute the image mount command
-------------------------------------------------*/

void debugger_commands::execute_mount(const std::vector<std::string> &params)
{
	for (device_image_interface &img : image_interface_enumerator(m_machine.root_device()))
	{
		if ((img.instance_name() == params[0]) || (img.brief_instance_name() == params[0]))
		{
			if (img.load(params[1]) != image_init_result::PASS)
				m_console.printf("Unable to mount file %s on %s\n", params[1], params[0]);
			else
				m_console.printf("File %s mounted on %s\n", params[1], params[0]);
			return;
		}
	}
	m_console.printf("No image instance %s\n", params[0]);
}

/*-------------------------------------------------
    execute_unmount - execute the image unmount command
-------------------------------------------------*/

void debugger_commands::execute_unmount(const std::vector<std::string> &params)
{
	for (device_image_interface &img : image_interface_enumerator(m_machine.root_device()))
	{
		if ((img.instance_name() == params[0]) || (img.brief_instance_name() == params[0]))
		{
			if (img.exists())
			{
				img.unload();
				m_console.printf("Unmounted media from %s\n", params[0]);
			}
			else
			{
				m_console.printf("No media mounted on %s\n", params[0]);
			}
			return;
		}
	}
	m_console.printf("No image instance %s\n", params[0]);
}


/*-------------------------------------------------
    execute_input - debugger command to enter
    natural keyboard input
-------------------------------------------------*/

void debugger_commands::execute_input(const std::vector<std::string> &params)
{
	m_machine.natkeyboard().post_coded(params[0]);
}


/*-------------------------------------------------
    execute_dumpkbd - debugger command to natural
    keyboard codes
-------------------------------------------------*/

void debugger_commands::execute_dumpkbd(const std::vector<std::string> &params)
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
