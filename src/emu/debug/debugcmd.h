// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Ryan Holtz
/*********************************************************************

    debugcmd.h

    Debugger command interface engine.

*********************************************************************/

#ifndef MAME_EMU_DEBUG_DEBUGCMD_H
#define MAME_EMU_DEBUG_DEBUGCMD_H

#pragma once

#include "debugcpu.h"
#include "debugcon.h"

class parsed_expression;
class symbol_table;

class debugger_commands
{
public:
	debugger_commands(running_machine& machine, debugger_cpu& cpu, debugger_console& console);

	/* validates a parameter as a boolean value */
	bool validate_boolean_parameter(const char *param, bool *result);

	/* validates a parameter as a numeric value */
	bool validate_number_parameter(const char *param, u64 *result);

	/* validates a parameter as a cpu */
	bool validate_cpu_parameter(const char *param, device_t **result);

	/* validates a parameter as a cpu and retrieves the given address space */
	bool validate_cpu_space_parameter(const char *param, int spacenum, address_space *&result);

private:
	struct global_entry
	{
		void *      base;
		u32         size;
	};


	struct cheat_map
	{
		u64         offset;
		u64         first_value;
		u64         previous_value;
		u8          state:1;
		u8          undo:7;
	};

	// TODO [RH 31 May 2016]: Move this cheat stuff into its own class
	struct cheat_system
	{
		char        cpu[2];
		u8          width;
		std::vector<cheat_map> cheatmap;
		u8          undo;
		u8          signed_cheat;
		u8          swapped_cheat;
	};


	struct cheat_region_map
	{
		u64         offset;
		u64         endoffset;
		const char *share;
		u8          disabled;
	};

	bool debug_command_parameter_expression(const char *param, parsed_expression &result);
	bool debug_command_parameter_command(const char *param);

	bool cheat_address_is_valid(address_space &space, offs_t address);
	u64 cheat_sign_extend(const cheat_system *cheatsys, u64 value);
	u64 cheat_byte_swap(const cheat_system *cheatsys, u64 value);
	u64 cheat_read_extended(const cheat_system *cheatsys, address_space &space, offs_t address);

	u64 execute_min(symbol_table &table, void *ref, int params, const u64 *param);
	u64 execute_max(symbol_table &table, void *ref, int params, const u64 *param);
	u64 execute_if(symbol_table &table, void *ref, int params, const u64 *param);

	u64 global_get(symbol_table &table, void *ref);
	void global_set(symbol_table &table, void *ref, u64 value);

	int mini_printf(char *buffer, const char *format, int params, u64 *param);

	void execute_trace_internal(int ref, int params, const char *param[], bool trace_over);

	void execute_help(int ref, int params, const char **param);
	void execute_print(int ref, int params, const char **param);
	void execute_printf(int ref, int params, const char **param);
	void execute_logerror(int ref, int params, const char **param);
	void execute_tracelog(int ref, int params, const char **param);
	void execute_tracesym(int ref, int params, const char **param);
	void execute_quit(int ref, int params, const char **param);
	void execute_do(int ref, int params, const char **param);
	void execute_step(int ref, int params, const char **param);
	void execute_over(int ref, int params, const char **param);
	void execute_out(int ref, int params, const char **param);
	void execute_go(int ref, int params, const char **param);
	void execute_go_vblank(int ref, int params, const char **param);
	void execute_go_interrupt(int ref, int params, const char **param);
	void execute_go_time(int ref, int params, const char *param[]);
	void execute_focus(int ref, int params, const char **param);
	void execute_ignore(int ref, int params, const char **param);
	void execute_observe(int ref, int params, const char **param);
	void execute_next(int ref, int params, const char **param);
	void execute_comment_add(int ref, int params, const char **param);
	void execute_comment_del(int ref, int params, const char **param);
	void execute_comment_save(int ref, int params, const char **param);
	void execute_comment_list(int ref, int params, const char **param);
	void execute_comment_commit(int ref, int params, const char **param);
	void execute_bpset(int ref, int params, const char **param);
	void execute_bpclear(int ref, int params, const char **param);
	void execute_bpdisenable(int ref, int params, const char **param);
	void execute_bplist(int ref, int params, const char **param);
	void execute_wpset(int ref, int params, const char **param);
	void execute_wpclear(int ref, int params, const char **param);
	void execute_wpdisenable(int ref, int params, const char **param);
	void execute_wplist(int ref, int params, const char **param);
	void execute_rpset(int ref, int params, const char **param);
	void execute_rpclear(int ref, int params, const char **param);
	void execute_rpdisenable(int ref, int params, const char **param);
	void execute_rplist(int ref, int params, const char **param);
	void execute_hotspot(int ref, int params, const char **param);
	void execute_statesave(int ref, int params, const char **param);
	void execute_stateload(int ref, int params, const char **param);
	void execute_save(int ref, int params, const char **param);
	void execute_load(int ref, int params, const char **param);
	void execute_dump(int ref, int params, const char **param);
	void execute_cheatinit(int ref, int params, const char **param);
	void execute_cheatnext(int ref, int params, const char **param);
	void execute_cheatlist(int ref, int params, const char **param);
	void execute_cheatundo(int ref, int params, const char **param);
	void execute_dasm(int ref, int params, const char **param);
	void execute_find(int ref, int params, const char **param);
	void execute_trace(int ref, int params, const char **param);
	void execute_traceover(int ref, int params, const char **param);
	void execute_traceflush(int ref, int params, const char **param);
	void execute_history(int ref, int params, const char **param);
	void execute_trackpc(int ref, int params, const char **param);
	void execute_trackmem(int ref, int params, const char **param);
	void execute_pcatmem(int ref, int params, const char **param);
	void execute_snap(int ref, int params, const char **param);
	void execute_source(int ref, int params, const char **param);
	void execute_map(int ref, int params, const char **param);
	void execute_memdump(int ref, int params, const char **param);
	void execute_symlist(int ref, int params, const char **param);
	void execute_softreset(int ref, int params, const char **param);
	void execute_hardreset(int ref, int params, const char **param);
	void execute_images(int ref, int params, const char **param);
	void execute_mount(int ref, int params, const char **param);
	void execute_unmount(int ref, int params, const char **param);
	void execute_input(int ref, int params, const char **param);
	void execute_dumpkbd(int ref, int params, const char **param);

	running_machine&    m_machine;
	debugger_cpu&       m_cpu;
	debugger_console&   m_console;

	global_entry *m_global_array;
	cheat_system m_cheat;

	static const size_t MAX_GLOBALS;
};

#endif // MAME_EMU_DEBUG_DEBUGCMD_H
