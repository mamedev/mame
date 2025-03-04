// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    debugcpu.h

    Debugger CPU/memory interface engine.

***************************************************************************/

#ifndef MAME_EMU_DEBUG_DEBUGCPU_H
#define MAME_EMU_DEBUG_DEBUGCPU_H

#pragma once

#include <set>
#include <utility>


//**************************************************************************
//  CONSTANTS
//**************************************************************************

constexpr int COMMENT_VERSION       = 1;



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> device_debug

// [TODO] This whole thing is terrible.

class device_debug
{
public:
	// construction/destruction
	device_debug(device_t &device);
	~device_debug();

	// getters
	symbol_table &symtable() { return *m_symtable; }

	// commonly-used pass-throughs
	int logaddrchars() const { return (m_memory != nullptr && m_memory->has_space(AS_PROGRAM)) ? m_memory->space(AS_PROGRAM).logaddrchars() : 8; }
	bool is_octal() const { return (m_memory != nullptr && m_memory->has_space(AS_PROGRAM)) ? m_memory->space(AS_PROGRAM).is_octal() : false; }
	device_t& device() const { return m_device; }

	// hooks used by the rest of the system
	void start_hook(const attotime &endtime);
	void stop_hook();
	void interrupt_hook(int irqline, offs_t pc);
	void exception_hook(int exception);
	void privilege_hook();
	void instruction_hook(offs_t curpc);
	void wait_hook();

	// debugger focus
	void ignore(bool ignore = true);
	bool observing() const { return ((m_flags & DEBUG_FLAG_OBSERVING) != 0); }

	// debugger suspend/unsuspend
	void suspend(bool suspend = true);
	bool suspended() const { return ((m_flags & DEBUG_FLAG_SUSPENDED) != 0); }

	// single stepping
	void single_step(int numsteps = 1, bool source_stepping = false);
	void single_step_over(int numsteps = 1, bool source_stepping = false);
	void single_step_out(bool source_stepping = false);

	// execution
	void go(offs_t targetpc = ~0);
	void go_vblank();
	void go_interrupt(int irqline = -1);
	void go_exception(int exception, const char *condition);
	void go_milliseconds(u64 milliseconds);
	void go_privilege(const char *condition);
	void go_branch(bool sense, const char *condition);
	void go_next_device();

	template <typename Format, typename... Params>
	void halt_on_next_instruction(Format &&fmt, Params &&... args)
	{
		halt_on_next_instruction_impl(util::make_format_argument_pack(std::forward<Format>(fmt), std::forward<Params>(args)...));
	}

	// breakpoints
	const auto &breakpoint_list() const { return m_bplist; }
	const debug_breakpoint *breakpoint_find(offs_t address) const;
	int breakpoint_set(offs_t address, const char *condition = nullptr, std::string_view action = {});
	bool breakpoint_clear(int index);
	void breakpoint_clear_all();
	bool breakpoint_enable(int index, bool enable = true);
	void breakpoint_enable_all(bool enable = true);
	debug_breakpoint *triggered_breakpoint() { debug_breakpoint *ret = m_triggered_breakpoint; m_triggered_breakpoint = nullptr; return ret; }

	// watchpoints
	int watchpoint_space_count() const { return m_wplist.size(); }
	const std::vector<std::unique_ptr<debug_watchpoint>> &watchpoint_vector(int spacenum) const { return m_wplist[spacenum]; }
	int watchpoint_set(address_space &space, read_or_write type, offs_t address, offs_t length, const char *condition = nullptr, std::string_view action = {});
	bool watchpoint_clear(int wpnum);
	void watchpoint_clear_all();
	bool watchpoint_enable(int index, bool enable = true);
	void watchpoint_enable_all(bool enable = true);
	void set_triggered_watchpoint(debug_watchpoint *wp) { m_triggered_watchpoint = wp; }
	debug_watchpoint *triggered_watchpoint() { debug_watchpoint *ret = m_triggered_watchpoint; m_triggered_watchpoint = nullptr; return ret; }

	// registerpoints
	const std::forward_list<debug_registerpoint> &registerpoint_list() const { return m_rplist; }
	int registerpoint_set(const char *condition, std::string_view action = {});
	bool registerpoint_clear(int index);
	void registerpoint_clear_all();
	bool registerpoint_enable(int index, bool enable = true);
	void registerpoint_enable_all(bool enable = true);

	// exception points
	const auto &exceptionpoint_list() const { return m_eplist; }
	int exceptionpoint_set(int exception, const char *condition = nullptr, std::string_view action = {});
	bool exceptionpoint_clear(int index);
	void exceptionpoint_clear_all();
	bool exceptionpoint_enable(int index, bool enable = true);
	void exceptionpoint_enable_all(bool enable = true);

	// comments
	void comment_add(offs_t address, const char *comment, rgb_t color);
	bool comment_remove(offs_t addr);
	const char *comment_text(offs_t addr) const;
	u32 comment_count() const { return m_comment_set.size(); }
	u32 comment_change_count() const { return m_comment_change; }
	bool comment_export(util::xml::data_node &node);
	bool comment_import(util::xml::data_node const &node, bool is_inline);
	u32 compute_opcode_crc32(offs_t pc) const;

	// history
	std::pair<offs_t, bool> history_pc(int index) const;

	// pc tracking
	void set_track_pc(bool value);
	bool track_pc_visited(offs_t pc) const;
	void set_track_pc_visited(offs_t pc);
	void track_pc_data_clear() { m_track_pc_set.clear(); }

	// memory tracking
	void set_track_mem(bool value);
	offs_t track_mem_pc_from_space_address_data(const int& space,
												const offs_t& address,
												const u64& data) const;
	void track_mem_data_clear() { m_track_mem_set.clear(); }

	// tracing
	void trace(std::unique_ptr<std::ostream> &&file, bool trace_over, bool detect_loops, bool logerror, std::string_view action);
	template <typename Format, typename... Params> void trace_printf(Format &&fmt, Params &&...args)
	{
		if (m_trace != nullptr)
			m_trace->vprintf(util::make_format_argument_pack(std::forward<Format>(fmt), std::forward<Params>(args)...));
	}
	void trace_flush() { if (m_trace != nullptr) m_trace->flush(); }

	void reset_transient_flag() { m_flags &= ~DEBUG_FLAG_TRANSIENT; }

	static const int HISTORY_SIZE = 256;

	// debugger_cpu helpers
	void compute_debug_flags();

	// source-level debugging
	void update_symbols_from_srcdbg(const srcdbg_provider_base & srcdbg_provider);

private:
	void halt_on_next_instruction_impl(util::format_argument_pack<char> &&args);

	// internal helpers
	void prepare_for_step_overout(offs_t pc);
	bool is_source_stepping_complete(offs_t pc);
	void errorlog_write_line(const char *line);

	// breakpoint and watchpoint helpers
	void breakpoint_update_flags();
	void breakpoint_check(offs_t pc);
	void registerpoint_check();
	void reinstall_all(read_or_write mode);
	void reinstall(address_space &space, read_or_write mode);
	void write_tracking(address_space &space, offs_t address, u64 data);

	// basic device information
	device_t &                 m_device;                // device we are attached to
	device_execute_interface * m_exec;                  // execute interface, if present
	device_memory_interface *  m_memory;                // memory interface, if present
	device_state_interface *   m_state;                 // state interface, if present
	device_disasm_interface *  m_disasm;                // disasm interface, if present

	// global state
	u32                           m_flags;                   // debugging flags for this CPU
	std::unique_ptr<symbol_table> m_symtable_device;         // storage for device symbols for expression evaluation
	std::unique_ptr<symbol_table> m_symtable_srcdbg_globals; // storage for source-debugging globals
	std::unique_ptr<symbol_table> m_symtable_srcdbg_locals;  // storage for source-debugging locals
	symbol_table *                m_symtable;                // Root of symbol table chain exposed by this class

	// stepping information
	offs_t                     m_stepaddr;                   // step target address for DEBUG_FLAG_STEPPING_OVER or DEBUG_FLAG_STEPPING_BRANCH
	int                        m_stepsleft;                  // number of steps left until done
	int                        m_delay_steps;                // number of steps until target address check
	std::unique_ptr<file_line> m_step_source_start;          // When source-level stepping, where did the step start?
	bool                       m_outs_encountered_return;    // When source-level stepping-out, have we encountered our first return yet?

	// execution information
	offs_t                  m_stopaddr;                 // stop address for DEBUG_FLAG_STOP_PC
	attotime                m_stoptime;                 // stop time for DEBUG_FLAG_STOP_TIME
	int                     m_stopirq;                  // stop IRQ number for DEBUG_FLAG_STOP_INTERRUPT
	int                     m_stopexception;            // stop exception number for DEBUG_FLAG_STOP_EXCEPTION
	std::unique_ptr<parsed_expression> m_stop_condition;           // expression to evaluate on privilege change
	std::unique_ptr<parsed_expression> m_exception_condition;      // expression to evaluate on exception hit
	attotime                m_endexectime;              // ending time of the current execution
	u64                     m_total_cycles;             // current total cycles
	u64                     m_last_total_cycles;        // last total cycles
	bool                    m_was_waiting;              // true if no instruction executed since last wait

	// history
	offs_t                  m_pc_history[HISTORY_SIZE]; // history of recent PCs
	u32                     m_pc_history_index;         // current history index
	u32                     m_pc_history_valid;         // number of valid PC history entries

	// breakpoints and watchpoints
	std::multimap<offs_t, std::unique_ptr<debug_breakpoint>> m_bplist;     // list of breakpoints
	std::vector<std::vector<std::unique_ptr<debug_watchpoint>>> m_wplist;  // watchpoint lists for each address space
	std::forward_list<debug_registerpoint> m_rplist;                       // list of registerpoints
	std::multimap<offs_t, std::unique_ptr<debug_exceptionpoint>> m_eplist; // list of exception points

	debug_breakpoint *      m_triggered_breakpoint;     // latest breakpoint that was triggered
	debug_watchpoint *      m_triggered_watchpoint;     // latest watchpoint that was triggered

	// tracing
	class tracer
	{
	public:
		tracer(device_debug &debug, std::unique_ptr<std::ostream> &&file, bool trace_over, bool detect_loops, bool logerror, std::string_view action);
		~tracer();

		void update(offs_t pc);
		void interrupt_update(int irqline, offs_t pc);
		void vprintf(util::format_argument_pack<char> const &args);
		void flush();
		bool logerror() const { return m_logerror; }

	private:
		static const int TRACE_LOOPS = 64;

		device_debug &      m_debug;                    // reference to our owner
		std::unique_ptr<std::ostream> m_file;           // tracing file for this CPU
		std::string         m_action;                   // action to perform during a trace
		offs_t              m_history[TRACE_LOOPS];     // history of recent PCs
		bool                m_detect_loops;             // whether or not we should detect loops
		bool                m_logerror;                 // whether or not we should collect logerror output
		int                 m_loops;                    // number of instructions in a loop
		int                 m_nextdex;                  // next index
		bool                m_trace_over;               // true if we're tracing over
		offs_t              m_trace_over_target;        // target for tracing over
														//    (0 = not tracing over,
														//    ~0 = not currently tracing over)
	};
	std::unique_ptr<tracer>                m_trace;     // tracer state

	std::vector<memory_passthrough_handler> m_phw;      // passthrough handler reference for each space, write mode
	std::vector<util::notifier_subscription> m_notifiers; // notifiers for each space

	// pc tracking
	class dasm_pc_tag
	{
	public:
		dasm_pc_tag(const offs_t& address, const u32& crc);

		// required to be included in a set
		bool operator < (const dasm_pc_tag& rhs) const
		{
			if (m_address == rhs.m_address)
				return m_crc < rhs.m_crc;
			return (m_address < rhs.m_address);
		}

		offs_t m_address;       // Stores [nothing] for a given address & crc32
		u32    m_crc;
	};
	std::set<dasm_pc_tag> m_track_pc_set;
	bool m_track_pc;

	// comments
	class dasm_comment : public dasm_pc_tag
	{
	public:
		dasm_comment(offs_t address, u32 crc, const char *text, rgb_t color);

		std::string  m_text;        // Stores comment text & color for a given address & crc32
		rgb_t        m_color;
	};
	std::set<dasm_comment> m_comment_set;               // collection of comments
	u32                 m_comment_change;            // change counter for comments

	// memory tracking
	class dasm_memory_access
	{
	public:
		dasm_memory_access(const int& address_space,
							const offs_t& address,
							const u64& data,
							const offs_t& pc);

		// required to be included in a set
		bool operator < (const dasm_memory_access& rhs) const
		{
			if ((m_address == rhs.m_address) && (m_address_space == rhs.m_address_space))
				return m_data < rhs.m_data;
			else if (m_address_space == rhs.m_address_space)
				return m_address < rhs.m_address;
			else
				return m_address_space < rhs.m_address_space;
		}

		// Stores the PC for a given address, memory region, and data value
		int m_address_space;
		offs_t           m_address;
		u64              m_data;
		mutable offs_t   m_pc;
	};
	std::set<dasm_memory_access> m_track_mem_set;
	bool m_track_mem;

	// internal flag values
	static constexpr u32 DEBUG_FLAG_OBSERVING       = 0x00000001;       // observing this CPU
	static constexpr u32 DEBUG_FLAG_HISTORY         = 0x00000002;       // tracking this CPU's history
	static constexpr u32 DEBUG_FLAG_TRACING         = 0x00000004;       // tracing this CPU
	static constexpr u32 DEBUG_FLAG_TRACING_OVER    = 0x00000008;       // tracing this CPU with step over behavior
	static constexpr u32 DEBUG_FLAG_STEPPING        = 0x00000020;       // CPU is single stepping
	static constexpr u32 DEBUG_FLAG_STEPPING_OVER   = 0x00000040;       // CPU is stepping over a function
	static constexpr u32 DEBUG_FLAG_STEPPING_OUT    = 0x00000080;       // CPU is stepping out of a function
	static constexpr u32 DEBUG_FLAG_STOP_PC         = 0x00000100;       // there is a pending stop at cpu->breakpc
	static constexpr u32 DEBUG_FLAG_STOP_INTERRUPT  = 0x00000400;       // there is a pending stop on the next interrupt
	static constexpr u32 DEBUG_FLAG_STOP_EXCEPTION  = 0x00000800;       // there is a pending stop on the next exception
	static constexpr u32 DEBUG_FLAG_STOP_VBLANK     = 0x00001000;       // there is a pending stop on the next VBLANK
	static constexpr u32 DEBUG_FLAG_STOP_TIME       = 0x00002000;       // there is a pending stop at cpu->stoptime
	static constexpr u32 DEBUG_FLAG_SUSPENDED       = 0x00004000;       // CPU currently suspended
	static constexpr u32 DEBUG_FLAG_LIVE_BP         = 0x00008000;       // there are live breakpoints for this CPU
	static constexpr u32 DEBUG_FLAG_LIVE_RP         = 0x00010000;       // there are live registerpoints for this CPU
	static constexpr u32 DEBUG_FLAG_STOP_PRIVILEGE  = 0x00020000;       // run until execution level changes
	static constexpr u32 DEBUG_FLAG_STEPPING_BRANCH_TRUE  = 0x0040000;  // run until true branch
	static constexpr u32 DEBUG_FLAG_STEPPING_BRANCH_FALSE = 0x0080000;  // run until false branch
	static constexpr u32 DEBUG_FLAG_CALL_IN_PROGRESS = 0x01000000;      // CPU is in the middle of a subroutine call
	static constexpr u32 DEBUG_FLAG_TEST_IN_PROGRESS = 0x02000000;      // CPU is performing a conditional test and branch
	static constexpr u32 DEBUG_FLAG_SOURCE_STEPPING  = 0x04000000;      // CPU is stepping in/over/out during source debugging (set in conjunction with other DEBUG_FLAG_STEPPING* flags)

	static constexpr u32 DEBUG_FLAG_STEPPING_BRANCH = DEBUG_FLAG_STEPPING_BRANCH_TRUE | DEBUG_FLAG_STEPPING_BRANCH_FALSE;
	static constexpr u32 DEBUG_FLAG_STEPPING_ANY    = DEBUG_FLAG_STEPPING | DEBUG_FLAG_STEPPING_OVER | DEBUG_FLAG_STEPPING_OUT | DEBUG_FLAG_STEPPING_BRANCH | DEBUG_FLAG_SOURCE_STEPPING;
	static constexpr u32 DEBUG_FLAG_TRACING_ANY     = DEBUG_FLAG_TRACING | DEBUG_FLAG_TRACING_OVER;
	static constexpr u32 DEBUG_FLAG_TRANSIENT       = DEBUG_FLAG_STEPPING_ANY | DEBUG_FLAG_STOP_PC |
			DEBUG_FLAG_STOP_INTERRUPT | DEBUG_FLAG_STOP_EXCEPTION | DEBUG_FLAG_STOP_VBLANK |
			DEBUG_FLAG_STOP_TIME | DEBUG_FLAG_STOP_PRIVILEGE | DEBUG_FLAG_CALL_IN_PROGRESS | DEBUG_FLAG_TEST_IN_PROGRESS;
};

//**************************************************************************
//  CPU DEBUGGING
//**************************************************************************

class debugger_cpu
{
public:
	enum class exec_state { STOPPED, RUNNING };

	debugger_cpu(running_machine &machine);

	/* ----- initialization and cleanup ----- */

	/* flushes all traces; this is useful if a trace is going on when we fatalerror */
	void flush_traces();


	/* ----- debugging status & information ----- */

	/* return true if the current execution state is stopped */
	bool is_stopped() const { return m_execution_state == exec_state::STOPPED; }
	bool is_running() const { return m_execution_state == exec_state::RUNNING; }


	/* ----- symbol table interfaces ----- */

	/* return the global symbol table */
	symbol_table &global_symtable() { return *m_symtable; }


	/* ----- debugger comment helpers ----- */

	// save all comments for a given machine
	bool comment_save();

	// load all comments for a given machine
	bool comment_load(bool is_inline);


	// getters
	bool within_instruction_hook() const { return m_within_instruction_hook; }
	bool memory_modified() const { return m_memory_modified; }
	exec_state execution_state() const { return m_execution_state; }
	device_t *live_cpu() { return m_livecpu; }
	u32 get_breakpoint_index() { return m_bpindex++; }
	u32 get_watchpoint_index() { return m_wpindex++; }
	u32 get_registerpoint_index() { return m_rpindex++; }
	u32 get_exceptionpoint_index() { return m_epindex++; }

	// setters
	void set_break_cpu(device_t * breakcpu) { m_breakcpu = breakcpu; }
	void set_within_instruction(bool within_instruction) { m_within_instruction_hook = within_instruction; }
	void set_memory_modified(bool memory_modified) { m_memory_modified = memory_modified; }
	void set_execution_stopped() { m_execution_state = exec_state::STOPPED; }
	void set_execution_running() { m_execution_state = exec_state::RUNNING; }
	void set_wpinfo(offs_t address, u64 data, offs_t size) { m_wpaddr = address; m_wpdata = data; m_wpsize = size; }

	// device_debug helpers
	// [TODO] [RH]: Look into this more later, can possibly merge these two classes
	void start_hook(device_t *device, bool stop_on_vblank);
	void stop_hook(device_t *device);
	void go_next_device(device_t *device);
	void go_vblank();
	void halt_on_next_instruction(device_t *device, util::format_argument_pack<char> &&args);
	void ensure_comments_loaded();
	void reset_transient_flags();
	void wait_for_debugger(device_t &device);

private:
	static const size_t NUM_TEMP_VARIABLES;

	// internal helpers
	void on_vblank(screen_device &device, bool vblank_state);

	running_machine&    m_machine;

	device_t *  m_livecpu;
	device_t *  m_breakcpu;

	std::unique_ptr<symbol_table> m_symtable;           // global symbol table

	bool        m_within_instruction_hook;
	bool        m_vblank_occurred;
	bool        m_memory_modified;

	exec_state  m_execution_state;
	device_t *  m_stop_when_not_device; // stop execution when the device ceases to be this

	u32         m_bpindex;
	u32         m_wpindex;
	u32         m_rpindex;
	u32         m_epindex;

	u64         m_wpdata;
	u64         m_wpaddr;
	u64         m_wpsize;
	std::unique_ptr<u64[]> m_tempvar;

	osd_ticks_t m_last_periodic_update_time;

	bool        m_comments_loaded;
};

#endif // MAME_EMU_DEBUG_DEBUGCPU_H
