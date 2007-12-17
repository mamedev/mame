/*********************************************************************

    debugcpu.h

    Debugger CPU/memory interface engine.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef __DEBUGCPU_H__
#define __DEBUGCPU_H__

#include "express.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define TRACE_LOOPS				64
#define DEBUG_HISTORY_SIZE		256

#define WATCHPOINT_READ			1
#define WATCHPOINT_WRITE		2
#define WATCHPOINT_READWRITE	(WATCHPOINT_READ | WATCHPOINT_WRITE)

enum
{
	EXECUTION_STATE_STOPPED,
	EXECUTION_STATE_RUNNING,
	EXECUTION_STATE_NEXT_CPU,
	EXECUTION_STATE_STEP_INTO,
	EXECUTION_STATE_STEP_OVER,
	EXECUTION_STATE_STEP_OUT
};



/***************************************************************************
    MACROS
***************************************************************************/

#define ADDR2BYTE(val,info,spc) (((val) << (info)->space[spc].addr2byte_lshift) >> (info)->space[spc].addr2byte_rshift)
#define ADDR2BYTE_MASKED(val,info,spc) (ADDR2BYTE(val,info,spc) & (info)->space[spc].logbytemask)
#define BYTE2ADDR(val,info,spc) (((val) << (info)->space[spc].addr2byte_rshift) >> (info)->space[spc].addr2byte_lshift)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef void (*debug_hook_read_ptr)(int spacenum, int size, offs_t address);
typedef void (*debug_hook_write_ptr)(int spacenum, int size, offs_t address, UINT64 data);


typedef struct _debug_trace_info debug_trace_info;
typedef struct _debug_space_info debug_space_info;
typedef struct _debug_hotspot_entry debug_hotspot_entry;
typedef struct _debug_cpu_info debug_cpu_info;
typedef struct _debug_cpu_breakpoint debug_cpu_breakpoint;
typedef struct _debug_cpu_watchpoint debug_cpu_watchpoint;


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
	debug_cpu_watchpoint *first_wp;				/* first watchpoint */
};


struct _debug_hotspot_entry
{
	offs_t			access;						/* access address */
	offs_t			pc;							/* PC of the access */
	int				spacenum;					/* space where the access occurred */
	UINT32			count;						/* number of hits */
};


struct _debug_cpu_info
{
	UINT8			valid;						/* are we valid? */
	UINT8			endianness;					/* little or bigendian */
	UINT8			opwidth;					/* width of an opcode */
	UINT8			ignoring;					/* are we ignoring this CPU's execution? */
	offs_t			temp_breakpoint_pc;			/* temporary breakpoint PC */
	int				read_watchpoints;			/* total read watchpoints on this CPU */
	int				write_watchpoints;			/* total write watchpoints on this CPU */
	symbol_table *	symtable;					/* symbol table for expression evaluation */
	debug_trace_info trace;						/* trace info */
	debug_cpu_breakpoint *first_bp;				/* first breakpoint */
	debug_space_info space[ADDRESS_SPACES];		/* per-address space info */
	debug_hotspot_entry *hotspots;				/* hotspot list */
	offs_t			pc_history[DEBUG_HISTORY_SIZE]; /* history of recent PCs */
	UINT32			pc_history_index;			/* current history index */
	int				hotspot_count;				/* number of hotspots */
	int				hotspot_threshhold;			/* threshhold for the number of hits to print */
	int				(*translate)(int space, offs_t *address);/* address translation routine */
	int 			(*read)(int space, UINT32 offset, int size, UINT64 *value); /* memory read routine */
	int				(*write)(int space, UINT32 offset, int size, UINT64 value); /* memory write routine */
	int				(*readop)(UINT32 offset, int size, UINT64 *value);	/* opcode read routine */
	int				(*instrhook)(offs_t pc);	/* per-instruction callback hook */
};


struct _debug_cpu_breakpoint
{
	int				index;						/* user reported index */
	UINT8			enabled;					/* enabled? */
	offs_t			address;					/* execution address */
	parsed_expression *condition;		/* condition */
	char *			action;						/* action */
	debug_cpu_breakpoint *next;					/* next in the list */
};


struct _debug_cpu_watchpoint
{
	int				index;						/* user reported index */
	UINT8			enabled;					/* enabled? */
	UINT8			type;						/* type (read/write) */
	offs_t			address;					/* start address */
	offs_t			length;						/* length of watch area */
	parsed_expression *condition;		/* condition */
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

/* initialization */
void				debug_cpu_init(running_machine *machine);

/* utilities */
const debug_cpu_info *debug_get_cpu_info(int cpunum);
void				debug_halt_on_next_instruction(void);
void				debug_refresh_display(void);
int					debug_get_execution_state(void);
UINT32				debug_get_execution_counter(void);
void				debug_trace_printf(int cpunum, const char *fmt, ...);
void				debug_source_script(const char *file);
void				debug_flush_traces(void);

/* debugging hooks */
void				debug_vblank_hook(void);
void				debug_interrupt_hook(int cpunum, int irqline);
void				debug_get_memory_hooks(int cpunum, debug_hook_read_ptr *read, debug_hook_write_ptr *write);
void				debug_set_instruction_hook(int cpunum, int (*hook)(offs_t pc));

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
void				debug_check_breakpoints(int cpunum, offs_t pc);
debug_cpu_breakpoint *debug_breakpoint_first(int cpunum);
int					debug_breakpoint_set(int cpunum, offs_t address, parsed_expression *condition, const char *action);
int					debug_breakpoint_clear(int bpnum);
int					debug_breakpoint_enable(int bpnum, int enable);

/* watchpoints */
debug_cpu_watchpoint *debug_watchpoint_first(int cpunum, int spacenum);
int					debug_watchpoint_set(int cpunum, int spacenum, int type, offs_t address, offs_t length, parsed_expression *condition, const char *action);
int					debug_watchpoint_clear(int wpnum);
int					debug_watchpoint_enable(int wpnum, int enable);

/* hotspots */
int					debug_hotspot_track(int cpunum, int numspots, int threshhold);

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
