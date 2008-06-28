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
#define DEBUG_FLAG_LIVE_WPR_PROGRAM	0x01000000		/* there are live read watchpoints in program address space */
#define DEBUG_FLAG_LIVE_WPR_DATA	0x02000000		/* there are live read watchpoints in data address space */
#define DEBUG_FLAG_LIVE_WPR_IO		0x04000000		/* there are live read watchpoints in io address space */
#define DEBUG_FLAG_LIVE_WPW_PROGRAM	0x10000000		/* there are live write watchpoints in program address space */
#define DEBUG_FLAG_LIVE_WPW_DATA	0x20000000		/* there are live write watchpoints in data address space */
#define DEBUG_FLAG_LIVE_WPW_IO		0x40000000		/* there are live write watchpoints in io address space */

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

#define DEBUG_FLAG_READ_WATCHPOINT	(DEBUG_FLAG_LIVE_WPR_PROGRAM | \
									 DEBUG_FLAG_LIVE_WPR_DATA | \
									 DEBUG_FLAG_LIVE_WPR_IO)

#define DEBUG_FLAG_WRITE_WATCHPOINT	(DEBUG_FLAG_LIVE_WPW_PROGRAM | \
									 DEBUG_FLAG_LIVE_WPW_DATA | \
									 DEBUG_FLAG_LIVE_WPW_IO)

#define DEBUG_FLAG_WATCHPOINT		(DEBUG_FLAG_READ_WATCHPOINT | \
									 DEBUG_FLAG_WRITE_WATCHPOINT)



/***************************************************************************
    MACROS
***************************************************************************/

#define ADDR2BYTE(val,info,spc) (((val) << (info)->space[spc].addr2byte_lshift) >> (info)->space[spc].addr2byte_rshift)
#define ADDR2BYTE_MASKED(val,info,spc) (ADDR2BYTE(val,info,spc) & (info)->space[spc].logbytemask)
#define BYTE2ADDR(val,info,spc) (((val) << (info)->space[spc].addr2byte_rshift) >> (info)->space[spc].addr2byte_lshift)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef void (*debug_hook_read_func)(int spacenum, offs_t address, UINT64 mem_mask);
typedef void (*debug_hook_write_func)(int spacenum, offs_t address, UINT64 data, UINT64 mem_mask);


typedef struct _debug_cpu_breakpoint debug_cpu_breakpoint;
typedef struct _debug_cpu_watchpoint debug_cpu_watchpoint;


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


typedef struct _debug_space_info debug_space_info;
struct _debug_space_info
{
	UINT8			databytes;					/* width of the data bus, in bytes */
	UINT8			pageshift;					/* page shift */
	UINT8			addr2byte_lshift;			/* left shift to convert CPU address to a byte value */
	UINT8			addr2byte_rshift;			/* right shift to convert CPU address to a byte value */
	UINT8			physchars;					/* number of characters to use for physical addresses */
	UINT8			logchars;					/* number of characters to use for logical addresses */
	offs_t			physaddrmask;				/* physical address mask */
	offs_t			logaddrmask;				/* logical address mask */
	offs_t			physbytemask;				/* physical byte mask */
	offs_t			logbytemask;				/* logical byte mask */
	debug_cpu_watchpoint *wplist;				/* list of watchpoints */
};


typedef struct _debug_hotspot_entry debug_hotspot_entry;
struct _debug_hotspot_entry
{
	offs_t			access;						/* access address */
	offs_t			pc;							/* PC of the access */
	int				spacenum;					/* space where the access occurred */
	UINT32			count;						/* number of hits */
};


typedef struct _debug_cpu_info debug_cpu_info;
struct _debug_cpu_info
{
	UINT8			valid;						/* are we valid? */
	UINT8			endianness;					/* little or bigendian */
	UINT8			opwidth;					/* width of an opcode */
	UINT32			flags;						/* debugging flags for this CPU */
	offs_t			stepaddr;					/* step target address for DEBUG_FLAG_STEPPING_OVER */
	int				stepsleft;					/* number of steps left until done */
	offs_t			stopaddr;					/* stop address for DEBUG_FLAG_STOP_PC */
	attotime		stoptime;					/* stop time for DEBUG_FLAG_STOP_TIME */
	int				stopirq;					/* stop IRQ number for DEBUG_FLAG_STOP_INTERRUPT */
	int				stopexception;				/* stop exception number for DEBUG_FLAG_STOP_EXCEPTION */
	attotime		endexectime;				/* ending time of the current execution */
	symbol_table *	symtable;					/* symbol table for expression evaluation */
	debug_trace_info trace;						/* trace info */
	debug_cpu_breakpoint *bplist;				/* list of breakpoints */
	debug_space_info space[ADDRESS_SPACES];		/* per-address space info */
	debug_hotspot_entry *hotspots;				/* hotspot list */
	offs_t			pc_history[DEBUG_HISTORY_SIZE]; /* history of recent PCs */
	UINT32			pc_history_index;			/* current history index */
	int				hotspot_count;				/* number of hotspots */
	int				hotspot_threshhold;			/* threshhold for the number of hits to print */
	cpu_translate_func translate;				/* address translation routine */
	cpu_read_func	read; 						/* memory read routine */
	cpu_write_func	write;						/* memory write routine */
	cpu_readop_func	readop;						/* opcode read routine */
	int				(*instrhook)(offs_t pc);	/* per-instruction callback hook */
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

extern FILE *debug_source_file;
extern symbol_table *global_symtable;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* ----- initialization ----- */

/* initialize the CPU tracking for the debugger */
void debug_cpu_init(running_machine *machine);


/* ----- core debugger hooks ----- */

/* the CPU execution system calls this hook before beginning execution for the given CPU */
void debug_cpu_start_hook(running_machine *machine, int cpunum, attotime endtime);

/* the CPU execution system calls this hook when ending execution for the given CPU */
void debug_cpu_stop_hook(running_machine *machine, int cpunum);

/* the CPU execution system calls this hook when an interrupt is acknowledged */
void debug_cpu_interrupt_hook(running_machine *machine, int cpunum, int irqline);

/* called by the CPU cores when an exception is generated */
void debug_cpu_exception_hook(running_machine *machine, int cpunum, int exception);

/* called by the CPU cores before executing each instruction */
void debug_cpu_instruction_hook(running_machine *machine, offs_t curpc);

/* the memory system calls this hook when watchpoints are enabled and a memory read happens */
void debug_cpu_memory_read_hook(running_machine *machine, int cpunum, int spacenum, offs_t address, UINT64 mem_mask);

/* the memory system calls this hook when watchpoints are enabled and a memory write happens */
void debug_cpu_memory_write_hook(running_machine *machine, int cpunum, int spacenum, offs_t address, UINT64 data, UINT64 mem_mask);



/* ----- core debugger functions ----- */

int debug_cpu_within_instruction_hook(running_machine *machine);
const debug_cpu_info *debug_get_cpu_info(int cpunum);
void debug_cpu_halt_on_next_instruction(running_machine *machine);
int	debug_cpu_is_stopped(running_machine *machine);
void debug_cpu_trace_printf(int cpunum, const char *fmt, ...) ATTR_PRINTF(2,3);
void debug_cpu_source_script(const char *file);
void debug_cpu_flush_traces(void);

/* debugging hooks */
void				debug_cpu_get_memory_hooks(int cpunum, debug_hook_read_func *read, debug_hook_write_func *write);
void				debug_cpu_set_instruction_hook(int cpunum, int (*hook)(offs_t pc));

/* execution control */
void				debug_cpu_single_step(int numsteps);
void				debug_cpu_single_step_over(int numsteps);
void				debug_cpu_single_step_out(void);
void				debug_cpu_go(offs_t targetpc);
void				debug_cpu_go_vblank(void);
void				debug_cpu_go_interrupt(int irqline);
void				debug_cpu_go_milliseconds(UINT64 milliseconds);
void				debug_cpu_next_cpu(void);
void				debug_cpu_ignore_cpu(int cpunum, int ignore);

/* tracing support */
void				debug_cpu_trace(int cpunum, FILE *file, int trace_over, const char *action);

/* breakpoints */
int					debug_cpu_breakpoint_set(int cpunum, offs_t address, parsed_expression *condition, const char *action);
int					debug_cpu_breakpoint_clear(int bpnum);
int					debug_cpu_breakpoint_enable(int bpnum, int enable);

/* watchpoints */
int					debug_cpu_watchpoint_set(int cpunum, int spacenum, int type, offs_t address, offs_t length, parsed_expression *condition, const char *action);
int					debug_cpu_watchpoint_clear(int wpnum);
int					debug_cpu_watchpoint_enable(int wpnum, int enable);

/* hotspots */
int					debug_cpu_hotspot_track(int cpunum, int numspots, int threshhold);

/* memory accessors */
UINT8				debug_read_byte(int spacenum, offs_t address, int apply_translation);
UINT16				debug_read_word(int spacenum, offs_t address, int apply_translation);
UINT32				debug_read_dword(int spacenum, offs_t address, int apply_translation);
UINT64				debug_read_qword(int spacenum, offs_t address, int apply_translation);
void				debug_write_byte(int spacenum, offs_t address, UINT8 data, int apply_translation);
void				debug_write_word(int spacenum, offs_t address, UINT16 data, int apply_translation);
void				debug_write_dword(int spacenum, offs_t address, UINT32 data, int apply_translation);
void				debug_write_qword(int spacenum, offs_t address, UINT64 data, int apply_translation);
UINT64				debug_read_opcode(UINT32 offset, int size, int arg);

#endif
