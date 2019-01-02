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


class debugger_commands
{
public:
	debugger_commands(running_machine& machine, debugger_cpu& cpu, debugger_console& console);

	/* validates a parameter as a boolean value */
	bool validate_boolean_parameter(const std::string &param, bool &result);

	/* validates a parameter as a numeric value */
	bool validate_number_parameter(const std::string &param, u64 &result);

	/* validates a parameter as a cpu */
	bool validate_cpu_parameter(const char *param, device_t *&result);

	/* validates a parameter as a cpu and retrieves the given address space */
	bool validate_cpu_space_parameter(const char *param, int spacenum, address_space *&result);

private:
	struct global_entry
	{
		global_entry() { }

		void *      base = nullptr;
		u32         size = 0;
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

	bool debug_command_parameter_expression(const std::string &param, parsed_expression &result);
	bool debug_command_parameter_command(const char *param);

	bool cheat_address_is_valid(address_space &space, offs_t address);
	u64 cheat_sign_extend(const cheat_system *cheatsys, u64 value);
	u64 cheat_byte_swap(const cheat_system *cheatsys, u64 value);
	u64 cheat_read_extended(const cheat_system *cheatsys, address_space &space, offs_t address);

	u64 execute_min(symbol_table &table, int params, const u64 *param);
	u64 execute_max(symbol_table &table, int params, const u64 *param);
	u64 execute_if(symbol_table &table, int params, const u64 *param);

	u64 global_get(symbol_table &table, global_entry *global);
	void global_set(symbol_table &table, global_entry *global, u64 value);

	int mini_printf(char *buffer, const char *format, int params, u64 *param);

	void execute_trace_internal(int ref, const std::vector<std::string> &params, bool trace_over);

	void execute_help(int ref, const std::vector<std::string> &params);
	void execute_print(int ref, const std::vector<std::string> &params);
	void execute_printf(int ref, const std::vector<std::string> &params);
	void execute_logerror(int ref, const std::vector<std::string> &params);
	void execute_tracelog(int ref, const std::vector<std::string> &params);
	void execute_tracesym(int ref, const std::vector<std::string> &params);
	void execute_quit(int ref, const std::vector<std::string> &params);
	void execute_do(int ref, const std::vector<std::string> &params);
	void execute_step(int ref, const std::vector<std::string> &params);
	void execute_over(int ref, const std::vector<std::string> &params);
	void execute_out(int ref, const std::vector<std::string> &params);
	void execute_go(int ref, const std::vector<std::string> &params);
	void execute_go_vblank(int ref, const std::vector<std::string> &params);
	void execute_go_interrupt(int ref, const std::vector<std::string> &params);
	void execute_go_exception(int ref, const std::vector<std::string> &params);
	void execute_go_time(int ref, const std::vector<std::string> &params);
	void execute_go_privilege(int ref, const std::vector<std::string> &params);
	void execute_focus(int ref, const std::vector<std::string> &params);
	void execute_ignore(int ref, const std::vector<std::string> &params);
	void execute_observe(int ref, const std::vector<std::string> &params);
	void execute_suspend(int ref, const std::vector<std::string> &params);
	void execute_resume(int ref, const std::vector<std::string> &params);
	void execute_next(int ref, const std::vector<std::string> &params);
	void execute_comment_add(int ref, const std::vector<std::string> &params);
	void execute_comment_del(int ref, const std::vector<std::string> &params);
	void execute_comment_save(int ref, const std::vector<std::string> &params);
	void execute_comment_list(int ref, const std::vector<std::string> &params);
	void execute_comment_commit(int ref, const std::vector<std::string> &params);
	void execute_bpset(int ref, const std::vector<std::string> &params);
	void execute_bpclear(int ref, const std::vector<std::string> &params);
	void execute_bpdisenable(int ref, const std::vector<std::string> &params);
	void execute_bplist(int ref, const std::vector<std::string> &params);
	void execute_wpset(int ref, const std::vector<std::string> &params);
	void execute_wpclear(int ref, const std::vector<std::string> &params);
	void execute_wpdisenable(int ref, const std::vector<std::string> &params);
	void execute_wplist(int ref, const std::vector<std::string> &params);
	void execute_rpset(int ref, const std::vector<std::string> &params);
	void execute_rpclear(int ref, const std::vector<std::string> &params);
	void execute_rpdisenable(int ref, const std::vector<std::string> &params);
	void execute_rplist(int ref, const std::vector<std::string> &params);
	void execute_hotspot(int ref, const std::vector<std::string> &params);
	void execute_statesave(int ref, const std::vector<std::string> &params);
	void execute_stateload(int ref, const std::vector<std::string> &params);
	void execute_rewind(int ref, const std::vector<std::string> &params);
	void execute_save(int ref, const std::vector<std::string> &params);
	void execute_load(int ref, const std::vector<std::string> &params);
	void execute_dump(int ref, const std::vector<std::string> &params);
	void execute_cheatinit(int ref, const std::vector<std::string> &params);
	void execute_cheatnext(int ref, const std::vector<std::string> &params);
	void execute_cheatlist(int ref, const std::vector<std::string> &params);
	void execute_cheatundo(int ref, const std::vector<std::string> &params);
	void execute_dasm(int ref, const std::vector<std::string> &params);
	void execute_find(int ref, const std::vector<std::string> &params);
	void execute_trace(int ref, const std::vector<std::string> &params);
	void execute_traceover(int ref, const std::vector<std::string> &params);
	void execute_traceflush(int ref, const std::vector<std::string> &params);
	void execute_history(int ref, const std::vector<std::string> &params);
	void execute_trackpc(int ref, const std::vector<std::string> &params);
	void execute_trackmem(int ref, const std::vector<std::string> &params);
	void execute_pcatmem(int ref, const std::vector<std::string> &params);
	void execute_snap(int ref, const std::vector<std::string> &params);
	void execute_source(int ref, const std::vector<std::string> &params);
	void execute_map(int ref, const std::vector<std::string> &params);
	void execute_memdump(int ref, const std::vector<std::string> &params);
	void execute_symlist(int ref, const std::vector<std::string> &params);
	void execute_softreset(int ref, const std::vector<std::string> &params);
	void execute_hardreset(int ref, const std::vector<std::string> &params);
	void execute_images(int ref, const std::vector<std::string> &params);
	void execute_mount(int ref, const std::vector<std::string> &params);
	void execute_unmount(int ref, const std::vector<std::string> &params);
	void execute_input(int ref, const std::vector<std::string> &params);
	void execute_dumpkbd(int ref, const std::vector<std::string> &params);

	running_machine&    m_machine;
	debugger_cpu&       m_cpu;
	debugger_console&   m_console;

	std::unique_ptr<global_entry []> m_global_array;
	cheat_system m_cheat;

	static const size_t MAX_GLOBALS;
};

#endif // MAME_EMU_DEBUG_DEBUGCMD_H
