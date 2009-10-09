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

/* in mame.h: typedef struct _debugcpu_private debugcpu_private; */
struct _debugcpu_private
{
	const device_config *livecpu;
	const device_config *visiblecpu;
	const device_config *breakcpu;

	FILE *			source_file;				/* script source file */

	symbol_table *	symtable;					/* global symbol table */

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
    FUNCTION PROTOTYPES
***************************************************************************/

/* internal helpers */
static void debug_cpu_exit(running_machine *machine);
static void on_vblank(const device_config *device, void *param, int vblank_state);
static void reset_transient_flags(running_machine *machine);
static void compute_debug_flags(const device_config *device);
static void perform_trace(cpu_debug_data *info);
static void prepare_for_step_overout(cpu_debug_data *info);
static void process_source_file(running_machine *machine);
static void breakpoint_update_flags(cpu_debug_data *info);
static void breakpoint_check(running_machine *machine, cpu_debug_data *info, offs_t pc);
static void watchpoint_update_flags(const address_space *space);
static void watchpoint_check(const address_space *space, int type, offs_t address, UINT64 value_to_write, UINT64 mem_mask);
static void check_hotspots(const address_space *space, offs_t address);
static UINT32 dasm_wrapped(const device_config *device, char *buffer, offs_t pc);

/* expression handlers */
static UINT64 expression_read_memory(void *param, const char *name, int space, UINT32 address, int size);
static UINT64 expression_read_program_direct(const address_space *space, int opcode, offs_t address, int size);
static UINT64 expression_read_memory_region(running_machine *machine, const char *rgntag, offs_t address, int size);
static UINT64 expression_read_eeprom(running_machine *machine, offs_t address, int size);
static void expression_write_memory(void *param, const char *name, int space, UINT32 address, int size, UINT64 data);
static void expression_write_program_direct(const address_space *space, int opcode, offs_t address, int size, UINT64 data);
static void expression_write_memory_region(running_machine *machine, const char *rgntag, offs_t address, int size, UINT64 data);
static void expression_write_eeprom(running_machine *machine, offs_t address, int size, UINT64 data);
static EXPRERR expression_validate(void *param, const char *name, int space);

/* variable getters/setters */
static UINT64 get_wpaddr(void *globalref, void *ref);
static UINT64 get_wpdata(void *globalref, void *ref);
static UINT64 get_cpunum(void *globalref, void *ref);
static UINT64 get_tempvar(void *globalref, void *ref);
static void set_tempvar(void *globalref, void *ref, UINT64 value);
static UINT64 get_beamx(void *globalref, void *ref);
static UINT64 get_beamy(void *globalref, void *ref);
static UINT64 get_frame(void *globalref, void *ref);
static UINT64 get_current_pc(void *globalref, void *ref);
static UINT64 get_cycles(void *globalref, void *ref);
static UINT64 get_logunmap(void *globalref, void *ref);
static void set_logunmap(void *globalref, void *ref, UINT64 value);
static UINT64 get_cpu_reg(void *globalref, void *ref);
static void set_cpu_reg(void *globalref, void *ref, UINT64 value);



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
    INITIALIZATION AND CLEANUP
***************************************************************************/

/*-------------------------------------------------
    debug_cpu_init - initialize the CPU
    information for debugging
-------------------------------------------------*/

void debug_cpu_init(running_machine *machine)
{
	const device_config *first_screen = video_screen_first(machine->config);
	debugcpu_private *global;
	const device_config *cpu;
	int regnum;

	/* allocate and reset globals */
	machine->debugcpu_data = global = auto_alloc_clear(machine, debugcpu_private);
	global->execution_state = EXECUTION_STATE_STOPPED;
	global->bpindex = 1;
	global->wpindex = 1;

	/* create a global symbol table */
	global->symtable = symtable_alloc(NULL, machine);

	/* add "wpaddr", "wpdata", "cycles", "cpunum", "logunmap" to the global symbol table */
	symtable_add_register(global->symtable, "wpaddr", NULL, get_wpaddr, NULL);
	symtable_add_register(global->symtable, "wpdata", NULL, get_wpdata, NULL);
	symtable_add_register(global->symtable, "cpunum", NULL, get_cpunum, NULL);
	symtable_add_register(global->symtable, "beamx", (void *)first_screen, get_beamx, NULL);
	symtable_add_register(global->symtable, "beamy", (void *)first_screen, get_beamy, NULL);
	symtable_add_register(global->symtable, "frame", (void *)first_screen, get_frame, NULL);

	/* add the temporary variables to the global symbol table */
	for (regnum = 0; regnum < NUM_TEMP_VARIABLES; regnum++)
	{
		char symname[10];
		sprintf(symname, "temp%d", regnum);
		symtable_add_register(global->symtable, symname, &global->tempvar[regnum], get_tempvar, set_tempvar);
	}

	/* loop over CPUs and build up their info */
	for (cpu = machine->firstcpu; cpu != NULL; cpu = cpu_next(cpu))
	{
		cpu_class_header *classheader = cpu_get_class_header(cpu);
		cpu_debug_data *info;

		/* allocate some information */
		info = auto_alloc_clear(machine, cpu_debug_data);
		classheader->debug = info;

		/* reset the PC data */
		info->flags = DEBUG_FLAG_OBSERVING | DEBUG_FLAG_HISTORY;
		info->device = cpu;
		info->opwidth = cpu_get_min_opcode_bytes(info->device);

		/* fetch the memory accessors */
		info->read = (cpu_read_func)device_get_info_fct(info->device, CPUINFO_FCT_READ);
		info->write = (cpu_write_func)device_get_info_fct(info->device, CPUINFO_FCT_WRITE);
		info->readop = (cpu_readop_func)device_get_info_fct(info->device, CPUINFO_FCT_READOP);
		info->translate = (cpu_translate_func)device_get_info_fct(info->device, CPUINFO_FCT_TRANSLATE);
		info->disassemble = (cpu_disassemble_func)device_get_info_fct(info->device, CPUINFO_FCT_DISASSEMBLE);

		/* allocate a symbol table */
		info->symtable = symtable_alloc(global->symtable, (void *)cpu);

		/* add a global symbol for the current instruction pointer */
		symtable_add_register(info->symtable, "cycles", NULL, get_cycles, NULL);
		if (cpu->space[ADDRESS_SPACE_PROGRAM] != NULL)
			symtable_add_register(info->symtable, "logunmap", (void *)cpu->space[ADDRESS_SPACE_PROGRAM], get_logunmap, set_logunmap);
		if (cpu->space[ADDRESS_SPACE_DATA] != NULL)
			symtable_add_register(info->symtable, "logunmapd", (void *)cpu->space[ADDRESS_SPACE_DATA], get_logunmap, set_logunmap);
		if (cpu->space[ADDRESS_SPACE_IO] != NULL)
			symtable_add_register(info->symtable, "logunmapi", (void *)cpu->space[ADDRESS_SPACE_IO], get_logunmap, set_logunmap);

		/* add all registers into it */
		for (regnum = 0; regnum < MAX_REGS; regnum++)
		{
			const char *str = cpu_get_reg_string(info->device, regnum);
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
				if (!isspace((UINT8)*str))
					symname[charnum++] = tolower((UINT8)*str);
			symname[charnum] = 0;

			/* add the symbol to the table */
			symtable_add_register(info->symtable, symname, (void *)(FPTR)regnum, get_cpu_reg, set_cpu_reg);
		}

		/* if no curpc, add one */
		if (symtable_find(info->symtable, "curpc") == NULL)
			symtable_add_register(info->symtable, "curpc", NULL, get_current_pc, 0);
	}

	/* first CPU is visible by default */
	global->visiblecpu = machine->firstcpu;

	/* add callback for breaking on VBLANK */
	if (machine->primary_screen != NULL)
		video_screen_register_vblank_callback(machine->primary_screen, on_vblank, NULL);

	add_exit_callback(machine, debug_cpu_exit);
}


/*-------------------------------------------------
    debug_cpu_flush_traces - flushes all traces;
    this is useful if a trace is going on when we
    fatalerror
-------------------------------------------------*/

void debug_cpu_flush_traces(running_machine *machine)
{
	const device_config *cpu;

	for (cpu = machine->firstcpu; cpu != NULL; cpu = cpu_next(cpu))
	{
		cpu_debug_data *info = cpu_get_debug_data(cpu);

		/* this can be called on exit even when no debugging is enabled, so
          make sure the info is valid before proceeding */
		if (info != NULL && info->trace.file != NULL)
			fflush(info->trace.file);
	}
}



/***************************************************************************
    DEBUGGING STATUS AND INFORMATION
***************************************************************************/

/*-------------------------------------------------
    cpu_get_visible_cpu - return the visible CPU
    device (the one that commands should apply to)
-------------------------------------------------*/

const device_config *debug_cpu_get_visible_cpu(running_machine *machine)
{
	return machine->debugcpu_data->visiblecpu;
}


/*-------------------------------------------------
    debug_cpu_within_instruction_hook - true if
    the debugger is currently live
-------------------------------------------------*/

int debug_cpu_within_instruction_hook(running_machine *machine)
{
	return machine->debugcpu_data->within_instruction_hook;
}


/*-------------------------------------------------
    debug_cpu_is_stopped - return TRUE if the
    current execution state is stopped
-------------------------------------------------*/

int debug_cpu_is_stopped(running_machine *machine)
{
	debugcpu_private *global = machine->debugcpu_data;
	return (global != NULL) ? (global->execution_state == EXECUTION_STATE_STOPPED) : FALSE;
}



/***************************************************************************
    SYMBOL TABLE INTERFACES
***************************************************************************/

/*-------------------------------------------------
    debug_cpu_get_global_symtable - return the
    global symbol table
-------------------------------------------------*/

symbol_table *debug_cpu_get_global_symtable(running_machine *machine)
{
	return machine->debugcpu_data->symtable;
}


/*-------------------------------------------------
    debug_cpu_get_visible_symtable - return the
    locally-visible symbol table
-------------------------------------------------*/

symbol_table *debug_cpu_get_visible_symtable(running_machine *machine)
{
	return cpu_get_debug_data(machine->debugcpu_data->visiblecpu)->symtable;
}


/*-------------------------------------------------
    debug_cpu_get_symtable - return a specific
    CPU's symbol table
-------------------------------------------------*/

symbol_table *debug_cpu_get_symtable(const device_config *device)
{
	return cpu_get_debug_data(device)->symtable;
}



/***************************************************************************
    MEMORY AND DISASSEMBLY HELPERS
***************************************************************************/

/*-------------------------------------------------
    debug_cpu_translate - return the physical
    address corresponding to the given logical
    address
-------------------------------------------------*/

int debug_cpu_translate(const address_space *space, int intention, offs_t *address)
{
	if (space->cpu != NULL && space->cpu->type == CPU)
	{
		cpu_debug_data *info = cpu_get_debug_data(space->cpu);
		if (info->translate != NULL)
			return (*info->translate)(space->cpu, space->spacenum, intention, address);
	}
	return TRUE;
}


/*-------------------------------------------------
    debug_cpu_disassemble - disassemble a line at
    a given PC on a given CPU
-------------------------------------------------*/

offs_t debug_cpu_disassemble(const device_config *device, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	cpu_debug_data *info = cpu_get_debug_data(device);
	offs_t result = 0;

	/* check for disassembler override */
	if (info->dasm_override != NULL)
		result = (*info->dasm_override)(device, buffer, pc, oprom, opram, 0);

	/* if we have a disassembler, run it */
	if (result == 0 && info->disassemble != NULL)
		result = (*info->disassemble)(device, buffer, pc, oprom, opram, 0);

	/* if we still have nothing, output vanilla bytes */
	if (result == 0)
	{
		result = cpu_get_min_opcode_bytes(device);
		switch (result)
		{
			case 1:
			default:
				sprintf(buffer, "$%02X", *(UINT8 *)oprom);
				break;

			case 2:
				sprintf(buffer, "$%04X", *(UINT16 *)oprom);
				break;

			case 4:
				sprintf(buffer, "$%08X", *(UINT32 *)oprom);
				break;

			case 8:
				sprintf(buffer, "$%08X%08X", (UINT32)(*(UINT64 *)oprom >> 32), (UINT32)(*(UINT64 *)oprom >> 0));
				break;
		}
	}

	/* make sure we get good results */
	assert((result & DASMFLAG_LENGTHMASK) != 0);
#ifdef MAME_DEBUG
{
	const address_space *space = cpu_get_address_space(device, ADDRESS_SPACE_PROGRAM);
	int bytes = memory_address_to_byte(space, result & DASMFLAG_LENGTHMASK);
	assert(bytes >= cpu_get_min_opcode_bytes(device));
	assert(bytes <= cpu_get_max_opcode_bytes(device));
	(void) bytes; /* appease compiler */
}
#endif

	return result;
}


/*-------------------------------------------------
    debug_cpu_set_dasm_override - set an override
    handler for disassembly
-------------------------------------------------*/

void debug_cpu_set_dasm_override(const device_config *device, cpu_disassemble_func dasm_override)
{
	cpu_debug_data *info = cpu_get_debug_data(device);
	if (info != NULL)
		info->dasm_override = dasm_override;
}



/***************************************************************************
    CORE DEBUGGER HOOKS
***************************************************************************/

/*-------------------------------------------------
    debug_cpu_start_hook - the CPU execution
    system calls this hook before beginning
    execution for the given CPU
-------------------------------------------------*/

void debug_cpu_start_hook(const device_config *device, attotime endtime)
{
	debugcpu_private *global = device->machine->debugcpu_data;
	cpu_debug_data *info = cpu_get_debug_data(device);

	assert((device->machine->debug_flags & DEBUG_FLAG_ENABLED) != 0);

	/* stash a pointer to the current live CPU */
	assert(global->livecpu == NULL);
	global->livecpu = device;

	/* update the target execution end time */
	info->endexectime = endtime;

	/* if we're running, do some periodic updating */
	if (global->execution_state != EXECUTION_STATE_STOPPED)
	{
		/* check for periodic updates */
		if (device == global->visiblecpu && osd_ticks() > global->last_periodic_update_time + osd_ticks_per_second()/4)
		{
			debug_view_update_all(device->machine);
			debug_view_flush_updates(device->machine);
			global->last_periodic_update_time = osd_ticks();
		}

		/* check for pending breaks */
		else if (device == global->breakcpu)
		{
			global->execution_state = EXECUTION_STATE_STOPPED;
			global->breakcpu = NULL;
		}

		/* if a VBLANK occurred, check on things */
		if (global->vblank_occurred)
		{
			global->vblank_occurred = FALSE;

			/* if we were waiting for a VBLANK, signal it now */
			if ((info->flags & DEBUG_FLAG_STOP_VBLANK) != 0)
			{
				global->execution_state = EXECUTION_STATE_STOPPED;
				debug_console_printf(device->machine, "Stopped at VBLANK\n");
			}

			/* check for debug keypresses */
			else if (ui_input_pressed(device->machine, IPT_UI_DEBUG_BREAK))
				debug_cpu_halt_on_next_instruction(global->visiblecpu, "User-initiated break\n");
		}
	}

	/* recompute the debugging mode */
	compute_debug_flags(device);
}


/*-------------------------------------------------
    debug_cpu_stop_hook - the CPU execution
    system calls this hook when ending execution
    for the given CPU
-------------------------------------------------*/

void debug_cpu_stop_hook(const device_config *device)
{
	debugcpu_private *global = device->machine->debugcpu_data;
	cpu_debug_data *info = cpu_get_debug_data(device);

	assert(global->livecpu == device);

	/* if we're stopping on a context switch, handle it now */
	if (info->flags & DEBUG_FLAG_STOP_CONTEXT)
	{
		global->execution_state = EXECUTION_STATE_STOPPED;
		reset_transient_flags(device->machine);
	}

	/* clear the live CPU */
	global->livecpu = NULL;
}


/*-------------------------------------------------
    debug_cpu_interrupt_hook - called when an
    interrupt is acknowledged
-------------------------------------------------*/

void debug_cpu_interrupt_hook(const device_config *device, int irqline)
{
	debugcpu_private *global = device->machine->debugcpu_data;
	cpu_debug_data *info = cpu_get_debug_data(device);

	/* see if this matches a pending interrupt request */
	if (info != NULL && (info->flags & DEBUG_FLAG_STOP_INTERRUPT) != 0 && (info->stopirq == -1 || info->stopirq == irqline))
	{
		global->execution_state = EXECUTION_STATE_STOPPED;
		debug_console_printf(device->machine, "Stopped on interrupt (CPU '%s', IRQ %d)\n", device->tag, irqline);
		compute_debug_flags(device);
	}
}


/*-------------------------------------------------
    debug_cpu_exception_hook - called when an
    exception is generated
-------------------------------------------------*/

void debug_cpu_exception_hook(const device_config *device, int exception)
{
	debugcpu_private *global = device->machine->debugcpu_data;
	cpu_debug_data *info = cpu_get_debug_data(device);

	/* see if this matches a pending interrupt request */
	if ((info->flags & DEBUG_FLAG_STOP_EXCEPTION) != 0 && (info->stopexception == -1 || info->stopexception == exception))
	{
		global->execution_state = EXECUTION_STATE_STOPPED;
		debug_console_printf(device->machine, "Stopped on exception (CPU '%s', exception %d)\n", device->tag, exception);
		compute_debug_flags(device);
	}
}


/*-------------------------------------------------
    debug_cpu_instruction_hook - called by the
    CPU cores before executing each instruction
-------------------------------------------------*/

void debug_cpu_instruction_hook(const device_config *device, offs_t curpc)
{
	debugcpu_private *global = device->machine->debugcpu_data;
	cpu_debug_data *info = cpu_get_debug_data(device);

	/* note that we are in the debugger code */
	global->within_instruction_hook = TRUE;

	/* update the history */
	info->pc_history[info->pc_history_index++ % DEBUG_HISTORY_SIZE] = curpc;

	/* are we tracing? */
	if (info->flags & DEBUG_FLAG_TRACING_ANY)
		perform_trace(info);

	/* per-instruction hook? */
	if (global->execution_state != EXECUTION_STATE_STOPPED && (info->flags & DEBUG_FLAG_HOOKED) != 0 && (*info->instrhook)(device, curpc))
		global->execution_state = EXECUTION_STATE_STOPPED;

	/* handle single stepping */
	if (global->execution_state != EXECUTION_STATE_STOPPED && (info->flags & DEBUG_FLAG_STEPPING_ANY) != 0)
	{
		/* is this an actual step? */
		if (info->stepaddr == ~0 || curpc == info->stepaddr)
		{
			/* decrement the count and reset the breakpoint */
			info->stepsleft--;
			info->stepaddr = ~0;

			/* if we hit 0, stop */
			if (info->stepsleft == 0)
				global->execution_state = EXECUTION_STATE_STOPPED;

			/* update every 100 steps until we are within 200 of the end */
			else if ((info->flags & DEBUG_FLAG_STEPPING_OUT) == 0 && (info->stepsleft < 200 || info->stepsleft % 100 == 0))
			{
				debug_view_update_all(device->machine);
				debug_view_flush_updates(device->machine);
				debugger_refresh_display(device->machine);
			}
		}
	}

	/* handle breakpoints */
	if (global->execution_state != EXECUTION_STATE_STOPPED && (info->flags & (DEBUG_FLAG_STOP_TIME | DEBUG_FLAG_STOP_PC | DEBUG_FLAG_LIVE_BP)) != 0)
	{
		/* see if we hit a target time */
		if ((info->flags & DEBUG_FLAG_STOP_TIME) != 0 && attotime_compare(timer_get_time(device->machine), info->stoptime) >= 0)
		{
			debug_console_printf(device->machine, "Stopped at time interval %.1g\n", attotime_to_double(timer_get_time(device->machine)));
			global->execution_state = EXECUTION_STATE_STOPPED;
		}

		/* check the temp running breakpoint and break if we hit it */
		else if ((info->flags & DEBUG_FLAG_STOP_PC) != 0 && info->stopaddr == curpc)
		{
			debug_console_printf(device->machine, "Stopped at temporary breakpoint %X on CPU '%s'\n", info->stopaddr, device->tag);
			global->execution_state = EXECUTION_STATE_STOPPED;
		}

		/* check for execution breakpoints */
		else if ((info->flags & DEBUG_FLAG_LIVE_BP) != 0)
			breakpoint_check(device->machine, info, curpc);
	}

	/* if we are supposed to halt, do it now */
	if (global->execution_state == EXECUTION_STATE_STOPPED)
	{
		int firststop = TRUE;

		/* reset any transient state */
		reset_transient_flags(device->machine);
		global->breakcpu = NULL;

		/* remember the last visible CPU in the debugger */
		global->visiblecpu = device;

		/* update all views */
		debug_view_update_all(device->machine);
		debugger_refresh_display(device->machine);

		/* wait for the debugger; during this time, disable sound output */
		sound_mute(device->machine, TRUE);
		while (global->execution_state == EXECUTION_STATE_STOPPED)
		{
			/* flush any pending updates before waiting again */
			debug_view_flush_updates(device->machine);

			/* clear the memory modified flag and wait */
			global->memory_modified = FALSE;
			osd_wait_for_debugger(device, firststop);
			firststop = FALSE;

			/* if something modified memory, update the screen */
			if (global->memory_modified)
			{
				debug_view_update_type(device->machine, DVT_DISASSEMBLY);
				debugger_refresh_display(device->machine);
			}

			/* check for commands in the source file */
			process_source_file(device->machine);

			/* if an event got scheduled, resume */
			if (mame_is_scheduled_event_pending(device->machine))
				global->execution_state = EXECUTION_STATE_RUNNING;
		}
		sound_mute(device->machine, FALSE);

		/* remember the last visible CPU in the debugger */
		global->visiblecpu = device;
	}

	/* handle step out/over on the instruction we are about to execute */
	if ((info->flags & (DEBUG_FLAG_STEPPING_OVER | DEBUG_FLAG_STEPPING_OUT)) != 0 && info->stepaddr == ~0)
		prepare_for_step_overout(info);

	/* no longer in debugger code */
	global->within_instruction_hook = FALSE;
}


/*-------------------------------------------------
    debug_cpu_memory_read_hook - the memory system
    calls this hook when watchpoints are enabled
    and a memory read happens
-------------------------------------------------*/

void debug_cpu_memory_read_hook(const address_space *space, offs_t address, UINT64 mem_mask)
{
	cpu_debug_data *info = cpu_get_debug_data(space->cpu);

	/* check watchpoints */
	watchpoint_check(space, WATCHPOINT_READ, address, 0, mem_mask);

	/* check hotspots */
	if (info->hotspots != NULL)
		check_hotspots(space, address);
}


/*-------------------------------------------------
    debug_cpu_memory_write_hook - the memory
    system calls this hook when watchpoints are
    enabled and a memory write happens
-------------------------------------------------*/

void debug_cpu_memory_write_hook(const address_space *space, offs_t address, UINT64 data, UINT64 mem_mask)
{
	watchpoint_check(space, WATCHPOINT_WRITE, address, data, mem_mask);
}



/***************************************************************************
    EXECUTION CONTROL
***************************************************************************/

/*-------------------------------------------------
    debug_cpu_halt_on_next_instruction - halt in
    the debugger on the next instruction
-------------------------------------------------*/

void debug_cpu_halt_on_next_instruction(const device_config *device, const char *fmt, ...)
{
	debugcpu_private *global = device->machine->debugcpu_data;
	cpu_debug_data *info = cpu_get_debug_data(device);
	va_list arg;

	/* if something is pending on this CPU already, ignore this request */
	if (info != NULL && device == global->breakcpu)
		return;

	/* output the message to the console */
	va_start(arg, fmt);
	debug_console_vprintf(device->machine, fmt, arg);
	va_end(arg);

	/* if we are live, stop now, otherwise note that we want to break there */
	if (device == global->livecpu)
	{
		global->execution_state = EXECUTION_STATE_STOPPED;
		if (global->livecpu != NULL)
			compute_debug_flags(global->livecpu);
	}
	else
		global->breakcpu = device;
}


/*-------------------------------------------------
    debug_cpu_ignore_cpu - ignore/observe a given
    CPU
-------------------------------------------------*/

void debug_cpu_ignore_cpu(const device_config *device, int ignore)
{
	debugcpu_private *global = device->machine->debugcpu_data;
	cpu_debug_data *info = cpu_get_debug_data(device);

	if (ignore)
		info->flags &= ~DEBUG_FLAG_OBSERVING;
	else
		info->flags |= DEBUG_FLAG_OBSERVING;

	if (device == global->livecpu && ignore)
		debug_cpu_next_cpu(device->machine);
}


/*-------------------------------------------------
    debug_cpu_single_step - single step the visible
    CPU past the requested number of instructions
-------------------------------------------------*/

void debug_cpu_single_step(running_machine *machine, int numsteps)
{
	debugcpu_private *global = machine->debugcpu_data;
	cpu_debug_data *info = cpu_get_debug_data(global->visiblecpu);

	assert(info != NULL);

	info->stepsleft = numsteps;
	info->stepaddr = ~0;
	info->flags |= DEBUG_FLAG_STEPPING;
	global->execution_state = EXECUTION_STATE_RUNNING;
}


/*-------------------------------------------------
    debug_cpu_single_step_over - single step the
    visible over the requested number of
    instructions
-------------------------------------------------*/

void debug_cpu_single_step_over(running_machine *machine, int numsteps)
{
	debugcpu_private *global = machine->debugcpu_data;
	cpu_debug_data *info = cpu_get_debug_data(global->visiblecpu);

	assert(info != NULL);

	info->stepsleft = numsteps;
	info->stepaddr = ~0;
	info->flags |= DEBUG_FLAG_STEPPING_OVER;
	global->execution_state = EXECUTION_STATE_RUNNING;
}


/*-------------------------------------------------
    debug_cpu_single_step_out - single step the
    visible CPU out of the current function
-------------------------------------------------*/

void debug_cpu_single_step_out(running_machine *machine)
{
	debugcpu_private *global = machine->debugcpu_data;
	cpu_debug_data *info = cpu_get_debug_data(global->visiblecpu);

	assert(info != NULL);

	info->stepsleft = 100;
	info->stepaddr = ~0;
	info->flags |= DEBUG_FLAG_STEPPING_OUT;
	global->execution_state = EXECUTION_STATE_RUNNING;
}


/*-------------------------------------------------
    debug_cpu_go - execute the visible CPU until
    it hits the given address
-------------------------------------------------*/

void debug_cpu_go(running_machine *machine, offs_t targetpc)
{
	debugcpu_private *global = machine->debugcpu_data;
	cpu_debug_data *info = cpu_get_debug_data(global->visiblecpu);

	assert(info != NULL);

	info->stopaddr = targetpc;
	info->flags |= DEBUG_FLAG_STOP_PC;
	global->execution_state = EXECUTION_STATE_RUNNING;
}


/*-------------------------------------------------
    debug_cpu_go_vblank - execute until the next
    VBLANK
-------------------------------------------------*/

void debug_cpu_go_vblank(running_machine *machine)
{
	debugcpu_private *global = machine->debugcpu_data;
	cpu_debug_data *info = cpu_get_debug_data(global->visiblecpu);

	assert(info != NULL);

	global->vblank_occurred = FALSE;
	info->flags |= DEBUG_FLAG_STOP_VBLANK;
	global->execution_state = EXECUTION_STATE_RUNNING;
}


/*-------------------------------------------------
    debug_cpu_go_interrupt - execute until the
    specified interrupt fires on the visible CPU
-------------------------------------------------*/

void debug_cpu_go_interrupt(running_machine *machine, int irqline)
{
	debugcpu_private *global = machine->debugcpu_data;
	cpu_debug_data *info = cpu_get_debug_data(global->visiblecpu);

	assert(info != NULL);

	info->stopirq = irqline;
	info->flags |= DEBUG_FLAG_STOP_INTERRUPT;
	global->execution_state = EXECUTION_STATE_RUNNING;
}


/*-------------------------------------------------
    debug_cpu_go_exception - execute until the
    specified exception fires on the visible CPU
-------------------------------------------------*/

void debug_cpu_go_exception(running_machine *machine, int exception)
{
	debugcpu_private *global = machine->debugcpu_data;
	cpu_debug_data *info = cpu_get_debug_data(global->visiblecpu);

	assert(info != NULL);

	info->stopexception = exception;
	info->flags |= DEBUG_FLAG_STOP_EXCEPTION;
	global->execution_state = EXECUTION_STATE_RUNNING;
}


/*-------------------------------------------------
    debug_cpu_go_milliseconds - execute until the
    specified delay elapses
-------------------------------------------------*/

void debug_cpu_go_milliseconds(running_machine *machine, UINT64 milliseconds)
{
	debugcpu_private *global = machine->debugcpu_data;
	cpu_debug_data *info = cpu_get_debug_data(global->visiblecpu);

	assert(info != NULL);

	info->stoptime = attotime_add(timer_get_time(machine), ATTOTIME_IN_MSEC(milliseconds));
	info->flags |= DEBUG_FLAG_STOP_TIME;
	global->execution_state = EXECUTION_STATE_RUNNING;
}


/*-------------------------------------------------
    debug_cpu_next_cpu - execute until we hit
    the next CPU
-------------------------------------------------*/

void debug_cpu_next_cpu(running_machine *machine)
{
	debugcpu_private *global = machine->debugcpu_data;
	cpu_debug_data *info = cpu_get_debug_data(global->visiblecpu);

	assert(info != NULL);

	info->flags |= DEBUG_FLAG_STOP_CONTEXT;
	global->execution_state = EXECUTION_STATE_RUNNING;
}



/***************************************************************************
    BREAKPOINTS
***************************************************************************/

/*-------------------------------------------------
    debug_cpu_breakpoint_set - set a new
    breakpoint, returning its index
-------------------------------------------------*/

int debug_cpu_breakpoint_set(const device_config *device, offs_t address, parsed_expression *condition, const char *action)
{
	debugcpu_private *global = device->machine->debugcpu_data;
	cpu_debug_data *info = cpu_get_debug_data(device);
	debug_cpu_breakpoint *bp;

	assert_always(device != NULL, "debug_cpu_breakpoint_set() called with invalid cpu!");

	/* allocate breakpoint */
	bp = alloc_or_die(debug_cpu_breakpoint);
	bp->index = global->bpindex++;
	bp->enabled = TRUE;
	bp->address = address;
	bp->condition = condition;
	bp->action = NULL;
	if (action != NULL)
	{
		bp->action = alloc_array_or_die(char, strlen(action) + 1);
		strcpy(bp->action, action);
	}

	/* hook us in */
	bp->next = info->bplist;
	info->bplist = bp;

	/* ensure the live breakpoint flag is set */
	breakpoint_update_flags(info);
	return bp->index;
}


/*-------------------------------------------------
    debug_cpu_breakpoint_clear - clear a
    breakpoint by index
-------------------------------------------------*/

int debug_cpu_breakpoint_clear(running_machine *machine, int bpnum)
{
	debug_cpu_breakpoint *bp, *pbp;
	const device_config *cpu;

	/* loop over CPUs and find the requested breakpoint */
	for (cpu = machine->firstcpu; cpu != NULL; cpu = cpu_next(cpu))
	{
		cpu_debug_data *info = cpu_get_debug_data(cpu);
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
				breakpoint_update_flags(info);
				return TRUE;
			}
	}

	/* we didn't find it; return an error */
	return FALSE;
}


/*-------------------------------------------------
    debug_cpu_breakpoint_enable - enable/disable
    a breakpoint by index
-------------------------------------------------*/

int debug_cpu_breakpoint_enable(running_machine *machine, int bpnum, int enable)
{
	debug_cpu_breakpoint *bp;
	const device_config *cpu;

	/* loop over CPUs and find the requested breakpoint */
	for (cpu = machine->firstcpu; cpu != NULL; cpu = cpu_next(cpu))
	{
		cpu_debug_data *info = cpu_get_debug_data(cpu);
		for (bp = info->bplist; bp != NULL; bp = bp->next)
			if (bp->index == bpnum)
			{
				bp->enabled = (enable != 0);
				breakpoint_update_flags(info);
				return TRUE;
			}
	}

	return FALSE;
}



/***************************************************************************
    WATCHPOINTS
***************************************************************************/

/*-------------------------------------------------
    debug_cpu_watchpoint_set - set a new
    watchpoint, returning its index
-------------------------------------------------*/

int debug_cpu_watchpoint_set(const address_space *space, int type, offs_t address, offs_t length, parsed_expression *condition, const char *action)
{
	debugcpu_private *global = space->machine->debugcpu_data;
	cpu_debug_data *info = cpu_get_debug_data(space->cpu);
	debug_cpu_watchpoint *wp = alloc_or_die(debug_cpu_watchpoint);

	/* fill in the structure */
	wp->index = global->wpindex++;
	wp->enabled = TRUE;
	wp->type = type;
	wp->address = memory_address_to_byte(space, address) & space->bytemask;
	wp->length = memory_address_to_byte(space, length);
	wp->condition = condition;
	wp->action = NULL;
	if (action != NULL)
	{
		wp->action = alloc_array_or_die(char, strlen(action) + 1);
		strcpy(wp->action, action);
	}

	/* hook us in */
	wp->next = info->wplist[space->spacenum];
	info->wplist[space->spacenum] = wp;

	watchpoint_update_flags(space);

	return wp->index;
}


/*-------------------------------------------------
    debug_cpu_watchpoint_clear - clear a
    watchpoint by index
-------------------------------------------------*/

int debug_cpu_watchpoint_clear(running_machine *machine, int wpnum)
{
	debug_cpu_watchpoint *wp, *pwp;
	const device_config *cpu;
	int spacenum;

	/* loop over CPUs and find the requested watchpoint */
	for (cpu = machine->firstcpu; cpu != NULL; cpu = cpu_next(cpu))
	{
		cpu_debug_data *info = cpu_get_debug_data(cpu);

		for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
			for (pwp = NULL, wp = info->wplist[spacenum]; wp != NULL; pwp = wp, wp = wp->next)
				if (wp->index == wpnum)
				{
					/* unlink us from the list */
					if (pwp == NULL)
						info->wplist[spacenum] = wp->next;
					else
						pwp->next = wp->next;

					/* free the memory */
					if (wp->condition != NULL)
						expression_free(wp->condition);
					if (wp->action != NULL)
						free(wp->action);
					free(wp);

					watchpoint_update_flags(cpu_get_address_space(cpu, spacenum));
					return TRUE;
				}
	}

	/* we didn't find it; return an error */
	return FALSE;
}


/*-------------------------------------------------
    debug_cpu_watchpoint_enable - enable/disable a
    watchpoint by index
-------------------------------------------------*/

int debug_cpu_watchpoint_enable(running_machine *machine, int wpnum, int enable)
{
	debug_cpu_watchpoint *wp;
	const device_config *cpu;
	int spacenum;

	/* loop over CPUs and address spaces and find the requested watchpoint */
	for (cpu = machine->firstcpu; cpu != NULL; cpu = cpu_next(cpu))
	{
		cpu_debug_data *info = cpu_get_debug_data(cpu);

		for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
			for (wp = info->wplist[spacenum]; wp != NULL; wp = wp->next)
				if (wp->index == wpnum)
				{
					wp->enabled = (enable != 0);
					watchpoint_update_flags(cpu_get_address_space(cpu, spacenum));
					return TRUE;
				}
	}
	return FALSE;
}



/***************************************************************************
    MISC DEBUGGER FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    debug_cpu_source_script - specifies a debug
    command script to execute
-------------------------------------------------*/

void debug_cpu_source_script(running_machine *machine, const char *file)
{
	debugcpu_private *global = machine->debugcpu_data;

	/* close any existing source file */
	if (global->source_file != NULL)
	{
		fclose(global->source_file);
		global->source_file = NULL;
	}

	/* open a new one if requested */
	if (file != NULL)
	{
		global->source_file = fopen(file, "r");
		if (!global->source_file)
		{
			if (mame_get_phase(machine) == MAME_PHASE_RUNNING)
				debug_console_printf(machine, "Cannot open command file '%s'\n", file);
			else
				fatalerror("Cannot open command file '%s'", file);
		}
	}
}


/*-------------------------------------------------
    debug_cpu_trace - trace execution of a given
    CPU
-------------------------------------------------*/

void debug_cpu_trace(const device_config *device, FILE *file, int trace_over, const char *action)
{
	cpu_debug_data *info = cpu_get_debug_data(device);

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
		info->trace.action = alloc_array_or_die(char, strlen(action) + 1);
		strcpy(info->trace.action, action);
	}

	/* update flags */
	if (info->trace.file != NULL)
		info->flags |= trace_over ? DEBUG_FLAG_TRACING_OVER : DEBUG_FLAG_TRACING;
	else
		info->flags &= ~DEBUG_FLAG_TRACING_ANY;
}


/*-------------------------------------------------
    debug_cpu_trace_printf - output data into the
    given CPU's tracefile, if tracing
-------------------------------------------------*/

void debug_cpu_trace_printf(const device_config *device, const char *fmt, ...)
{
	va_list va;

	cpu_debug_data *info = cpu_get_debug_data(device);

	if (info->trace.file)
	{
		va_start(va, fmt);
		vfprintf(info->trace.file, fmt, va);
		va_end(va);
	}
}


/*-------------------------------------------------
    debug_cpu_set_instruction_hook - set a hook to
    be called on each instruction for a given CPU
-------------------------------------------------*/

void debug_cpu_set_instruction_hook(const device_config *device, debug_instruction_hook_func hook)
{
	cpu_debug_data *info = cpu_get_debug_data(device);

	/* set the hook and also the CPU's flag for fast knowledge of the hook */
	info->instrhook = hook;
	if (hook != NULL)
		info->flags |= DEBUG_FLAG_HOOKED;
	else
		info->flags &= ~DEBUG_FLAG_HOOKED;
}


/*-------------------------------------------------
    debug_cpu_hotspot_track - enable/disable
    tracking of hotspots
-------------------------------------------------*/

int debug_cpu_hotspot_track(const device_config *device, int numspots, int threshhold)
{
	cpu_debug_data *info = cpu_get_debug_data(device);

	/* if we already have tracking info, kill it */
	if (info->hotspots)
		free(info->hotspots);
	info->hotspots = NULL;

	/* only start tracking if we have a non-zero count */
	if (numspots > 0)
	{
		/* allocate memory for hotspots */
		info->hotspots = alloc_array_or_die(debug_hotspot_entry, numspots);
		memset(info->hotspots, 0xff, sizeof(*info->hotspots) * numspots);

		/* fill in the info */
		info->hotspot_count = numspots;
		info->hotspot_threshhold = threshhold;
	}

	watchpoint_update_flags(cpu_get_address_space(device, ADDRESS_SPACE_PROGRAM));
	return TRUE;
}



/***************************************************************************
    DEBUGGER MEMORY ACCESSORS
***************************************************************************/

/*-------------------------------------------------
    debug_read_byte - return a byte from the
    the specified memory space
-------------------------------------------------*/

UINT8 debug_read_byte(const address_space *space, offs_t address, int apply_translation)
{
	cpu_debug_data *info = (space->cpu->type == CPU) ? cpu_get_debug_data(space->cpu) : NULL;
	debugcpu_private *global = space->machine->debugcpu_data;
	UINT64 custom;
	UINT8 result;

	/* mask against the logical byte mask */
	address &= space->logbytemask;

	/* all accesses from this point on are for the debugger */
	memory_set_debugger_access(space, global->debugger_access = TRUE);

	/* translate if necessary; if not mapped, return 0xff */
	if (apply_translation && !debug_cpu_translate(space, TRANSLATE_READ_DEBUG, &address))
		result = 0xff;

	/* if there is a custom read handler, and it returns TRUE, use that value */
	else if (info != NULL && info->read != NULL && (*info->read)(space->cpu, space->spacenum, address, 1, &custom))
		result = custom;

	/* otherwise, call the byte reading function for the translated address */
	else
		result = memory_read_byte(space, address);

	/* no longer accessing via the debugger */
	memory_set_debugger_access(space, global->debugger_access = FALSE);
	return result;
}


/*-------------------------------------------------
    debug_read_word - return a word from the
    specified memory space
-------------------------------------------------*/

UINT16 debug_read_word(const address_space *space, offs_t address, int apply_translation)
{
	debugcpu_private *global = space->machine->debugcpu_data;
	UINT16 result;

	/* mask against the logical byte mask */
	address &= space->logbytemask;

	/* if this is misaligned read, or if there are no word readers, just read two bytes */
	if ((address & 1) != 0)
	{
		UINT8 byte0 = debug_read_byte(space, address + 0, apply_translation);
		UINT8 byte1 = debug_read_byte(space, address + 1, apply_translation);

		/* based on the endianness, the result is assembled differently */
		if (space->endianness == ENDIANNESS_LITTLE)
			result = byte0 | (byte1 << 8);
		else
			result = byte1 | (byte0 << 8);
	}

	/* otherwise, this proceeds like the byte case */
	else
	{
		cpu_debug_data *info = (space->cpu->type == CPU) ? cpu_get_debug_data(space->cpu) : NULL;
		UINT64 custom;

		/* all accesses from this point on are for the debugger */
		memory_set_debugger_access(space, global->debugger_access = TRUE);

		/* translate if necessary; if not mapped, return 0xffff */
		if (apply_translation && !debug_cpu_translate(space, TRANSLATE_READ_DEBUG, &address))
			result = 0xffff;

		/* if there is a custom read handler, and it returns TRUE, use that value */
		else if (info != NULL && info->read != NULL && (*info->read)(space->cpu, space->spacenum, address, 2, &custom))
			result = custom;

		/* otherwise, call the byte reading function for the translated address */
		else
			result = memory_read_word(space, address);

		/* no longer accessing via the debugger */
		memory_set_debugger_access(space, global->debugger_access = FALSE);
	}

	return result;
}


/*-------------------------------------------------
    debug_read_dword - return a dword from the
    specified memory space
-------------------------------------------------*/

UINT32 debug_read_dword(const address_space *space, offs_t address, int apply_translation)
{
	debugcpu_private *global = space->machine->debugcpu_data;
	UINT32 result;

	/* mask against the logical byte mask */
	address &= space->logbytemask;

	/* if this is misaligned read, or if there are no dword readers, just read two words */
	if ((address & 3) != 0)
	{
		UINT16 word0 = debug_read_word(space, address + 0, apply_translation);
		UINT16 word1 = debug_read_word(space, address + 2, apply_translation);

		/* based on the endianness, the result is assembled differently */
		if (space->endianness == ENDIANNESS_LITTLE)
			result = word0 | (word1 << 16);
		else
			result = word1 | (word0 << 16);
	}

	/* otherwise, this proceeds like the byte case */
	else
	{
		cpu_debug_data *info = (space->cpu->type == CPU) ? cpu_get_debug_data(space->cpu) : NULL;
		UINT64 custom;

		/* all accesses from this point on are for the debugger */
		memory_set_debugger_access(space, global->debugger_access = TRUE);

		/* translate if necessary; if not mapped, return 0xffffffff */
		if (apply_translation && !debug_cpu_translate(space, TRANSLATE_READ_DEBUG, &address))
			result = 0xffffffff;

		/* if there is a custom read handler, and it returns TRUE, use that value */
		else if (info != NULL && info->read != NULL && (*info->read)(space->cpu, space->spacenum, address, 4, &custom))
			result = custom;

		/* otherwise, call the byte reading function for the translated address */
		else
			result = memory_read_dword(space, address);

		/* no longer accessing via the debugger */
		memory_set_debugger_access(space, global->debugger_access = FALSE);
	}

	return result;
}


/*-------------------------------------------------
    debug_read_qword - return a qword from the
    specified memory space
-------------------------------------------------*/

UINT64 debug_read_qword(const address_space *space, offs_t address, int apply_translation)
{
	debugcpu_private *global = space->machine->debugcpu_data;
	UINT64 result;

	/* mask against the logical byte mask */
	address &= space->logbytemask;

	/* if this is misaligned read, or if there are no qword readers, just read two dwords */
	if ((address & 7) != 0)
	{
		UINT32 dword0 = debug_read_dword(space, address + 0, apply_translation);
		UINT32 dword1 = debug_read_dword(space, address + 4, apply_translation);

		/* based on the endianness, the result is assembled differently */
		if (space->endianness == ENDIANNESS_LITTLE)
			result = dword0 | ((UINT64)dword1 << 32);
		else
			result = dword1 | ((UINT64)dword0 << 32);
	}

	/* otherwise, this proceeds like the byte case */
	else
	{
		cpu_debug_data *info = (space->cpu->type == CPU) ? cpu_get_debug_data(space->cpu) : NULL;
		UINT64 custom;

		/* all accesses from this point on are for the debugger */
		memory_set_debugger_access(space, global->debugger_access = TRUE);

		/* translate if necessary; if not mapped, return 0xffffffffffffffff */
		if (apply_translation && !debug_cpu_translate(space, TRANSLATE_READ_DEBUG, &address))
			result = ~(UINT64)0;

		/* if there is a custom read handler, and it returns TRUE, use that value */
		else if (info != NULL && info->read != NULL && (*info->read)(space->cpu, space->spacenum, address, 8, &custom))
			result = custom;

		/* otherwise, call the byte reading function for the translated address */
		else
			result = memory_read_qword(space, address);

		/* no longer accessing via the debugger */
		memory_set_debugger_access(space, global->debugger_access = FALSE);
	}

	return result;
}


/*-------------------------------------------------
    debug_read_memory - return 1,2,4 or 8 bytes
    from the specified memory space
-------------------------------------------------*/

UINT64 debug_read_memory(const address_space *space, offs_t address, int size, int apply_translation)
{
	UINT64 result = ~(UINT64)0 >> (64 - 8*size);
	switch (size)
	{
		case 1:		result = debug_read_byte(space, address, apply_translation);	break;
		case 2:		result = debug_read_word(space, address, apply_translation);	break;
		case 4:		result = debug_read_dword(space, address, apply_translation);	break;
		case 8:		result = debug_read_qword(space, address, apply_translation);	break;
	}
	return result;
}


/*-------------------------------------------------
    debug_write_byte - write a byte to the
    specified memory space
-------------------------------------------------*/

void debug_write_byte(const address_space *space, offs_t address, UINT8 data, int apply_translation)
{
	cpu_debug_data *info = (space->cpu->type == CPU) ? cpu_get_debug_data(space->cpu) : NULL;
	debugcpu_private *global = space->machine->debugcpu_data;

	/* mask against the logical byte mask */
	address &= space->logbytemask;

	/* all accesses from this point on are for the debugger */
	memory_set_debugger_access(space, global->debugger_access = TRUE);

	/* translate if necessary; if not mapped, we're done */
	if (apply_translation && !debug_cpu_translate(space, TRANSLATE_WRITE_DEBUG, &address))
		;

	/* if there is a custom write handler, and it returns TRUE, use that */
	else if (info != NULL && info->write != NULL && (*info->write)(space->cpu, space->spacenum, address, 1, data))
		;

	/* otherwise, call the byte reading function for the translated address */
	else
		memory_write_byte(space, address, data);

	/* no longer accessing via the debugger */
	memory_set_debugger_access(space, global->debugger_access = FALSE);
	global->memory_modified = TRUE;
}


/*-------------------------------------------------
    debug_write_word - write a word to the
    specified memory space
-------------------------------------------------*/

void debug_write_word(const address_space *space, offs_t address, UINT16 data, int apply_translation)
{
	debugcpu_private *global = space->machine->debugcpu_data;

	/* mask against the logical byte mask */
	address &= space->logbytemask;

	/* if this is a misaligned write, or if there are no word writers, just read two bytes */
	if ((address & 1) != 0)
	{
		if (space->endianness == ENDIANNESS_LITTLE)
		{
			debug_write_byte(space, address + 0, data >> 0, apply_translation);
			debug_write_byte(space, address + 1, data >> 8, apply_translation);
		}
		else
		{
			debug_write_byte(space, address + 0, data >> 8, apply_translation);
			debug_write_byte(space, address + 1, data >> 0, apply_translation);
		}
	}

	/* otherwise, this proceeds like the byte case */
	else
	{
		cpu_debug_data *info = (space->cpu->type == CPU) ? cpu_get_debug_data(space->cpu) : NULL;

		/* all accesses from this point on are for the debugger */
		memory_set_debugger_access(space, global->debugger_access = TRUE);

		/* translate if necessary; if not mapped, we're done */
		if (apply_translation && !debug_cpu_translate(space, TRANSLATE_WRITE_DEBUG, &address))
			;

		/* if there is a custom write handler, and it returns TRUE, use that */
		else if (info != NULL && info->write != NULL && (*info->write)(space->cpu, space->spacenum, address, 2, data))
			;

		/* otherwise, call the byte reading function for the translated address */
		else
			memory_write_word(space, address, data);

		/* no longer accessing via the debugger */
		memory_set_debugger_access(space, global->debugger_access = FALSE);
		global->memory_modified = TRUE;
	}
}


/*-------------------------------------------------
    debug_write_dword - write a dword to the
    specified memory space
-------------------------------------------------*/

void debug_write_dword(const address_space *space, offs_t address, UINT32 data, int apply_translation)
{
	debugcpu_private *global = space->machine->debugcpu_data;

	/* mask against the logical byte mask */
	address &= space->logbytemask;

	/* if this is a misaligned write, or if there are no dword writers, just read two words */
	if ((address & 3) != 0)
	{
		if (space->endianness == ENDIANNESS_LITTLE)
		{
			debug_write_word(space, address + 0, data >> 0, apply_translation);
			debug_write_word(space, address + 2, data >> 16, apply_translation);
		}
		else
		{
			debug_write_word(space, address + 0, data >> 16, apply_translation);
			debug_write_word(space, address + 2, data >> 0, apply_translation);
		}
	}

	/* otherwise, this proceeds like the byte case */
	else
	{
		cpu_debug_data *info = (space->cpu->type == CPU) ? cpu_get_debug_data(space->cpu) : NULL;

		/* all accesses from this point on are for the debugger */
		memory_set_debugger_access(space, global->debugger_access = TRUE);

		/* translate if necessary; if not mapped, we're done */
		if (apply_translation && !debug_cpu_translate(space, TRANSLATE_WRITE_DEBUG, &address))
			;

		/* if there is a custom write handler, and it returns TRUE, use that */
		else if (info != NULL && info->write != NULL && (*info->write)(space->cpu, space->spacenum, address, 4, data))
			;

		/* otherwise, call the byte reading function for the translated address */
		else
			memory_write_dword(space, address, data);

		/* no longer accessing via the debugger */
		memory_set_debugger_access(space, global->debugger_access = FALSE);
		global->memory_modified = TRUE;
	}
}


/*-------------------------------------------------
    debug_write_qword - write a qword to the
    specified memory space
-------------------------------------------------*/

void debug_write_qword(const address_space *space, offs_t address, UINT64 data, int apply_translation)
{
	debugcpu_private *global = space->machine->debugcpu_data;

	/* mask against the logical byte mask */
	address &= space->logbytemask;

	/* if this is a misaligned write, or if there are no qword writers, just read two dwords */
	if ((address & 7) != 0)
	{
		if (space->endianness == ENDIANNESS_LITTLE)
		{
			debug_write_dword(space, address + 0, data >> 0, apply_translation);
			debug_write_dword(space, address + 4, data >> 32, apply_translation);
		}
		else
		{
			debug_write_dword(space, address + 0, data >> 32, apply_translation);
			debug_write_dword(space, address + 4, data >> 0, apply_translation);
		}
	}

	/* otherwise, this proceeds like the byte case */
	else
	{
		cpu_debug_data *info = (space->cpu->type == CPU) ? cpu_get_debug_data(space->cpu) : NULL;

		/* all accesses from this point on are for the debugger */
		memory_set_debugger_access(space, global->debugger_access = TRUE);

		/* translate if necessary; if not mapped, we're done */
		if (apply_translation && !debug_cpu_translate(space, TRANSLATE_WRITE_DEBUG, &address))
			;

		/* if there is a custom write handler, and it returns TRUE, use that */
		else if (info != NULL && info->write != NULL && (*info->write)(space->cpu, space->spacenum, address, 8, data))
			;

		/* otherwise, call the byte reading function for the translated address */
		else
			memory_write_qword(space, address, data);

		/* no longer accessing via the debugger */
		memory_set_debugger_access(space, global->debugger_access = FALSE);
		global->memory_modified = TRUE;
	}
}


/*-------------------------------------------------
    debug_write_memory - write 1,2,4 or 8 bytes
    to the specified memory space
-------------------------------------------------*/

void debug_write_memory(const address_space *space, offs_t address, UINT64 data, int size, int apply_translation)
{
	switch (size)
	{
		case 1:		debug_write_byte(space, address, data, apply_translation);	break;
		case 2:		debug_write_word(space, address, data, apply_translation);	break;
		case 4:		debug_write_dword(space, address, data, apply_translation);	break;
		case 8:		debug_write_qword(space, address, data, apply_translation);	break;
	}
}


/*-------------------------------------------------
    debug_read_opcode - read 1,2,4 or 8 bytes at
    the given offset from opcode space
-------------------------------------------------*/

UINT64 debug_read_opcode(const address_space *space, offs_t address, int size, int arg)
{
	UINT64 result = ~(UINT64)0 & (~(UINT64)0 >> (64 - 8*size)), result2;
	debugcpu_private *global = space->machine->debugcpu_data;
	cpu_debug_data *info = cpu_get_debug_data(space->cpu);

	/* keep in logical range */
	address &= space->logbytemask;

	/* shortcut if we have a custom routine */
	if (info->readop != NULL)
	{
		/* return early if we got the result directly */
		memory_set_debugger_access(space, global->debugger_access = TRUE);
		if ((*info->readop)(space->cpu, address, size, &result2))
		{
			memory_set_debugger_access(space, global->debugger_access = FALSE);
			return result2;
		}
	}

	/* if we're bigger than the address bus, break into smaller pieces */
	if (size > space->dbits / 8)
	{
		int halfsize = size / 2;
		UINT64 r0 = debug_read_opcode(space, address + 0, halfsize, arg);
		UINT64 r1 = debug_read_opcode(space, address + halfsize, halfsize, arg);

		if (space->endianness == ENDIANNESS_LITTLE)
			return r0 | (r1 << (8 * halfsize));
		else
			return r1 | (r0 << (8 * halfsize));
	}

	/* translate to physical first */
	if (!debug_cpu_translate(space, TRANSLATE_FETCH_DEBUG, &address))
		return result;

	/* keep in physical range */
	address &= space->bytemask;
	switch (space->dbits / 8 * 10 + size)
	{
		/* dump opcodes in bytes from a byte-sized bus */
		case 11:
			break;

		/* dump opcodes in bytes from a word-sized bus */
		case 21:
			address ^= (space->endianness == ENDIANNESS_LITTLE) ? BYTE_XOR_LE(0) : BYTE_XOR_BE(0);
			break;

		/* dump opcodes in words from a word-sized bus */
		case 22:
			break;

		/* dump opcodes in bytes from a dword-sized bus */
		case 41:
			address ^= (space->endianness == ENDIANNESS_LITTLE) ? BYTE4_XOR_LE(0) : BYTE4_XOR_BE(0);
			break;

		/* dump opcodes in words from a dword-sized bus */
		case 42:
			address ^= (space->endianness == ENDIANNESS_LITTLE) ? WORD_XOR_LE(0) : WORD_XOR_BE(0);
			break;

		/* dump opcodes in dwords from a dword-sized bus */
		case 44:
			break;

		/* dump opcodes in bytes from a qword-sized bus */
		case 81:
			address ^= (space->endianness == ENDIANNESS_LITTLE) ? BYTE8_XOR_LE(0) : BYTE8_XOR_BE(0);
			break;

		/* dump opcodes in words from a qword-sized bus */
		case 82:
			address ^= (space->endianness == ENDIANNESS_LITTLE) ? WORD2_XOR_LE(0) : WORD2_XOR_BE(0);
			break;

		/* dump opcodes in dwords from a qword-sized bus */
		case 84:
			address ^= (space->endianness == ENDIANNESS_LITTLE) ? DWORD_XOR_LE(0) : DWORD_XOR_BE(0);
			break;

		/* dump opcodes in qwords from a qword-sized bus */
		case 88:
			break;

		default:
			fatalerror("debug_read_opcode: unknown type = %d", space->dbits / 8 * 10 + size);
			break;
	}

	/* turn on debugger access */
	if (!global->debugger_access)
		memory_set_debugger_access(space, global->debugger_access = TRUE);

	/* switch off the size and handle unaligned accesses */
	switch (size)
	{
		case 1:
			result = (arg) ? memory_raw_read_byte(space, address) : memory_decrypted_read_byte(space, address);
			break;

		case 2:
			result = (arg) ? memory_raw_read_word(space, address & ~1) : memory_decrypted_read_word(space, address & ~1);
			if ((address & 1) != 0)
			{
				result2 = (arg) ? memory_raw_read_word(space, (address & ~1) + 2) : memory_decrypted_read_word(space, (address & ~1) + 2);
				if (space->endianness == ENDIANNESS_LITTLE)
					result = (result >> (8 * (address & 1))) | (result2 << (16 - 8 * (address & 1)));
				else
					result = (result << (8 * (address & 1))) | (result2 >> (16 - 8 * (address & 1)));
				result &= 0xffff;
			}
			break;

		case 4:
			result = (arg) ? memory_raw_read_dword(space, address & ~3) : memory_decrypted_read_dword(space, address & ~3);
			if ((address & 3) != 0)
			{
				result2 = (arg) ? memory_raw_read_dword(space, (address & ~3) + 4) : memory_decrypted_read_dword(space, (address & ~3) + 4);
				if (space->endianness == ENDIANNESS_LITTLE)
					result = (result >> (8 * (address & 3))) | (result2 << (32 - 8 * (address & 3)));
				else
					result = (result << (8 * (address & 3))) | (result2 >> (32 - 8 * (address & 3)));
				result &= 0xffffffff;
			}
			break;

		case 8:
			result = (arg) ? memory_raw_read_qword(space, address & ~7) : memory_decrypted_read_qword(space, address & ~7);
			if ((address & 7) != 0)
			{
				result2 = (arg) ? memory_raw_read_qword(space, (address & ~7) + 8) : memory_decrypted_read_qword(space, (address & ~7) + 8);
				if (space->endianness == ENDIANNESS_LITTLE)
					result = (result >> (8 * (address & 7))) | (result2 << (64 - 8 * (address & 7)));
				else
					result = (result << (8 * (address & 7))) | (result2 >> (64 - 8 * (address & 7)));
			}
			break;
	}

	/* no longer accessing via the debugger */
	memory_set_debugger_access(space, global->debugger_access = FALSE);
	return result;
}



/***************************************************************************
    INTERNAL HELPERS
***************************************************************************/

/*-------------------------------------------------
    debug_cpu_exit - free all memory
-------------------------------------------------*/

static void debug_cpu_exit(running_machine *machine)
{
	debugcpu_private *global = machine->debugcpu_data;
	const device_config *cpu;
	int spacenum;

	/* loop over all watchpoints and breakpoints to free their memory */
	for (cpu = machine->firstcpu; cpu != NULL; cpu = cpu_next(cpu))
	{
		cpu_debug_data *info = cpu_get_debug_data(cpu);

		/* close any tracefiles */
		if (info->trace.file != NULL)
			fclose(info->trace.file);
		if (info->trace.action != NULL)
			free(info->trace.action);

		/* free the symbol table */
		if (info->symtable != NULL)
			symtable_free(info->symtable);

		/* free all breakpoints */
		while (info->bplist != NULL)
			debug_cpu_breakpoint_clear(machine, info->bplist->index);

		/* loop over all address spaces */
		for (spacenum = 0; spacenum < ARRAY_LENGTH(info->wplist); spacenum++)
		{
			/* free all watchpoints */
			while (info->wplist[spacenum] != NULL)
				debug_cpu_watchpoint_clear(machine, info->wplist[spacenum]->index);
		}
	}

	/* free the global symbol table */
	if (global != NULL && global->symtable != NULL)
		symtable_free(global->symtable);
}


/*-------------------------------------------------
    on_vblank - called when a VBLANK hits
-------------------------------------------------*/

static void on_vblank(const device_config *device, void *param, int vblank_state)
{
	/* just set a global flag to be consumed later */
	if (vblank_state)
		device->machine->debugcpu_data->vblank_occurred = TRUE;
}


/*-------------------------------------------------
    reset_transient_flags - reset the transient
    flags on all CPUs
-------------------------------------------------*/

static void reset_transient_flags(running_machine *machine)
{
	const device_config *cpu;

	/* loop over CPUs and reset the transient flags */
	for (cpu = machine->firstcpu; cpu != NULL; cpu = cpu_next(cpu))
		cpu_get_debug_data(cpu)->flags &= ~DEBUG_FLAG_TRANSIENT;
}


/*-------------------------------------------------
    compute_debug_flags - compute the global
    debug flags for optimal efficiency
-------------------------------------------------*/

static void compute_debug_flags(const device_config *device)
{
	cpu_debug_data *info = cpu_get_debug_data(device);
	running_machine *machine = device->machine;
	debugcpu_private *global = machine->debugcpu_data;

	/* clear out all global flags by default */
	machine->debug_flags = DEBUG_FLAG_ENABLED;

	/* if we are ignoring this CPU, or if events are pending, we're done */
	if ((info->flags & DEBUG_FLAG_OBSERVING) == 0 || mame_is_scheduled_event_pending(machine) || mame_is_save_or_load_pending(machine))
		return;

	/* many of our states require us to be called on each instruction */
	if (global->execution_state == EXECUTION_STATE_STOPPED)
		machine->debug_flags |= DEBUG_FLAG_CALL_HOOK;
	if ((info->flags & (DEBUG_FLAG_HISTORY | DEBUG_FLAG_TRACING_ANY | DEBUG_FLAG_HOOKED |
						DEBUG_FLAG_STEPPING_ANY | DEBUG_FLAG_STOP_PC | DEBUG_FLAG_LIVE_BP)) != 0)
		machine->debug_flags |= DEBUG_FLAG_CALL_HOOK;

	/* if we are stopping at a particular time and that time is within the current timeslice, we need to be called */
	if ((info->flags & DEBUG_FLAG_STOP_TIME) && attotime_compare(info->endexectime, info->stoptime) <= 0)
		machine->debug_flags |= DEBUG_FLAG_CALL_HOOK;
}


/*-------------------------------------------------
    perform_trace - log to the tracefile the
    data for a given instruction
-------------------------------------------------*/

static void perform_trace(cpu_debug_data *info)
{
	offs_t pc = cpu_get_pc(info->device);
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
		const address_space *space = cpu_get_address_space(info->device, ADDRESS_SPACE_PROGRAM);

		/* if we just finished looping, indicate as much */
		if (info->trace.loops != 0)
			fprintf(info->trace.file, "\n   (loops for %d instructions)\n\n", info->trace.loops);
		info->trace.loops = 0;

		/* execute any trace actions first */
		if (info->trace.action != NULL)
			debug_console_execute_command(info->device->machine, info->trace.action, 0);

		/* print the address */
		offset = sprintf(buffer, "%0*X: ", space->logaddrchars, pc);

		/* print the disassembly */
		dasmresult = dasm_wrapped(info->device, &buffer[offset], pc);

		/* output the result */
		fprintf(info->trace.file, "%s\n", buffer);

		/* do we need to step the trace over this instruction? */
		if ((info->flags & DEBUG_FLAG_TRACING_OVER) != 0 && (dasmresult & DASMFLAG_SUPPORTED) != 0 && (dasmresult & DASMFLAG_STEP_OVER) != 0)
		{
			int extraskip = (dasmresult & DASMFLAG_OVERINSTMASK) >> DASMFLAG_OVERINSTSHIFT;
			offs_t trace_over_target = pc + (dasmresult & DASMFLAG_LENGTHMASK);

			/* if we need to skip additional instructions, advance as requested */
			while (extraskip-- > 0)
				trace_over_target += dasm_wrapped(info->device, buffer, trace_over_target) & DASMFLAG_LENGTHMASK;

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

static void prepare_for_step_overout(cpu_debug_data *info)
{
	offs_t pc = cpu_get_pc(info->device);
	char dasmbuffer[100];
	offs_t dasmresult;

	/* disassemble the current instruction and get the flags */
	dasmresult = dasm_wrapped(info->device, dasmbuffer, pc);

	/* if flags are supported and it's a call-style opcode, set a temp breakpoint after that instruction */
	if ((dasmresult & DASMFLAG_SUPPORTED) != 0 && (dasmresult & DASMFLAG_STEP_OVER) != 0)
	{
		int extraskip = (dasmresult & DASMFLAG_OVERINSTMASK) >> DASMFLAG_OVERINSTSHIFT;
		pc += dasmresult & DASMFLAG_LENGTHMASK;

		/* if we need to skip additional instructions, advance as requested */
		while (extraskip-- > 0)
			pc += dasm_wrapped(info->device, dasmbuffer, pc) & DASMFLAG_LENGTHMASK;
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

static void process_source_file(running_machine *machine)
{
	debugcpu_private *global = machine->debugcpu_data;

	/* loop until the file is exhausted or until we are executing again */
	while (global->source_file != NULL && global->execution_state == EXECUTION_STATE_STOPPED)
	{
		char buf[512];
		int i;
		char *s;

		/* stop at the end of file */
		if (feof(global->source_file))
		{
			fclose(global->source_file);
			global->source_file = NULL;
			return;
		}

		/* fetch the next line */
		memset(buf, 0, sizeof(buf));
		fgets(buf, sizeof(buf), global->source_file);

		/* strip out comments (text after '//') */
		s = strstr(buf, "//");
		if (s)
			*s = '\0';

		/* strip whitespace */
		i = (int)strlen(buf);
		while((i > 0) && (isspace((UINT8)buf[i-1])))
			buf[--i] = '\0';

		/* execute the command */
		if (buf[0])
			debug_console_execute_command(machine, buf, 1);
	}
}


/*-------------------------------------------------
    breakpoint_update_flags - update the CPU's
    breakpoint flags
-------------------------------------------------*/

static void breakpoint_update_flags(cpu_debug_data *info)
{
	debugcpu_private *global = info->device->machine->debugcpu_data;
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
	if (global->livecpu != NULL)
		compute_debug_flags(global->livecpu);
}


/*-------------------------------------------------
    breakpoint_check - check the breakpoints for
    a given CPU
-------------------------------------------------*/

static void breakpoint_check(running_machine *machine, cpu_debug_data *info, offs_t pc)
{
	debugcpu_private *global = machine->debugcpu_data;
	debug_cpu_breakpoint *bp;
	UINT64 result;

	/* see if we match */
	for (bp = info->bplist; bp != NULL; bp = bp->next)
		if (bp->enabled && bp->address == pc)

			/* if we do, evaluate the condition */
			if (bp->condition == NULL || (expression_execute(bp->condition, &result) == EXPRERR_NONE && result != 0))
			{
				/* halt in the debugger by default */
				global->execution_state = EXECUTION_STATE_STOPPED;

				/* if we hit, evaluate the action */
				if (bp->action != NULL)
					debug_console_execute_command(machine, bp->action, 0);

				/* print a notification, unless the action made us go again */
				if (global->execution_state == EXECUTION_STATE_STOPPED)
					debug_console_printf(machine, "Stopped at breakpoint %X\n", bp->index);
				break;
			}
}


/*-------------------------------------------------
    watchpoint_update_flags - update the CPU's
    watchpoint flags
-------------------------------------------------*/

static void watchpoint_update_flags(const address_space *space)
{
	const cpu_debug_data *info = cpu_get_debug_data(space->cpu);
	debug_cpu_watchpoint *wp;
	int enablewrite = FALSE;
	int enableread = FALSE;

	/* if hotspots are enabled, turn on all reads */
	if (info->hotspots != NULL)
		enableread = TRUE;

	/* see if there are any enabled breakpoints */
	for (wp = info->wplist[space->spacenum]; wp != NULL; wp = wp->next)
		if (wp->enabled)
		{
			if (wp->type & WATCHPOINT_READ)
				enableread = TRUE;
			if (wp->type & WATCHPOINT_WRITE)
				enablewrite = TRUE;
		}

	/* push the flags out globally */
	memory_enable_read_watchpoints(space, enableread);
	memory_enable_write_watchpoints(space, enablewrite);
}


/*-------------------------------------------------
    watchpoint_check - check the watchpoints
    for a given CPU and address space
-------------------------------------------------*/

static void watchpoint_check(const address_space *space, int type, offs_t address, UINT64 value_to_write, UINT64 mem_mask)
{
	const cpu_debug_data *info = cpu_get_debug_data(space->cpu);
	debugcpu_private *global = space->machine->debugcpu_data;
	debug_cpu_watchpoint *wp;
	offs_t size = 0;
	UINT64 result;

	/* if we're within debugger code, don't stop */
	if (global->within_instruction_hook || global->debugger_access)
		return;

	global->within_instruction_hook = TRUE;

	/* adjust address, size & value_to_write based on mem_mask. */
	if (mem_mask != 0)
	{
		int bus_size = space->dbits / 8;
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

		if (space->endianness == ENDIANNESS_LITTLE)
			address += address_offset;
		else
			address += bus_size - size - address_offset;
	}

	/* if we are a write watchpoint, stash the value that will be written */
	global->wpaddr = address;
	if (type & WATCHPOINT_WRITE)
		global->wpdata = value_to_write;

	/* see if we match */
	for (wp = info->wplist[space->spacenum]; wp != NULL; wp = wp->next)
		if (wp->enabled && (wp->type & type) != 0 && address + size > wp->address && address < wp->address + wp->length)

			/* if we do, evaluate the condition */
			if (wp->condition == NULL || (expression_execute(wp->condition, &result) == EXPRERR_NONE && result != 0))
			{
				/* halt in the debugger by default */
				global->execution_state = EXECUTION_STATE_STOPPED;

				/* if we hit, evaluate the action */
				if (wp->action != NULL)
					debug_console_execute_command(space->machine, wp->action, 0);

				/* print a notification, unless the action made us go again */
				if (global->execution_state == EXECUTION_STATE_STOPPED)
				{
					static const char *const sizes[] =
					{
						"0bytes", "byte", "word", "3bytes", "dword", "5bytes", "6bytes", "7bytes", "qword"
					};
					char buffer[100];

					if (type & WATCHPOINT_WRITE)
					{
						sprintf(buffer, "Stopped at watchpoint %X writing %s to %08X (PC=%X)", wp->index, sizes[size], memory_byte_to_address(space, address), cpu_get_pc(space->cpu));
						if (value_to_write >> 32)
							sprintf(&buffer[strlen(buffer)], " (data=%X%08X)", (UINT32)(value_to_write >> 32), (UINT32)value_to_write);
						else
							sprintf(&buffer[strlen(buffer)], " (data=%X)", (UINT32)value_to_write);
					}
					else
						sprintf(buffer, "Stopped at watchpoint %X reading %s from %08X (PC=%X)", wp->index, sizes[size], memory_byte_to_address(space, address), cpu_get_pc(space->cpu));
					debug_console_printf(space->machine, "%s\n", buffer);
					compute_debug_flags(space->cpu);
				}
				break;
			}

	global->within_instruction_hook = FALSE;
}


/*-------------------------------------------------
    check_hotspots - check for
    hotspots on a memory read access
-------------------------------------------------*/

static void check_hotspots(const address_space *space, offs_t address)
{
	cpu_debug_data *info = cpu_get_debug_data(space->cpu);
	offs_t pc = cpu_get_pc(space->cpu);
	int hotindex;

	/* see if we have a match in our list */
	for (hotindex = 0; hotindex < info->hotspot_count; hotindex++)
		if (info->hotspots[hotindex].access == address && info->hotspots[hotindex].pc == pc && info->hotspots[hotindex].space == space)
			break;

	/* if we didn't find any, make a new entry */
	if (hotindex == info->hotspot_count)
	{
		/* if the bottom of the list is over the threshhold, print it */
		debug_hotspot_entry *spot = &info->hotspots[info->hotspot_count - 1];
		if (spot->count > info->hotspot_threshhold)
			debug_console_printf(space->machine, "Hotspot @ %s %08X (PC=%08X) hit %d times (fell off bottom)\n", space->name, spot->access, spot->pc, spot->count);

		/* move everything else down and insert this one at the top */
		memmove(&info->hotspots[1], &info->hotspots[0], sizeof(info->hotspots[0]) * (info->hotspot_count - 1));
		info->hotspots[0].access = address;
		info->hotspots[0].pc = pc;
		info->hotspots[0].space = space;
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


/*-------------------------------------------------
    dasm_wrapped - wraps calls to the disassembler
    by fetching the opcode bytes to a temporary
    buffer and then disassembling them
-------------------------------------------------*/

static UINT32 dasm_wrapped(const device_config *device, char *buffer, offs_t pc)
{
	const address_space *space = cpu_get_address_space(device, ADDRESS_SPACE_PROGRAM);
	int maxbytes = cpu_get_max_opcode_bytes(device);
	UINT8 opbuf[64], argbuf[64];
	offs_t pcbyte;
	int numbytes;

	/* fetch the bytes up to the maximum */
	pcbyte = memory_address_to_byte(space, pc) & space->bytemask;
	for (numbytes = 0; numbytes < maxbytes; numbytes++)
	{
		opbuf[numbytes] = debug_read_opcode(space, pcbyte + numbytes, 1, FALSE);
		argbuf[numbytes] = debug_read_opcode(space, pcbyte + numbytes, 1, TRUE);
	}

	return debug_cpu_disassemble(device, buffer, pc, opbuf, argbuf);
}



/***************************************************************************
    EXPRESSION HANDLERS
***************************************************************************/

/*-------------------------------------------------
    expression_cpu_index - return the CPU index
    based on a case insensitive tag search
-------------------------------------------------*/

static const device_config *expression_cpu_index(running_machine *machine, const char *tag)
{
	const device_config *cpu;

	for (cpu = machine->firstcpu; cpu != NULL; cpu = cpu_next(cpu))
		if (mame_stricmp(cpu->tag, tag) == 0)
			return cpu;

	return NULL;
}


/*-------------------------------------------------
    expression_read_memory - read 1,2,4 or 8 bytes
    at the given offset in the given address
    space
-------------------------------------------------*/

static UINT64 expression_read_memory(void *param, const char *name, int spacenum, UINT32 address, int size)
{
	running_machine *machine = (running_machine *)param;
	UINT64 result = ~(UINT64)0 >> (64 - 8*size);
	const device_config *cpu = NULL;
	const address_space *space;

	switch (spacenum)
	{
		case EXPSPACE_PROGRAM_LOGICAL:
		case EXPSPACE_DATA_LOGICAL:
		case EXPSPACE_IO_LOGICAL:
		case EXPSPACE_SPACE3_LOGICAL:
			if (name != NULL)
				cpu = expression_cpu_index(machine, name);
			if (cpu == NULL)
				cpu = debug_cpu_get_visible_cpu(machine);
			space = cpu_get_address_space(cpu, ADDRESS_SPACE_PROGRAM + (spacenum - EXPSPACE_PROGRAM_LOGICAL));
			if (space != NULL)
				result = debug_read_memory(space, memory_address_to_byte(space, address), size, TRUE);
			break;

		case EXPSPACE_PROGRAM_PHYSICAL:
		case EXPSPACE_DATA_PHYSICAL:
		case EXPSPACE_IO_PHYSICAL:
		case EXPSPACE_SPACE3_PHYSICAL:
			if (name != NULL)
				cpu = expression_cpu_index(machine, name);
			if (cpu == NULL)
				cpu = debug_cpu_get_visible_cpu(machine);
			space = cpu_get_address_space(cpu, ADDRESS_SPACE_PROGRAM + (spacenum - EXPSPACE_PROGRAM_PHYSICAL));
			if (space != NULL)
				result = debug_read_memory(space, memory_address_to_byte(space, address), size, FALSE);
			break;

		case EXPSPACE_OPCODE:
		case EXPSPACE_RAMWRITE:
			if (name != NULL)
				cpu = expression_cpu_index(machine, name);
			if (cpu == NULL)
				cpu = debug_cpu_get_visible_cpu(machine);
			result = expression_read_program_direct(cpu_get_address_space(cpu, ADDRESS_SPACE_PROGRAM), (spacenum == EXPSPACE_OPCODE), address, size);
			break;

		case EXPSPACE_EEPROM:
			result = expression_read_eeprom(machine, address, size);
			break;

		case EXPSPACE_REGION:
			if (name == NULL)
				break;
			result = expression_read_memory_region(machine, name, address, size);
			break;
	}
	return result;
}


/*-------------------------------------------------
    expression_read_program_direct - read memory
    directly from an opcode or RAM pointer
-------------------------------------------------*/

static UINT64 expression_read_program_direct(const address_space *space, int opcode, offs_t address, int size)
{
	UINT64 result = ~(UINT64)0 >> (64 - 8*size);

	if (space != NULL)
	{
		UINT8 *base;

		/* adjust the address into a byte address, but not if being called recursively */
		if ((opcode & 2) == 0)
			address = memory_address_to_byte(space, address);

		/* call ourself recursively until we are byte-sized */
		if (size > 1)
		{
			int halfsize = size / 2;
			UINT64 r0, r1;

			/* read each half, from lower address to upper address */
			r0 = expression_read_program_direct(space, opcode | 2, address + 0, halfsize);
			r1 = expression_read_program_direct(space, opcode | 2, address + halfsize, halfsize);

			/* assemble based on the target endianness */
			if (space->endianness == ENDIANNESS_LITTLE)
				result = r0 | (r1 << (8 * halfsize));
			else
				result = r1 | (r0 << (8 * halfsize));
		}

		/* handle the byte-sized final requests */
		else
		{
			/* lowmask specified which address bits are within the databus width */
			offs_t lowmask = space->dbits / 8 - 1;

			/* get the base of memory, aligned to the address minus the lowbits */
			if (opcode & 1)
				base = (UINT8 *)memory_decrypted_read_ptr(space, address & ~lowmask);
			else
				base = (UINT8 *)memory_get_read_ptr(space, address & ~lowmask);

			/* if we have a valid base, return the appropriate byte */
			if (base != NULL)
			{
				if (space->endianness == ENDIANNESS_LITTLE)
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

static UINT64 expression_read_memory_region(running_machine *machine, const char *rgntag, offs_t address, int size)
{
	UINT8 *base = memory_region(machine, rgntag);
	UINT64 result = ~(UINT64)0 >> (64 - 8*size);

	/* make sure we get a valid base before proceeding */
	if (base != NULL)
	{
		UINT32 length = memory_region_length(machine, rgntag);
		UINT32 flags = memory_region_flags(machine, rgntag);

		/* call ourself recursively until we are byte-sized */
		if (size > 1)
		{
			int halfsize = size / 2;
			UINT64 r0, r1;

			/* read each half, from lower address to upper address */
			r0 = expression_read_memory_region(machine, rgntag, address + 0, halfsize);
			r1 = expression_read_memory_region(machine, rgntag, address + halfsize, halfsize);

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

static UINT64 expression_read_eeprom(running_machine *machine, offs_t address, int size)
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

static void expression_write_memory(void *param, const char *name, int spacenum, UINT32 address, int size, UINT64 data)
{
	running_machine *machine = (running_machine *)param;
	const device_config *cpu = NULL;
	const address_space *space;

	switch (spacenum)
	{
		case EXPSPACE_PROGRAM_LOGICAL:
		case EXPSPACE_DATA_LOGICAL:
		case EXPSPACE_IO_LOGICAL:
		case EXPSPACE_SPACE3_LOGICAL:
			if (name != NULL)
				cpu = expression_cpu_index(machine, name);
			if (cpu == NULL)
				cpu = debug_cpu_get_visible_cpu(machine);
			space = cpu_get_address_space(cpu, ADDRESS_SPACE_PROGRAM + (spacenum - EXPSPACE_PROGRAM_LOGICAL));
			if (space != NULL)
				debug_write_memory(space, memory_address_to_byte(space, address), data, size, TRUE);
			break;

		case EXPSPACE_PROGRAM_PHYSICAL:
		case EXPSPACE_DATA_PHYSICAL:
		case EXPSPACE_IO_PHYSICAL:
		case EXPSPACE_SPACE3_PHYSICAL:
			if (name != NULL)
				cpu = expression_cpu_index(machine, name);
			if (cpu == NULL)
				cpu = debug_cpu_get_visible_cpu(machine);
			space = cpu_get_address_space(cpu, ADDRESS_SPACE_PROGRAM + (spacenum - EXPSPACE_PROGRAM_PHYSICAL));
			if (space != NULL)
				debug_write_memory(space, memory_address_to_byte(space, address), data, size, FALSE);
			break;

		case EXPSPACE_OPCODE:
		case EXPSPACE_RAMWRITE:
			if (name != NULL)
				cpu = expression_cpu_index(machine, name);
			if (cpu == NULL)
				cpu = debug_cpu_get_visible_cpu(machine);
			expression_write_program_direct(cpu_get_address_space(cpu, ADDRESS_SPACE_PROGRAM), (spacenum == EXPSPACE_OPCODE), address, size, data);
			break;

		case EXPSPACE_EEPROM:
			expression_write_eeprom(machine, address, size, data);
			break;

		case EXPSPACE_REGION:
			if (name == NULL)
				break;
			expression_write_memory_region(machine, name, address, size, data);
			break;
	}
}


/*-------------------------------------------------
    expression_write_program_direct - write memory
    directly to an opcode or RAM pointer
-------------------------------------------------*/

static void expression_write_program_direct(const address_space *space, int opcode, offs_t address, int size, UINT64 data)
{
	if (space != NULL)
	{
		debugcpu_private *global = space->machine->debugcpu_data;
		UINT8 *base;

		/* adjust the address into a byte address, but not if being called recursively */
		if ((opcode & 2) == 0)
			address = memory_address_to_byte(space, address);

		/* call ourself recursively until we are byte-sized */
		if (size > 1)
		{
			int halfsize = size / 2;
			UINT64 r0, r1, halfmask;

			/* break apart based on the target endianness */
			halfmask = ~(UINT64)0 >> (64 - 8 * halfsize);
			if (space->endianness == ENDIANNESS_LITTLE)
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
			expression_write_program_direct(space, opcode | 2, address + 0, halfsize, r0);
			expression_write_program_direct(space, opcode | 2, address + halfsize, halfsize, r1);
		}

		/* handle the byte-sized final case */
		else
		{
			/* lowmask specified which address bits are within the databus width */
			offs_t lowmask = space->dbits / 8 - 1;

			/* get the base of memory, aligned to the address minus the lowbits */
			if (opcode & 1)
				base = (UINT8 *)memory_decrypted_read_ptr(space, address & ~lowmask);
			else
				base = (UINT8 *)memory_get_read_ptr(space, address & ~lowmask);

			/* if we have a valid base, write the appropriate byte */
			if (base != NULL)
			{
				if (space->endianness == ENDIANNESS_LITTLE)
					base[BYTE8_XOR_LE(address) & lowmask] = data;
				else
					base[BYTE8_XOR_BE(address) & lowmask] = data;
				global->memory_modified = TRUE;
			}
		}
	}
}


/*-------------------------------------------------
    expression_write_memory_region - write memory
    from a memory region
-------------------------------------------------*/

static void expression_write_memory_region(running_machine *machine, const char *rgntag, offs_t address, int size, UINT64 data)
{
	debugcpu_private *global = machine->debugcpu_data;
	UINT8 *base = memory_region(machine, rgntag);

	/* make sure we get a valid base before proceeding */
	if (base != NULL)
	{
		UINT32 length = memory_region_length(machine, rgntag);
		UINT32 flags = memory_region_flags(machine, rgntag);

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
			expression_write_memory_region(machine, rgntag, address + 0, halfsize, r0);
			expression_write_memory_region(machine, rgntag, address + halfsize, halfsize, r1);
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
			global->memory_modified = TRUE;
		}
	}
}


/*-------------------------------------------------
    expression_write_eeprom - write EEPROM data
-------------------------------------------------*/

static void expression_write_eeprom(running_machine *machine, offs_t address, int size, UINT64 data)
{
	debugcpu_private *global = machine->debugcpu_data;
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
		global->memory_modified = TRUE;
	}
}


/*-------------------------------------------------
    expression_validate - validate that the
    provided expression references an
    appropriate name
-------------------------------------------------*/

static EXPRERR expression_validate(void *param, const char *name, int space)
{
	running_machine *machine = (running_machine *)param;
	const device_config *cpu = NULL;

	switch (space)
	{
		case EXPSPACE_PROGRAM_LOGICAL:
		case EXPSPACE_DATA_LOGICAL:
		case EXPSPACE_IO_LOGICAL:
		case EXPSPACE_SPACE3_LOGICAL:
			if (name != NULL)
			{
				cpu = expression_cpu_index(machine, name);
				if (cpu == NULL)
					return EXPRERR_INVALID_MEMORY_NAME;
			}
			if (cpu == NULL)
				cpu = debug_cpu_get_visible_cpu(machine);
			if (cpu_get_address_space(cpu, ADDRESS_SPACE_PROGRAM + (space - EXPSPACE_PROGRAM_LOGICAL)) == NULL)
				return EXPRERR_NO_SUCH_MEMORY_SPACE;
			break;

		case EXPSPACE_PROGRAM_PHYSICAL:
		case EXPSPACE_DATA_PHYSICAL:
		case EXPSPACE_IO_PHYSICAL:
		case EXPSPACE_SPACE3_PHYSICAL:
			if (name != NULL)
			{
				cpu = expression_cpu_index(machine, name);
				if (cpu == NULL)
					return EXPRERR_INVALID_MEMORY_NAME;
			}
			if (cpu == NULL)
				cpu = debug_cpu_get_visible_cpu(machine);
			if (cpu_get_address_space(cpu, ADDRESS_SPACE_PROGRAM + (space - EXPSPACE_PROGRAM_PHYSICAL)) == NULL)
				return EXPRERR_NO_SUCH_MEMORY_SPACE;
			break;

		case EXPSPACE_OPCODE:
		case EXPSPACE_RAMWRITE:
			if (name != NULL)
			{
				cpu = expression_cpu_index(machine, name);
				if (cpu == NULL)
					return EXPRERR_INVALID_MEMORY_NAME;
			}
			if (cpu == NULL)
				cpu = debug_cpu_get_visible_cpu(machine);
			if (cpu_get_address_space(cpu, ADDRESS_SPACE_PROGRAM) == NULL)
				return EXPRERR_NO_SUCH_MEMORY_SPACE;
			break;

		case EXPSPACE_EEPROM:
			if (name != NULL)
				return EXPRERR_INVALID_MEMORY_NAME;
			break;

		case EXPSPACE_REGION:
			if (name == NULL)
				return EXPRERR_MISSING_MEMORY_NAME;
			if (memory_region(machine, name) == NULL)
				return EXPRERR_INVALID_MEMORY_NAME;
			break;
	}
	return EXPRERR_NONE;
}



/***************************************************************************
    VARIABLE GETTERS/SETTERS
***************************************************************************/

/*-------------------------------------------------
    get_wpaddr - getter callback for the
    'wpaddr' symbol
-------------------------------------------------*/

static UINT64 get_wpaddr(void *globalref, void *ref)
{
	running_machine *machine = (running_machine *)globalref;
	return machine->debugcpu_data->wpaddr;
}


/*-------------------------------------------------
    get_wpdata - getter callback for the
    'wpdata' symbol
-------------------------------------------------*/

static UINT64 get_wpdata(void *globalref, void *ref)
{
	running_machine *machine = (running_machine *)globalref;
	return machine->debugcpu_data->wpdata;
}


/*-------------------------------------------------
    get_cpunum - getter callback for the
    'cpunum' symbol
-------------------------------------------------*/

static UINT64 get_cpunum(void *globalref, void *ref)
{
	running_machine *machine = (running_machine *)globalref;
	return cpu_get_index(machine->debugcpu_data->visiblecpu);
}


/*-------------------------------------------------
    get_tempvar - getter callback for the
    'tempX' symbols
-------------------------------------------------*/

static UINT64 get_tempvar(void *globalref, void *ref)
{
	return *(UINT64 *)ref;
}


/*-------------------------------------------------
    set_tempvar - setter callback for the
    'tempX' symbols
-------------------------------------------------*/

static void set_tempvar(void *globalref, void *ref, UINT64 value)
{
	*(UINT64 *)ref = value;
}


/*-------------------------------------------------
    get_beamx - get beam horizontal position
-------------------------------------------------*/

static UINT64 get_beamx(void *globalref, void *ref)
{
	const device_config *screen = (const device_config *)ref;
	return (screen != NULL) ? video_screen_get_hpos(screen) : 0;
}


/*-------------------------------------------------
    get_beamy - get beam vertical position
-------------------------------------------------*/

static UINT64 get_beamy(void *globalref, void *ref)
{
	const device_config *screen = (const device_config *)ref;
	return (screen != NULL) ? video_screen_get_vpos(screen) : 0;
}


/*-------------------------------------------------
    get_frame - get current frame number
-------------------------------------------------*/

static UINT64 get_frame(void *globalref, void *ref)
{
	const device_config *screen = (const device_config *)ref;
	return (screen != NULL) ? video_screen_get_frame_number(screen) : 0;
}


/*-------------------------------------------------
    get_current_pc - getter callback for a CPU's
    current instruction pointer
-------------------------------------------------*/

static UINT64 get_current_pc(void *globalref, void *ref)
{
	const device_config *device = (const device_config *)globalref;
	return cpu_get_pc(device);
}


/*-------------------------------------------------
    get_cycles - getter callback for the
    'cycles' symbol
-------------------------------------------------*/

static UINT64 get_cycles(void *globalref, void *ref)
{
	const device_config *device = (const device_config *)globalref;
	return *cpu_get_icount_ptr(device);
}


/*-------------------------------------------------
    get_logunmap - getter callback for the logumap
    symbols
-------------------------------------------------*/

static UINT64 get_logunmap(void *globalref, void *ref)
{
	const address_space *space = (const address_space *)ref;
	return (space != NULL) ? memory_get_log_unmap(space) : TRUE;
}


/*-------------------------------------------------
    set_logunmap - setter callback for the logumap
    symbols
-------------------------------------------------*/

static void set_logunmap(void *globalref, void *ref, UINT64 value)
{
	const address_space *space = (const address_space *)ref;
	if (space != NULL)
		memory_set_log_unmap(space, value ? 1 : 0);
}


/*-------------------------------------------------
    get_cpu_reg - getter callback for a CPU's
    register symbols
-------------------------------------------------*/

static UINT64 get_cpu_reg(void *globalref, void *ref)
{
	const device_config *device = (const device_config *)globalref;
	return cpu_get_reg(device, (FPTR)ref);
}


/*-------------------------------------------------
    set_cpu_reg - setter callback for a CPU's
    register symbols
-------------------------------------------------*/

static void set_cpu_reg(void *globalref, void *ref, UINT64 value)
{
	const device_config *device = (const device_config *)globalref;
	cpu_set_reg(device, (FPTR)ref, value);
}
