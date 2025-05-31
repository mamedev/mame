// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Ryan Holtz
/*********************************************************************

    debugcmd.h

    Debugger command interface engine.

*********************************************************************/

#ifndef MAME_EMU_DEBUG_DEBUGCMD_H
#define MAME_EMU_DEBUG_DEBUGCMD_H

#pragma once

#include <string_view>


class debugger_commands
{
public:
	debugger_commands(running_machine &machine, debugger_cpu &cpu, debugger_console &console);

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
		address_space *space;
		u8          width;
		u8          signed_cheat;
		u8          swapped_cheat;
		std::vector<cheat_map> cheatmap;
		u8          undo;

		u64 sign_extend(u64 value) const;
		u64 byte_swap(u64 value) const;
		u64 read_extended(offs_t address) const;
	};

	struct cheat_region_map
	{
		u64         offset = 0U;
		u64         endoffset = 0U;
		const char *share = nullptr;
		u8          disabled = 0U;
	};

	bool cheat_address_is_valid(address_space &space, offs_t address);

	u64 get_cpunum();

	u64 global_get(global_entry *global);
	void global_set(global_entry *global, u64 value);

	bool mini_printf(std::ostream &stream, const std::vector<std::string_view> &params);
	template <typename T>
	void execute_index_command(std::vector<std::string_view> const &params, T &&apply, char const *unused_message);

	void execute_help(const std::vector<std::string_view> &params);
	void execute_print(const std::vector<std::string_view> &params);
	void execute_printf(const std::vector<std::string_view> &params);
	void execute_logerror(const std::vector<std::string_view> &params);
	void execute_tracelog(const std::vector<std::string_view> &params);
	void execute_tracesym(const std::vector<std::string_view> &params);
	void execute_cls(const std::vector<std::string_view> &params);
	void execute_quit(const std::vector<std::string_view> &params);
	void execute_do(const std::vector<std::string_view> &params);
	void execute_step(const std::vector<std::string_view> &params);
	void execute_step_source(const std::vector<std::string_view> &params);
	void execute_over(const std::vector<std::string_view> &params);
	void execute_over_source(const std::vector<std::string_view> &params);
	void execute_out(const std::vector<std::string_view> &params);
	void execute_out_source(const std::vector<std::string_view> &params);
	void execute_go(const std::vector<std::string_view> &params);
	void execute_go_vblank(const std::vector<std::string_view> &params);
	void execute_go_interrupt(const std::vector<std::string_view> &params);
	void execute_go_exception(const std::vector<std::string_view> &params);
	void execute_go_time(const std::vector<std::string_view> &params);
	void execute_go_privilege(const std::vector<std::string_view> &params);
	void execute_go_branch(bool sense, const std::vector<std::string_view> &params);
	void execute_go_next_instruction(const std::vector<std::string_view> &params);
	void execute_focus(const std::vector<std::string_view> &params);
	void execute_ignore(const std::vector<std::string_view> &params);
	void execute_observe(const std::vector<std::string_view> &params);
	void execute_suspend(const std::vector<std::string_view> &params);
	void execute_resume(const std::vector<std::string_view> &params);
	void execute_next(const std::vector<std::string_view> &params);
	void execute_cpulist(const std::vector<std::string_view> &params);
	void execute_time(const std::vector<std::string_view> &params);
	void execute_srcdbg_set_offset(const std::vector<std::string_view> &params);
	void execute_comment_add(const std::vector<std::string_view> &params);
	void execute_comment_del(const std::vector<std::string_view> &params);
	void execute_comment_save(const std::vector<std::string_view> &params);
	void execute_comment_list(const std::vector<std::string_view> &params);
	void execute_comment_commit(const std::vector<std::string_view> &params);
	void execute_bpset(const std::vector<std::string_view> &params);
	void execute_bpclear(const std::vector<std::string_view> &params);
	void execute_bpdisenable(bool enable, const std::vector<std::string_view> &params);
	void execute_bplist(const std::vector<std::string_view> &params);
	void execute_wpset(int spacenum, const std::vector<std::string_view> &params);
	void execute_wpclear(const std::vector<std::string_view> &params);
	void execute_wpdisenable(bool enable, const std::vector<std::string_view> &params);
	void execute_wplist(const std::vector<std::string_view> &params);
	void execute_rpset(const std::vector<std::string_view> &params);
	void execute_rpclear(const std::vector<std::string_view> &params);
	void execute_rpdisenable(bool enable, const std::vector<std::string_view> &params);
	void execute_rplist(const std::vector<std::string_view> &params);
	void execute_epset(const std::vector<std::string_view> &params);
	void execute_epclear(const std::vector<std::string_view> &params);
	void execute_epdisenable(bool enable, const std::vector<std::string_view> &params);
	void execute_eplist(const std::vector<std::string_view> &params);
	void execute_statesave(const std::vector<std::string_view> &params);
	void execute_stateload(const std::vector<std::string_view> &params);
	void execute_rewind(const std::vector<std::string_view> &params);
	void execute_save(int spacenum, const std::vector<std::string_view> &params);
	void execute_saveregion(const std::vector<std::string_view> &params);
	void execute_load(int spacenum, const std::vector<std::string_view> &params);
	void execute_loadregion(const std::vector<std::string_view> &params);
	void execute_dump(int spacenum, const std::vector<std::string_view> &params);
	void execute_strdump(int spacenum, const std::vector<std::string_view> &params);
	void execute_cheatrange(bool init, const std::vector<std::string_view> &params);
	void execute_cheatnext(bool initial, const std::vector<std::string_view> &params);
	void execute_cheatlist(const std::vector<std::string_view> &params);
	void execute_cheatundo(const std::vector<std::string_view> &params);
	void execute_dasm(const std::vector<std::string_view> &params);
	void execute_find(int spacenum, const std::vector<std::string_view> &params);
	void execute_fill(int spacenum, const std::vector<std::string_view> &params);
	void execute_trace(const std::vector<std::string_view> &params, bool trace_over);
	void execute_traceflush(const std::vector<std::string_view> &params);
	void execute_history(const std::vector<std::string_view> &params);
	void execute_trackpc(const std::vector<std::string_view> &params);
	void execute_trackmem(const std::vector<std::string_view> &params);
	void execute_pcatmem(int spacenum, const std::vector<std::string_view> &params);
	void execute_snap(const std::vector<std::string_view> &params);
	void execute_source(const std::vector<std::string_view> &params);
	void execute_map(int spacenum, const std::vector<std::string_view> &params);
	void execute_memdump(const std::vector<std::string_view> &params);
	void execute_symlist(const std::vector<std::string_view> &params);
	void execute_softreset(const std::vector<std::string_view> &params);
	void execute_hardreset(const std::vector<std::string_view> &params);
	void execute_images(const std::vector<std::string_view> &params);
	void execute_mount(const std::vector<std::string_view> &params);
	void execute_unmount(const std::vector<std::string_view> &params);
	void execute_input(const std::vector<std::string_view> &params);
	void execute_dumpkbd(const std::vector<std::string_view> &params);

	running_machine&    m_machine;
	debugger_console&   m_console;

	std::unique_ptr<global_entry []> m_global_array;
	cheat_system m_cheat;

	static const size_t MAX_GLOBALS;
};

#endif // MAME_EMU_DEBUG_DEBUGCMD_H
