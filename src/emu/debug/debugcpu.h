/*********************************************************************

    debugcpu.h

    Debugger CPU/memory interface engine.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef __DEBUGCPU_H__
#define __DEBUGCPU_H__

#include "express.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define TRACE_LOOPS					64
#define DEBUG_HISTORY_SIZE			256

#define WATCHPOINT_READ				1
#define WATCHPOINT_WRITE			2
#define WATCHPOINT_READWRITE		(WATCHPOINT_READ | WATCHPOINT_WRITE)

#define DEBUG_FLAG_OBSERVING		0x00000001		/* observing this CPU */
#define DEBUG_FLAG_HISTORY			0x00000002		/* tracking this CPU's history */
#define DEBUG_FLAG_TRACING			0x00000004		/* tracing this CPU */
#define DEBUG_FLAG_TRACING_OVER		0x00000008		/* tracing this CPU with step over behavior */
#define DEBUG_FLAG_HOOKED			0x00000010		/* per-instruction callback hook */
#define DEBUG_FLAG_STEPPING			0x00000020		/* CPU is single stepping */
#define DEBUG_FLAG_STEPPING_OVER	0x00000040		/* CPU is stepping over a function */
#define DEBUG_FLAG_STEPPING_OUT		0x00000080		/* CPU is stepping out of a function */
#define DEBUG_FLAG_STOP_PC			0x00000100		/* there is a pending stop at cpu->breakpc */
#define DEBUG_FLAG_STOP_CONTEXT		0x00000200		/* there is a pending stop on next context switch */
#define DEBUG_FLAG_STOP_INTERRUPT	0x00000400		/* there is a pending stop on the next interrupt */
#define DEBUG_FLAG_STOP_EXCEPTION	0x00000800		/* there is a pending stop on the next exception */
#define DEBUG_FLAG_STOP_VBLANK		0x00001000		/* there is a pending stop on the next VBLANK */
#define DEBUG_FLAG_STOP_TIME		0x00002000		/* there is a pending stop at cpu->stoptime */
#define DEBUG_FLAG_LIVE_BP			0x00010000		/* there are live breakpoints for this CPU */

#define DEBUG_FLAG_STEPPING_ANY		(DEBUG_FLAG_STEPPING | \
									 DEBUG_FLAG_STEPPING_OVER | \
									 DEBUG_FLAG_STEPPING_OUT)

#define DEBUG_FLAG_TRACING_ANY		(DEBUG_FLAG_TRACING | \
									 DEBUG_FLAG_TRACING_OVER)

#define DEBUG_FLAG_TRANSIENT		(DEBUG_FLAG_STEPPING_ANY | \
									 DEBUG_FLAG_STOP_PC | \
									 DEBUG_FLAG_STOP_CONTEXT | \
									 DEBUG_FLAG_STOP_INTERRUPT | \
									 DEBUG_FLAG_STOP_EXCEPTION | \
									 DEBUG_FLAG_STOP_VBLANK | \
									 DEBUG_FLAG_STOP_TIME)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef int (*debug_instruction_hook_func)(device_t &device, offs_t curpc);


class debug_cpu_breakpoint;
class debug_cpu_watchpoint;
class debug_cpu_comment_group;


class debug_trace_info
{
public:
	debug_trace_info();
	
	FILE *			file;						/* tracing file for this CPU */
	astring			action;						/* action to perform during a trace */
	offs_t			history[TRACE_LOOPS];		/* history of recent PCs */
	int				loops;						/* number of instructions in a loop */
	int				nextdex;					/* next index */
	offs_t			trace_over_target;			/* target for tracing over
                                                    (0 = not tracing over,
                                                    ~0 = not currently tracing over) */
};


struct debug_hotspot_entry
{
	offs_t			access;						/* access address */
	offs_t			pc;							/* PC of the access */
	const address_space *space;					/* space where the access occurred */
	UINT32			count;						/* number of hits */
};


class device_debug
{
	typedef offs_t (*dasm_override_func)(device_t &device, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, int options);

public:
	device_debug(device_t &device, symbol_table *globalsyms);
	~device_debug();
	
	symbol_table *symtable() const { return m_symtable; }
	
	int logaddrchars(int spacenum = AS_PROGRAM) { return (m_memory != NULL && m_memory->space(spacenum) != NULL) ? m_memory->space(spacenum)->logaddrchars : 8; }
	
	offs_t disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
	void set_dasm_override(dasm_override_func dasm_override) { m_dasm_override = dasm_override; }
	int min_opcode_bytes() const { return (m_disasm != NULL) ? m_disasm->max_opcode_bytes() : 1; }
	int max_opcode_bytes() const { return (m_disasm != NULL) ? m_disasm->max_opcode_bytes() : 1; }
	
	void start_hook(attotime endtime);
	void stop_hook();
	void interrupt_hook(int irqline);
	void exception_hook(int exception);
	void instruction_hook(offs_t curpc);
	void memory_read_hook(const address_space &space, offs_t address, UINT64 mem_mask);
	void memory_write_hook(const address_space &space, offs_t address, UINT64 data, UINT64 mem_mask);

	void halt_on_next_instruction(const char *fmt, ...);
	void ignore(bool ignore = true);
	bool observing() const { return ((m_flags & DEBUG_FLAG_OBSERVING) != 0); }

	void single_step(int numsteps = 1);
	void single_step_over(int numsteps = 1);
	void single_step_out();
	
	void go(offs_t targetpc = ~0);
	void go_vblank();
	void go_interrupt(int irqline = -1);
	void go_exception(int exception);
	void go_milliseconds(UINT64 milliseconds);
	void go_next_device();

	debug_cpu_breakpoint *breakpoint_first() const { return m_bplist; }
	int breakpoint_set(offs_t address, parsed_expression *condition = NULL, const char *action = NULL);
	bool breakpoint_clear(int index);
	void breakpoint_clear_all();
	bool breakpoint_enable(int index, bool enable = true);
	void breakpoint_enable_all(bool enable = true);

	debug_cpu_watchpoint *watchpoint_first(int spacenum) const { return m_wplist[spacenum]; }
	int watchpoint_set(const address_space &space, int type, offs_t address, offs_t length, parsed_expression *condition, const char *action);
	bool watchpoint_clear(int wpnum);
	void watchpoint_clear_all();
	bool watchpoint_enable(int index, bool enable = true);
	void watchpoint_enable_all(bool enable = true);

	bool hotspot_tracking_enabled() const { return (m_hotspots != NULL); }
	void hotspot_track(int numspots, int threshhold);
	
	offs_t history_pc(int index) const;

	void trace(FILE *file, bool trace_over, const char *action);
	void trace_printf(const char *fmt, ...);
	void trace_flush() { if (m_trace.file != NULL) fflush(m_trace.file); }

	void set_instruction_hook(debug_instruction_hook_func hook);

	void reset_transient_flag() { m_flags &= ~DEBUG_FLAG_TRANSIENT; }

private:
	void compute_debug_flags();

	void perform_trace();
	void prepare_for_step_overout();

	void breakpoint_update_flags();
	void breakpoint_check(offs_t pc);

	void watchpoint_update_flags(const address_space &space);
	void watchpoint_check(const address_space &space, int type, offs_t address, UINT64 value_to_write, UINT64 mem_mask);

	void hotspot_check(const address_space &space, offs_t address);
	UINT32 dasm_wrapped(astring &buffer, offs_t pc);
	
	device_t &				m_device;
	device_execute_interface *m_exec;
	device_memory_interface *m_memory;
	device_state_interface *m_state;
	device_disasm_interface *m_disasm;

	symbol_table *			m_symtable;					/* symbol table for expression evaluation */
	UINT32					m_flags;						/* debugging flags for this CPU */
	UINT8					m_opwidth;					/* width of an opcode */
	offs_t					m_stepaddr;					/* step target address for DEBUG_FLAG_STEPPING_OVER */
	int						m_stepsleft;					/* number of steps left until done */
	offs_t					m_stopaddr;					/* stop address for DEBUG_FLAG_STOP_PC */
	attotime				m_stoptime;					/* stop time for DEBUG_FLAG_STOP_TIME */
	int						m_stopirq;					/* stop IRQ number for DEBUG_FLAG_STOP_INTERRUPT */
	int						m_stopexception;				/* stop exception number for DEBUG_FLAG_STOP_EXCEPTION */
	attotime				m_endexectime;				/* ending time of the current execution */
	debug_trace_info		m_trace;						/* trace info */
	debug_cpu_breakpoint *	m_bplist;						/* list of breakpoints */
	debug_hotspot_entry *	m_hotspots;					/* hotspot list */
	offs_t					m_pc_history[DEBUG_HISTORY_SIZE]; /* history of recent PCs */
	UINT32					m_pc_history_index;			/* current history index */
	int						m_hotspot_count;				/* number of hotspots */
	int						m_hotspot_threshhold;			/* threshhold for the number of hits to print */
	dasm_override_func		m_dasm_override;				/* pointer to provided override function */
	debug_instruction_hook_func m_instrhook;				/* per-instruction callback hook */
	debug_cpu_watchpoint *	m_wplist[ADDRESS_SPACES];		/* watchpoint lists for each address space */

public: // until comments get folded in
	debug_cpu_comment_group *m_comments;					/* disassembly comments */
};


class debug_cpu_breakpoint
{
	friend class device_debug;
	
public:
	debug_cpu_breakpoint(int index, offs_t address, parsed_expression *condition = NULL, const char *action = NULL);
	~debug_cpu_breakpoint();
	
	debug_cpu_breakpoint *next() const { return m_next; }
	int index() const { return m_index; }
	bool enabled() const { return m_enabled; }
	offs_t address() const { return m_address; }
	const char *condition() const { return (m_condition != NULL) ? expression_original_string(m_condition) : NULL; }
	const char *action() const { return m_action; }
	
private:
	bool hit(offs_t pc);

	debug_cpu_breakpoint *m_next;					/* next in the list */
	int				m_index;						/* user reported index */
	UINT8			m_enabled;					/* enabled? */
	offs_t			m_address;					/* execution address */
	parsed_expression *m_condition;				/* condition */
	astring			m_action;						/* action */
};


class debug_cpu_watchpoint
{
	friend class device_debug;

public:
	debug_cpu_watchpoint(int index, const address_space &space, int type, offs_t address, offs_t length, parsed_expression *condition = NULL, const char *action = NULL);
	~debug_cpu_watchpoint();

	debug_cpu_watchpoint *next() const { return m_next; }
	const address_space &space() const { return m_space; }
	int index() const { return m_index; }
	int type() const { return m_type; }
	bool enabled() const { return m_enabled; }
	offs_t address() const { return m_address; }
	offs_t length() const { return m_length; }
	const char *condition() const { return (m_condition != NULL) ? expression_original_string(m_condition) : NULL; }
	const char *action() const { return m_action; }
	
private:
	bool hit(int type, offs_t address, int size);

	debug_cpu_watchpoint *m_next;					/* next in the list */
	const address_space &m_space;					// address space
	int				m_index;						/* user reported index */
	bool			m_enabled;					/* enabled? */
	UINT8			m_type;						/* type (read/write) */
	offs_t			m_address;					/* start address */
	offs_t			m_length;						/* length of watch area */
	parsed_expression *m_condition;				/* condition */
	astring			m_action;						/* action */
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

extern const express_callbacks debug_expression_callbacks;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* ----- initialization and cleanup ----- */

/* initialize the CPU tracking for the debugger */
void debug_cpu_init(running_machine *machine);

/* flushes all traces; this is useful if a trace is going on when we fatalerror */
void debug_cpu_flush_traces(running_machine *machine);



/* ----- debugging status & information ----- */

/* return the visible CPU device (the one that commands should apply to) */
device_t *debug_cpu_get_visible_cpu(running_machine *machine);

/* TRUE if the debugger is currently stopped within an instruction hook callback */
int debug_cpu_within_instruction_hook(running_machine *machine);

/* return TRUE if the current execution state is stopped */
int	debug_cpu_is_stopped(running_machine *machine);



/* ----- symbol table interfaces ----- */

/* return the global symbol table */
symbol_table *debug_cpu_get_global_symtable(running_machine *machine);

/* return the locally-visible symbol table */
symbol_table *debug_cpu_get_visible_symtable(running_machine *machine);



/* ----- memory and disassembly helpers ----- */

/* return the physical address corresponding to the given logical address */
int debug_cpu_translate(const address_space *space, int intention, offs_t *address);



/* ----- misc debugger functions ----- */

/* specifies a debug command script to execute */
void debug_cpu_source_script(running_machine *machine, const char *file);



/* ----- debugger memory accessors ----- */

/* return a byte from the the specified memory space */
UINT8 debug_read_byte(const address_space *space, offs_t address, int apply_translation);

/* return a word from the the specified memory space */
UINT16 debug_read_word(const address_space *space, offs_t address, int apply_translation);

/* return a dword from the the specified memory space */
UINT32 debug_read_dword(const address_space *space, offs_t address, int apply_translation);

/* return a qword from the the specified memory space */
UINT64 debug_read_qword(const address_space *space, offs_t address, int apply_translation);

/* return 1,2,4 or 8 bytes from the specified memory space */
UINT64 debug_read_memory(const address_space *space, offs_t address, int size, int apply_translation);

/* write a byte to the specified memory space */
void debug_write_byte(const address_space *space, offs_t address, UINT8 data, int apply_translation);

/* write a word to the specified memory space */
void debug_write_word(const address_space *space, offs_t address, UINT16 data, int apply_translation);

/* write a dword to the specified memory space */
void debug_write_dword(const address_space *space, offs_t address, UINT32 data, int apply_translation);

/* write a qword to the specified memory space */
void debug_write_qword(const address_space *space, offs_t address, UINT64 data, int apply_translation);

/* write 1,2,4 or 8 bytes to the specified memory space */
void debug_write_memory(const address_space *space, offs_t address, UINT64 data, int size, int apply_translation);

/* read 1,2,4 or 8 bytes at the given offset from opcode space */
UINT64 debug_read_opcode(const address_space *space, offs_t offset, int size, int arg);

#endif
