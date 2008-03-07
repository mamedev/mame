/*********************************************************************

    debugcpu.c

    Debugger CPU/memory interface engine.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "osdepend.h"
#include "driver.h"
#include "debugcpu.h"
#include "debugcmd.h"
#include "debugcmt.h"
#include "debugcon.h"
#include "express.h"
#include "debugvw.h"
#include "deprecat.h"
#include <ctype.h>



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define NUM_TEMP_VARIABLES	10



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/



/***************************************************************************
    LOCAL VARIABLES
***************************************************************************/

FILE *debug_source_file;
symbol_table *global_symtable;

static UINT64 wpdata;
static UINT64 wpaddr;

static int execution_state;
static UINT32 execution_counter;
static int next_index = 1;
static int within_debugger_code = FALSE;
static int last_cpunum;
static int last_stopped_cpunum;
static int steps_until_stop;
static offs_t step_overout_breakpoint;
static int step_overout_cpunum;
static int key_check_counter;
static osd_ticks_t last_periodic_update_time;
static int break_on_vblank;
static int break_on_interrupt;
static int break_on_interrupt_cpunum;
static int break_on_interrupt_irqline;
static int break_on_time;
static attotime break_on_time_target;
static int memory_modified;
static int memory_hook_cpunum;

static debug_cpu_info debug_cpuinfo[MAX_CPU];

static UINT64 tempvar[NUM_TEMP_VARIABLES];



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void debug_cpu_exit(running_machine *machine);
static void perform_trace(debug_cpu_info *info);
static void prepare_for_step_overout(void);
static void process_source_file(void);
static UINT64 get_wpaddr(UINT32 ref);
static UINT64 get_wpdata(UINT32 ref);
static UINT64 get_cycles(UINT32 ref);
static UINT64 get_cpunum(UINT32 ref);
static UINT64 get_tempvar(UINT32 ref);
static UINT64 get_logunmap(UINT32 ref);
static UINT64 get_beamx(UINT32 ref);
static UINT64 get_beamy(UINT32 ref);
static void set_tempvar(UINT32 ref, UINT64 value);
static void set_logunmap(UINT32 ref, UINT64 value);
static UINT64 get_current_pc(UINT32 ref);
static UINT64 get_cpu_reg(UINT32 ref);
static void set_cpu_reg(UINT32 ref, UINT64 value);
static void check_watchpoints(int cpunum, int spacenum, int type, offs_t address, offs_t size, UINT64 value_to_write);
static void check_hotspots(int cpunum, int spacenum, offs_t address);



/***************************************************************************
    FRONTENDS FOR OLDER FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    mame_debug_init - start up all subsections
-------------------------------------------------*/

void mame_debug_init(running_machine *machine)
{
	/* initialize the various subsections */
	debug_cpu_init(machine);
	debug_command_init(machine);
	debug_console_init(machine);
	debug_view_init(machine);
	debug_comment_init(machine);
	atexit(debug_flush_traces);
	add_logerror_callback(machine, debug_errorlog_write_line);
}


/*-------------------------------------------------
    mame_debug_break - break into the debugger
-------------------------------------------------*/

void mame_debug_break(void)
{
	debug_halt_on_next_instruction();
}


/*-------------------------------------------------
    mame_debug_is_active - true if the debugger
    is currently live
-------------------------------------------------*/

int mame_debug_is_active(void)
{
	return within_debugger_code;
}


/*-------------------------------------------------
    on_vblank - called when a VBLANK hits
-------------------------------------------------*/

static void on_vblank(const device_config *device, int vblank_state)
{
	/* if we're configured to stop on VBLANK, break */
	if (vblank_state && break_on_vblank)
	{
		execution_state = EXECUTION_STATE_STOPPED;
		debug_console_printf("Stopped at VBLANK\n");
		break_on_vblank = 0;
	}
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
	execution_state = EXECUTION_STATE_STOPPED;
	execution_counter = 0;
	next_index = 1;
	within_debugger_code = FALSE;
	last_cpunum = 0;
	last_stopped_cpunum = 0;
	steps_until_stop = 0;
	step_overout_breakpoint = ~0;
	step_overout_cpunum = 0;
	key_check_counter = 0;

	/* create a global symbol table */
	global_symtable = symtable_alloc(NULL);

	/* add "wpaddr", "wpdata", "cycles", "cpunum", "logunmap" to the global symbol table */
	symtable_add_register(global_symtable, "wpaddr", 0, get_wpaddr, NULL);
	symtable_add_register(global_symtable, "wpdata", 0, get_wpdata, NULL);
	symtable_add_register(global_symtable, "cycles", 0, get_cycles, NULL);
	symtable_add_register(global_symtable, "cpunum", 0, get_cpunum, NULL);
	symtable_add_register(global_symtable, "logunmap", ADDRESS_SPACE_PROGRAM, get_logunmap, set_logunmap);
	symtable_add_register(global_symtable, "logunmapd", ADDRESS_SPACE_DATA, get_logunmap, set_logunmap);
	symtable_add_register(global_symtable, "logunmapi", ADDRESS_SPACE_IO, get_logunmap, set_logunmap);
	symtable_add_register(global_symtable, "beamx", 0, get_beamx, NULL);
	symtable_add_register(global_symtable, "beamy", 0, get_beamy, NULL);

	/* add the temporary variables to the global symbol table */
	for (regnum = 0; regnum < NUM_TEMP_VARIABLES; regnum++)
	{
		char symname[10];
		sprintf(symname, "temp%d", regnum);
		symtable_add_register(global_symtable, symname, regnum, get_tempvar, set_tempvar);
	}

	/* reset the CPU info */
	memset(debug_cpuinfo, 0, sizeof(debug_cpuinfo));

	/* loop over CPUs and build up their info */
	for (cpunum = 0; cpunum < MAX_CPU; cpunum++)
	{
		cpu_type cputype = Machine->config->cpu[cpunum].type;

		/* if this is a dummy, stop looking */
		if (cputype == CPU_DUMMY)
			break;

		/* reset the PC data */
		debug_cpuinfo[cpunum].valid = 1;
		debug_cpuinfo[cpunum].endianness = cpunum_endianness(cpunum);
		debug_cpuinfo[cpunum].opwidth = cpunum_min_instruction_bytes(cpunum);
		debug_cpuinfo[cpunum].ignoring = 0;
		debug_cpuinfo[cpunum].temp_breakpoint_pc = ~0;

		/* fetch the memory accessors */
		debug_cpuinfo[cpunum].translate = (int (*)(int, offs_t *))cpunum_get_info_fct(cpunum, CPUINFO_PTR_TRANSLATE);
		debug_cpuinfo[cpunum].read = (int (*)(int, UINT32, int, UINT64 *))cpunum_get_info_fct(cpunum, CPUINFO_PTR_READ);
		debug_cpuinfo[cpunum].write = (int (*)(int, UINT32, int, UINT64))cpunum_get_info_fct(cpunum, CPUINFO_PTR_WRITE);
		debug_cpuinfo[cpunum].readop = (int (*)(UINT32, int, UINT64 *))cpunum_get_info_fct(cpunum, CPUINFO_PTR_READOP);

		/* allocate a symbol table */
		debug_cpuinfo[cpunum].symtable = symtable_alloc(global_symtable);

		/* add a global symbol for the current instruction pointer */
		symtable_add_register(debug_cpuinfo[cpunum].symtable, "curpc", 0, get_current_pc, 0);

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
			symtable_add_register(debug_cpuinfo[cpunum].symtable, symname, regnum, get_cpu_reg, set_cpu_reg);
		}

		/* loop over address spaces and get info */
		for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
		{
			debug_space_info *spaceinfo = &debug_cpuinfo[cpunum].space[spacenum];
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
	video_screen_register_vbl_cb(machine, NULL, on_vblank);

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
		/* close any tracefiles */
		if (debug_cpuinfo[cpunum].trace.file)
			fclose(debug_cpuinfo[cpunum].trace.file);
		if (debug_cpuinfo[cpunum].trace.action)
			free(debug_cpuinfo[cpunum].trace.action);

		/* free the symbol table */
		if (debug_cpuinfo[cpunum].symtable)
			symtable_free(debug_cpuinfo[cpunum].symtable);

		/* free all breakpoints */
		while (debug_cpuinfo[cpunum].first_bp)
			debug_breakpoint_clear(debug_cpuinfo[cpunum].first_bp->index);

		/* loop over all address spaces */
		for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
		{
			/* free all watchpoints */
			while (debug_cpuinfo[cpunum].space[spacenum].first_wp)
				debug_watchpoint_clear(debug_cpuinfo[cpunum].space[spacenum].first_wp->index);
		}
	}

	/* free the global symbol table */
	if (global_symtable)
		symtable_free(global_symtable);
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
	if (!within_debugger_code)
		return;
	steps_until_stop = numsteps;
	execution_state = EXECUTION_STATE_STEP_INTO;
}


/*-------------------------------------------------
    debug_cpu_single_step_over - single step over
    a single instruction
-------------------------------------------------*/

void debug_cpu_single_step_over(int numsteps)
{
	if (!within_debugger_code)
		return;
	steps_until_stop = numsteps;
	step_overout_cpunum = cpu_getactivecpu();
	execution_state = EXECUTION_STATE_STEP_OVER;
}


/*-------------------------------------------------
    debug_cpu_single_step_out - single step out of
    the current function
-------------------------------------------------*/

void debug_cpu_single_step_out(void)
{
	if (!within_debugger_code)
		return;
	steps_until_stop = 100;
	step_overout_cpunum = cpu_getactivecpu();
	execution_state = EXECUTION_STATE_STEP_OUT;
}


/*-------------------------------------------------
    debug_cpu_go - resume execution
-------------------------------------------------*/

void debug_cpu_go(offs_t targetpc)
{
	if (!within_debugger_code)
		return;
	execution_state = EXECUTION_STATE_RUNNING;
	debug_cpuinfo[cpu_getactivecpu()].temp_breakpoint_pc = targetpc;
}


/*-------------------------------------------------
    debug_cpu_go_vblank - run until the next
    VBLANK
-------------------------------------------------*/

void debug_cpu_go_vblank(void)
{
	if (!within_debugger_code)
		return;
	execution_state = EXECUTION_STATE_RUNNING;
	debug_cpuinfo[cpu_getactivecpu()].temp_breakpoint_pc = ~0;
	break_on_vblank = 1;
}


/*-------------------------------------------------
    debug_cpu_go_interrupt - run until the
    specified interrupt fires
-------------------------------------------------*/

void debug_cpu_go_interrupt(int irqline)
{
	if (!within_debugger_code)
		return;
	execution_state = EXECUTION_STATE_RUNNING;
	debug_cpuinfo[cpu_getactivecpu()].temp_breakpoint_pc = ~0;
	break_on_interrupt = 1;
	break_on_interrupt_cpunum = cpu_getactivecpu();
	break_on_interrupt_irqline = irqline;
}


/*-------------------------------------------------
    debug_cpu_go_milliseconds - run until the
    specified delay elapses
-------------------------------------------------*/

void debug_cpu_go_milliseconds(UINT64 milliseconds)
{
	if (!within_debugger_code)
		return;
	execution_state = EXECUTION_STATE_RUNNING;
	debug_cpuinfo[cpu_getactivecpu()].temp_breakpoint_pc = ~0;
	break_on_time = 1;
	break_on_time_target = attotime_add(
		timer_get_time(),
		attotime_make(milliseconds / 1000, (milliseconds % 1000) * (ATTOSECONDS_PER_SECOND / 1000)));
}


/*-------------------------------------------------
    debug_cpu_next_cpu - execute until we hit
    the next CPU
-------------------------------------------------*/

void debug_cpu_next_cpu(void)
{
	if (!within_debugger_code)
		return;
	execution_state = EXECUTION_STATE_NEXT_CPU;
}


/*-------------------------------------------------
    debug_cpu_ignore_cpu - ignore/observe a given
    CPU
-------------------------------------------------*/

void debug_cpu_ignore_cpu(int cpunum, int ignore)
{
	debug_cpuinfo[cpunum].ignoring = ignore;
	if (!within_debugger_code)
		return;
	if (cpunum == cpu_getactivecpu() && debug_cpuinfo[cpunum].ignoring)
		execution_state = EXECUTION_STATE_NEXT_CPU;
}


/*-------------------------------------------------
    debug_cpu_trace - trace execution of a given
    CPU
-------------------------------------------------*/

void debug_cpu_trace(int cpunum, FILE *file, int trace_over, const char *action)
{
	/* close existing files and delete expressions */
	if (debug_cpuinfo[cpunum].trace.file)
		fclose(debug_cpuinfo[cpunum].trace.file);
	debug_cpuinfo[cpunum].trace.file = NULL;

	if (debug_cpuinfo[cpunum].trace.action)
		free(debug_cpuinfo[cpunum].trace.action);
	debug_cpuinfo[cpunum].trace.action = NULL;

	/* open any new files */
	debug_cpuinfo[cpunum].trace.file = file;
	debug_cpuinfo[cpunum].trace.action = NULL;
	if (action)
	{
		debug_cpuinfo[cpunum].trace.action = malloc(strlen(action) + 1);
		if (debug_cpuinfo[cpunum].trace.action)
			strcpy(debug_cpuinfo[cpunum].trace.action, action);
	}

	/* specify trace over */
	debug_cpuinfo[cpunum].trace.trace_over_target = trace_over ? ~0 : 0;
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
	return &debug_cpuinfo[cpunum];
}


/*-------------------------------------------------
    debug_halt_on_next_instruction - halt in
    the debugger on the next instruction
-------------------------------------------------*/

void debug_halt_on_next_instruction(void)
{
	debug_console_printf("Internal breakpoint\n");
	execution_state = EXECUTION_STATE_STOPPED;
}


/*-------------------------------------------------
    debug_refresh_display - redraw the current
    video display
-------------------------------------------------*/

void debug_refresh_display(void)
{
	video_frame_update(Machine, TRUE);
}


/*-------------------------------------------------
    debug_get_execution_state - return the
    current execution state
-------------------------------------------------*/

int debug_get_execution_state(void)
{
	return execution_state;
}


/*-------------------------------------------------
    debug_get_execution_counter - return the
    current execution counter
-------------------------------------------------*/

UINT32 debug_get_execution_counter(void)
{
	return execution_counter;
}


/*-------------------------------------------------
    get_wpaddr - getter callback for the
    'wpaddr' symbol
-------------------------------------------------*/

static UINT64 get_wpaddr(UINT32 ref)
{
	return wpaddr;
}


/*-------------------------------------------------
    get_wpdata - getter callback for the
    'wpdata' symbol
-------------------------------------------------*/

static UINT64 get_wpdata(UINT32 ref)
{
	return wpdata;
}


/*-------------------------------------------------
    get_cycles - getter callback for the
    'cycles' symbol
-------------------------------------------------*/

static UINT64 get_cycles(UINT32 ref)
{
	return activecpu_get_icount();
}


/*-------------------------------------------------
    get_cpunum - getter callback for the
    'cpunum' symbol
-------------------------------------------------*/

static UINT64 get_cpunum(UINT32 ref)
{
	return cpu_getactivecpu();
}


/*-------------------------------------------------
    get_tempvar - getter callback for the
    'tempX' symbols
-------------------------------------------------*/

static UINT64 get_tempvar(UINT32 ref)
{
	return tempvar[ref];
}


/*-------------------------------------------------
    set_tempvar - setter callback for the
    'tempX' symbols
-------------------------------------------------*/

static void set_tempvar(UINT32 ref, UINT64 value)
{
	tempvar[ref] = value;
}


/*-------------------------------------------------
    get_logunmap - getter callback for the logumap
    symbols
-------------------------------------------------*/

static UINT64 get_logunmap(UINT32 ref)
{
	return memory_get_log_unmap(ref);
}


/*-------------------------------------------------
    get_beamx - get beam horizontal position
-------------------------------------------------*/

static UINT64 get_beamx(UINT32 ref)
{
	return video_screen_get_hpos(ref);
}


/*-------------------------------------------------
    get_beamy - get beam vertical position
-------------------------------------------------*/

static UINT64 get_beamy(UINT32 ref)
{
	return video_screen_get_vpos(ref);
}


/*-------------------------------------------------
    set_logunmap - setter callback for the logumap
    symbols
-------------------------------------------------*/

static void set_logunmap(UINT32 ref, UINT64 value)
{
	memory_set_log_unmap(ref, value ? 1 : 0);
}


/*-------------------------------------------------
    get_current_pc - getter callback for a CPU's
    current instruction pointer
-------------------------------------------------*/

static UINT64 get_current_pc(UINT32 ref)
{
	return activecpu_get_pc();
}


/*-------------------------------------------------
    get_cpu_reg - getter callback for a CPU's
    register symbols
-------------------------------------------------*/

static UINT64 get_cpu_reg(UINT32 ref)
{
	return activecpu_get_reg(ref);
}


/*-------------------------------------------------
    set_cpu_reg - setter callback for a CPU's
    register symbols
-------------------------------------------------*/

static void set_cpu_reg(UINT32 ref, UINT64 value)
{
	activecpu_set_reg(ref, value);
}



/***************************************************************************
    MAIN CPU CALLBACK
***************************************************************************/

/*-------------------------------------------------
    mame_debug_hook - called by the CPU cores
    before executing each instruction
-------------------------------------------------*/

void mame_debug_hook(offs_t curpc)
{
	int cpunum = cpu_getactivecpu();
	debug_cpu_info *info = &debug_cpuinfo[cpunum];

	/* update the history */
	info->pc_history[info->pc_history_index++ % DEBUG_HISTORY_SIZE] = curpc;

	/* quick out if we are ignoring */
	if (info->ignoring)
		return;

	/* note that we are in the debugger code */
	within_debugger_code = TRUE;

	/* bump the counter */
	execution_counter++;

	/* are we tracing? */
	if (info->trace.file)
		perform_trace(info);

	/* per-instruction hook? */
	if (info->instrhook != NULL && (*info->instrhook)(curpc))
		execution_state = EXECUTION_STATE_STOPPED;

	/* check for execution breakpoints */
	if (execution_state != EXECUTION_STATE_STOPPED)
	{
		/* see if we hit an interrupt break */
		if (break_on_interrupt == 2 && break_on_interrupt_cpunum == cpunum)
		{
			debug_console_printf("Stopped on interrupt (CPU %d, IRQ %d)\n", break_on_interrupt_cpunum, break_on_interrupt_irqline);
			break_on_interrupt = 0;
			execution_state = EXECUTION_STATE_STOPPED;
		}

		/* see if we hit a target time */
		if (break_on_time && attotime_compare(timer_get_time(), break_on_time_target) > 0)
		{
			debug_console_printf("Stopped at time interval %.1g\n", attotime_to_double(timer_get_time()));
			break_on_time = 0;
			execution_state = EXECUTION_STATE_STOPPED;
		}

		/* see if the CPU changed and break if we are waiting for that to happen */
		if (cpunum != last_cpunum)
		{
			if (execution_state == EXECUTION_STATE_NEXT_CPU)
				execution_state = EXECUTION_STATE_STOPPED;
			last_cpunum = cpunum;
		}

		/* check the temp running breakpoint and break if we hit it */
		if (info->temp_breakpoint_pc != ~0 && execution_state == EXECUTION_STATE_RUNNING && curpc == info->temp_breakpoint_pc)
		{
			execution_state = EXECUTION_STATE_STOPPED;
			debug_console_printf("Stopped at temporary breakpoint %X on CPU %d\n", info->temp_breakpoint_pc, cpunum);
			info->temp_breakpoint_pc = ~0;
		}

		/* check for execution breakpoints */
		if (info->first_bp)
			debug_check_breakpoints(cpunum, curpc);

		/* handle single stepping */
		if (steps_until_stop > 0 && (execution_state >= EXECUTION_STATE_STEP_INTO && execution_state <= EXECUTION_STATE_STEP_OUT))
		{
			/* is this an actual step? */
			if (step_overout_breakpoint == ~0 || (cpunum == step_overout_cpunum && curpc == step_overout_breakpoint))
			{
				/* decrement the count and reset the breakpoint */
				steps_until_stop--;
				step_overout_breakpoint = ~0;

				/* if we hit 0, stop; otherwise, we might want to update everything */
				if (steps_until_stop == 0)
					execution_state = EXECUTION_STATE_STOPPED;
				else if (execution_state != EXECUTION_STATE_STEP_OUT && (steps_until_stop < 200 || steps_until_stop % 100 == 0))
				{
					debug_view_update_all();
					debug_refresh_display();
				}
			}
		}

		/* check for debug keypresses */
		if (execution_state != EXECUTION_STATE_STOPPED && ++key_check_counter > 10000)
		{
			key_check_counter = 0;
			if (input_ui_pressed(IPT_UI_DEBUG_BREAK))
			{
				execution_state = EXECUTION_STATE_STOPPED;
				debug_console_printf("User-initiated break\n");
			}

			/* while we're here, check for a periodic update */
			if (cpunum == last_stopped_cpunum && execution_state != EXECUTION_STATE_STOPPED && osd_ticks() > last_periodic_update_time + osd_ticks_per_second()/4)
			{
				debug_view_update_all();
				last_periodic_update_time = osd_ticks();
			}
		}
	}

	/* if we are supposed to halt, do it now */
	if (execution_state == EXECUTION_STATE_STOPPED)
	{
		/* reset the state */
		steps_until_stop = 0;
		step_overout_breakpoint = ~0;

		/* update all views */
		debug_view_update_all();
		debug_refresh_display();

		/* wait for the debugger; during this time, disable sound output */
		sound_mute(TRUE);
		while (execution_state == EXECUTION_STATE_STOPPED)
		{
			/* clear the memory modified flag and wait */
			memory_modified = 0;
			osd_wait_for_debugger();

			/* if something modified memory, update the screen */
			if (memory_modified)
				debug_refresh_display();

			/* check for commands in the source file */
			process_source_file();

			/* if an event got scheduled, resume */
			if (mame_is_scheduled_event_pending(Machine))
				execution_state = EXECUTION_STATE_RUNNING;
		}
		sound_mute(FALSE);

		/* remember the last cpunum where we stopped */
		last_stopped_cpunum = cpunum;
	}

	/* handle step out/over on the instruction we are about to execute */
	if ((execution_state == EXECUTION_STATE_STEP_OVER || execution_state == EXECUTION_STATE_STEP_OUT) && cpunum == step_overout_cpunum && step_overout_breakpoint == ~0)
		prepare_for_step_overout();

	/* no longer in debugger code */
	within_debugger_code = FALSE;
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
	if (info->trace.trace_over_target && (info->trace.trace_over_target != ~0))
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
		if (info->trace.loops)
			fprintf(info->trace.file, "\n   (loops for %d instructions)\n\n", info->trace.loops);
		info->trace.loops = 0;

		/* execute any trace actions first */
		if (info->trace.action)
			debug_console_execute_command(info->trace.action, 0);

		/* print the address */
		offset = sprintf(buffer, "%0*X: ", info->space[ADDRESS_SPACE_PROGRAM].logchars, pc);

		/* print the disassembly */
		dasmresult = dasm_wrapped(&buffer[offset], pc);

		/* output the result */
		fprintf(info->trace.file, "%s\n", buffer);

		/* do we need to step the trace over this instruction? */
		if (info->trace.trace_over_target && (dasmresult & DASMFLAG_SUPPORTED)
			&& (dasmresult & DASMFLAG_STEP_OVER))
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

static void prepare_for_step_overout(void)
{
	offs_t pc = activecpu_get_pc();
	char dasmbuffer[100];
	offs_t dasmresult;

	/* disassemble the current instruction and get the flags */
	dasmresult = dasm_wrapped(dasmbuffer, pc);

	/* if flags are supported and it's a call-style opcode, set a temp breakpoint after that instruction */
	if ((dasmresult & DASMFLAG_SUPPORTED) && (dasmresult & DASMFLAG_STEP_OVER))
	{
		int extraskip = (dasmresult & DASMFLAG_OVERINSTMASK) >> DASMFLAG_OVERINSTSHIFT;
		pc += dasmresult & DASMFLAG_LENGTHMASK;

		/* if we need to skip additional instructions, advance as requested */
		while (extraskip-- > 0)
			pc += dasm_wrapped(dasmbuffer, pc) & DASMFLAG_LENGTHMASK;
		step_overout_breakpoint = pc;
	}

	/* if we're stepping out and this isn't a step out instruction, reset the steps until stop to a high number */
	if (execution_state == EXECUTION_STATE_STEP_OUT)
	{
		if ((dasmresult & DASMFLAG_SUPPORTED) && !(dasmresult & DASMFLAG_STEP_OUT))
			steps_until_stop = 100;
		else
			steps_until_stop = 1;
	}
}


/*-------------------------------------------------
    process_source_file - executes commands from
    a source file
-------------------------------------------------*/

static void process_source_file(void)
{
	/* loop until the file is exhausted or until we are executing again */
	while (debug_source_file && (execution_state == EXECUTION_STATE_STOPPED))
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
    debug_vblank_hook - called when an interrupt
    is acknowledged
-------------------------------------------------*/

void debug_interrupt_hook(int cpunum, int irqline)
{
	/* if we're configured to stop on interrupt, break */
	if (break_on_interrupt && cpunum == break_on_interrupt_cpunum && (break_on_interrupt_irqline == -1 || irqline == break_on_interrupt_irqline))
	{
		break_on_interrupt = 2;
		break_on_interrupt_irqline = irqline;
	}
}


/*-------------------------------------------------
    standard_debug_hook_read - standard read hook
-------------------------------------------------*/

static void standard_debug_hook_read(int spacenum, int size, offs_t address)
{
	debug_cpu_info *info = &debug_cpuinfo[memory_hook_cpunum];

	/* check watchpoints */
	if (info->read_watchpoints)
		check_watchpoints(memory_hook_cpunum, spacenum, WATCHPOINT_READ, address, size, 0);

	/* check hotspots */
	if (info->hotspots)
		check_hotspots(memory_hook_cpunum, spacenum, address);
}


/*-------------------------------------------------
    standard_debug_hook_write - standard write hook
-------------------------------------------------*/

static void standard_debug_hook_write(int spacenum, int size, offs_t address, UINT64 data)
{
	debug_cpu_info *info = &debug_cpuinfo[memory_hook_cpunum];

	/* check watchpoints */
	if (info->write_watchpoints)
		check_watchpoints(memory_hook_cpunum, spacenum, WATCHPOINT_WRITE, address, size, data);
}


/*-------------------------------------------------
    debug_get_memory_hooks - get memory hooks
    for the specified CPU
-------------------------------------------------*/

void debug_get_memory_hooks(int cpunum, debug_hook_read_func *read, debug_hook_write_func *write)
{
	memory_hook_cpunum = cpunum;

	if (debug_cpuinfo[cpunum].read_watchpoints || debug_cpuinfo[cpunum].hotspots)
		*read = standard_debug_hook_read;
	else
		*read = NULL;

	if (debug_cpuinfo[cpunum].write_watchpoints)
		*write = standard_debug_hook_write;
	else
		*write = NULL;
}


/*-------------------------------------------------
    debug_set_instruction_hook - set a hook to
    be called on each instruction for a given CPU
-------------------------------------------------*/

void debug_set_instruction_hook(int cpunum, int (*hook)(offs_t pc))
{
	debug_cpuinfo[cpunum].instrhook = hook;
}



/***************************************************************************
    BREAKPOINTS
***************************************************************************/

/*-------------------------------------------------
    debug_check_breakpoints - check the
    breakpoints for a given CPU
-------------------------------------------------*/

void debug_check_breakpoints(int cpunum, offs_t pc)
{
	debug_cpu_breakpoint *bp;
	UINT64 result;

	/* see if we match */
	for (bp = debug_cpuinfo[cpunum].first_bp; bp; bp = bp->next)
		if (bp->enabled && bp->address == pc)

			/* if we do, evaluate the condition */
			if (bp->condition == NULL || (expression_execute(bp->condition, &result) == EXPRERR_NONE && result))
			{
				/* halt in the debugger by default */
				execution_state = EXECUTION_STATE_STOPPED;

				/* if we hit, evaluate the action */
				if (bp->action != NULL)
					debug_console_execute_command(bp->action, 0);

				/* print a notification, unless the action made us go again */
				if (execution_state == EXECUTION_STATE_STOPPED)
					debug_console_printf("Stopped at breakpoint %X\n", bp->index);
				break;
			}
}


/*-------------------------------------------------
    debug_breakpoint_first - find the first
    breakpoint for a given CPU
-------------------------------------------------*/

static debug_cpu_breakpoint *find_breakpoint(int bpnum)
{
	debug_cpu_breakpoint *bp;
	int cpunum;

	/* loop over CPUs and find the requested breakpoint */
	for (cpunum = 0; cpunum < MAX_CPU; cpunum++)
		for (bp = debug_cpuinfo[cpunum].first_bp; bp; bp = bp->next)
			if (bp->index == bpnum)
				return bp;

	return NULL;
}


/*-------------------------------------------------
    debug_breakpoint_first - return the first
    breakpoint for a given CPU
-------------------------------------------------*/

debug_cpu_breakpoint *debug_breakpoint_first(int cpunum)
{
	return (cpunum < MAX_CPU) ? debug_cpuinfo[cpunum].first_bp : NULL;
}


/*-------------------------------------------------
    debug_breakpoint_set - set a new breakpoint
-------------------------------------------------*/

int debug_breakpoint_set(int cpunum, offs_t address, parsed_expression *condition, const char *action)
{
	debug_cpu_breakpoint *bp;

	assert_always((cpunum >= 0) && (cpunum < cpu_gettotalcpu()), "debug_breakpoint_set() called with invalid cpunum!");

	/* allocate breakpoint */
	bp = malloc(sizeof(*bp));

	/* if we can't allocate, return failure */
	if (!bp)
		return 0;

	/* fill in the structure */
	bp->index = next_index++;
	bp->enabled = 1;
	bp->address = address;
	bp->condition = condition;
	bp->action = NULL;
	if (action)
	{
		bp->action = malloc(strlen(action) + 1);
		if (bp->action)
			strcpy(bp->action, action);
	}

	/* hook us in */
	bp->next = debug_cpuinfo[cpunum].first_bp;
	debug_cpuinfo[cpunum].first_bp = bp;
	return bp->index;
}


/*-------------------------------------------------
    debug_breakpoint_clear - clear a breakpoint
-------------------------------------------------*/

int debug_breakpoint_clear(int bpnum)
{
	debug_cpu_breakpoint *bp, *pbp;
	int cpunum;

	/* loop over CPUs and find the requested breakpoint */
	for (cpunum = 0; cpunum < MAX_CPU; cpunum++)
		for (pbp = NULL, bp = debug_cpuinfo[cpunum].first_bp; bp; pbp = bp, bp = bp->next)
			if (bp->index == bpnum)
			{
				/* unlink us from the list */
				if (pbp == NULL)
					debug_cpuinfo[cpunum].first_bp = bp->next;
				else
					pbp->next = bp->next;

				/* free the memory */
				if (bp->condition)
					expression_free(bp->condition);
				if (bp->action)
					free(bp->action);
				free(bp);
				return 1;
			}

	/* we didn't find it; return an error */
	return 0;
}


/*-------------------------------------------------
    debug_breakpoint_enable - enable/disable a
    breakpoint
-------------------------------------------------*/

int debug_breakpoint_enable(int bpnum, int enable)
{
	debug_cpu_breakpoint *bp = find_breakpoint(bpnum);

	/* if we found it, set it */
	if (bp != NULL)
	{
		bp->enabled = (enable != 0);
		return 1;
	}
	return 0;
}



/***************************************************************************
    WATCHPOINTS
***************************************************************************/

/*-------------------------------------------------
    check_watchpoints - check the
    breakpoints for a given CPU and address space
-------------------------------------------------*/

static void check_watchpoints(int cpunum, int spacenum, int type, offs_t address, offs_t size, UINT64 value_to_write)
{
	debug_cpu_watchpoint *wp;
	UINT64 result;

	/* if we're within debugger code, don't stop */
	if (within_debugger_code)
		return;

	within_debugger_code = TRUE;

	/* if we are a write watchpoint, stash the value that will be written */
	wpaddr = address;
	if (type & WATCHPOINT_WRITE)
		wpdata = value_to_write;

	/* see if we match */
	for (wp = debug_cpuinfo[cpunum].space[spacenum].first_wp; wp; wp = wp->next)
		if (wp->enabled && (wp->type & type) && address + size > wp->address && address < wp->address + wp->length)

			/* if we do, evaluate the condition */
			if (wp->condition == NULL || (expression_execute(wp->condition, &result) == EXPRERR_NONE && result))
			{
				static const char *const sizes[] =
				{
					"0bytes", "byte", "word", "3bytes", "dword", "5bytes", "6bytes", "7bytes", "qword"
				};
				char buffer[100];

				/* halt in the debugger by default */
				execution_state = EXECUTION_STATE_STOPPED;

				/* if we hit, evaluate the action */
				if (wp->action != NULL)
					debug_console_execute_command(wp->action, 0);

				/* print a notification, unless the action made us go again */
				if (execution_state == EXECUTION_STATE_STOPPED)
				{
					if (type & WATCHPOINT_WRITE)
					{
						sprintf(buffer, "Stopped at watchpoint %X writing %s to %08X (PC=%X)", wp->index, sizes[size], BYTE2ADDR(address, &debug_cpuinfo[cpunum], spacenum), activecpu_get_pc());
						if (value_to_write >> 32)
							sprintf(&buffer[strlen(buffer)], " (data=%X%08X)", (UINT32)(value_to_write >> 32), (UINT32)value_to_write);
						else
							sprintf(&buffer[strlen(buffer)], " (data=%X)", (UINT32)value_to_write);
					}
					else
						sprintf(buffer, "Stopped at watchpoint %X reading %s from %08X (PC=%X)", wp->index, sizes[size], BYTE2ADDR(address, &debug_cpuinfo[cpunum], spacenum), activecpu_get_pc());
					debug_console_printf("%s\n", buffer);
				}
				break;
			}

	within_debugger_code = FALSE;
}


/*-------------------------------------------------
    debug_watchpoint_first - find the first
    watchpoint for a given CPU
-------------------------------------------------*/

static debug_cpu_watchpoint *find_watchpoint(int wpnum)
{
	debug_cpu_watchpoint *wp;
	int cpunum, spacenum;

	/* loop over CPUs and address spaces and find the requested watchpoint */
	for (cpunum = 0; cpunum < MAX_CPU; cpunum++)
		for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
			for (wp = debug_cpuinfo[cpunum].space[spacenum].first_wp; wp; wp = wp->next)
				if (wp->index == wpnum)
					return wp;

	return NULL;
}


/*-------------------------------------------------
    debug_watchpoint_first - return the first
    watchpoint for a given CPU
-------------------------------------------------*/

debug_cpu_watchpoint *debug_watchpoint_first(int cpunum, int spacenum)
{
	return (cpunum < MAX_CPU && spacenum < ADDRESS_SPACES) ? debug_cpuinfo[cpunum].space[spacenum].first_wp : NULL;
}


/*-------------------------------------------------
    debug_watchpoint_set - set a new watchpoint
-------------------------------------------------*/

int debug_watchpoint_set(int cpunum, int spacenum, int type, offs_t address, offs_t length, parsed_expression *condition, const char *action)
{
	debug_cpu_watchpoint *wp = malloc(sizeof(*wp));

	/* if we can't allocate, return failure */
	if (!wp)
		return 0;

	/* fill in the structure */
	wp->index = next_index++;
	wp->enabled = 1;
	wp->type = type;
	wp->address = ADDR2BYTE_MASKED(address, &debug_cpuinfo[cpunum], spacenum);
	wp->length = ADDR2BYTE(length, &debug_cpuinfo[cpunum], spacenum);
	wp->condition = condition;
	wp->action = NULL;
	if (action)
	{
		wp->action = malloc(strlen(action) + 1);
		if (wp->action)
			strcpy(wp->action, action);
	}

	/* hook us in */
	wp->next = debug_cpuinfo[cpunum].space[spacenum].first_wp;
	debug_cpuinfo[cpunum].space[spacenum].first_wp = wp;
	if (wp->type & WATCHPOINT_READ)
		debug_cpuinfo[cpunum].read_watchpoints++;
	if (wp->type & WATCHPOINT_WRITE)
		debug_cpuinfo[cpunum].write_watchpoints++;

	/* force debug_get_memory_hooks() to be called */
	cpuintrf_push_context(-1);
	cpuintrf_pop_context();

	return wp->index;
}


/*-------------------------------------------------
    debug_watchpoint_clear - clear a watchpoint
-------------------------------------------------*/

int debug_watchpoint_clear(int wpnum)
{
	debug_cpu_watchpoint *wp, *pwp;
	int cpunum, spacenum;

	/* loop over CPUs and find the requested watchpoint */
	for (cpunum = 0; cpunum < MAX_CPU; cpunum++)
		for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
			for (pwp = NULL, wp = debug_cpuinfo[cpunum].space[spacenum].first_wp; wp; pwp = wp, wp = wp->next)
				if (wp->index == wpnum)
				{
					/* unlink us from the list */
					if (pwp == NULL)
						debug_cpuinfo[cpunum].space[spacenum].first_wp = wp->next;
					else
						pwp->next = wp->next;

					/* free the memory */
					if (wp->condition)
						expression_free(wp->condition);
					if (wp->action)
						free(wp->action);
					if (wp->type & WATCHPOINT_READ)
						debug_cpuinfo[cpunum].read_watchpoints--;
					if (wp->type & WATCHPOINT_WRITE)
						debug_cpuinfo[cpunum].write_watchpoints--;
					free(wp);

					/* force debug_get_memory_hooks() to be called */
					cpuintrf_push_context(-1);
					cpuintrf_pop_context();

					return 1;
				}

	/* we didn't find it; return an error */
	return 0;
}


/*-------------------------------------------------
    debug_watchpoint_enable - enable/disable a
    watchpoint
-------------------------------------------------*/

int debug_watchpoint_enable(int wpnum, int enable)
{
	debug_cpu_watchpoint *wp = find_watchpoint(wpnum);

	/* if we found it, set it */
	if (wp != NULL)
	{
		wp->enabled = (enable != 0);
		return 1;
	}
	return 0;
}



/***************************************************************************
    HOTSPOTS
***************************************************************************/

/*-------------------------------------------------
    debug_hotspot_track - enable/disable tracking
    of hotspots
-------------------------------------------------*/

int debug_hotspot_track(int cpunum, int numspots, int threshhold)
{
	debug_cpu_info *info = &debug_cpuinfo[cpunum];

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

	/* force debug_get_memory_hooks() to be called */
	cpuintrf_push_context(-1);
	cpuintrf_pop_context();

	return 1;
}


/*-------------------------------------------------
    check_hotspots - check for
    hotspots on a memory read access
-------------------------------------------------*/

static void check_hotspots(int cpunum, int spacenum, offs_t address)
{
	debug_cpu_info *info = &debug_cpuinfo[cpunum];
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
	const debug_cpu_info *info = &debug_cpuinfo[cpu_getactivecpu()];
	UINT64 custom;
	UINT8 result;

	/* mask against the logical byte mask */
	address &= info->space[spacenum].logbytemask;

	/* all accesses from this point on are for the debugger */
	memory_set_debugger_access(1);

	/* translate if necessary; if not mapped, return 0xff */
	if (apply_translation && info->translate != NULL && !(*info->translate)(spacenum, &address))
		result = 0xff;

	/* if there is a custom read handler, and it returns TRUE, use that value */
	else if (info->read && (*info->read)(spacenum, address, 1, &custom))
		result = custom;

	/* otherwise, call the byte reading function for the translated address */
	else
		result = (*active_address_space[spacenum].accessors->read_byte)(address);

	/* no longer accessing via the debugger */
	memory_set_debugger_access(0);
	return result;
}


/*-------------------------------------------------
    debug_read_word - return a word from the
    current cpu in the specified memory space
-------------------------------------------------*/

UINT16 debug_read_word(int spacenum, offs_t address, int apply_translation)
{
	const debug_cpu_info *info = &debug_cpuinfo[cpu_getactivecpu()];
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
		if (debug_cpuinfo[cpu_getactivecpu()].endianness == CPU_IS_LE)
			result = byte0 | (byte1 << 8);
		else
			result = byte1 | (byte0 << 8);
	}

	/* otherwise, this proceeds like the byte case */
	else
	{
		/* all accesses from this point on are for the debugger */
		memory_set_debugger_access(1);

		/* translate if necessary; if not mapped, return 0xffff */
		if (apply_translation && info->translate != NULL && !(*info->translate)(spacenum, &address))
			result = 0xffff;

		/* if there is a custom read handler, and it returns TRUE, use that value */
		else if (info->read && (*info->read)(spacenum, address, 2, &custom))
			result = custom;

		/* otherwise, call the byte reading function for the translated address */
		else
			result = (*active_address_space[spacenum].accessors->read_word)(address);

		/* no longer accessing via the debugger */
		memory_set_debugger_access(0);
	}

	return result;
}


/*-------------------------------------------------
    debug_read_dword - return a dword from the
    current cpu in the specified memory space
-------------------------------------------------*/

UINT32 debug_read_dword(int spacenum, offs_t address, int apply_translation)
{
	const debug_cpu_info *info = &debug_cpuinfo[cpu_getactivecpu()];
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
		if (debug_cpuinfo[cpu_getactivecpu()].endianness == CPU_IS_LE)
			result = word0 | (word1 << 16);
		else
			result = word1 | (word0 << 16);
	}

	/* otherwise, this proceeds like the byte case */
	else
	{
		/* all accesses from this point on are for the debugger */
		memory_set_debugger_access(1);

		/* translate if necessary; if not mapped, return 0xffffffff */
		if (apply_translation && info->translate != NULL && !(*info->translate)(spacenum, &address))
			result = 0xffffffff;

		/* if there is a custom read handler, and it returns TRUE, use that value */
		else if (info->read && (*info->read)(spacenum, address, 4, &custom))
			result = custom;

		/* otherwise, call the byte reading function for the translated address */
		else
			result = (*active_address_space[spacenum].accessors->read_dword)(address);

		/* no longer accessing via the debugger */
		memory_set_debugger_access(0);
	}

	return result;
}


/*-------------------------------------------------
    debug_read_qword - return a qword from the
    current cpu in the specified memory space
-------------------------------------------------*/

UINT64 debug_read_qword(int spacenum, offs_t address, int apply_translation)
{
	const debug_cpu_info *info = &debug_cpuinfo[cpu_getactivecpu()];
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
		if (debug_cpuinfo[cpu_getactivecpu()].endianness == CPU_IS_LE)
			result = dword0 | ((UINT64)dword1 << 32);
		else
			result = dword1 | ((UINT64)dword0 << 32);
	}

	/* otherwise, this proceeds like the byte case */
	else
	{
		/* all accesses from this point on are for the debugger */
		memory_set_debugger_access(1);

		/* translate if necessary; if not mapped, return 0xffffffffffffffff */
		if (apply_translation && info->translate != NULL && !(*info->translate)(spacenum, &address))
			result = ~(UINT64)0;

		/* if there is a custom read handler, and it returns TRUE, use that value */
		else if (info->read && (*info->read)(spacenum, address, 8, &custom))
			result = custom;

		/* otherwise, call the byte reading function for the translated address */
		else
			result = (*active_address_space[spacenum].accessors->read_qword)(address);

		/* no longer accessing via the debugger */
		memory_set_debugger_access(0);
	}

	return result;
}


/*-------------------------------------------------
    debug_write_byte - write a byte to the
    current cpu in the specified memory space
-------------------------------------------------*/

void debug_write_byte(int spacenum, offs_t address, UINT8 data, int apply_translation)
{
	const debug_cpu_info *info = &debug_cpuinfo[cpu_getactivecpu()];

	/* mask against the logical byte mask */
	address &= info->space[spacenum].logbytemask;

	/* all accesses from this point on are for the debugger */
	memory_set_debugger_access(1);

	/* translate if necessary; if not mapped, we're done */
	if (apply_translation && info->translate != NULL && !(*info->translate)(spacenum, &address))
		;

	/* if there is a custom write handler, and it returns TRUE, use that */
	else if (info->write && (*info->write)(spacenum, address, 1, data))
		;

	/* otherwise, call the byte reading function for the translated address */
	else
		(*active_address_space[spacenum].accessors->write_byte)(address, data);

	/* no longer accessing via the debugger */
	memory_set_debugger_access(0);
	memory_modified = 1;
}


/*-------------------------------------------------
    debug_write_word - write a word to the
    current cpu in the specified memory space
-------------------------------------------------*/

void debug_write_word(int spacenum, offs_t address, UINT16 data, int apply_translation)
{
	const debug_cpu_info *info = &debug_cpuinfo[cpu_getactivecpu()];

	/* mask against the logical byte mask */
	address &= info->space[spacenum].logbytemask;

	/* if this is a misaligned write, or if there are no word writers, just read two bytes */
	if ((address & 1) || !active_address_space[spacenum].accessors->write_word)
	{
		if (debug_cpuinfo[cpu_getactivecpu()].endianness == CPU_IS_LE)
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
		memory_set_debugger_access(1);

		/* translate if necessary; if not mapped, we're done */
		if (apply_translation && info->translate && !(*info->translate)(spacenum, &address))
			;

		/* if there is a custom write handler, and it returns TRUE, use that */
		else if (info->write && (*info->write)(spacenum, address, 2, data))
			;

		/* otherwise, call the byte reading function for the translated address */
		else
			(*active_address_space[spacenum].accessors->write_word)(address, data);

		/* no longer accessing via the debugger */
		memory_set_debugger_access(0);
		memory_modified = 1;
	}
}


/*-------------------------------------------------
    debug_write_dword - write a dword to the
    current cpu in the specified memory space
-------------------------------------------------*/

void debug_write_dword(int spacenum, offs_t address, UINT32 data, int apply_translation)
{
	const debug_cpu_info *info = &debug_cpuinfo[cpu_getactivecpu()];

	/* mask against the logical byte mask */
	address &= info->space[spacenum].logbytemask;

	/* if this is a misaligned write, or if there are no dword writers, just read two words */
	if ((address & 3) || !active_address_space[spacenum].accessors->write_dword)
	{
		if (debug_cpuinfo[cpu_getactivecpu()].endianness == CPU_IS_LE)
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
		memory_set_debugger_access(1);

		/* translate if necessary; if not mapped, we're done */
		if (apply_translation && info->translate && !(*info->translate)(spacenum, &address))
			;

		/* if there is a custom write handler, and it returns TRUE, use that */
		else if (info->write && (*info->write)(spacenum, address, 4, data))
			;

		/* otherwise, call the byte reading function for the translated address */
		else
			(*active_address_space[spacenum].accessors->write_dword)(address, data);

		/* no longer accessing via the debugger */
		memory_set_debugger_access(0);
		memory_modified = 1;
	}
}


/*-------------------------------------------------
    debug_write_qword - write a qword to the
    current cpu in the specified memory space
-------------------------------------------------*/

void debug_write_qword(int spacenum, offs_t address, UINT64 data, int apply_translation)
{
	const debug_cpu_info *info = &debug_cpuinfo[cpu_getactivecpu()];

	/* mask against the logical byte mask */
	address &= info->space[spacenum].logbytemask;

	/* if this is a misaligned write, or if there are no qword writers, just read two dwords */
	if ((address & 7) || !active_address_space[spacenum].accessors->write_qword)
	{
		if (debug_cpuinfo[cpu_getactivecpu()].endianness == CPU_IS_LE)
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
		memory_set_debugger_access(1);

		/* translate if necessary; if not mapped, we're done */
		if (apply_translation && info->translate && !(*info->translate)(spacenum, &address))
			;

		/* if there is a custom write handler, and it returns TRUE, use that */
		else if (info->write && (*info->write)(spacenum, address, 8, data))
			;

		/* otherwise, call the byte reading function for the translated address */
		else
			(*active_address_space[spacenum].accessors->write_qword)(address, data);

		/* no longer accessing via the debugger */
		memory_set_debugger_access(0);
		memory_modified = 1;
	}
}


/*-------------------------------------------------
    debug_read_opcode - read 1,2,4 or 8 bytes at
    the given offset from opcode space
-------------------------------------------------*/

UINT64 debug_read_opcode(offs_t address, int size, int arg)
{
	const debug_cpu_info *info = &debug_cpuinfo[cpu_getactivecpu()];
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
	if (info->translate && !(*info->translate)(ADDRESS_SPACE_PROGRAM, &address))
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
    external_read_memory - read 1,2,4 or 8 bytes at
    the given offset in the given address space
-------------------------------------------------*/

UINT64 external_read_memory(int space, UINT32 offset, int size)
{
	const debug_cpu_info *info = &debug_cpuinfo[cpu_getactivecpu()];
	if (info->space[space].databytes == 0)
		return ~0;

	/* adjust the address into a byte address */
	offset = ADDR2BYTE(offset, info, space);
	switch (size)
	{
		case 1:		return debug_read_byte(space, offset, TRUE);
		case 2:		return debug_read_word(space, offset, TRUE);
		case 4:		return debug_read_dword(space, offset, TRUE);
		case 8:		return debug_read_qword(space, offset, TRUE);
	}
	return ~0;
}


/*-------------------------------------------------
    external_write_memory - write 1,2,4 or 8 bytes to
    the given offset in the given address space
-------------------------------------------------*/

void external_write_memory(int space, UINT32 offset, int size, UINT64 value)
{
	const debug_cpu_info *info = &debug_cpuinfo[cpu_getactivecpu()];
	if (info->space[space].databytes == 0)
		return;

	/* adjust the address into a byte address */
	offset = ADDR2BYTE(offset, info, space);
	switch (size)
	{
		case 1:		debug_write_byte(space, offset, value, TRUE); break;
		case 2:		debug_write_word(space, offset, value, TRUE); break;
		case 4:		debug_write_dword(space, offset, value, TRUE); break;
		case 8:		debug_write_qword(space, offset, value, TRUE); break;
	}
}


/*-------------------------------------------------
    debug_trace_printf - writes text to a given
    CPU's trace file
-------------------------------------------------*/

void debug_trace_printf(int cpunum, const char *fmt, ...)
{
	va_list va;

	debug_cpu_info *info = &debug_cpuinfo[cpunum];

	if (info->trace.file)
	{
		va_start(va, fmt);
		vfprintf(info->trace.file, fmt, va);
		va_end(va);
	}
}


/*-------------------------------------------------
    debug_source_script - specifies a debug command
    script to use
-------------------------------------------------*/

void debug_source_script(const char *file)
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
			if (mame_get_phase(Machine) == MAME_PHASE_RUNNING)
				debug_console_printf("Cannot open command file '%s'\n", file);
			else
				fatalerror("Cannot open command file '%s'", file);
		}
	}
}


/*-------------------------------------------------
    debug_flush_traces - flushes all traces; this is
    useful if a trace is going on when we fatalerror
-------------------------------------------------*/

void debug_flush_traces(void)
{
	int cpunum;

	for (cpunum = 0; cpunum < cpu_gettotalcpu(); cpunum++)
	{
		if (debug_cpuinfo[cpunum].trace.file)
			fflush(debug_cpuinfo[cpunum].trace.file);
	}
}
