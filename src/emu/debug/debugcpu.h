// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    debugcpu.h

    Debugger CPU/memory interface engine.

***************************************************************************/

#pragma once

#ifndef __DEBUGCPU_H__
#define __DEBUGCPU_H__

#include "express.h"

#include <set>


//**************************************************************************
//  CONSTANTS
//**************************************************************************

const UINT8 WATCHPOINT_READ             = 1;
const UINT8 WATCHPOINT_WRITE            = 2;
const UINT8 WATCHPOINT_READWRITE        = WATCHPOINT_READ | WATCHPOINT_WRITE;

const int COMMENT_VERSION               = 1;



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

typedef int (*debug_instruction_hook_func)(device_t &device, offs_t curpc);


struct xml_data_node;


// ======================> device_debug

class device_debug
{
	typedef offs_t (*dasm_override_func)(device_t &device, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, int options);

public:
	// breakpoint class
	class breakpoint
	{
		friend class device_debug;

	public:
		// construction/destruction
		breakpoint(device_debug* debugInterface,
					symbol_table &symbols,
					int index,
					offs_t address,
					const char *condition = nullptr,
					const char *action = nullptr);

		// getters
		const device_debug *debugInterface() const { return m_debugInterface; }
		breakpoint *next() const { return m_next; }
		int index() const { return m_index; }
		bool enabled() const { return m_enabled; }
		offs_t address() const { return m_address; }
		const char *condition() const { return m_condition.original_string(); }
		const char *action() const { return m_action.c_str(); }

		// setters
		void setEnabled(bool value) { m_enabled = value; }

	private:
		// internals
		bool hit(offs_t pc);

		const device_debug * m_debugInterface;           // the interface we were created from
		breakpoint *         m_next;                     // next in the list
		int                  m_index;                    // user reported index
		UINT8                m_enabled;                  // enabled?
		offs_t               m_address;                  // execution address
		parsed_expression    m_condition;                // condition
		std::string          m_action;                   // action
	};

	// watchpoint class
	class watchpoint
	{
		friend class device_debug;

	public:
		// construction/destruction
		watchpoint(device_debug* debugInterface,
					symbol_table &symbols,
					int index,
					address_space &space,
					int type,
					offs_t address,
					offs_t length,
					const char *condition = nullptr,
					const char *action = nullptr);

		// getters
		const device_debug *debugInterface() const { return m_debugInterface; }
		watchpoint *next() const { return m_next; }
		address_space &space() const { return m_space; }
		int index() const { return m_index; }
		int type() const { return m_type; }
		bool enabled() const { return m_enabled; }
		offs_t address() const { return m_address; }
		offs_t length() const { return m_length; }
		const char *condition() const { return m_condition.original_string(); }
		const char *action() const { return m_action.c_str(); }

		// setters
		void setEnabled(bool value) { m_enabled = value; }

	private:
		// internals
		bool hit(int type, offs_t address, int size);

		const device_debug * m_debugInterface;           // the interface we were created from
		watchpoint *         m_next;                     // next in the list
		address_space &      m_space;                    // address space
		int                  m_index;                    // user reported index
		bool                 m_enabled;                  // enabled?
		UINT8                m_type;                     // type (read/write)
		offs_t               m_address;                  // start address
		offs_t               m_length;                   // length of watch area
		parsed_expression    m_condition;                // condition
		std::string          m_action;                   // action
	};

	// registerpoint class
	class registerpoint
	{
		friend class device_debug;

	public:
		// construction/destruction
		registerpoint(symbol_table &symbols, int index, const char *condition, const char *action = nullptr);

		// getters
		registerpoint *next() const { return m_next; }
		int index() const { return m_index; }
		bool enabled() const { return m_enabled; }
		const char *condition() const { return m_condition.original_string(); }
		const char *action() const { return m_action.c_str(); }

	private:
		// internals
		bool hit();

		registerpoint *     m_next;                     // next in the list
		int                 m_index;                    // user reported index
		UINT8               m_enabled;                  // enabled?
		parsed_expression   m_condition;                // condition
		std::string         m_action;                   // action
	};

public:
	// construction/destruction
	device_debug(device_t &device);
	~device_debug();

	// getters
	symbol_table &symtable() { return m_symtable; }

	// commonly-used pass-throughs
	offs_t pc() const { return (m_state != nullptr) ? m_state->pc() : 0; }
	int logaddrchars(address_spacenum spacenum = AS_0) const { return (m_memory != nullptr && m_memory->has_space(spacenum)) ? m_memory->space(spacenum).logaddrchars() : 8; }
	int min_opcode_bytes() const { return (m_disasm != nullptr) ? m_disasm->max_opcode_bytes() : 1; }
	int max_opcode_bytes() const { return (m_disasm != nullptr) ? m_disasm->max_opcode_bytes() : 1; }
	device_t& device() const { return m_device; }


	// hooks used by the rest of the system
	void start_hook(const attotime &endtime);
	void stop_hook();
	void interrupt_hook(int irqline);
	void exception_hook(int exception);
	void instruction_hook(offs_t curpc);
	void memory_read_hook(address_space &space, offs_t address, UINT64 mem_mask);
	void memory_write_hook(address_space &space, offs_t address, UINT64 data, UINT64 mem_mask);

	// hooks into our operations
	void set_instruction_hook(debug_instruction_hook_func hook);
	void set_dasm_override(dasm_override_func dasm_override) { m_dasm_override = dasm_override; }

	// disassembly
	offs_t disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram) const;

	// debugger focus
	void ignore(bool ignore = true);
	bool observing() const { return ((m_flags & DEBUG_FLAG_OBSERVING) != 0); }

	// single stepping
	void single_step(int numsteps = 1);
	void single_step_over(int numsteps = 1);
	void single_step_out();

	// execution
	void go(offs_t targetpc = ~0);
	void go_vblank();
	void go_interrupt(int irqline = -1);
	void go_exception(int exception);
	void go_milliseconds(UINT64 milliseconds);
	void go_next_device();
	void halt_on_next_instruction(const char *fmt, ...) ATTR_PRINTF(2,3);

	// breakpoints
	breakpoint *breakpoint_first() const { return m_bplist; }
	int breakpoint_set(offs_t address, const char *condition = nullptr, const char *action = nullptr);
	bool breakpoint_clear(int index);
	void breakpoint_clear_all();
	bool breakpoint_enable(int index, bool enable = true);
	void breakpoint_enable_all(bool enable = true);

	// watchpoints
	watchpoint *watchpoint_first(address_spacenum spacenum) const { return m_wplist[spacenum]; }
	int watchpoint_set(address_space &space, int type, offs_t address, offs_t length, const char *condition, const char *action);
	bool watchpoint_clear(int wpnum);
	void watchpoint_clear_all();
	bool watchpoint_enable(int index, bool enable = true);
	void watchpoint_enable_all(bool enable = true);

	// registerpoints
	registerpoint *registerpoint_first() const { return m_rplist; }
	int registerpoint_set(const char *condition, const char *action = nullptr);
	bool registerpoint_clear(int index);
	void registerpoint_clear_all();
	bool registerpoint_enable(int index, bool enable = true);
	void registerpoint_enable_all(bool enable = true );

	// hotspots
	bool hotspot_tracking_enabled() const { return !m_hotspots.empty(); }
	void hotspot_track(int numspots, int threshhold);

	// comments
	void comment_add(offs_t address, const char *comment, rgb_t color);
	bool comment_remove(offs_t addr);
	const char *comment_text(offs_t addr) const;
	UINT32 comment_count() const { return m_comment_set.size(); }
	UINT32 comment_change_count() const { return m_comment_change; }
	bool comment_export(xml_data_node &node);
	bool comment_import(xml_data_node &node);
	UINT32 compute_opcode_crc32(offs_t pc) const;

	// history
	offs_t history_pc(int index) const;

	// pc tracking
	void set_track_pc(bool value) { m_track_pc = value; }
	bool track_pc_visited(const offs_t& pc) const;
	void set_track_pc_visited(const offs_t& pc);
	void track_pc_data_clear() { m_track_pc_set.clear(); }

	// memory tracking
	void set_track_mem(bool value) { m_track_mem = value; }
	offs_t track_mem_pc_from_space_address_data(const address_spacenum& space,
												const offs_t& address,
												const UINT64& data) const;
	void track_mem_data_clear() { m_track_mem_set.clear(); }

	// tracing
	void trace(FILE *file, bool trace_over, const char *action);
	void trace_printf(const char *fmt, ...) ATTR_PRINTF(2,3);
	void trace_flush() { if (m_trace != nullptr) m_trace->flush(); }

	void reset_transient_flag() { m_flags &= ~DEBUG_FLAG_TRANSIENT; }

	static const int HISTORY_SIZE = 256;

private:
	// internal helpers
	void compute_debug_flags();
	void prepare_for_step_overout(offs_t pc);
	UINT32 dasm_wrapped(std::string &buffer, offs_t pc);

	// breakpoint and watchpoint helpers
	void breakpoint_update_flags();
	void breakpoint_check(offs_t pc);
	void watchpoint_update_flags(address_space &space);
	void watchpoint_check(address_space &space, int type, offs_t address, UINT64 value_to_write, UINT64 mem_mask);
	void hotspot_check(address_space &space, offs_t address);

	// symbol get/set callbacks
	static UINT64 get_current_pc(symbol_table &table, void *ref);
	static UINT64 get_cycles(symbol_table &table, void *ref);
	static UINT64 get_totalcycles(symbol_table &table, void *ref);
	static UINT64 get_lastinstructioncycles(symbol_table &table, void *ref);
	static UINT64 get_logunmap(symbol_table &table, void *ref);
	static void set_logunmap(symbol_table &table, void *ref, UINT64 value);
	static UINT64 get_state(symbol_table &table, void *ref);
	static void set_state(symbol_table &table, void *ref, UINT64 value);

	// basic device information
	device_t &                 m_device;                // device we are attached to
	device_execute_interface * m_exec;                  // execute interface, if present
	device_memory_interface *  m_memory;                // memory interface, if present
	device_state_interface *   m_state;                 // state interface, if present
	device_disasm_interface *  m_disasm;                // disasm interface, if present

	// global state
	UINT32                      m_flags;                // debugging flags for this CPU
	symbol_table                m_symtable;             // symbol table for expression evaluation
	debug_instruction_hook_func m_instrhook;            // per-instruction callback hook

	// disassembly
	dasm_override_func      m_dasm_override;            // pointer to provided override function
	UINT8                   m_opwidth;                  // width of an opcode

	// stepping information
	offs_t                  m_stepaddr;                 // step target address for DEBUG_FLAG_STEPPING_OVER
	int                     m_stepsleft;                // number of steps left until done

	// execution information
	offs_t                  m_stopaddr;                 // stop address for DEBUG_FLAG_STOP_PC
	attotime                m_stoptime;                 // stop time for DEBUG_FLAG_STOP_TIME
	int                     m_stopirq;                  // stop IRQ number for DEBUG_FLAG_STOP_INTERRUPT
	int                     m_stopexception;            // stop exception number for DEBUG_FLAG_STOP_EXCEPTION
	attotime                m_endexectime;              // ending time of the current execution
	UINT64                  m_total_cycles;             // current total cycles
	UINT64                  m_last_total_cycles;        // last total cycles

	// history
	offs_t                  m_pc_history[HISTORY_SIZE]; // history of recent PCs
	UINT32                  m_pc_history_index;         // current history index

	// breakpoints and watchpoints
	breakpoint *            m_bplist;                   // list of breakpoints
	watchpoint *            m_wplist[ADDRESS_SPACES];   // watchpoint lists for each address space
	registerpoint *         m_rplist;                   // list of registerpoints

	// tracing
	class tracer
	{
	public:
		tracer(device_debug &debug, FILE &file, bool trace_over, const char *action);
		~tracer();

		void update(offs_t pc);
		void vprintf(const char *format, va_list va);
		void flush();

	private:
		static const int TRACE_LOOPS = 64;

		device_debug &      m_debug;                    // reference to our owner
		FILE &              m_file;                     // tracing file for this CPU
		std::string         m_action;                   // action to perform during a trace
		offs_t              m_history[TRACE_LOOPS];     // history of recent PCs
		int                 m_loops;                    // number of instructions in a loop
		int                 m_nextdex;                  // next index
		bool                m_trace_over;               // true if we're tracing over
		offs_t              m_trace_over_target;        // target for tracing over
														//    (0 = not tracing over,
														//    ~0 = not currently tracing over)
	};
	tracer *                m_trace;                    // tracer state

	// hotspots
	struct hotspot_entry
	{
		offs_t              m_access;                   // access address
		offs_t              m_pc;                       // PC of the access
		address_space *     m_space;                    // space where the access occurred
		UINT32              m_count;                    // number of hits
	};
	std::vector<hotspot_entry> m_hotspots;            // hotspot list
	int                     m_hotspot_threshhold;       // threshhold for the number of hits to print

	// pc tracking
	class dasm_pc_tag
	{
	public:
		dasm_pc_tag(const offs_t& address, const UINT32& crc);

		// required to be included in a set
		bool operator < (const dasm_pc_tag& rhs) const
		{
			if (m_address == rhs.m_address)
				return m_crc < rhs.m_crc;
			return (m_address < rhs.m_address);
		}

		offs_t m_address;       // Stores [nothing] for a given address & crc32
		UINT32 m_crc;
	};
	std::set<dasm_pc_tag> m_track_pc_set;
	bool m_track_pc;

	// comments
	class dasm_comment : public dasm_pc_tag
	{
	public:
		dasm_comment(offs_t address, UINT32 crc, const char *text, rgb_t color);

		std::string  m_text;        // Stores comment text & color for a given address & crc32
		rgb_t    m_color;
	};
	std::set<dasm_comment> m_comment_set;               // collection of comments
	UINT32                 m_comment_change;            // change counter for comments

	// memory tracking
	class dasm_memory_access
	{
	public:
		dasm_memory_access(const address_spacenum& address_space,
							const offs_t& address,
							const UINT64& data,
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
		address_spacenum m_address_space;
		offs_t           m_address;
		UINT64           m_data;
		mutable offs_t   m_pc;
	};
	std::set<dasm_memory_access> m_track_mem_set;
	bool m_track_mem;

	// internal flag values
	static const UINT32 DEBUG_FLAG_OBSERVING        = 0x00000001;       // observing this CPU
	static const UINT32 DEBUG_FLAG_HISTORY          = 0x00000002;       // tracking this CPU's history
	static const UINT32 DEBUG_FLAG_TRACING          = 0x00000004;       // tracing this CPU
	static const UINT32 DEBUG_FLAG_TRACING_OVER     = 0x00000008;       // tracing this CPU with step over behavior
	static const UINT32 DEBUG_FLAG_HOOKED           = 0x00000010;       // per-instruction callback hook
	static const UINT32 DEBUG_FLAG_STEPPING         = 0x00000020;       // CPU is single stepping
	static const UINT32 DEBUG_FLAG_STEPPING_OVER    = 0x00000040;       // CPU is stepping over a function
	static const UINT32 DEBUG_FLAG_STEPPING_OUT     = 0x00000080;       // CPU is stepping out of a function
	static const UINT32 DEBUG_FLAG_STOP_PC          = 0x00000100;       // there is a pending stop at cpu->breakpc
	static const UINT32 DEBUG_FLAG_STOP_INTERRUPT   = 0x00000400;       // there is a pending stop on the next interrupt
	static const UINT32 DEBUG_FLAG_STOP_EXCEPTION   = 0x00000800;       // there is a pending stop on the next exception
	static const UINT32 DEBUG_FLAG_STOP_VBLANK      = 0x00001000;       // there is a pending stop on the next VBLANK
	static const UINT32 DEBUG_FLAG_STOP_TIME        = 0x00002000;       // there is a pending stop at cpu->stoptime
	static const UINT32 DEBUG_FLAG_LIVE_BP          = 0x00010000;       // there are live breakpoints for this CPU

	static const UINT32 DEBUG_FLAG_STEPPING_ANY     = DEBUG_FLAG_STEPPING | DEBUG_FLAG_STEPPING_OVER | DEBUG_FLAG_STEPPING_OUT;
	static const UINT32 DEBUG_FLAG_TRACING_ANY      = DEBUG_FLAG_TRACING | DEBUG_FLAG_TRACING_OVER;
	static const UINT32 DEBUG_FLAG_TRANSIENT        = DEBUG_FLAG_STEPPING_ANY | DEBUG_FLAG_STOP_PC |
			DEBUG_FLAG_STOP_INTERRUPT | DEBUG_FLAG_STOP_EXCEPTION | DEBUG_FLAG_STOP_VBLANK | DEBUG_FLAG_STOP_TIME;
};



//**************************************************************************
//  FUNCTION PROTOTYPES
//**************************************************************************

/* ----- initialization and cleanup ----- */

/* initialize the CPU tracking for the debugger */
void debug_cpu_init(running_machine &machine);
void debug_cpu_configure_memory(running_machine &machine, symbol_table &table);

/* flushes all traces; this is useful if a trace is going on when we fatalerror */
void debug_cpu_flush_traces(running_machine &machine);



/* ----- debugging status & information ----- */

/* return the visible CPU device (the one that commands should apply to) */
device_t *debug_cpu_get_visible_cpu(running_machine &machine);

/* TRUE if the debugger is currently stopped within an instruction hook callback */
int debug_cpu_within_instruction_hook(running_machine &machine);

/* return TRUE if the current execution state is stopped */
int debug_cpu_is_stopped(running_machine &machine);



/* ----- symbol table interfaces ----- */

/* return the global symbol table */
symbol_table *debug_cpu_get_global_symtable(running_machine &machine);

/* return the locally-visible symbol table */
symbol_table *debug_cpu_get_visible_symtable(running_machine &machine);



/* ----- misc debugger functions ----- */

/* specifies a debug command script to execute */
void debug_cpu_source_script(running_machine &machine, const char *file);



/* ----- debugger comment helpers ----- */

// save all comments for a given machine
bool debug_comment_save(running_machine &machine);

// load all comments for a given machine
bool debug_comment_load(running_machine &machine);



/* ----- debugger memory accessors ----- */

/* return the physical address corresponding to the given logical address */
int debug_cpu_translate(address_space &space, int intention, offs_t *address);

/* return a byte from the specified memory space */
UINT8 debug_read_byte(address_space &space, offs_t address, int apply_translation);

/* return a word from the specified memory space */
UINT16 debug_read_word(address_space &space, offs_t address, int apply_translation);

/* return a dword from the specified memory space */
UINT32 debug_read_dword(address_space &space, offs_t address, int apply_translation);

/* return a qword from the specified memory space */
UINT64 debug_read_qword(address_space &space, offs_t address, int apply_translation);

/* return 1,2,4 or 8 bytes from the specified memory space */
UINT64 debug_read_memory(address_space &space, offs_t address, int size, int apply_translation);

/* write a byte to the specified memory space */
void debug_write_byte(address_space &space, offs_t address, UINT8 data, int apply_translation);

/* write a word to the specified memory space */
void debug_write_word(address_space &space, offs_t address, UINT16 data, int apply_translation);

/* write a dword to the specified memory space */
void debug_write_dword(address_space &space, offs_t address, UINT32 data, int apply_translation);

/* write a qword to the specified memory space */
void debug_write_qword(address_space &space, offs_t address, UINT64 data, int apply_translation);

/* write 1,2,4 or 8 bytes to the specified memory space */
void debug_write_memory(address_space &space, offs_t address, UINT64 data, int size, int apply_translation);

/* read 1,2,4 or 8 bytes at the given offset from opcode space */
UINT64 debug_read_opcode(address_space &space, offs_t offset, int size);


#endif
