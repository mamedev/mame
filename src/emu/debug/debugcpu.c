/*********************************************************************

    debugcpu.c

    Debugger CPU/memory interface engine.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************

    Future work:

    - enable history to be enabled/disabled to improve performance

*********************************************************************/

#include "osdepend.h"
#include "driver.h"
#include "debugcpu.h"
#include "debugcmd.h"
#include "debugcmt.h"
#include "debugcon.h"
#include "express.h"
#include "debugvw.h"
#include "debugger.h"
#include "deprecat.h"
#include "uiinput.h"
#include "machine/eeprom.h"
#include <ctype.h>



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define NUM_TEMP_VARIABLES	10

enum
{
	EXECUTION_STATE_STOPPED,
	EXECUTION_STATE_RUNNING
};



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _debugger_private debugger_private;
struct _debugger_private
{
	debug_cpu_info	cpuinfo[MAX_CPU];
	debug_cpu_info *livecpu;
	debug_cpu_info *visiblecpu;

	UINT8			within_instruction_hook;
	UINT8			vblank_occurred;
	UINT8			memory_modified;
	UINT8			debugger_access;

	int				execution_state;

	UINT32			bpindex;
	UINT32			wpindex;

	UINT64			wpdata;
	UINT64			wpaddr;
	UINT64 			tempvar[NUM_TEMP_VARIABLES];

	osd_ticks_t 	last_periodic_update_time;
};



/***************************************************************************
    LOCAL VARIABLES
***************************************************************************/

FILE *debug_source_file;
symbol_table *global_symtable;

static debugger_private global;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void debug_cpu_exit(running_machine *machine);
static void perform_trace(debug_cpu_info *info);
static void prepare_for_step_overout(debug_cpu_info *info);
static void process_source_file(void);
static void breakpoint_check(debug_cpu_info *info, offs_t pc);
static void watchpoint_check(running_machine *machine, int cpunum, int spacenum, int type, offs_t address, UINT64 value_to_write, UINT64 mem_mask);
static void check_hotspots(int cpunum, int spacenum, offs_t address);

/* expression handlers */
static UINT64 expression_read_memory(const char *name, int space, UINT32 address, int size);
static UINT64 expression_read_address_space(int cpuindex, int space, offs_t address, int size);
static UINT64 expression_read_program_direct(int cpuindex, int opcode, offs_t address, int size);
static UINT64 expression_read_memory_region(const char *rgntag, offs_t address, int size);
static UINT64 expression_read_eeprom(offs_t address, int size);
static void expression_write_memory(const char *name, int space, UINT32 address, int size, UINT64 data);
static void expression_write_address_space(int cpuindex, int space, offs_t address, int size, UINT64 data);
static void expression_write_program_direct(int cpuindex, int opcode, offs_t address, int size, UINT64 data);
static void expression_write_memory_region(const char *rgntag, offs_t address, int size, UINT64 data);
static void expression_write_eeprom(offs_t address, int size, UINT64 data);
static EXPRERR expression_validate(const char *name, int space);

/* variable getters/setters */
static UINT64 get_wpaddr(void *ref);
static UINT64 get_wpdata(void *ref);
static UINT64 get_cycles(void *ref);
static UINT64 get_cpunum(void *ref);
static UINT64 get_tempvar(void *ref);
static UINT64 get_logunmap(void *ref);
static UINT64 get_beamx(void *ref);
static UINT64 get_beamy(void *ref);
static void set_tempvar(void *ref, UINT64 value);
static void set_logunmap(void *ref, UINT64 value);
static UINT64 get_current_pc(void *ref);
static UINT64 get_cpu_reg(void *ref);
static void set_cpu_reg(void *ref, UINT64 value);


/***************************************************************************
    GLOBAL CONSTANTS
***************************************************************************/

const express_callbacks debug_expression_callbacks =
{
	expression_read_memory,
	expression_write_memory,
	expression_validate
};



/***************************************************************************
    FRONTENDS FOR OLDER FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    debug_cpu_within_instruction_hook - true if
    the debugger is currently live
-------------------------------------------------*/

int debug_cpu_within_instruction_hook(running_machine *machine)
{
	return global.within_instruction_hook;
}


/*-------------------------------------------------
    on_vblank - called when a VBLANK hits
-------------------------------------------------*/

static void on_vblank(const device_config *device, void *param, int vblank_state)
{
	/* just set a global flag to be consumed later */
	if (vblank_state)
		global.vblank_occurred = TRUE;
}


/***************************************************************************
    INITIALIZATION
***************************************************************************/

/*-------------------------------------------------
    debug_cpu_init - initialize the CPU
    information for debugging
-------------------------------------------------*/

void debug_cpu_init(running_machine *machine)
{
	int cpunum, spacenum, regnum;

	/* reset globals */
	global.execution_state = EXECUTION_STATE_STOPPED;
	global.bpindex = 1;
	global.wpindex = 1;
	global.within_instruction_hook = FALSE;
	global.visiblecpu = NULL;

	/* create a global symbol table */
	global_symtable = symtable_alloc(NULL);

	/* add "wpaddr", "wpdata", "cycles", "cpunum", "logunmap" to the global symbol table */
	symtable_add_register(global_symtable, "wpaddr", NULL, get_wpaddr, NULL);
	symtable_add_register(global_symtable, "wpdata", NULL, get_wpdata, NULL);
	symtable_add_register(global_symtable, "cycles", NULL, get_cycles, NULL);
	symtable_add_register(global_symtable, "cpunum", NULL, get_cpunum, NULL);
	symtable_add_register(global_symtable, "logunmap", (void *)ADDRESS_SPACE_PROGRAM, get_logunmap, set_logunmap);
	symtable_add_register(global_symtable, "logunmapd", (void *)ADDRESS_SPACE_DATA, get_logunmap, set_logunmap);
	symtable_add_register(global_symtable, "logunmapi", (void *)ADDRESS_SPACE_IO, get_logunmap, set_logunmap);
	symtable_add_register(global_symtable, "beamx", NULL, get_beamx, NULL);
	symtable_add_register(global_symtable, "beamy", NULL, get_beamy, NULL);

	/* add the temporary variables to the global symbol table */
	for (regnum = 0; regnum < NUM_TEMP_VARIABLES; regnum++)
	{
		char symname[10];
		sprintf(symname, "temp%d", regnum);
		symtable_add_register(global_symtable, symname, &global.tempvar, get_tempvar, set_tempvar);
	}

	/* loop over CPUs and build up their info */
	for (cpunum = 0; cpunum < MAX_CPU; cpunum++)
	{
		cpu_type cputype = machine->config->cpu[cpunum].type;
		debug_cpu_info *info = &global.cpuinfo[cpunum];

		/* if this is a dummy, stop looking */
		memset(info, 0, sizeof(*info));
		if (cputype == CPU_DUMMY)
			break;

		/* reset the PC data */
		info->valid = TRUE;
		info->flags = DEBUG_FLAG_OBSERVING | DEBUG_FLAG_HISTORY;
		info->endianness = cpunum_endianness(cpunum);
		info->opwidth = cpunum_min_instruction_bytes(cpunum);

		/* fetch the memory accessors */
		info->translate = (cpu_translate_func)cpunum_get_info_fct(cpunum, CPUINFO_PTR_TRANSLATE);
		info->read = (cpu_read_func)cpunum_get_info_fct(cpunum, CPUINFO_PTR_READ);
		info->write = (cpu_write_func)cpunum_get_info_fct(cpunum, CPUINFO_PTR_WRITE);
		info->readop = (cpu_readop_func)cpunum_get_info_fct(cpunum, CPUINFO_PTR_READOP);

		/* allocate a symbol table */
		info->symtable = symtable_alloc(global_symtable);

		/* add a global symbol for the current instruction pointer */
		symtable_add_register(info->symtable, "curpc", NULL, get_current_pc, 0);

		/* add all registers into it */
		for (regnum = 0; regnum < MAX_REGS; regnum++)
		{
			const char *str = cpunum_reg_string(cpunum, regnum);
			const char *colon;
			char symname[256];
			int charnum;

			/* skip if we don't get a valid string, or one without a colon */
			if (str == NULL)
				continue;
			if (str[0] == '~')
				str++;
			colon = strchr(str, ':');
			if (colon == NULL)
				continue;

			/* strip all spaces from the name and convert to lowercase */
			for (charnum = 0; charnum < sizeof(symname) - 1 && str < colon; str++)
				if (!isspace(*str))
					symname[charnum++] = tolower(*str);
			symname[charnum] = 0;

			/* add the symbol to the table */
			symtable_add_register(info->symtable, symname, (void *)(FPTR)regnum, get_cpu_reg, set_cpu_reg);
		}

		/* loop over address spaces and get info */
		for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
		{
			debug_space_info *spaceinfo = &info->space[spacenum];
			int datawidth = cpunum_databus_width(cpunum, spacenum);
			int logwidth = cpunum_logaddr_width(cpunum, spacenum);
			int physwidth = cpunum_addrbus_width(cpunum, spacenum);
			int addrshift = cpunum_addrbus_shift(cpunum, spacenum);
			int pageshift = cpunum_page_shift(cpunum, spacenum);

			if (logwidth == 0)
				logwidth = physwidth;

			spaceinfo->databytes = datawidth / 8;
			spaceinfo->pageshift = pageshift;

			/* left/right shifts to convert addresses to bytes */
			spaceinfo->addr2byte_lshift = (addrshift < 0) ? -addrshift : 0;
			spaceinfo->addr2byte_rshift = (addrshift > 0) ?  addrshift : 0;

			/* number of character used to display addresses */
			spaceinfo->physchars = (physwidth + 3) / 4;
			spaceinfo->logchars = (logwidth + 3) / 4;

			/* masks to apply to addresses */
			spaceinfo->physaddrmask = (0xfffffffful >> (32 - physwidth));
			spaceinfo->logaddrmask = (0xfffffffful >> (32 - logwidth));

			/* masks to apply to byte addresses */
			spaceinfo->physbytemask = ((spaceinfo->physaddrmask << spaceinfo->addr2byte_lshift) | ((1 << spaceinfo->addr2byte_lshift) - 1)) >> spaceinfo->addr2byte_rshift;
			spaceinfo->logbytemask = ((spaceinfo->logaddrmask << spaceinfo->addr2byte_lshift) | ((1 << spaceinfo->addr2byte_lshift) - 1)) >> spaceinfo->addr2byte_rshift;
		}
	}

	/* add callback for breaking on VBLANK */
	if (machine->primary_screen != NULL)
		video_screen_register_vblank_callback(machine->primary_screen, on_vblank, NULL);

	add_exit_callback(machine, debug_cpu_exit);
}


/*-------------------------------------------------
    debug_cpu_exit - free all memory
-------------------------------------------------*/

static void debug_cpu_exit(running_machine *machine)
{
	int cpunum, spacenum;

	/* loop over all watchpoints and breakpoints to free their memory */
	for (cpunum = 0; cpunum < MAX_CPU; cpunum++)
	{
		debug_cpu_info *info = &global.cpuinfo[cpunum];

		/* close any tracefiles */
		if (info->trace.file)
			fclose(info->trace.file);
		if (info->trace.action)
			free(info->trace.action);

		/* free the symbol table */
		if (info->symtable)
			symtable_free(info->symtable);

		/* free all breakpoints */
		while (info->bplist)
			debug_cpu_breakpoint_clear(machine, info->bplist->index);

		/* loop over all address spaces */
		for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
		{
			/* free all watchpoints */
			while (info->space[spacenum].wplist)
				debug_cpu_watchpoint_clear(machine, info->space[spacenum].wplist->index);
		}
	}

	/* free the global symbol table */
	if (global_symtable)
		symtable_free(global_symtable);
}



/***************************************************************************
    MAIN CPU CALLBACK
***************************************************************************/

/*-------------------------------------------------
    compute_debug_flags - compute the global
    debug flags for optimal efficiency
-------------------------------------------------*/

static void compute_debug_flags(running_machine *machine, const debug_cpu_info *info)
{
	/* clear out all global flags by default */
	machine->debug_flags = DEBUG_FLAG_ENABLED;

	/* if we are ignoring this CPU, or if events are pending, we're done */
	if ((info->flags & DEBUG_FLAG_OBSERVING) == 0 || mame_is_scheduled_event_pending(machine) || mame_is_save_or_load_pending(machine))
		return;

	/* many of our states require us to be called on each instruction */
	if (global.execution_state == EXECUTION_STATE_STOPPED)
		machine->debug_flags |= DEBUG_FLAG_CALL_HOOK;
	if ((info->flags & (DEBUG_FLAG_HISTORY | DEBUG_FLAG_TRACING_ANY | DEBUG_FLAG_HOOKED |
						DEBUG_FLAG_STEPPING_ANY | DEBUG_FLAG_STOP_PC | DEBUG_FLAG_LIVE_BP)) != 0)
		machine->debug_flags |= DEBUG_FLAG_CALL_HOOK;

	/* if we are stopping at a particular time and that time is within the current timeslice, we need to be called */
	if ((info->flags & DEBUG_FLAG_STOP_TIME) && attotime_compare(info->endexectime, info->stoptime) <= 0)
		machine->debug_flags |= DEBUG_FLAG_CALL_HOOK;

	/* add in the watchpoint flags */
	machine->debug_flags |= (info->flags & DEBUG_FLAG_WATCHPOINT) >> (24 - 4);

	/* if any of the watchpoint flags are set and we're live, tell the memory system */
	if (global.livecpu != NULL && ((info->flags & DEBUG_FLAG_WATCHPOINT) != 0))
		memory_set_context(-1);
}


/*-------------------------------------------------
    reset_transient_flags - reset the transient
    flags on all CPUs
-------------------------------------------------*/

static void reset_transient_flags(running_machine *machine)
{
	int cpunum;

	/* loop over CPUs and reset the transient flags */
	for (cpunum = 0; cpunum < ARRAY_LENGTH(global.cpuinfo); cpunum++)
		global.cpuinfo[cpunum].flags &= ~DEBUG_FLAG_TRANSIENT;
}


/*-------------------------------------------------
    debug_cpu_start_hook - the CPU execution
    system calls this hook before beginning
    execution for the given CPU
-------------------------------------------------*/

void debug_cpu_start_hook(running_machine *machine, int cpunum, attotime endtime)
{
	debug_cpu_info *info = &global.cpuinfo[cpunum];

	assert((machine->debug_flags & DEBUG_FLAG_ENABLED) != 0);

	/* stash a pointer to the current live CPU */
	assert(global.livecpu == NULL);
	global.livecpu = info;

	/* update the target execution end time */
	info->endexectime = endtime;

	/* if a VBLANK occurred, check on things */
	if (global.vblank_occurred && global.execution_state != EXECUTION_STATE_STOPPED)
	{
		global.vblank_occurred = FALSE;

		/* if we were waiting for a VBLANK, signal it now */
		if ((info->flags & DEBUG_FLAG_STOP_VBLANK) != 0)
		{
			global.execution_state = EXECUTION_STATE_STOPPED;
			debug_console_printf("Stopped at VBLANK\n");
		}

		/* check for debug keypresses */
		else if (ui_input_pressed(machine, IPT_UI_DEBUG_BREAK))
		{
			global.execution_state = EXECUTION_STATE_STOPPED;
			debug_console_printf("User-initiated break\n");
		}

		/* while we're here, check for a periodic update */
		else if (info == global.visiblecpu && osd_ticks() > global.last_periodic_update_time + osd_ticks_per_second()/4)
		{
			debug_view_update_all();
			global.last_periodic_update_time = osd_ticks();
		}
	}

	/* recompute the debugging mode */
	compute_debug_flags(machine, info);
}


/*-------------------------------------------------
    debug_cpu_stop_hook - the CPU execution
    system calls this hook when ending execution
    for the given CPU
-------------------------------------------------*/

void debug_cpu_stop_hook(running_machine *machine, int cpunum)
{
	debug_cpu_info *info = &global.cpuinfo[cpunum];

	assert(global.livecpu == info);

	/* if we're stopping on a context switch, handle it now */
	if (info->flags & DEBUG_FLAG_STOP_CONTEXT)
	{
		global.execution_state = EXECUTION_STATE_STOPPED;
		reset_transient_flags(machine);
	}

	/* clear the live CPU */
	global.livecpu = NULL;
}


/*-------------------------------------------------
    debug_cpu_interrupt_hook - called when an
    interrupt is acknowledged
-------------------------------------------------*/

void debug_cpu_interrupt_hook(running_machine *machine, int cpunum, int irqline)
{
	debug_cpu_info *info = &global.cpuinfo[cpunum];

	/* see if this matches a pending interrupt request */
	if ((info->flags & DEBUG_FLAG_STOP_INTERRUPT) != 0 && (info->stopirq == -1 || info->stopirq == irqline))
	{
		global.execution_state = EXECUTION_STATE_STOPPED;
		debug_console_printf("Stopped on interrupt (CPU %d, IRQ %d)\n", cpunum, irqline);
		compute_debug_flags(machine, info);
	}
}


/*-------------------------------------------------
    debug_cpu_exception_hook - called when an
    exception is generated
-------------------------------------------------*/

void debug_cpu_exception_hook(running_machine *machine, int cpunum, int exception)
{
	debug_cpu_info *info = &global.cpuinfo[cpunum];

	/* see if this matches a pending interrupt request */
	if ((info->flags & DEBUG_FLAG_STOP_EXCEPTION) != 0 && (info->stopexception == -1 || info->stopexception == exception))
	{
		global.execution_state = EXECUTION_STATE_STOPPED;
		debug_console_printf("Stopped on exception (CPU %d, exception %d)\n", cpunum, exception);
		compute_debug_flags(machine, info);
	}
}


/*-------------------------------------------------
    debug_cpu_instruction_hook - called by the
    CPU cores before executing each instruction
-------------------------------------------------*/

void debug_cpu_instruction_hook(running_machine *machine, offs_t curpc)
{
	debug_cpu_info *info = global.livecpu;

	/* note that we are in the debugger code */
	global.within_instruction_hook = TRUE;

	/* update the history */
	info->pc_history[info->pc_history_index++ % DEBUG_HISTORY_SIZE] = curpc;

	/* are we tracing? */
	if (info->flags & DEBUG_FLAG_TRACING_ANY)
		perform_trace(info);

	/* per-instruction hook? */
	if (global.execution_state != EXECUTION_STATE_STOPPED && (info->flags & DEBUG_FLAG_HOOKED) != 0 && (*info->instrhook)(curpc))
		global.execution_state = EXECUTION_STATE_STOPPED;

	/* handle single stepping */
	if (global.execution_state != EXECUTION_STATE_STOPPED && (info->flags & DEBUG_FLAG_STEPPING_ANY) != 0)
	{
		/* is this an actual step? */
		if (info->stepaddr == ~0 || curpc == info->stepaddr)
		{
			/* decrement the count and reset the breakpoint */
			info->stepsleft--;
			info->stepaddr = ~0;

			/* if we hit 0, stop */
			if (info->stepsleft == 0)
				global.execution_state = EXECUTION_STATE_STOPPED;

			/* update every 100 steps until we are within 200 of the end */
			else if ((info->flags & DEBUG_FLAG_STEPPING_OUT) == 0 && (info->stepsleft < 200 || info->stepsleft % 100 == 0))
			{
				debug_view_update_all();
				debugger_refresh_display(machine);
			}
		}
	}

	/* handle breakpoints */
	if (global.execution_state != EXECUTION_STATE_STOPPED && (info->flags & (DEBUG_FLAG_STOP_TIME | DEBUG_FLAG_STOP_PC | DEBUG_FLAG_LIVE_BP)) != 0)
	{
		/* see if we hit a target time */
		if ((info->flags & DEBUG_FLAG_STOP_TIME) != 0 && attotime_compare(timer_get_time(), info->stoptime) >= 0)
		{
			debug_console_printf("Stopped at time interval %.1g\n", attotime_to_double(timer_get_time()));
			global.execution_state = EXECUTION_STATE_STOPPED;
		}

		/* check the temp running breakpoint and break if we hit it */
		else if ((info->flags & DEBUG_FLAG_STOP_PC) != 0 && info->stopaddr == curpc)
		{
			debug_console_printf("Stopped at temporary breakpoint %X on CPU %d\n", info->stopaddr, (int)(info - global.cpuinfo));
			global.execution_state = EXECUTION_STATE_STOPPED;
		}

		/* check for execution breakpoints */
		else if ((info->flags & DEBUG_FLAG_LIVE_BP) != 0)
			breakpoint_check(info, curpc);
	}

	/* if we are supposed to halt, do it now */
	if (global.execution_state == EXECUTION_STATE_STOPPED)
	{
		int firststop = TRUE;

		/* reset any transient state */
		reset_transient_flags(machine);

		/* update all views */
		debug_view_update_all();
		debugger_refresh_display(machine);

		/* wait for the debugger; during this time, disable sound output */
		sound_mute(TRUE);
		while (global.execution_state == EXECUTION_STATE_STOPPED)
		{
			/* clear the memory modified flag and wait */
			global.memory_modified = FALSE;
			osd_wait_for_debugger(machine, firststop);
			firststop = FALSE;

			/* if something modified memory, update the screen */
			if (global.memory_modified)
			{
				debug_disasm_update_all();
				debugger_refresh_display(machine);
			}

			/* check for commands in the source file */
			process_source_file();

			/* if an event got scheduled, resume */
			if (mame_is_scheduled_event_pending(machine))
				global.execution_state = EXECUTION_STATE_RUNNING;
		}
		sound_mute(FALSE);

		/* remember the last visible CPU in the debugger */
		global.visiblecpu = info;
	}

	/* handle step out/over on the instruction we are about to execute */
	if ((info->flags & (DEBUG_FLAG_STEPPING_OVER | DEBUG_FLAG_STEPPING_OUT)) != 0 && info->stepaddr == ~0)
		prepare_for_step_overout(info);

	/* no longer in debugger code */
	global.within_instruction_hook = FALSE;
}


/*-------------------------------------------------
    debug_cpu_memory_read_hook - the memory system
    calls this hook when watchpoints are enabled
    and a memory read happens
-------------------------------------------------*/

void debug_cpu_memory_read_hook(running_machine *machine, int cpunum, int spacenum, offs_t address, UINT64 mem_mask)
{
	debug_cpu_info *info = &global.cpuinfo[cpunum];

	/* check watchpoints */
	if ((info->flags & (DEBUG_FLAG_LIVE_WPR_PROGRAM << spacenum)) != 0)
		watchpoint_check(machine, cpunum, spacenum, WATCHPOINT_READ, address, 0, mem_mask);

	/* check hotspots */
	if (info->hotspots != NULL)
		check_hotspots(cpunum, spacenum, address);
}


/*-------------------------------------------------
    debug_cpu_memory_write_hook - the memory
    system calls this hook when watchpoints are
    enabled and a memory write happens
-------------------------------------------------*/

void debug_cpu_memory_write_hook(running_machine *machine, int cpunum, int spacenum, offs_t address, UINT64 data, UINT64 mem_mask)
{
	debug_cpu_info *info = &global.cpuinfo[cpunum];

	/* check watchpoints */
	if ((info->flags & (DEBUG_FLAG_LIVE_WPW_PROGRAM << spacenum)) != 0)
		watchpoint_check(machine, cpunum, spacenum, WATCHPOINT_WRITE, address, data, mem_mask);
}



/***************************************************************************
    EXECUTION CONTROL
***************************************************************************/

/*-------------------------------------------------
    debug_cpu_single_step - single step past the
    requested number of instructions
-------------------------------------------------*/

void debug_cpu_single_step(int numsteps)
{
	debug_cpu_info *info = global.livecpu;

	if (!global.within_instruction_hook)
		return;
	assert(info != NULL);

	info->stepsleft = numsteps;
	info->stepaddr = ~0;
	info->flags |= DEBUG_FLAG_STEPPING;
	global.execution_state = EXECUTION_STATE_RUNNING;
}


/*-------------------------------------------------
    debug_cpu_single_step_over - single step over
    a single instruction
-------------------------------------------------*/

void debug_cpu_single_step_over(int numsteps)
{
	debug_cpu_info *info = global.livecpu;

	if (!global.within_instruction_hook)
		return;
	assert(info != NULL);

	info->stepsleft = numsteps;
	info->stepaddr = ~0;
	info->flags |= DEBUG_FLAG_STEPPING_OVER;
	global.execution_state = EXECUTION_STATE_RUNNING;
}


/*-------------------------------------------------
    debug_cpu_single_step_out - single step out of
    the current function
-------------------------------------------------*/

void debug_cpu_single_step_out(void)
{
	debug_cpu_info *info = global.livecpu;

	if (!global.within_instruction_hook)
		return;
	assert(info != NULL);

	info->stepsleft = 100;
	info->stepaddr = ~0;
	info->flags |= DEBUG_FLAG_STEPPING_OUT;
	global.execution_state = EXECUTION_STATE_RUNNING;
}


/*-------------------------------------------------
    debug_cpu_go - resume execution
-------------------------------------------------*/

void debug_cpu_go(offs_t targetpc)
{
	debug_cpu_info *info = global.livecpu;

	if (!global.within_instruction_hook)
		return;
	assert(info != NULL);

	info->stopaddr = targetpc;
	info->flags |= DEBUG_FLAG_STOP_PC;
	global.execution_state = EXECUTION_STATE_RUNNING;
}


/*-------------------------------------------------
    debug_cpu_go_vblank - run until the next
    VBLANK
-------------------------------------------------*/

void debug_cpu_go_vblank(void)
{
	debug_cpu_info *info = global.livecpu;

	if (!global.within_instruction_hook)
		return;
	assert(info != NULL);

	global.vblank_occurred = FALSE;
	info->flags |= DEBUG_FLAG_STOP_VBLANK;
	global.execution_state = EXECUTION_STATE_RUNNING;
}


/*-------------------------------------------------
    debug_cpu_go_interrupt - run until the
    specified interrupt fires
-------------------------------------------------*/

void debug_cpu_go_interrupt(int irqline)
{
	debug_cpu_info *info = global.livecpu;

	if (!global.within_instruction_hook)
		return;
	assert(info != NULL);

	info->stopirq = irqline;
	info->flags |= DEBUG_FLAG_STOP_INTERRUPT;
	global.execution_state = EXECUTION_STATE_RUNNING;
}


/*-------------------------------------------------
    debug_cpu_go_exception - run until the
    specified exception fires
-------------------------------------------------*/

void debug_cpu_go_exception(int exception)
{
	debug_cpu_info *info = global.livecpu;

	if (!global.within_instruction_hook)
		return;
	assert(info != NULL);

	info->stopexception = exception;
	info->flags |= DEBUG_FLAG_STOP_EXCEPTION;
	global.execution_state = EXECUTION_STATE_RUNNING;
}


/*-------------------------------------------------
    debug_cpu_go_milliseconds - run until the
    specified delay elapses
-------------------------------------------------*/

void debug_cpu_go_milliseconds(UINT64 milliseconds)
{
	debug_cpu_info *info = global.livecpu;

	if (!global.within_instruction_hook)
		return;
	assert(info != NULL);

	info->stoptime = attotime_add(timer_get_time(), ATTOTIME_IN_MSEC(milliseconds));
	info->flags |= DEBUG_FLAG_STOP_TIME;
	global.execution_state = EXECUTION_STATE_RUNNING;
}


/*-------------------------------------------------
    debug_cpu_next_cpu - execute until we hit
    the next CPU
-------------------------------------------------*/

void debug_cpu_next_cpu(void)
{
	debug_cpu_info *info = global.livecpu;

	if (!global.within_instruction_hook)
		return;
	assert(info != NULL);

	info->flags |= DEBUG_FLAG_STOP_CONTEXT;
	global.execution_state = EXECUTION_STATE_RUNNING;
}


/*-------------------------------------------------
    debug_cpu_ignore_cpu - ignore/observe a given
    CPU
-------------------------------------------------*/

void debug_cpu_ignore_cpu(int cpunum, int ignore)
{
	debug_cpu_info *info = &global.cpuinfo[cpunum];

	if (!global.within_instruction_hook)
		return;

	if (ignore)
		info->flags &= ~DEBUG_FLAG_OBSERVING;
	else
		info->flags |= DEBUG_FLAG_OBSERVING;

	if (info == global.livecpu && ignore)
		debug_cpu_next_cpu();
}


/*-------------------------------------------------
    debug_cpu_trace - trace execution of a given
    CPU
-------------------------------------------------*/

void debug_cpu_trace(int cpunum, FILE *file, int trace_over, const char *action)
{
	debug_cpu_info *info = &global.cpuinfo[cpunum];

	/* close existing files and delete expressions */
	if (info->trace.file != NULL)
		fclose(info->trace.file);
	info->trace.file = NULL;

	if (info->trace.action != NULL)
		free(info->trace.action);
	info->trace.action = NULL;

	/* open any new files */
	info->trace.file = file;
	info->trace.action = NULL;
	info->trace.trace_over_target = ~0;
	if (action != NULL)
	{
		info->trace.action = malloc_or_die(strlen(action) + 1);
		strcpy(info->trace.action, action);
	}

	/* update flags */
	if (info->trace.file != NULL)
		info->flags |= trace_over ? DEBUG_FLAG_TRACING_OVER : DEBUG_FLAG_TRACING;
	else
		info->flags &= ~DEBUG_FLAG_TRACING_ANY;
}



/***************************************************************************
    UTILITIES
***************************************************************************/

/*-------------------------------------------------
    debug_get_cpu_info - returns the cpu info
    block for a given CPU
-------------------------------------------------*/

const debug_cpu_info *debug_get_cpu_info(int cpunum)
{
	return &global.cpuinfo[cpunum];
}


/*-------------------------------------------------
    debug_cpu_halt_on_next_instruction - halt in
    the debugger on the next instruction
-------------------------------------------------*/

void debug_cpu_halt_on_next_instruction(running_machine *machine, const char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	debug_console_vprintf(fmt, arg);
	va_end(arg);

	global.execution_state = EXECUTION_STATE_STOPPED;
	if (global.livecpu != NULL)
		compute_debug_flags(machine, global.livecpu);
}


/*-------------------------------------------------
    debug_cpu_is_stopped - return the
    current execution state
-------------------------------------------------*/

int debug_cpu_is_stopped(running_machine *machine)
{
	return global.execution_state == EXECUTION_STATE_STOPPED;
}


/*-------------------------------------------------
    perform_trace - log to the tracefile the
    data for a given instruction
-------------------------------------------------*/

static UINT32 dasm_wrapped(char *buffer, offs_t pc)
{
	const debug_cpu_info *cpuinfo = debug_get_cpu_info(cpu_getactivecpu());
	int maxbytes = activecpu_max_instruction_bytes();
	UINT8 opbuf[64], argbuf[64];
	offs_t pcbyte;
	int numbytes;

	/* fetch the bytes up to the maximum */
	pcbyte = ADDR2BYTE_MASKED(pc, cpuinfo, ADDRESS_SPACE_PROGRAM);
	for (numbytes = 0; numbytes < maxbytes; numbytes++)
	{
		opbuf[numbytes] = debug_read_opcode(pcbyte + numbytes, 1, FALSE);
		argbuf[numbytes] = debug_read_opcode(pcbyte + numbytes, 1, TRUE);
	}

	return activecpu_dasm(buffer, pc, opbuf, argbuf);
}


static void perform_trace(debug_cpu_info *info)
{
	offs_t pc = activecpu_get_pc();
	int offset, count, i;
	char buffer[100];
	offs_t dasmresult;

	/* are we in trace over mode and in a subroutine? */
	if ((info->flags & DEBUG_FLAG_TRACING_OVER) != 0 && info->trace.trace_over_target != ~0)
	{
		if (info->trace.trace_over_target != pc)
			return;
		info->trace.trace_over_target = ~0;
	}

	/* check for a loop condition */
	for (i = count = 0; i < TRACE_LOOPS; i++)
		if (info->trace.history[i] == pc)
			count++;

	/* if no more than 1 hit, process normally */
	if (count <= 1)
	{
		/* if we just finished looping, indicate as much */
		if (info->trace.loops != 0)
			fprintf(info->trace.file, "\n   (loops for %d instructions)\n\n", info->trace.loops);
		info->trace.loops = 0;

		/* execute any trace actions first */
		if (info->trace.action != NULL)
			debug_console_execute_command(info->trace.action, 0);

		/* print the address */
		offset = sprintf(buffer, "%0*X: ", info->space[ADDRESS_SPACE_PROGRAM].logchars, pc);

		/* print the disassembly */
		dasmresult = dasm_wrapped(&buffer[offset], pc);

		/* output the result */
		fprintf(info->trace.file, "%s\n", buffer);

		/* do we need to step the trace over this instruction? */
		if ((info->flags & DEBUG_FLAG_TRACING_OVER) != 0 && (dasmresult & DASMFLAG_SUPPORTED) != 0 && (dasmresult & DASMFLAG_STEP_OVER) != 0)
		{
			int extraskip = (dasmresult & DASMFLAG_OVERINSTMASK) >> DASMFLAG_OVERINSTSHIFT;
			offs_t trace_over_target = pc + (dasmresult & DASMFLAG_LENGTHMASK);

			/* if we need to skip additional instructions, advance as requested */
			while (extraskip-- > 0)
				trace_over_target += dasm_wrapped(buffer, trace_over_target) & DASMFLAG_LENGTHMASK;

			info->trace.trace_over_target = trace_over_target;
		}

		/* log this PC */
		info->trace.nextdex = (info->trace.nextdex + 1) % TRACE_LOOPS;
		info->trace.history[info->trace.nextdex] = pc;
	}

	/* else just count the loop */
	else
		info->trace.loops++;
}


/*-------------------------------------------------
    prepare_for_step_overout - prepare things for
    stepping over an instruction
-------------------------------------------------*/

static void prepare_for_step_overout(debug_cpu_info *info)
{
	offs_t pc = activecpu_get_pc();
	char dasmbuffer[100];
	offs_t dasmresult;

	/* disassemble the current instruction and get the flags */
	dasmresult = dasm_wrapped(dasmbuffer, pc);

	/* if flags are supported and it's a call-style opcode, set a temp breakpoint after that instruction */
	if ((dasmresult & DASMFLAG_SUPPORTED) != 0 && (dasmresult & DASMFLAG_STEP_OVER) != 0)
	{
		int extraskip = (dasmresult & DASMFLAG_OVERINSTMASK) >> DASMFLAG_OVERINSTSHIFT;
		pc += dasmresult & DASMFLAG_LENGTHMASK;

		/* if we need to skip additional instructions, advance as requested */
		while (extraskip-- > 0)
			pc += dasm_wrapped(dasmbuffer, pc) & DASMFLAG_LENGTHMASK;
		info->stepaddr = pc;
	}

	/* if we're stepping out and this isn't a step out instruction, reset the steps until stop to a high number */
	if ((info->flags & DEBUG_FLAG_STEPPING_OUT) != 0)
	{
		if ((dasmresult & DASMFLAG_SUPPORTED) != 0 && (dasmresult & DASMFLAG_STEP_OUT) == 0)
			info->stepsleft = 100;
		else
			info->stepsleft = 1;
	}
}


/*-------------------------------------------------
    process_source_file - executes commands from
    a source file
-------------------------------------------------*/

static void process_source_file(void)
{
	/* loop until the file is exhausted or until we are executing again */
	while (debug_source_file != NULL && global.execution_state == EXECUTION_STATE_STOPPED)
	{
		char buf[512];
		int i;
		char *s;

		/* stop at the end of file */
		if (feof(debug_source_file))
		{
			fclose(debug_source_file);
			debug_source_file = NULL;
			return;
		}

		/* fetch the next line */
		memset(buf, 0, sizeof(buf));
		fgets(buf, sizeof(buf), debug_source_file);

		/* strip out comments (text after '//') */
		s = strstr(buf, "//");
		if (s)
			*s = '\0';

		/* strip whitespace */
		i = (int)strlen(buf);
		while((i > 0) && (isspace(buf[i-1])))
			buf[--i] = '\0';

		/* execute the command */
		if (buf[0])
			debug_console_execute_command(buf, 1);
	}
}


/*-------------------------------------------------
    debug_cpu_set_instruction_hook - set a hook to
    be called on each instruction for a given CPU
-------------------------------------------------*/

void debug_cpu_set_instruction_hook(int cpunum, int (*hook)(offs_t pc))
{
	debug_cpu_info *info = &global.cpuinfo[cpunum];

	/* set the hook and also the CPU's flag for fast knowledge of the hook */
	info->instrhook = hook;
	if (hook != NULL)
		info->flags |= DEBUG_FLAG_HOOKED;
	else
		info->flags &= ~DEBUG_FLAG_HOOKED;
}



/***************************************************************************
    BREAKPOINTS
***************************************************************************/

/*-------------------------------------------------
    breakpoint_update_flags - update the CPU's
    breakpoint flags
-------------------------------------------------*/

static void breakpoint_update_flags(running_machine *machine, debug_cpu_info *info)
{
	debug_cpu_breakpoint *bp;

	/* see if there are any enabled breakpoints */
	info->flags &= ~DEBUG_FLAG_LIVE_BP;
	for (bp = info->bplist; bp != NULL; bp = bp->next)
		if (bp->enabled)
		{
			info->flags |= DEBUG_FLAG_LIVE_BP;
			break;
		}

	/* push the flags out globally */
	if (global.livecpu != NULL)
		compute_debug_flags(machine, global.livecpu);
}


/*-------------------------------------------------
    breakpoint_check - check the breakpoints for
    a given CPU
-------------------------------------------------*/

static void breakpoint_check(debug_cpu_info *info, offs_t pc)
{
	debug_cpu_breakpoint *bp;
	UINT64 result;

	/* see if we match */
	for (bp = info->bplist; bp != NULL; bp = bp->next)
		if (bp->enabled && bp->address == pc)

			/* if we do, evaluate the condition */
			if (bp->condition == NULL || (expression_execute(bp->condition, &result) == EXPRERR_NONE && result != 0))
			{
				/* halt in the debugger by default */
				global.execution_state = EXECUTION_STATE_STOPPED;

				/* if we hit, evaluate the action */
				if (bp->action != NULL)
					debug_console_execute_command(bp->action, 0);

				/* print a notification, unless the action made us go again */
				if (global.execution_state == EXECUTION_STATE_STOPPED)
					debug_console_printf("Stopped at breakpoint %X\n", bp->index);
				break;
			}
}


/*-------------------------------------------------
    debug_cpu_breakpoint_set - set a new breakpoint
-------------------------------------------------*/

int debug_cpu_breakpoint_set(running_machine *machine, int cpunum, offs_t address, parsed_expression *condition, const char *action)
{
	debug_cpu_info *info = &global.cpuinfo[cpunum];
	debug_cpu_breakpoint *bp;

	assert_always(cpunum >= 0 && cpunum < cpu_gettotalcpu(), "debug_cpu_breakpoint_set() called with invalid cpunum!");

	/* allocate breakpoint */
	bp = malloc_or_die(sizeof(*bp));
	bp->index = global.bpindex++;
	bp->enabled = TRUE;
	bp->address = address;
	bp->condition = condition;
	bp->action = NULL;
	if (action != NULL)
	{
		bp->action = malloc_or_die(strlen(action) + 1);
		strcpy(bp->action, action);
	}

	/* hook us in */
	bp->next = info->bplist;
	info->bplist = bp;

	/* ensure the live breakpoint flag is set */
	breakpoint_update_flags(machine, info);
	return bp->index;
}


/*-------------------------------------------------
    debug_cpu_breakpoint_clear - clear a breakpoint
-------------------------------------------------*/

int debug_cpu_breakpoint_clear(running_machine *machine, int bpnum)
{
	debug_cpu_breakpoint *bp, *pbp;
	int cpunum;

	/* loop over CPUs and find the requested breakpoint */
	for (cpunum = 0; cpunum < MAX_CPU; cpunum++)
	{
		debug_cpu_info *info = &global.cpuinfo[cpunum];
		for (pbp = NULL, bp = info->bplist; bp != NULL; pbp = bp, bp = bp->next)
			if (bp->index == bpnum)
			{
				/* unlink us from the list */
				if (pbp == NULL)
					info->bplist = bp->next;
				else
					pbp->next = bp->next;

				/* free the memory */
				if (bp->condition != NULL)
					expression_free(bp->condition);
				if (bp->action != NULL)
					free(bp->action);
				free(bp);

				/* update the flags */
				breakpoint_update_flags(machine, info);
				return 1;
			}
	}

	/* we didn't find it; return an error */
	return 0;
}


/*-------------------------------------------------
    debug_cpu_breakpoint_enable - enable/disable a
    breakpoint
-------------------------------------------------*/

int debug_cpu_breakpoint_enable(running_machine *machine, int bpnum, int enable)
{
	debug_cpu_breakpoint *bp;
	int cpunum;

	/* loop over CPUs and find the requested breakpoint */
	for (cpunum = 0; cpunum < MAX_CPU; cpunum++)
	{
		debug_cpu_info *info = &global.cpuinfo[cpunum];
		for (bp = info->bplist; bp != NULL; bp = bp->next)
			if (bp->index == bpnum)
			{
				bp->enabled = (enable != 0);
				breakpoint_update_flags(machine, info);
				return 1;
			}
	}

	return 0;
}



/***************************************************************************
    WATCHPOINTS
***************************************************************************/

/*-------------------------------------------------
    watchpoint_update_flags - update the CPU's
    watchpoint flags
-------------------------------------------------*/

static void watchpoint_update_flags(running_machine *machine, debug_cpu_info *info, int spacenum)
{
	UINT32 writeflag = DEBUG_FLAG_LIVE_WPW_PROGRAM << spacenum;
	UINT32 readflag = DEBUG_FLAG_LIVE_WPR_PROGRAM << spacenum;
	debug_cpu_watchpoint *wp;

	/* if hotspots are enabled, turn on all reads */
	if (info->hotspots != NULL)
	{
		info->flags |= DEBUG_FLAG_READ_WATCHPOINT;
		readflag = 0;
	}

	/* see if there are any enabled breakpoints */
	info->flags &= ~(readflag | writeflag);
	for (wp = info->space[spacenum].wplist; wp != NULL; wp = wp->next)
		if (wp->enabled)
		{
			if (wp->type & WATCHPOINT_READ)
			{
				info->flags |= readflag;
				readflag = 0;
			}
			if (wp->type & WATCHPOINT_WRITE)
			{
				info->flags |= writeflag;
				writeflag = 0;
			}
			if ((readflag | writeflag) == 0)
				break;
		}

	/* push the flags out globally */
	if (global.livecpu != NULL)
		compute_debug_flags(machine, global.livecpu);
}


/*-------------------------------------------------
    watchpoint_check - check the watchpoints
    for a given CPU and address space
-------------------------------------------------*/

static void watchpoint_check(running_machine *machine, int cpunum, int spacenum, int type, offs_t address, UINT64 value_to_write, UINT64 mem_mask)
{
	const debug_cpu_info *info = &global.cpuinfo[cpunum];
	debug_cpu_watchpoint *wp;
	offs_t size = 0;
	UINT64 result;

	/* if we're within debugger code, don't stop */
	if (global.within_instruction_hook || global.debugger_access)
		return;

	global.within_instruction_hook = TRUE;

	/* adjust address, size & value_to_write based on mem_mask. */
	if (mem_mask != 0)
	{
		int bus_size = info->space[spacenum].databytes;
		int address_offset = 0;

		while (address_offset < bus_size && (mem_mask & 0xff) == 0)
		{
			address_offset++;
			value_to_write >>= 8;
			mem_mask >>= 8;
		}

		while (mem_mask != 0)
		{
			size++;
			mem_mask >>= 8;
		}

		if (info->endianness == CPU_IS_LE)
			address += address_offset;
		else
			address += bus_size - size - address_offset;
	}

	/* if we are a write watchpoint, stash the value that will be written */
	global.wpaddr = address;
	if (type & WATCHPOINT_WRITE)
		global.wpdata = value_to_write;

	/* see if we match */
	for (wp = info->space[spacenum].wplist; wp != NULL; wp = wp->next)
		if (wp->enabled && (wp->type & type) != 0 && address + size > wp->address && address < wp->address + wp->length)

			/* if we do, evaluate the condition */
			if (wp->condition == NULL || (expression_execute(wp->condition, &result) == EXPRERR_NONE && result != 0))
			{
				/* halt in the debugger by default */
				global.execution_state = EXECUTION_STATE_STOPPED;

				/* if we hit, evaluate the action */
				if (wp->action != NULL)
					debug_console_execute_command(wp->action, 0);

				/* print a notification, unless the action made us go again */
				if (global.execution_state == EXECUTION_STATE_STOPPED)
				{
					static const char *const sizes[] =
					{
						"0bytes", "byte", "word", "3bytes", "dword", "5bytes", "6bytes", "7bytes", "qword"
					};
					char buffer[100];

					if (type & WATCHPOINT_WRITE)
					{
						sprintf(buffer, "Stopped at watchpoint %X writing %s to %08X (PC=%X)", wp->index, sizes[size], BYTE2ADDR(address, &global.cpuinfo[cpunum], spacenum), activecpu_get_pc());
						if (value_to_write >> 32)
							sprintf(&buffer[strlen(buffer)], " (data=%X%08X)", (UINT32)(value_to_write >> 32), (UINT32)value_to_write);
						else
							sprintf(&buffer[strlen(buffer)], " (data=%X)", (UINT32)value_to_write);
					}
					else
						sprintf(buffer, "Stopped at watchpoint %X reading %s from %08X (PC=%X)", wp->index, sizes[size], BYTE2ADDR(address, &global.cpuinfo[cpunum], spacenum), activecpu_get_pc());
					debug_console_printf("%s\n", buffer);
					compute_debug_flags(machine, info);
				}
				break;
			}

	global.within_instruction_hook = FALSE;
}


/*-------------------------------------------------
    debug_cpu_watchpoint_set - set a new watchpoint
-------------------------------------------------*/

int debug_cpu_watchpoint_set(running_machine *machine, int cpunum, int spacenum, int type, offs_t address, offs_t length, parsed_expression *condition, const char *action)
{
	debug_cpu_info *info = &global.cpuinfo[cpunum];
	debug_cpu_watchpoint *wp = malloc_or_die(sizeof(*wp));

	/* fill in the structure */
	wp->index = global.wpindex++;
	wp->enabled = TRUE;
	wp->type = type;
	wp->address = ADDR2BYTE_MASKED(address, &global.cpuinfo[cpunum], spacenum);
	wp->length = ADDR2BYTE(length, &global.cpuinfo[cpunum], spacenum);
	wp->condition = condition;
	wp->action = NULL;
	if (action != NULL)
	{
		wp->action = malloc_or_die(strlen(action) + 1);
		strcpy(wp->action, action);
	}

	/* hook us in */
	wp->next = info->space[spacenum].wplist;
	info->space[spacenum].wplist = wp;

	watchpoint_update_flags(machine, info, spacenum);

	return wp->index;
}


/*-------------------------------------------------
    debug_cpu_watchpoint_clear - clear a watchpoint
-------------------------------------------------*/

int debug_cpu_watchpoint_clear(running_machine *machine, int wpnum)
{
	debug_cpu_watchpoint *wp, *pwp;
	int cpunum, spacenum;

	/* loop over CPUs and find the requested watchpoint */
	for (cpunum = 0; cpunum < MAX_CPU; cpunum++)
	{
		debug_cpu_info *info = &global.cpuinfo[cpunum];

		for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
			for (pwp = NULL, wp = info->space[spacenum].wplist; wp != NULL; pwp = wp, wp = wp->next)
				if (wp->index == wpnum)
				{
					/* unlink us from the list */
					if (pwp == NULL)
						info->space[spacenum].wplist = wp->next;
					else
						pwp->next = wp->next;

					/* free the memory */
					if (wp->condition != NULL)
						expression_free(wp->condition);
					if (wp->action != NULL)
						free(wp->action);
					free(wp);

					watchpoint_update_flags(machine, info, spacenum);
					return 1;
				}
	}

	/* we didn't find it; return an error */
	return 0;
}


/*-------------------------------------------------
    debug_cpu_watchpoint_enable - enable/disable a
    watchpoint
-------------------------------------------------*/

int debug_cpu_watchpoint_enable(running_machine *machine, int wpnum, int enable)
{
	debug_cpu_watchpoint *wp;
	int cpunum, spacenum;

	/* loop over CPUs and address spaces and find the requested watchpoint */
	for (cpunum = 0; cpunum < MAX_CPU; cpunum++)
	{
		debug_cpu_info *info = &global.cpuinfo[cpunum];

		for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
			for (wp = info->space[spacenum].wplist; wp; wp = wp->next)
				if (wp->index == wpnum)
				{
					wp->enabled = (enable != 0);
					watchpoint_update_flags(machine, info, spacenum);
					return 1;
				}
	}
	return 0;
}



/***************************************************************************
    HOTSPOTS
***************************************************************************/

/*-------------------------------------------------
    debug_cpu_hotspot_track - enable/disable tracking
    of hotspots
-------------------------------------------------*/

int debug_cpu_hotspot_track(running_machine *machine, int cpunum, int numspots, int threshhold)
{
	debug_cpu_info *info = &global.cpuinfo[cpunum];

	/* if we already have tracking info, kill it */
	if (info->hotspots)
		free(info->hotspots);
	info->hotspots = NULL;

	/* only start tracking if we have a non-zero count */
	if (numspots > 0)
	{
		/* allocate memory for hotspots */
		info->hotspots = malloc_or_die(sizeof(*info->hotspots) * numspots);
		memset(info->hotspots, 0xff, sizeof(*info->hotspots) * numspots);

		/* fill in the info */
		info->hotspot_count = numspots;
		info->hotspot_threshhold = threshhold;
	}

	watchpoint_update_flags(machine, info, ADDRESS_SPACE_PROGRAM);
	return 1;
}


/*-------------------------------------------------
    check_hotspots - check for
    hotspots on a memory read access
-------------------------------------------------*/

static void check_hotspots(int cpunum, int spacenum, offs_t address)
{
	debug_cpu_info *info = &global.cpuinfo[cpunum];
	offs_t pc = activecpu_get_pc();
	int hotindex;

	/* see if we have a match in our list */
	for (hotindex = 0; hotindex < info->hotspot_count; hotindex++)
		if (info->hotspots[hotindex].access == address && info->hotspots[hotindex].pc == pc && info->hotspots[hotindex].spacenum == spacenum)
			break;

	/* if we didn't find any, make a new entry */
	if (hotindex == info->hotspot_count)
	{
		/* if the bottom of the list is over the threshhold, print it */
		debug_hotspot_entry *spot = &info->hotspots[info->hotspot_count - 1];
		if (spot->count > info->hotspot_threshhold)
			debug_console_printf("Hotspot @ %s %08X (PC=%08X) hit %d times (fell off bottom)\n", address_space_names[spot->spacenum], spot->access, spot->pc, spot->count);

		/* move everything else down and insert this one at the top */
		memmove(&info->hotspots[1], &info->hotspots[0], sizeof(info->hotspots[0]) * (info->hotspot_count - 1));
		info->hotspots[0].access = address;
		info->hotspots[0].pc = pc;
		info->hotspots[0].spacenum = spacenum;
		info->hotspots[0].count = 1;
	}

	/* if we did find one, increase the count and move it to the top */
	else
	{
		info->hotspots[hotindex].count++;
		if (hotindex != 0)
		{
			debug_hotspot_entry temp = info->hotspots[hotindex];
			memmove(&info->hotspots[1], &info->hotspots[0], sizeof(info->hotspots[0]) * hotindex);
			info->hotspots[0] = temp;
		}
	}
}


/***************************************************************************
    MEMORY ACCESSORS
***************************************************************************/

/*-------------------------------------------------
    debug_read_byte - return a byte from the
    current cpu in the specified memory space
-------------------------------------------------*/

UINT8 debug_read_byte(int spacenum, offs_t address, int apply_translation)
{
	const debug_cpu_info *info = &global.cpuinfo[cpu_getactivecpu()];
	UINT64 custom;
	UINT8 result;

	/* mask against the logical byte mask */
	address &= info->space[spacenum].logbytemask;

	/* all accesses from this point on are for the debugger */
	memory_set_debugger_access(global.debugger_access = TRUE);

	/* translate if necessary; if not mapped, return 0xff */
	if (apply_translation && info->translate != NULL && !(*info->translate)(spacenum, TRANSLATE_READ_DEBUG, &address))
		result = 0xff;

	/* if there is a custom read handler, and it returns TRUE, use that value */
	else if (info->read && (*info->read)(spacenum, address, 1, &custom))
		result = custom;

	/* otherwise, call the byte reading function for the translated address */
	else
		result = (*active_address_space[spacenum].accessors->read_byte)(address);

	/* no longer accessing via the debugger */
	memory_set_debugger_access(global.debugger_access = FALSE);
	return result;
}


/*-------------------------------------------------
    debug_read_word - return a word from the
    current cpu in the specified memory space
-------------------------------------------------*/

UINT16 debug_read_word(int spacenum, offs_t address, int apply_translation)
{
	const debug_cpu_info *info = &global.cpuinfo[cpu_getactivecpu()];
	UINT64 custom;
	UINT16 result;

	/* mask against the logical byte mask */
	address &= info->space[spacenum].logbytemask;

	/* if this is misaligned read, or if there are no word readers, just read two bytes */
	if ((address & 1) || !active_address_space[spacenum].accessors->read_word)
	{
		UINT8 byte0 = debug_read_byte(spacenum, address + 0, apply_translation);
		UINT8 byte1 = debug_read_byte(spacenum, address + 1, apply_translation);

		/* based on the endianness, the result is assembled differently */
		if (global.cpuinfo[cpu_getactivecpu()].endianness == CPU_IS_LE)
			result = byte0 | (byte1 << 8);
		else
			result = byte1 | (byte0 << 8);
	}

	/* otherwise, this proceeds like the byte case */
	else
	{
		/* all accesses from this point on are for the debugger */
		memory_set_debugger_access(global.debugger_access = TRUE);

		/* translate if necessary; if not mapped, return 0xffff */
		if (apply_translation && info->translate != NULL && !(*info->translate)(spacenum, TRANSLATE_READ_DEBUG, &address))
			result = 0xffff;

		/* if there is a custom read handler, and it returns TRUE, use that value */
		else if (info->read && (*info->read)(spacenum, address, 2, &custom))
			result = custom;

		/* otherwise, call the byte reading function for the translated address */
		else
			result = (*active_address_space[spacenum].accessors->read_word)(address);

		/* no longer accessing via the debugger */
		memory_set_debugger_access(global.debugger_access = FALSE);
	}

	return result;
}


/*-------------------------------------------------
    debug_read_dword - return a dword from the
    current cpu in the specified memory space
-------------------------------------------------*/

UINT32 debug_read_dword(int spacenum, offs_t address, int apply_translation)
{
	const debug_cpu_info *info = &global.cpuinfo[cpu_getactivecpu()];
	UINT64 custom;
	UINT32 result;

	/* mask against the logical byte mask */
	address &= info->space[spacenum].logbytemask;

	/* if this is misaligned read, or if there are no dword readers, just read two words */
	if ((address & 3) || !active_address_space[spacenum].accessors->read_dword)
	{
		UINT16 word0 = debug_read_word(spacenum, address + 0, apply_translation);
		UINT16 word1 = debug_read_word(spacenum, address + 2, apply_translation);

		/* based on the endianness, the result is assembled differently */
		if (global.cpuinfo[cpu_getactivecpu()].endianness == CPU_IS_LE)
			result = word0 | (word1 << 16);
		else
			result = word1 | (word0 << 16);
	}

	/* otherwise, this proceeds like the byte case */
	else
	{
		/* all accesses from this point on are for the debugger */
		memory_set_debugger_access(global.debugger_access = TRUE);

		/* translate if necessary; if not mapped, return 0xffffffff */
		if (apply_translation && info->translate != NULL && !(*info->translate)(spacenum, TRANSLATE_READ_DEBUG, &address))
			result = 0xffffffff;

		/* if there is a custom read handler, and it returns TRUE, use that value */
		else if (info->read && (*info->read)(spacenum, address, 4, &custom))
			result = custom;

		/* otherwise, call the byte reading function for the translated address */
		else
			result = (*active_address_space[spacenum].accessors->read_dword)(address);

		/* no longer accessing via the debugger */
		memory_set_debugger_access(global.debugger_access = FALSE);
	}

	return result;
}


/*-------------------------------------------------
    debug_read_qword - return a qword from the
    current cpu in the specified memory space
-------------------------------------------------*/

UINT64 debug_read_qword(int spacenum, offs_t address, int apply_translation)
{
	const debug_cpu_info *info = &global.cpuinfo[cpu_getactivecpu()];
	UINT64 custom;
	UINT64 result;

	/* mask against the logical byte mask */
	address &= info->space[spacenum].logbytemask;

	/* if this is misaligned read, or if there are no qword readers, just read two dwords */
	if ((address & 7) || !active_address_space[spacenum].accessors->read_qword)
	{
		UINT32 dword0 = debug_read_dword(spacenum, address + 0, apply_translation);
		UINT32 dword1 = debug_read_dword(spacenum, address + 4, apply_translation);

		/* based on the endianness, the result is assembled differently */
		if (global.cpuinfo[cpu_getactivecpu()].endianness == CPU_IS_LE)
			result = dword0 | ((UINT64)dword1 << 32);
		else
			result = dword1 | ((UINT64)dword0 << 32);
	}

	/* otherwise, this proceeds like the byte case */
	else
	{
		/* all accesses from this point on are for the debugger */
		memory_set_debugger_access(global.debugger_access = TRUE);

		/* translate if necessary; if not mapped, return 0xffffffffffffffff */
		if (apply_translation && info->translate != NULL && !(*info->translate)(spacenum, TRANSLATE_READ_DEBUG, &address))
			result = ~(UINT64)0;

		/* if there is a custom read handler, and it returns TRUE, use that value */
		else if (info->read && (*info->read)(spacenum, address, 8, &custom))
			result = custom;

		/* otherwise, call the byte reading function for the translated address */
		else
			result = (*active_address_space[spacenum].accessors->read_qword)(address);

		/* no longer accessing via the debugger */
		memory_set_debugger_access(global.debugger_access = FALSE);
	}

	return result;
}


/*-------------------------------------------------
    debug_write_byte - write a byte to the
    current cpu in the specified memory space
-------------------------------------------------*/

void debug_write_byte(int spacenum, offs_t address, UINT8 data, int apply_translation)
{
	const debug_cpu_info *info = &global.cpuinfo[cpu_getactivecpu()];

	/* mask against the logical byte mask */
	address &= info->space[spacenum].logbytemask;

	/* all accesses from this point on are for the debugger */
	memory_set_debugger_access(global.debugger_access = TRUE);

	/* translate if necessary; if not mapped, we're done */
	if (apply_translation && info->translate != NULL && !(*info->translate)(spacenum, TRANSLATE_WRITE_DEBUG, &address))
		;

	/* if there is a custom write handler, and it returns TRUE, use that */
	else if (info->write && (*info->write)(spacenum, address, 1, data))
		;

	/* otherwise, call the byte reading function for the translated address */
	else
		(*active_address_space[spacenum].accessors->write_byte)(address, data);

	/* no longer accessing via the debugger */
	memory_set_debugger_access(global.debugger_access = FALSE);
	global.memory_modified = TRUE;
}


/*-------------------------------------------------
    debug_write_word - write a word to the
    current cpu in the specified memory space
-------------------------------------------------*/

void debug_write_word(int spacenum, offs_t address, UINT16 data, int apply_translation)
{
	const debug_cpu_info *info = &global.cpuinfo[cpu_getactivecpu()];

	/* mask against the logical byte mask */
	address &= info->space[spacenum].logbytemask;

	/* if this is a misaligned write, or if there are no word writers, just read two bytes */
	if ((address & 1) || !active_address_space[spacenum].accessors->write_word)
	{
		if (global.cpuinfo[cpu_getactivecpu()].endianness == CPU_IS_LE)
		{
			debug_write_byte(spacenum, address + 0, data >> 0, apply_translation);
			debug_write_byte(spacenum, address + 1, data >> 8, apply_translation);
		}
		else
		{
			debug_write_byte(spacenum, address + 0, data >> 8, apply_translation);
			debug_write_byte(spacenum, address + 1, data >> 0, apply_translation);
		}
	}

	/* otherwise, this proceeds like the byte case */
	else
	{
		/* all accesses from this point on are for the debugger */
		memory_set_debugger_access(global.debugger_access = TRUE);

		/* translate if necessary; if not mapped, we're done */
		if (apply_translation && info->translate && !(*info->translate)(spacenum, TRANSLATE_WRITE_DEBUG, &address))
			;

		/* if there is a custom write handler, and it returns TRUE, use that */
		else if (info->write && (*info->write)(spacenum, address, 2, data))
			;

		/* otherwise, call the byte reading function for the translated address */
		else
			(*active_address_space[spacenum].accessors->write_word)(address, data);

		/* no longer accessing via the debugger */
		memory_set_debugger_access(global.debugger_access = FALSE);
		global.memory_modified = TRUE;
	}
}


/*-------------------------------------------------
    debug_write_dword - write a dword to the
    current cpu in the specified memory space
-------------------------------------------------*/

void debug_write_dword(int spacenum, offs_t address, UINT32 data, int apply_translation)
{
	const debug_cpu_info *info = &global.cpuinfo[cpu_getactivecpu()];

	/* mask against the logical byte mask */
	address &= info->space[spacenum].logbytemask;

	/* if this is a misaligned write, or if there are no dword writers, just read two words */
	if ((address & 3) || !active_address_space[spacenum].accessors->write_dword)
	{
		if (global.cpuinfo[cpu_getactivecpu()].endianness == CPU_IS_LE)
		{
			debug_write_word(spacenum, address + 0, data >> 0, apply_translation);
			debug_write_word(spacenum, address + 2, data >> 16, apply_translation);
		}
		else
		{
			debug_write_word(spacenum, address + 0, data >> 16, apply_translation);
			debug_write_word(spacenum, address + 2, data >> 0, apply_translation);
		}
	}

	/* otherwise, this proceeds like the byte case */
	else
	{
		/* all accesses from this point on are for the debugger */
		memory_set_debugger_access(global.debugger_access = TRUE);

		/* translate if necessary; if not mapped, we're done */
		if (apply_translation && info->translate && !(*info->translate)(spacenum, TRANSLATE_WRITE_DEBUG, &address))
			;

		/* if there is a custom write handler, and it returns TRUE, use that */
		else if (info->write && (*info->write)(spacenum, address, 4, data))
			;

		/* otherwise, call the byte reading function for the translated address */
		else
			(*active_address_space[spacenum].accessors->write_dword)(address, data);

		/* no longer accessing via the debugger */
		memory_set_debugger_access(global.debugger_access = FALSE);
		global.memory_modified = TRUE;
	}
}


/*-------------------------------------------------
    debug_write_qword - write a qword to the
    current cpu in the specified memory space
-------------------------------------------------*/

void debug_write_qword(int spacenum, offs_t address, UINT64 data, int apply_translation)
{
	const debug_cpu_info *info = &global.cpuinfo[cpu_getactivecpu()];

	/* mask against the logical byte mask */
	address &= info->space[spacenum].logbytemask;

	/* if this is a misaligned write, or if there are no qword writers, just read two dwords */
	if ((address & 7) || !active_address_space[spacenum].accessors->write_qword)
	{
		if (global.cpuinfo[cpu_getactivecpu()].endianness == CPU_IS_LE)
		{
			debug_write_dword(spacenum, address + 0, data >> 0, apply_translation);
			debug_write_dword(spacenum, address + 4, data >> 32, apply_translation);
		}
		else
		{
			debug_write_dword(spacenum, address + 0, data >> 32, apply_translation);
			debug_write_dword(spacenum, address + 4, data >> 0, apply_translation);
		}
	}
	/* otherwise, this proceeds like the byte case */
	else
	{
		/* all accesses from this point on are for the debugger */
		memory_set_debugger_access(global.debugger_access = TRUE);

		/* translate if necessary; if not mapped, we're done */
		if (apply_translation && info->translate && !(*info->translate)(spacenum, TRANSLATE_WRITE_DEBUG, &address))
			;

		/* if there is a custom write handler, and it returns TRUE, use that */
		else if (info->write && (*info->write)(spacenum, address, 8, data))
			;

		/* otherwise, call the byte reading function for the translated address */
		else
			(*active_address_space[spacenum].accessors->write_qword)(address, data);

		/* no longer accessing via the debugger */
		memory_set_debugger_access(global.debugger_access = FALSE);
		global.memory_modified = TRUE;
	}
}


/*-------------------------------------------------
    debug_read_opcode - read 1,2,4 or 8 bytes at
    the given offset from opcode space
-------------------------------------------------*/

UINT64 debug_read_opcode(offs_t address, int size, int arg)
{
	const debug_cpu_info *info = &global.cpuinfo[cpu_getactivecpu()];
	offs_t lowbits_mask;
	const void *ptr;

	/* keep in logical range */
	address &= info->space[ADDRESS_SPACE_PROGRAM].logbytemask;

	/* shortcut if we have a custom routine */
	if (info->readop)
	{
		UINT64 result;
		if ((*info->readop)(address, size, &result))
			return result;
	}

	/* if we're bigger than the address bus, break into smaller pieces */
	if (size > info->space[ADDRESS_SPACE_PROGRAM].databytes)
	{
		int halfsize = size / 2;
		UINT64 r0 = debug_read_opcode(address + 0, halfsize, arg);
		UINT64 r1 = debug_read_opcode(address + halfsize, halfsize, arg);

		if (info->endianness == CPU_IS_LE)
			return r0 | (r1 << (8 * halfsize));
		else
			return r1 | (r0 << (8 * halfsize));
	}

	/* translate to physical first */
	if (info->translate && !(*info->translate)(ADDRESS_SPACE_PROGRAM, TRANSLATE_FETCH_DEBUG, &address))
		return ~(UINT64)0 & (~(UINT64)0 >> (64 - 8*size));

	/* keep in physical range */
	address &= info->space[ADDRESS_SPACE_PROGRAM].physbytemask;

	/* adjust the address */
	memory_set_opbase(address);

	switch (info->space[ADDRESS_SPACE_PROGRAM].databytes * 10 + size)
	{
		/* dump opcodes in bytes from a byte-sized bus */
		case 11:
			break;

		/* dump opcodes in bytes from a word-sized bus */
		case 21:
			address ^= (info->endianness == CPU_IS_LE) ? BYTE_XOR_LE(0) : BYTE_XOR_BE(0);
			break;

		/* dump opcodes in words from a word-sized bus */
		case 22:
			break;

		/* dump opcodes in bytes from a dword-sized bus */
		case 41:
			address ^= (info->endianness == CPU_IS_LE) ? BYTE4_XOR_LE(0) : BYTE4_XOR_BE(0);
			break;

		/* dump opcodes in words from a dword-sized bus */
		case 42:
			address ^= (info->endianness == CPU_IS_LE) ? WORD_XOR_LE(0) : WORD_XOR_BE(0);
			break;

		/* dump opcodes in dwords from a dword-sized bus */
		case 44:
			break;

		/* dump opcodes in bytes from a qword-sized bus */
		case 81:
			address ^= (info->endianness == CPU_IS_LE) ? BYTE8_XOR_LE(0) : BYTE8_XOR_BE(0);
			break;

		/* dump opcodes in words from a qword-sized bus */
		case 82:
			address ^= (info->endianness == CPU_IS_LE) ? WORD2_XOR_LE(0) : WORD2_XOR_BE(0);
			break;

		/* dump opcodes in dwords from a qword-sized bus */
		case 84:
			address ^= (info->endianness == CPU_IS_LE) ? DWORD_XOR_LE(0) : DWORD_XOR_BE(0);
			break;

		/* dump opcodes in qwords from a qword-sized bus */
		case 88:
			break;

		default:
			fatalerror("debug_read_opcode: unknown type = %d", info->space[ADDRESS_SPACE_PROGRAM].databytes * 10 + size);
			break;
	}

	/* get pointer to data */
	/* note that we query aligned to the bus width, and then add back the low bits */
	lowbits_mask = info->space[ADDRESS_SPACE_PROGRAM].databytes - 1;
	ptr = memory_get_op_ptr(cpu_getactivecpu(), address & ~lowbits_mask, arg);
	if (!ptr)
		return ~(UINT64)0 & (~(UINT64)0 >> (64 - 8*size));
	ptr = (UINT8 *)ptr + (address & lowbits_mask);

	/* gross! */
//  if (osd_is_bad_read_ptr(ptr, size))
//      fatalerror("debug_read_opcode: cpu %d address %x mapped to invalid memory %p", cpu_getactivecpu(), address, ptr);

	/* return based on the size */
	switch (size)
	{
		case 1:	return *(UINT8 *) ptr;
		case 2:	return *(UINT16 *)ptr;
		case 4:	return *(UINT32 *)ptr;
		case 8:	return *(UINT64 *)ptr;
	}

	return 0;	/* appease compiler */
}


/*-------------------------------------------------
    expression_cpu_index - return the CPU index
    based on a case insensitive tag search
-------------------------------------------------*/

static int expression_cpu_index(running_machine *machine, const char *tag)
{
	int index;

	for (index = 0; index < ARRAY_LENGTH(machine->config->cpu); index++)
		if (machine->config->cpu[index].tag != NULL && mame_stricmp(machine->config->cpu[index].tag, tag) == 0)
			return index;

	return -1;
}


/*-------------------------------------------------
    expression_read_memory - read 1,2,4 or 8 bytes
    at the given offset in the given address
    space
-------------------------------------------------*/

static UINT64 expression_read_memory(const char *name, int space, UINT32 address, int size)
{
	int cpuindex;

	switch (space)
	{
		case EXPSPACE_PROGRAM:
		case EXPSPACE_DATA:
		case EXPSPACE_IO:
			cpuindex = (name != NULL) ? expression_cpu_index(Machine, name) : cpu_getactivecpu();
			if (cpuindex < 0)
				break;
			return expression_read_address_space(cpuindex, ADDRESS_SPACE_PROGRAM + (space - EXPSPACE_PROGRAM), address, size);

		case EXPSPACE_OPCODE:
		case EXPSPACE_RAMWRITE:
			cpuindex = (name != NULL) ? expression_cpu_index(Machine, name) : cpu_getactivecpu();
			if (cpuindex < 0)
				break;
			if (name == NULL)
				name = Machine->config->cpu[cpu_getactivecpu()].tag;
			return expression_read_program_direct(cpuindex, (space == EXPSPACE_OPCODE), address, size);

		case EXPSPACE_EEPROM:
			return expression_read_eeprom(address, size);

		case EXPSPACE_REGION:
			if (name == NULL)
				break;
			return expression_read_memory_region(name, address, size);
	}
	return ~(UINT64)0 >> (64 - 8*size);
}


/*-------------------------------------------------
    expression_read_address_space - read memory
    from a specific CPU's address space
-------------------------------------------------*/

static UINT64 expression_read_address_space(int cpuindex, int space, offs_t address, int size)
{
	const debug_cpu_info *info = &global.cpuinfo[cpuindex];
	UINT64 result = ~(UINT64)0 >> (64 - 8*size);

	/* only process if in of range and we have a bus */
	if (cpuindex < ARRAY_LENGTH(global.cpuinfo) && info->space[space].databytes != 0)
	{
		/* adjust the address into a byte address */
		address = ADDR2BYTE(address, info, space);

		/* switch contexts and do the read */
		cpuintrf_push_context(cpuindex);
		switch (size)
		{
			case 1:		result = debug_read_byte(space, address, TRUE);		break;
			case 2:		result = debug_read_word(space, address, TRUE);		break;
			case 4:		result = debug_read_dword(space, address, TRUE);	break;
			case 8:		result = debug_read_qword(space, address, TRUE);	break;
		}
		cpuintrf_pop_context();
	}
	return result;
}


/*-------------------------------------------------
    expression_read_program_direct - read memory
    directly from an opcode or RAM pointer
-------------------------------------------------*/

static UINT64 expression_read_program_direct(int cpuindex, int opcode, offs_t address, int size)
{
	const debug_cpu_info *info = &global.cpuinfo[cpuindex];
	UINT64 result = ~(UINT64)0 >> (64 - 8*size);
	UINT8 *base;

	/* only process if in of range and we have a bus */
	if (cpuindex < ARRAY_LENGTH(global.cpuinfo) && info->space[ADDRESS_SPACE_PROGRAM].databytes != 0)
	{
		/* adjust the address into a byte address, but not if being called recursively */
		if ((opcode & 2) == 0)
			address = ADDR2BYTE(address, info, ADDRESS_SPACE_PROGRAM);

		/* call ourself recursively until we are byte-sized */
		if (size > 1)
		{
			int halfsize = size / 2;
			UINT64 r0, r1;

			/* read each half, from lower address to upper address */
			r0 = expression_read_program_direct(cpuindex, opcode | 2, address + 0, halfsize);
			r1 = expression_read_program_direct(cpuindex, opcode | 2, address + halfsize, halfsize);

			/* assemble based on the target endianness */
			if (info->endianness == CPU_IS_LE)
				result = r0 | (r1 << (8 * halfsize));
			else
				result = r1 | (r0 << (8 * halfsize));
		}

		/* handle the byte-sized final requests */
		else
		{
			/* lowmask specified which address bits are within the databus width */
			offs_t lowmask = info->space[ADDRESS_SPACE_PROGRAM].databytes - 1;

			/* get the base of memory, aligned to the address minus the lowbits */
			if (opcode & 1)
				base = memory_get_op_ptr(cpuindex, address & ~lowmask, FALSE);
			else
				base = memory_get_read_ptr(cpuindex, ADDRESS_SPACE_PROGRAM, address & ~lowmask);

			/* if we have a valid base, return the appropriate byte */
			if (base != NULL)
			{
				if (info->endianness == CPU_IS_LE)
					result = base[BYTE8_XOR_LE(address) & lowmask];
				else
					result = base[BYTE8_XOR_BE(address) & lowmask];
			}
		}
	}
	return result;
}


/*-------------------------------------------------
    expression_read_memory_region - read memory
    from a memory region
-------------------------------------------------*/

static UINT64 expression_read_memory_region(const char *rgntag, offs_t address, int size)
{
	UINT8 *base = memory_region(Machine, rgntag);
	UINT64 result = ~(UINT64)0 >> (64 - 8*size);

	/* make sure we get a valid base before proceeding */
	if (base != NULL)
	{
		UINT32 length = memory_region_length(Machine, rgntag);
		UINT32 flags = memory_region_flags(Machine, rgntag);

		/* call ourself recursively until we are byte-sized */
		if (size > 1)
		{
			int halfsize = size / 2;
			UINT64 r0, r1;

			/* read each half, from lower address to upper address */
			r0 = expression_read_memory_region(rgntag, address + 0, halfsize);
			r1 = expression_read_memory_region(rgntag, address + halfsize, halfsize);

			/* assemble based on the target endianness */
			if ((flags & ROMREGION_ENDIANMASK) == ROMREGION_LE)
				result = r0 | (r1 << (8 * halfsize));
			else
				result = r1 | (r0 << (8 * halfsize));
		}

		/* only process if we're within range */
		else if (address < length)
		{
			/* lowmask specified which address bits are within the databus width */
			UINT32 lowmask = (1 << ((flags & ROMREGION_WIDTHMASK) >> 8)) - 1;
			base += address & ~lowmask;

			/* if we have a valid base, return the appropriate byte */
			if ((flags & ROMREGION_ENDIANMASK) == ROMREGION_LE)
				result = base[BYTE8_XOR_LE(address) & lowmask];
			else
				result = base[BYTE8_XOR_BE(address) & lowmask];
		}
	}
	return result;
}


/*-------------------------------------------------
    expression_read_eeprom - read EEPROM data
-------------------------------------------------*/

static UINT64 expression_read_eeprom(offs_t address, int size)
{
	UINT64 result = ~(UINT64)0 >> (64 - 8*size);
	UINT32 eelength, eesize;
	void *base;

	/* make sure we get a valid base before proceeding */
	base = eeprom_get_data_pointer(&eelength, &eesize);
	if (base != NULL && address < eelength)
	{
		/* switch off the size */
		switch (eesize)
		{
			case 1:		result &= ((UINT8 *)base)[address];							break;
			case 2:		result &= BIG_ENDIANIZE_INT16(((UINT16 *)base)[address]);	break;
		}
	}
	return result;
}


/*-------------------------------------------------
    expression_write_memory - write 1,2,4 or 8
    bytes at the given offset in the given address
    space
-------------------------------------------------*/

static void expression_write_memory(const char *name, int space, UINT32 address, int size, UINT64 data)
{
	int cpuindex;

	switch (space)
	{
		case EXPSPACE_PROGRAM:
		case EXPSPACE_DATA:
		case EXPSPACE_IO:
			cpuindex = (name != NULL) ? expression_cpu_index(Machine, name) : cpu_getactivecpu();
			if (cpuindex < 0)
				break;
			expression_write_address_space(cpuindex, ADDRESS_SPACE_PROGRAM + (space - EXPSPACE_PROGRAM), address, size, data);
			break;

		case EXPSPACE_OPCODE:
		case EXPSPACE_RAMWRITE:
			cpuindex = (name != NULL) ? expression_cpu_index(Machine, name) : cpu_getactivecpu();
			if (cpuindex < 0)
				break;
			expression_write_program_direct(cpuindex, (space == EXPSPACE_OPCODE), address, size, data);
			break;

		case EXPSPACE_EEPROM:
			expression_write_eeprom(address, size, data);
			break;

		case EXPSPACE_REGION:
			if (name == NULL)
				break;
			expression_write_memory_region(name, address, size, data);
			break;
	}
}


/*-------------------------------------------------
    expression_write_address_space - write memory
    to a specific CPU's address space
-------------------------------------------------*/

static void expression_write_address_space(int cpuindex, int space, offs_t address, int size, UINT64 data)
{
	const debug_cpu_info *info = &global.cpuinfo[cpuindex];

	/* only process if in of range and we have a bus */
	if (cpuindex < ARRAY_LENGTH(global.cpuinfo) && info->space[space].databytes != 0)
	{
		/* adjust the address into a byte address */
		address = ADDR2BYTE(address, info, space);

		/* switch contexts and do the write */
		cpuintrf_push_context(cpuindex);
		switch (size)
		{
			case 1:		debug_write_byte(space, address, data, TRUE);	break;
			case 2:		debug_write_word(space, address, data, TRUE);	break;
			case 4:		debug_write_dword(space, address, data, TRUE);	break;
			case 8:		debug_write_qword(space, address, data, TRUE);	break;
		}
		cpuintrf_pop_context();
	}
}


/*-------------------------------------------------
    expression_write_program_direct - write memory
    directly to an opcode or RAM pointer
-------------------------------------------------*/

static void expression_write_program_direct(int cpuindex, int opcode, offs_t address, int size, UINT64 data)
{
	const debug_cpu_info *info = &global.cpuinfo[cpuindex];
	UINT8 *base;

	/* only process if in of range and we have a bus */
	if (cpuindex < ARRAY_LENGTH(global.cpuinfo) && info->space[ADDRESS_SPACE_PROGRAM].databytes != 0)
	{
		/* adjust the address into a byte address, but not if being called recursively */
		if ((opcode & 2) == 0)
			address = ADDR2BYTE(address, info, ADDRESS_SPACE_PROGRAM);

		/* call ourself recursively until we are byte-sized */
		if (size > 1)
		{
			int halfsize = size / 2;
			UINT64 r0, r1, halfmask;

			/* break apart based on the target endianness */
			halfmask = ~(UINT64)0 >> (64 - 8 * halfsize);
			if (info->endianness == CPU_IS_LE)
			{
				r0 = data & halfmask;
				r1 = (data >> (8 * halfsize)) & halfmask;
			}
			else
			{
				r0 = (data >> (8 * halfsize)) & halfmask;
				r1 = data & halfmask;
			}

			/* write each half, from lower address to upper address */
			expression_write_program_direct(cpuindex, opcode | 2, address + 0, halfsize, r0);
			expression_write_program_direct(cpuindex, opcode | 2, address + halfsize, halfsize, r1);
		}

		/* handle the byte-sized final case */
		else
		{
			/* lowmask specified which address bits are within the databus width */
			offs_t lowmask = info->space[ADDRESS_SPACE_PROGRAM].databytes - 1;

			/* get the base of memory, aligned to the address minus the lowbits */
			if (opcode & 1)
				base = memory_get_op_ptr(cpuindex, address & ~lowmask, FALSE);
			else
				base = memory_get_read_ptr(cpuindex, ADDRESS_SPACE_PROGRAM, address & ~lowmask);

			/* if we have a valid base, write the appropriate byte */
			if (base != NULL)
			{
				if (info->endianness == CPU_IS_LE)
					base[BYTE8_XOR_LE(address) & lowmask] = data;
				else
					base[BYTE8_XOR_BE(address) & lowmask] = data;
				global.memory_modified = TRUE;
			}
		}
	}
}


/*-------------------------------------------------
    expression_write_memory_region - write memory
    from a memory region
-------------------------------------------------*/

static void expression_write_memory_region(const char *rgntag, offs_t address, int size, UINT64 data)
{
	UINT8 *base = memory_region(Machine, rgntag);

	/* make sure we get a valid base before proceeding */
	if (base != NULL)
	{
		UINT32 length = memory_region_length(Machine, rgntag);
		UINT32 flags = memory_region_flags(Machine, rgntag);

		/* call ourself recursively until we are byte-sized */
		if (size > 1)
		{
			int halfsize = size / 2;
			UINT64 r0, r1, halfmask;

			/* break apart based on the target endianness */
			halfmask = ~(UINT64)0 >> (64 - 8 * halfsize);
			if ((flags & ROMREGION_ENDIANMASK) == ROMREGION_LE)
			{
				r0 = data & halfmask;
				r1 = (data >> (8 * halfsize)) & halfmask;
			}
			else
			{
				r0 = (data >> (8 * halfsize)) & halfmask;
				r1 = data & halfmask;
			}

			/* write each half, from lower address to upper address */
			expression_write_memory_region(rgntag, address + 0, halfsize, r0);
			expression_write_memory_region(rgntag, address + halfsize, halfsize, r1);
		}

		/* only process if we're within range */
		else if (address < length)
		{
			/* lowmask specified which address bits are within the databus width */
			UINT32 lowmask = (1 << ((flags & ROMREGION_WIDTHMASK) >> 8)) - 1;
			base += address & ~lowmask;

			/* if we have a valid base, set the appropriate byte */
			if ((flags & ROMREGION_ENDIANMASK) == ROMREGION_LE)
				base[BYTE8_XOR_LE(address) & lowmask] = data;
			else
				base[BYTE8_XOR_BE(address) & lowmask] = data;
			global.memory_modified = TRUE;
		}
	}
}


/*-------------------------------------------------
    expression_write_eeprom - write EEPROM data
-------------------------------------------------*/

static void expression_write_eeprom(offs_t address, int size, UINT64 data)
{
	UINT32 eelength, eesize;
	void *vbase = eeprom_get_data_pointer(&eelength, &eesize);

	/* make sure we get a valid base before proceeding */
	if (vbase != NULL && address < eelength)
	{
		UINT64 mask = ~(UINT64)0 >> (64 - 8*size);

		/* switch off the size */
		switch (eesize)
		{
			case 1:
			{
				UINT8 *base = (UINT8 *)vbase + address;
				*base = (*base & ~mask) | (data & mask);
				break;
			}

			case 2:
			{
				UINT16 *base = (UINT16 *)vbase + address;
				UINT16 value = BIG_ENDIANIZE_INT16(*base);
				value = (value & ~mask) | (data & mask);
				*base = BIG_ENDIANIZE_INT16(value);
				break;
			}
		}
		global.memory_modified = TRUE;
	}
}


/*-------------------------------------------------
    expression_validate - validate that the
    provided expression references an
    appropriate name
-------------------------------------------------*/

static EXPRERR expression_validate(const char *name, int space)
{
	int cpuindex;

	switch (space)
	{
		case EXPSPACE_PROGRAM:
		case EXPSPACE_DATA:
		case EXPSPACE_IO:
			cpuindex = (name != NULL) ? expression_cpu_index(Machine, name) : cpu_getactivecpu();
			if (cpuindex < 0)
				return (name == NULL) ? EXPRERR_MISSING_MEMORY_NAME : EXPRERR_INVALID_MEMORY_NAME;
			if (cpunum_addrbus_width(cpuindex, ADDRESS_SPACE_PROGRAM + (space - EXPSPACE_PROGRAM)) == 0)
				return EXPRERR_NO_SUCH_MEMORY_SPACE;
			break;

		case EXPSPACE_OPCODE:
		case EXPSPACE_RAMWRITE:
			cpuindex = (name != NULL) ? expression_cpu_index(Machine, name) : cpu_getactivecpu();
			if (cpuindex < 0)
				return (name == NULL) ? EXPRERR_MISSING_MEMORY_NAME : EXPRERR_INVALID_MEMORY_NAME;
			break;

		case EXPSPACE_EEPROM:
			if (name != NULL)
				return EXPRERR_INVALID_MEMORY_NAME;
			break;

		case EXPSPACE_REGION:
			if (name == NULL)
				return EXPRERR_MISSING_MEMORY_NAME;
			if (memory_region(Machine, name) == NULL)
				return EXPRERR_INVALID_MEMORY_NAME;
			break;
	}
	return EXPRERR_NONE;
}


/*-------------------------------------------------
    debug_cpu_trace_printf - writes text to a given
    CPU's trace file
-------------------------------------------------*/

void debug_cpu_trace_printf(int cpunum, const char *fmt, ...)
{
	va_list va;

	debug_cpu_info *info = &global.cpuinfo[cpunum];

	if (info->trace.file)
	{
		va_start(va, fmt);
		vfprintf(info->trace.file, fmt, va);
		va_end(va);
	}
}


/*-------------------------------------------------
    debug_cpu_source_script - specifies a debug command
    script to use
-------------------------------------------------*/

void debug_cpu_source_script(running_machine *machine, const char *file)
{
	if (debug_source_file)
	{
		fclose(debug_source_file);
		debug_source_file = NULL;
	}

	if (file)
	{
		debug_source_file = fopen(file, "r");
		if (!debug_source_file)
		{
			if (mame_get_phase(machine) == MAME_PHASE_RUNNING)
				debug_console_printf("Cannot open command file '%s'\n", file);
			else
				fatalerror("Cannot open command file '%s'", file);
		}
	}
}


/*-------------------------------------------------
    debug_cpu_flush_traces - flushes all traces; this is
    useful if a trace is going on when we fatalerror
-------------------------------------------------*/

void debug_cpu_flush_traces(void)
{
	int cpunum;

	for (cpunum = 0; cpunum < cpu_gettotalcpu(); cpunum++)
	{
		if (global.cpuinfo[cpunum].trace.file)
			fflush(global.cpuinfo[cpunum].trace.file);
	}
}



/***************************************************************************
    VARIABLE GETTERS/SETTERS
***************************************************************************/

/*-------------------------------------------------
    get_wpaddr - getter callback for the
    'wpaddr' symbol
-------------------------------------------------*/

static UINT64 get_wpaddr(void *ref)
{
	return global.wpaddr;
}


/*-------------------------------------------------
    get_wpdata - getter callback for the
    'wpdata' symbol
-------------------------------------------------*/

static UINT64 get_wpdata(void *ref)
{
	return global.wpdata;
}


/*-------------------------------------------------
    get_cycles - getter callback for the
    'cycles' symbol
-------------------------------------------------*/

static UINT64 get_cycles(void *ref)
{
	return activecpu_get_icount();
}


/*-------------------------------------------------
    get_cpunum - getter callback for the
    'cpunum' symbol
-------------------------------------------------*/

static UINT64 get_cpunum(void *ref)
{
	return cpu_getactivecpu();
}


/*-------------------------------------------------
    get_tempvar - getter callback for the
    'tempX' symbols
-------------------------------------------------*/

static UINT64 get_tempvar(void *ref)
{
	return *(UINT64 *)ref;
}


/*-------------------------------------------------
    set_tempvar - setter callback for the
    'tempX' symbols
-------------------------------------------------*/

static void set_tempvar(void *ref, UINT64 value)
{
	*(UINT64 *)ref = value;
}


/*-------------------------------------------------
    get_logunmap - getter callback for the logumap
    symbols
-------------------------------------------------*/

static UINT64 get_logunmap(void *ref)
{
	return memory_get_log_unmap((FPTR)ref);
}


/*-------------------------------------------------
    get_beamx - get beam horizontal position
-------------------------------------------------*/

static UINT64 get_beamx(void *ref)
{
	UINT64 ret = 0;
	const device_config *screen = device_list_find_by_index(Machine->config->devicelist, VIDEO_SCREEN, 0);

	if (screen != NULL)
		ret = video_screen_get_hpos(screen);

	return ret;
}


/*-------------------------------------------------
    get_beamy - get beam vertical position
-------------------------------------------------*/

static UINT64 get_beamy(void *ref)
{
	UINT64 ret = 0;
	const device_config *screen = device_list_find_by_index(Machine->config->devicelist, VIDEO_SCREEN, 0);

	if (screen != NULL)
		ret = video_screen_get_vpos(screen);

	return ret;
}


/*-------------------------------------------------
    set_logunmap - setter callback for the logumap
    symbols
-------------------------------------------------*/

static void set_logunmap(void *ref, UINT64 value)
{
	memory_set_log_unmap((FPTR)ref, value ? 1 : 0);
}


/*-------------------------------------------------
    get_current_pc - getter callback for a CPU's
    current instruction pointer
-------------------------------------------------*/

static UINT64 get_current_pc(void *ref)
{
	return activecpu_get_pc();
}


/*-------------------------------------------------
    get_cpu_reg - getter callback for a CPU's
    register symbols
-------------------------------------------------*/

static UINT64 get_cpu_reg(void *ref)
{
	return activecpu_get_reg((FPTR)ref);
}


/*-------------------------------------------------
    set_cpu_reg - setter callback for a CPU's
    register symbols
-------------------------------------------------*/

static void set_cpu_reg(void *ref, UINT64 value)
{
	activecpu_set_reg((FPTR)ref, value);
}
