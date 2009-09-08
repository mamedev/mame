/*********************************************************************

    debugcpu.h

    Debugger CPU/memory interface engine.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef __DEBUGCPU_H__
#define __DEBUGCPU_H__

#include "cpuintrf.h"
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

typedef int (*debug_instruction_hook_func)(const device_config *device, offs_t curpc);


typedef struct _debug_cpu_breakpoint debug_cpu_breakpoint;
typedef struct _debug_cpu_watchpoint debug_cpu_watchpoint;
typedef struct _debug_cpu_comment_group debug_cpu_comment_group;


typedef struct _debug_trace_info debug_trace_info;
struct _debug_trace_info
{
	FILE *			file;						/* tracing file for this CPU */
	char *			action;						/* action to perform during a trace */
	offs_t			history[TRACE_LOOPS];		/* history of recent PCs */
	int				loops;						/* number of instructions in a loop */
	int				nextdex;					/* next index */
	offs_t			trace_over_target;			/* target for tracing over
                                                    (0 = not tracing over,
                                                    ~0 = not currently tracing over) */
};


typedef struct _debug_hotspot_entry debug_hotspot_entry;
struct _debug_hotspot_entry
{
	offs_t			access;						/* access address */
	offs_t			pc;							/* PC of the access */
	const address_space *space;					/* space where the access occurred */
	UINT32			count;						/* number of hits */
};


/* In cpuintrf.h: typedef struct _cpu_debug_data cpu_debug_data; */
struct _cpu_debug_data
{
	const device_config *	device;						/* CPU device object */
	symbol_table *			symtable;					/* symbol table for expression evaluation */
	UINT32					flags;						/* debugging flags for this CPU */
	UINT8					opwidth;					/* width of an opcode */
	offs_t					stepaddr;					/* step target address for DEBUG_FLAG_STEPPING_OVER */
	int						stepsleft;					/* number of steps left until done */
	offs_t					stopaddr;					/* stop address for DEBUG_FLAG_STOP_PC */
	attotime				stoptime;					/* stop time for DEBUG_FLAG_STOP_TIME */
	int						stopirq;					/* stop IRQ number for DEBUG_FLAG_STOP_INTERRUPT */
	int						stopexception;				/* stop exception number for DEBUG_FLAG_STOP_EXCEPTION */
	attotime				endexectime;				/* ending time of the current execution */
	debug_trace_info	 	trace;						/* trace info */
	debug_cpu_breakpoint *	bplist;						/* list of breakpoints */
	debug_hotspot_entry *	hotspots;					/* hotspot list */
	offs_t					pc_history[DEBUG_HISTORY_SIZE]; /* history of recent PCs */
	UINT32					pc_history_index;			/* current history index */
	int						hotspot_count;				/* number of hotspots */
	int						hotspot_threshhold;			/* threshhold for the number of hits to print */
	cpu_read_func			read; 						/* memory read routine */
	cpu_write_func			write;						/* memory write routine */
	cpu_readop_func			readop;						/* opcode read routine */
	cpu_translate_func 		translate;					/* pointer to CPU's translate function */
	cpu_disassemble_func	disassemble;				/* pointer to CPU's dissasemble function */
	cpu_disassemble_func 	dasm_override;				/* pointer to provided override function */
	debug_instruction_hook_func instrhook;				/* per-instruction callback hook */
	debug_cpu_watchpoint *	wplist[ADDRESS_SPACES];		/* watchpoint lists for each address space */
	debug_cpu_comment_group *comments;					/* disassembly comments */
};


struct _debug_cpu_breakpoint
{
	debug_cpu_breakpoint *next;					/* next in the list */
	int				index;						/* user reported index */
	UINT8			enabled;					/* enabled? */
	offs_t			address;					/* execution address */
	parsed_expression *condition;				/* condition */
	char *			action;						/* action */
};


struct _debug_cpu_watchpoint
{
	int				index;						/* user reported index */
	UINT8			enabled;					/* enabled? */
	UINT8			type;						/* type (read/write) */
	offs_t			address;					/* start address */
	offs_t			length;						/* length of watch area */
	parsed_expression *condition;				/* condition */
	char *			action;						/* action */
	debug_cpu_watchpoint *next;					/* next in the list */
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
const device_config *debug_cpu_get_visible_cpu(running_machine *machine);

/* TRUE if the debugger is currently stopped within an instruction hook callback */
int debug_cpu_within_instruction_hook(running_machine *machine);

/* return TRUE if the current execution state is stopped */
int	debug_cpu_is_stopped(running_machine *machine);



/* ----- symbol table interfaces ----- */

/* return the global symbol table */
symbol_table *debug_cpu_get_global_symtable(running_machine *machine);

/* return the locally-visible symbol table */
symbol_table *debug_cpu_get_visible_symtable(running_machine *machine);

/* return a specific CPU's symbol table */
symbol_table *debug_cpu_get_symtable(const device_config *device);



/* ----- memory and disassembly helpers ----- */

/* return the physical address corresponding to the given logical address */
int debug_cpu_translate(const address_space *space, int intention, offs_t *address);

/* disassemble a line at a given PC on a given CPU */
offs_t debug_cpu_disassemble(const device_config *device, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);

/* set an override handler for disassembly */
void debug_cpu_set_dasm_override(const device_config *device, cpu_disassemble_func dasm_override);



/* ----- core debugger hooks ----- */

/* the CPU execution system calls this hook before beginning execution for the given CPU */
void debug_cpu_start_hook(const device_config *device, attotime endtime);

/* the CPU execution system calls this hook when ending execution for the given CPU */
void debug_cpu_stop_hook(const device_config *device);

/* the CPU execution system calls this hook when an interrupt is acknowledged */
void debug_cpu_interrupt_hook(const device_config *device, int irqline);

/* called by the CPU cores when an exception is generated */
void debug_cpu_exception_hook(const device_config *device, int exception);

/* called by the CPU cores before executing each instruction */
void debug_cpu_instruction_hook(const device_config *device, offs_t curpc);

/* the memory system calls this hook when watchpoints are enabled and a memory read happens */
void debug_cpu_memory_read_hook(const address_space *space, offs_t address, UINT64 mem_mask);

/* the memory system calls this hook when watchpoints are enabled and a memory write happens */
void debug_cpu_memory_write_hook(const address_space *space, offs_t address, UINT64 data, UINT64 mem_mask);



/* ----- execution control ----- */

/* halt in the debugger on the next instruction */
void debug_cpu_halt_on_next_instruction(const device_config *device, const char *fmt, ...) ATTR_PRINTF(2,3);

/* ignore/observe a given CPU */
void debug_cpu_ignore_cpu(const device_config *cpu, int ignore);

/* single step the visible CPU past the requested number of instructions */
void debug_cpu_single_step(running_machine *machine, int numsteps);

/* single step the visible over the requested number of instructions */
void debug_cpu_single_step_over(running_machine *machine, int numsteps);

/* single step the visible CPU out of the current function */
void debug_cpu_single_step_out(running_machine *machine);

/* execute the visible CPU until it hits the given address */
void debug_cpu_go(running_machine *machine, offs_t targetpc);

/* execute until the next VBLANK */
void debug_cpu_go_vblank(running_machine *machine);

/* execute until the specified interrupt fires on the visible CPU */
void debug_cpu_go_interrupt(running_machine *machine, int irqline);

/* execute until the specified exception fires on the visible CPU */
void debug_cpu_go_exception(running_machine *machine, int exception);

/* execute until the specified delay elapses */
void debug_cpu_go_milliseconds(running_machine *machine, UINT64 milliseconds);

/* execute until we hit the next CPU */
void debug_cpu_next_cpu(running_machine *machine);



/* ----- breakpoints ----- */

/* set a new breakpoint, returning its index */
int	debug_cpu_breakpoint_set(const device_config *device, offs_t address, parsed_expression *condition, const char *action);

/* clear a breakpoint by index */
int	debug_cpu_breakpoint_clear(running_machine *machine, int bpnum);

/* enable/disable a breakpoint by index */
int	debug_cpu_breakpoint_enable(running_machine *machine, int bpnum, int enable);



/* ----- watchpoints ----- */

/* set a new watchpoint, returning its index */
int	debug_cpu_watchpoint_set(const address_space *space, int type, offs_t address, offs_t length, parsed_expression *condition, const char *action);

/* clear a watchpoint by index */
int	debug_cpu_watchpoint_clear(running_machine *machine, int wpnum);

/* enable/disable a watchpoint by index */
int	debug_cpu_watchpoint_enable(running_machine *machine, int wpnum, int enable);



/* ----- misc debugger functions ----- */

/* specifies a debug command script to execute */
void debug_cpu_source_script(running_machine *machine, const char *file);

/* trace execution of a given CPU */
void debug_cpu_trace(const device_config *device, FILE *file, int trace_over, const char *action);

/* output data into the given CPU's tracefile, if tracing */
void debug_cpu_trace_printf(const device_config *device, const char *fmt, ...) ATTR_PRINTF(2,3);

/* set a hook to be called on each instruction for a given CPU */
void debug_cpu_set_instruction_hook(const device_config *device, debug_instruction_hook_func hook);

/* hotspots */
int	debug_cpu_hotspot_track(const device_config *device, int numspots, int threshhold);



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
