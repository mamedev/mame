/***************************************************************************

    cpuexec.c

    Core multi-CPU execution engine.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "driver.h"
#include "profiler.h"
#include "eminline.h"
#include "debugger.h"


/***************************************************************************
    DEBUGGING
***************************************************************************/

#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* internal trigger IDs */
enum
{
	TRIGGER_INT 		= -2000,
	TRIGGER_YIELDTIME 	= -3000,
	TRIGGER_SUSPENDTIME = -4000
};



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* internal information about the state of inputs */
typedef struct _cpu_input_data cpu_input_data;
struct _cpu_input_data
{
	INT32			vector;				/* most recently written vector */
	INT32			curvector;			/* most recently processed vector */
	UINT8			curstate;			/* most recently processed state */
	INT32			queue[MAX_INPUT_EVENTS]; /* queue of pending events */
	int				qindex;				/* index within the queue */
};


/* internal data hanging off of the classtoken */
typedef struct _cpu_class_data cpu_class_data;
struct _cpu_class_data
{
	/* cycle counting and executing */
	int				profiler;				/* profiler tag */ 
	int *			icount;					/* pointer to the icount */
	int 			cycles_running;			/* number of cycles we are executing */
	int				cycles_stolen;			/* number of cycles we artificially stole */

	/* input states and IRQ callbacks */
	cpu_irq_callback driver_irq;			/* driver-specific IRQ callback */
	cpu_input_data	input[MAX_INPUT_LINES]; /* data about inputs */

	/* suspend states */
	UINT8			suspend;				/* suspend reason mask (0 = not suspended) */
	UINT8			nextsuspend;			/* pending suspend reason mask */
	UINT8			eatcycles;				/* true if we eat cycles while suspended */
	UINT8			nexteatcycles;			/* pending value */
	INT32			trigger;				/* pending trigger to release a trigger suspension */
	INT32			inttrigger;				/* interrupt trigger index */

	/* clock and timing information */
	UINT64 			totalcycles;			/* total CPU cycles executed */
	attotime 		localtime;				/* local time, relative to the timer system's global time */
	INT32			clock;					/* current active clock */
	double			clockscale;				/* current active clock scale factor */
	INT32			divisor;				/* 32-bit attoseconds_per_cycle divisor */
	UINT8			divshift;				/* right shift amount to fit the divisor into 32 bits */
	emu_timer *		timedint_timer;			/* reference to this CPU's periodic interrupt timer */
	UINT32			cycles_per_second;		/* cycles per second, adjusted for multipliers */
	attoseconds_t	attoseconds_per_cycle;	/* attoseconds per adjusted clock cycle */

	/* these below are hacks to support multiple interrupts per frame */
	INT32 			iloops; 				/* number of interrupts remaining this frame */
	emu_timer *		partial_frame_timer;	/* the timer that triggers partial frame interrupts */
	attotime		partial_frame_period;	/* the length of one partial frame for interrupt purposes */
};


/* global data stored in the machine */
/* In mame.h: typedef struct _cpuexec_private cpuexec_private; */
struct _cpuexec_private
{
	const device_config *executingcpu;		/* pointer to the currently executing CPU */
	char			statebuf[256];			/* string buffer containing state description */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void update_clock_information(const device_config *device);
static void compute_perfect_interleave(running_machine *machine);
static void on_vblank(const device_config *device, void *param, int vblank_state);
static TIMER_CALLBACK( trigger_partial_frame_interrupt );
static TIMER_CALLBACK( trigger_periodic_interrupt );
static TIMER_CALLBACK( triggertime_callback );
static TIMER_CALLBACK( empty_event_queue );
static IRQ_CALLBACK( standard_irq_callback );
static void register_save_states(const device_config *device);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    cpu_get_class_data - return a pointer to
    the class data
-------------------------------------------------*/

INLINE cpu_class_data *cpu_get_class_data(const device_config *device)
{
	assert(device != NULL);
	assert(device->class == DEVICE_CLASS_CPU_CHIP);
	assert(device->token != NULL);
	return (cpu_class_data *)cpu_get_class_header(device) - 1;
}


/*-------------------------------------------------
    suspend_until_trigger - suspend execution
    until the given trigger fires
-------------------------------------------------*/

INLINE void suspend_until_trigger(const device_config *device, int trigger, int eatcycles)
{
	cpu_class_data *classdata = cpu_get_class_data(device);

	/* suspend the CPU immediately if it's not already */
	cpu_suspend(device, SUSPEND_REASON_TRIGGER, eatcycles);

	/* set the trigger */
	classdata->trigger = trigger;
}



/***************************************************************************
    CORE CPU EXECUTION
***************************************************************************/

/*-------------------------------------------------
    cpuexec_init - initialize internal states of
    all CPUs
-------------------------------------------------*/

void cpuexec_init(running_machine *machine)
{
	int numscreens = video_screen_count(machine->config);
	attoseconds_t refresh_attosecs;
	int ipf;

	/* allocate global state */
	machine->cpuexec_data = auto_malloc(sizeof(*machine->cpuexec_data));
	memset(machine->cpuexec_data, 0, sizeof(*machine->cpuexec_data));

	/* set the core scheduling quantum */
	ipf = machine->config->cpu_slices_per_frame;
	if (ipf <= 0)
		ipf = 1;
	refresh_attosecs = (numscreens == 0) ? HZ_TO_ATTOSECONDS(60) : video_screen_get_frame_period(machine->primary_screen).attoseconds;
	timer_add_scheduling_quantum(machine, refresh_attosecs / ipf, attotime_never);
}


/*-------------------------------------------------
    cpuexec_timeslice - execute all CPUs for a
    single timeslice
-------------------------------------------------*/

void cpuexec_timeslice(running_machine *machine)
{
	int call_debugger = ((machine->debug_flags & DEBUG_FLAG_ENABLED) != 0);
	cpuexec_private *global = machine->cpuexec_data;
	attotime target = timer_next_fire_time(machine);
	attotime base = timer_get_time(machine);
	const device_config *cpu;
	int ran;

	LOG(("------------------\n"));
	LOG(("cpu_timeslice: target = %s\n", attotime_string(target, 9)));

	/* apply pending suspension changes */
	for (cpu = machine->cpu[0]; cpu != NULL; cpu = cpu->typenext)
	{
		cpu_class_data *classdata = cpu_get_class_data(cpu);
		classdata->suspend = classdata->nextsuspend;
		classdata->nextsuspend &= ~SUSPEND_REASON_TIMESLICE;
		classdata->eatcycles = classdata->nexteatcycles;
	}

	/* loop over non-suspended CPUs */
	for (cpu = machine->cpu[0]; cpu != NULL; cpu = cpu->typenext)
	{
		cpu_class_data *classdata = cpu_get_class_data(cpu);
		if (classdata->suspend == 0)
		{
			attotime delta = attotime_sub(target, classdata->localtime);
			if (delta.seconds >= 0 && delta.attoseconds >= classdata->attoseconds_per_cycle)
			{
				/* compute how long to run */
				classdata->cycles_running = div_64x32(delta.attoseconds >> classdata->divshift, classdata->divisor);
				LOG(("  cpu '%s': %d cycles\n", cpu->tag, classdata->cycles_running));

				profiler_mark(classdata->profiler);

				/* note that this global variable cycles_stolen can be modified */
				/* via the call to cpu_execute */
				classdata->cycles_stolen = 0;
				global->executingcpu = cpu;
				if (!call_debugger)
					ran = cpu_execute(cpu, classdata->cycles_running);
				else
				{
					debugger_start_cpu_hook(cpu, target);
					ran = cpu_execute(cpu, classdata->cycles_running);
					debugger_stop_cpu_hook(cpu);
				}

#ifdef MAME_DEBUG
				if (ran < classdata->cycles_stolen)
					fatalerror("Negative CPU cycle count!");
#endif /* MAME_DEBUG */

				ran -= classdata->cycles_stolen;
				profiler_mark(PROFILER_END);

				/* account for these cycles */
				classdata->totalcycles += ran;
				classdata->localtime = attotime_add_attoseconds(classdata->localtime, ran * classdata->attoseconds_per_cycle);
				LOG(("         %d ran, %d total, time = %s\n", ran, (INT32)classdata->totalcycles, attotime_string(classdata->localtime, 9)));

				/* if the new local CPU time is less than our target, move the target up */
				if (attotime_compare(classdata->localtime, target) < 0)
				{
					target = attotime_max(classdata->localtime, base);
					LOG(("         (new target)\n"));
				}
			}
		}
	}
	global->executingcpu = NULL;

	/* update the local times of all CPUs */
	for (cpu = machine->cpu[0]; cpu != NULL; cpu = cpu->typenext)
	{
		cpu_class_data *classdata = cpu_get_class_data(cpu);

		/* if we're suspended and counting, process */
		if (classdata->suspend != 0 && classdata->eatcycles && attotime_compare(classdata->localtime, target) < 0)
		{
			attotime delta = attotime_sub(target, classdata->localtime);

			/* compute how long to run */
			classdata->cycles_running = div_64x32(delta.attoseconds >> classdata->divshift, classdata->divisor);
			LOG(("  cpu '%s': %d cycles (suspended)\n", cpu->tag, classdata->cycles_running));

			classdata->totalcycles += classdata->cycles_running;
			classdata->localtime = attotime_add_attoseconds(classdata->localtime, classdata->cycles_running * classdata->attoseconds_per_cycle);
			LOG(("         %d skipped, %d total, time = %s\n", classdata->cycles_running, (INT32)classdata->totalcycles, attotime_string(classdata->localtime, 9)));
		}

		/* update the suspend state (breaks steeltal if we don't) */
		classdata->suspend = classdata->nextsuspend;
		classdata->eatcycles = classdata->nexteatcycles;
	}

	/* update the global time */
	timer_set_global_time(machine, target);
}


/*-------------------------------------------------
    cpuexec_boost_interleave - temporarily boosts
    the interleave factor
-------------------------------------------------*/

void cpuexec_boost_interleave(running_machine *machine, attotime timeslice_time, attotime boost_duration)
{
	/* ignore timeslices > 1 second */
	if (timeslice_time.seconds > 0)
		return;
	timer_add_scheduling_quantum(machine, timeslice_time.attoseconds, boost_duration);
}



/***************************************************************************
    GLOBAL HELPERS
***************************************************************************/

/*-------------------------------------------------
    cpuexec_abort_timeslice - abort execution
    for the current timeslice
-------------------------------------------------*/

void cpuexec_abort_timeslice(running_machine *machine)
{
	const device_config *executingcpu = machine->cpuexec_data->executingcpu;
	if (executingcpu != NULL)
		cpu_abort_timeslice(executingcpu);
}


/*-------------------------------------------------
    cpuexec_describe_context - return a string
    describing which CPUs are currently executing
    and their PC
-------------------------------------------------*/

const char *cpuexec_describe_context(running_machine *machine)
{
	cpuexec_private *global = machine->cpuexec_data;
	const device_config *executingcpu = global->executingcpu;

	/* if we have an executing CPU, output data */
	if (executingcpu != NULL)
		sprintf(global->statebuf, "'%s'(%08X)", executingcpu->tag, cpu_get_pc(executingcpu));
	else
		strcpy(global->statebuf, "(no context)");

	return global->statebuf;
}



/***************************************************************************
    VIDEO SCREEN DEVICE INTERFACE
***************************************************************************/

/*-------------------------------------------------
    device_start_cpu - device start callback
-------------------------------------------------*/

static DEVICE_START( cpu )
{
	int index = cpu_get_index(device);
	cpu_class_header *header;
	cpu_class_data *classdata;
	const cpu_config *config;
	cpu_init_func init;
	int spacenum, line;
	int num_regs;

	/* validate some basic stuff */
	assert(device != NULL);
	assert(device->inline_config != NULL);
	assert(device->machine != NULL);
	assert(device->machine->config != NULL);

	/* get pointers to our data */
	config = device->inline_config;
	header = cpu_get_class_header(device);
	classdata = cpu_get_class_data(device);
	
	/* add ourself to the global array */
	if (index < ARRAY_LENGTH(device->machine->cpu))
		device->machine->cpu[index] = device;

	/* build the header */
	header->debug = NULL;
	for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
		header->space[spacenum] = memory_find_address_space(device, spacenum);
	
	header->set_info = (cpu_set_info_func)device_get_info_fct(device, CPUINFO_PTR_SET_INFO);
	header->execute = (cpu_execute_func)device_get_info_fct(device, CPUINFO_PTR_EXECUTE);
	header->burn = (cpu_burn_func)device_get_info_fct(device, CPUINFO_PTR_BURN);
	header->translate = (cpu_translate_func)device_get_info_fct(device, CPUINFO_PTR_TRANSLATE);
	header->disassemble = (cpu_disassemble_func)device_get_info_fct(device, CPUINFO_PTR_DISASSEMBLE);
	header->dasm_override = NULL;

	/* fill in the input states and IRQ callback information */
	for (line = 0; line < ARRAY_LENGTH(classdata->input); line++)
	{
		cpu_input_data *inputline = &classdata->input[line];
		/* vector and curvector are initialized later */
		inputline->curstate = CLEAR_LINE;
		inputline->qindex = 0;
	}

	/* fill in the suspend states */
	classdata->profiler = index + PROFILER_CPU1;
	classdata->suspend = SUSPEND_REASON_RESET;
	classdata->inttrigger = index + TRIGGER_INT;

	/* fill in the clock and timing information */
	classdata->clock = (UINT64)config->clock * cpu_get_clock_multiplier(device) / cpu_get_clock_divider(device);
	classdata->clockscale = 1.0;

	/* allocate timers if we need them */
	if (config->vblank_interrupts_per_frame > 1)
		classdata->partial_frame_timer = timer_alloc(device->machine, trigger_partial_frame_interrupt, (void *)device);
	if (config->timed_interrupt_period != 0)
		classdata->timedint_timer = timer_alloc(device->machine, trigger_periodic_interrupt, (void *)device);

	/* initialize this CPU */
	num_regs = state_save_get_reg_count(device->machine);
	init = (cpu_init_func)device_get_info_fct(device, CPUINFO_PTR_INIT);
	(*init)(device, index, classdata->clock, standard_irq_callback);
	num_regs = state_save_get_reg_count(device->machine) - num_regs;
	
	/* fetch post-initialization data */
	classdata->icount = cpu_get_icount_ptr(device);
	for (line = 0; line < ARRAY_LENGTH(classdata->input); line++)
	{
		cpu_input_data *inputline = &classdata->input[line];
		inputline->vector = cpu_get_default_irq_vector(device);
		inputline->curvector = inputline->vector;
	}
	update_clock_information(device);

	/* if no state registered for saving, we can't save */
	if (num_regs == 0)
	{
		logerror("CPU '%s' did not register any state to save!\n", cpu_get_name(device));
		if (device->machine->gamedrv->flags & GAME_SUPPORTS_SAVE)
			fatalerror("CPU '%s' did not register any state to save!", cpu_get_name(device));
	}

	/* register some internal states as well */
	register_save_states(device);

	return DEVICE_START_OK;
}


/*-------------------------------------------------
    device_reset_cpu - device reset callback
-------------------------------------------------*/

static DEVICE_RESET( cpu )
{
	cpu_class_data *classdata = cpu_get_class_data(device);
	const cpu_config *config = device->inline_config;
	cpu_reset_func reset;
	int line;

	/* enable all CPUs (except for disabled CPUs) */
	if (!(config->flags & CPU_DISABLE))
		cpu_resume(device, SUSPEND_ANY_REASON);
	else
		cpu_suspend(device, SUSPEND_REASON_DISABLE, 1);

	/* reset the total number of cycles */
	classdata->totalcycles = 0;

	/* then reset the CPU directly */
	reset = (cpu_reset_func)device_get_info_fct(device, CPUINFO_PTR_RESET);
	if (reset != NULL)
		(*reset)(device);

	/* reset the interrupt vectors and queues */
	for (line = 0; line < ARRAY_LENGTH(classdata->input); line++)
	{
		cpu_input_data *inputline = &classdata->input[line];
		inputline->vector = cpu_get_default_irq_vector(device);
		inputline->qindex = 0;
	}

	/* reconfingure VBLANK interrupts */
	if (config->vblank_interrupts_per_frame > 0 || config->vblank_interrupt_screen != NULL)
	{
		const device_config *screen;

		/* get the screen that will trigger the VBLANK */

		/* new style - use screen tag directly */
		if (config->vblank_interrupt_screen != NULL)
			screen = devtag_get_device(device->machine, VIDEO_SCREEN, config->vblank_interrupt_screen);

		/* old style 'hack' setup - use screen #0 */
		else
			screen = video_screen_first(device->machine->config);

		assert(screen != NULL);
		video_screen_register_vblank_callback(screen, on_vblank, NULL);
	}

	/* reconfigure periodic interrupts */
	if (config->timed_interrupt_period != 0)
	{
		attotime timedint_period = UINT64_ATTOTIME_TO_ATTOTIME(config->timed_interrupt_period);
		assert(classdata->timedint_timer != NULL);
		timer_adjust_periodic(classdata->timedint_timer, timedint_period, 0, timedint_period);
	}
}


/*-------------------------------------------------
    device_stop_cpu - device stop callback
-------------------------------------------------*/

static DEVICE_STOP( cpu )
{
	cpu_exit_func exit;
	
	/* call the CPU's exit function if present */
	exit = (cpu_exit_func)device_get_info_fct(device, CPUINFO_PTR_EXIT);
	if (exit != NULL)
		(*exit)(device);
}


/*-------------------------------------------------
    cpu_set_info - device set info callback
-------------------------------------------------*/

static DEVICE_SET_INFO( cpu )
{
	cpu_class_header *header = cpu_get_class_header(device);
	cpu_set_info_func set_info;
	cpuinfo cinfo;

	/* if we are live and have a header, save ourself a call */
	if (header != NULL)
		set_info = header->set_info;
	else
		set_info = (cpu_set_info_func)device_get_info_fct(device, CPUINFO_PTR_SET_INFO);
	
	switch (state)
	{
		/* no parameters to set */

		default:
			/* integer data */
			if (state >= DEVINFO_INT_FIRST && state <= DEVINFO_INT_LAST)
			{
				cinfo.i = info->i;
				(*set_info)(device, state, &cinfo);
			}

			/* pointer data */
			else if ((state >= DEVINFO_PTR_FIRST && state <= DEVINFO_PTR_LAST) ||
					 (state >= DEVINFO_FCT_FIRST && state <= DEVINFO_FCT_LAST) ||
					 (state >= DEVINFO_STR_FIRST && state <= DEVINFO_STR_LAST))
			{
				cinfo.p = info->p;
				(*set_info)(device, state, &cinfo);
			}
			break;
	}
}


/*-------------------------------------------------
    cpu_get_info - device get info
    callback
-------------------------------------------------*/

DEVICE_GET_INFO( cpu )
{
	const cpu_config *config = (device != NULL) ? device->inline_config : NULL;
	cpuinfo cinfo = { 0 };
	
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:
			cinfo.i = 0;
			(*config->type)(device, CPUINFO_INT_CONTEXT_SIZE, &cinfo);
			info->i = cinfo.i + sizeof(cpu_class_data) + sizeof(cpu_class_header);			
			break;

		case DEVINFO_INT_INLINE_CONFIG_BYTES:	info->i = sizeof(cpu_config);				break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_CPU_CHIP;			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_SET_INFO:				info->set_info = DEVICE_SET_INFO_NAME(cpu); break;
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(cpu); 		break;
		case DEVINFO_FCT_STOP:					info->stop = DEVICE_STOP_NAME(cpu); 		break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(cpu); 		break;

		default:
			/* integer data */
			if (state >= DEVINFO_INT_FIRST && state <= DEVINFO_INT_LAST)
			{
				cinfo.i = info->i;
				(*config->type)(device, state, &cinfo);
				info->i = cinfo.i;
			}

			/* pointer data */
			else if ((state >= DEVINFO_PTR_FIRST && state <= DEVINFO_PTR_LAST) ||
					 (state >= DEVINFO_FCT_FIRST && state <= DEVINFO_FCT_LAST) ||
					 (state >= DEVINFO_STR_FIRST && state <= DEVINFO_STR_LAST))
			{
				cinfo.p = info->p;
				(*config->type)(device, state, &cinfo);
				info->p = cinfo.p;
			}
			break;
	}
}



/***************************************************************************
    CPU SCHEDULING
***************************************************************************/

/*-------------------------------------------------
    cpu_suspend - set a suspend reason for the
    given CPU
-------------------------------------------------*/

void cpu_suspend(const device_config *device, int reason, int eatcycles)
{
	cpu_class_data *classdata = cpu_get_class_data(device);

	/* set the suspend reason and eat cycles flag */
	classdata->nextsuspend |= reason;
	classdata->nexteatcycles = eatcycles;

	/* if we're active, synchronize */
	cpu_abort_timeslice(device);
}


/*-------------------------------------------------
    cpu_resume - clear a suspend reason for the
    given CPU
-------------------------------------------------*/

void cpu_resume(const device_config *device, int reason)
{
	cpu_class_data *classdata = cpu_get_class_data(device);

	/* clear the suspend reason and eat cycles flag */
	classdata->nextsuspend &= ~reason;

	/* if we're active, synchronize */
	cpu_abort_timeslice(device);
}


/*-------------------------------------------------
    cpu_is_executing - return TRUE if the given
    CPU is within its execute function
-------------------------------------------------*/

int cpu_is_executing(const device_config *device)
{
	return (device == device->machine->cpuexec_data->executingcpu);
}


/*-------------------------------------------------
    cpu_is_suspended - returns TRUE if the
    given CPU is suspended for any of the given
    reasons
-------------------------------------------------*/

int cpu_is_suspended(const device_config *device, int reason)
{
	cpu_class_data *classdata = cpu_get_class_data(device);

	/* return true if the given reason is indicated */
	return ((classdata->nextsuspend & reason) != 0);
}



/***************************************************************************
    CPU CLOCK MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    cpu_get_clock - gets the given CPU's
    clock speed
-------------------------------------------------*/

int cpu_get_clock(const device_config *device)
{
	cpu_class_data *classdata = cpu_get_class_data(device);

	/* return the current clock value */
	return classdata->clock;
}


/*-------------------------------------------------
    cpu_set_clock - sets the given CPU's
    clock speed
-------------------------------------------------*/

void cpu_set_clock(const device_config *device, int clock)
{
	cpu_class_data *classdata = cpu_get_class_data(device);

	/* set the clock and update the information */
	classdata->clock = clock;
	update_clock_information(device);
}


/*-------------------------------------------------
    cpu_get_clockscale - returns the current
    scaling factor for a CPU's clock speed
-------------------------------------------------*/

double cpu_get_clockscale(const device_config *device)
{
	cpu_class_data *classdata = cpu_get_class_data(device);

	/* return the current clock scale factor */
	return classdata->clockscale;
}


/*-------------------------------------------------
    cpu_set_clockscale - sets the current
    scaling factor for a CPU's clock speed
-------------------------------------------------*/

void cpu_set_clockscale(const device_config *device, double clockscale)
{
	cpu_class_data *classdata = cpu_get_class_data(device);

	/* set the scale factor and update the information */
	classdata->clockscale = clockscale;
	update_clock_information(device);
}


/*-------------------------------------------------
    cpu_clocks_to_attotime - converts a number of
    clock ticks to an attotime
-------------------------------------------------*/

attotime cpu_clocks_to_attotime(const device_config *device, UINT64 clocks)
{
	cpu_class_data *classdata = cpu_get_class_data(device);
	if (clocks < classdata->cycles_per_second)
		return attotime_make(0, clocks * classdata->attoseconds_per_cycle);
	else
	{
		UINT32 remainder;
		UINT32 quotient = divu_64x32_rem(clocks, classdata->cycles_per_second, &remainder);
		return attotime_make(quotient, (UINT64)remainder * (UINT64)classdata->attoseconds_per_cycle);
	}
}


/*-------------------------------------------------
    cpu_attotime_to_clocks - converts a duration
    as attotime to CPU clock ticks
-------------------------------------------------*/

UINT64 cpu_attotime_to_clocks(const device_config *device, attotime duration)
{
	cpu_class_data *classdata = cpu_get_class_data(device);
	return mulu_32x32(duration.seconds, classdata->cycles_per_second) + (UINT64)duration.attoseconds * (UINT64)classdata->attoseconds_per_cycle;
}



/***************************************************************************
    CPU TIMING
***************************************************************************/

/*-------------------------------------------------
    cpu_get_local_time - returns the current
    local time for a CPU
-------------------------------------------------*/

attotime cpu_get_local_time(const device_config *device)
{
	cpu_class_data *classdata = cpu_get_class_data(device);
	attotime result;

	/* if we're active, add in the time from the current slice */
	result = classdata->localtime;
	if (device == device->machine->cpuexec_data->executingcpu)
	{
		int cycles = classdata->cycles_running - *classdata->icount;
		result = attotime_add(result, cpu_clocks_to_attotime(device, cycles));
	}
	return result;
}


/*-------------------------------------------------
    cpuexec_override_local_time - overrides the
    given time with the executing CPU's local
    time, if present (this function is private
    to timer.c)
-------------------------------------------------*/

attotime cpuexec_override_local_time(running_machine *machine, attotime default_time)
{
	if (machine->cpuexec_data != NULL && machine->cpuexec_data->executingcpu != NULL)
		return cpu_get_local_time(machine->cpuexec_data->executingcpu);
	return default_time;
}


/*-------------------------------------------------
    cpu_get_total_cycles - return the total
    number of CPU cycles executed on the active
    CPU
-------------------------------------------------*/

UINT64 cpu_get_total_cycles(const device_config *device)
{
	cpu_class_data *classdata = cpu_get_class_data(device);

	if (device == device->machine->cpuexec_data->executingcpu)
		return classdata->totalcycles + classdata->cycles_running - *classdata->icount;
	else
		return classdata->totalcycles;
}


/*-------------------------------------------------
    cpu_eat_cycles - safely eats cycles so
    we don't cross a timeslice boundary
-------------------------------------------------*/

void cpu_eat_cycles(const device_config *device, int cycles)
{
	cpu_class_data *classdata = cpu_get_class_data(device);

	/* ignore if not the executing CPU */
	if (device != device->machine->cpuexec_data->executingcpu)
		return;

	if (cycles > *classdata->icount)
		cycles = *classdata->icount + 1;
	*classdata->icount -= cycles;
}


/*-------------------------------------------------
    cpu_adjust_icount - apply a +/- to the current
    icount
-------------------------------------------------*/

void cpu_adjust_icount(const device_config *device, int delta)
{
	cpu_class_data *classdata = cpu_get_class_data(device);

	/* ignore if not the executing CPU */
	if (device != device->machine->cpuexec_data->executingcpu)
		return;

	*classdata->icount += delta;
}


/*-------------------------------------------------
    cpu_abort_timeslice - abort execution
    for the current timeslice, allowing other
    CPUs to run before we run again
-------------------------------------------------*/

void cpu_abort_timeslice(const device_config *device)
{
	cpu_class_data *classdata = cpu_get_class_data(device);
	int delta;

	/* ignore if not the executing CPU */
	if (device != device->machine->cpuexec_data->executingcpu)
		return;

	/* swallow the remaining cycles */
	if (classdata->icount != NULL)
	{
		delta = *classdata->icount + 1;
		classdata->cycles_stolen += delta;
		classdata->cycles_running -= delta;
		*classdata->icount -= delta;
	}
}



/***************************************************************************
    SYNCHRONIZATION HELPERS
***************************************************************************/

/*-------------------------------------------------
    cpu_yield - yield the given CPU until the end
    of the current timeslice
-------------------------------------------------*/

void cpu_yield(const device_config *device)
{
	/* suspend against the timeslice */
	cpu_suspend(device, SUSPEND_REASON_TIMESLICE, FALSE);
}


/*-------------------------------------------------
    cpu_spin - burn CPU cycles until our timeslice
    is up
-------------------------------------------------*/

void cpu_spin(const device_config *device)
{
	/* suspend against the timeslice */
	cpu_suspend(device, SUSPEND_REASON_TIMESLICE, TRUE);
}


/*-------------------------------------------------
    cpu_spinuntil_trigger - burn specified CPU
    cycles until a timer trigger
-------------------------------------------------*/

void cpu_spinuntil_trigger(const device_config *device, int trigger)
{
	/* suspend until the given trigger fires */
	suspend_until_trigger(device, trigger, TRUE);
}


/*-------------------------------------------------
    cpu_spinuntil_int - burn CPU cycles until the
    next interrupt
-------------------------------------------------*/

void cpu_spinuntil_int(const device_config *device)
{
	cpu_class_data *classdata = cpu_get_class_data(device);

	/* suspend until the given trigger fires */
	suspend_until_trigger(device, classdata->inttrigger, TRUE);
}


/*-------------------------------------------------
    cpu_spinuntil_time - burn CPU cycles for a
    specific period of time
-------------------------------------------------*/

void cpu_spinuntil_time(const device_config *device, attotime duration)
{
	static int timetrig = 0;

	/* suspend until the given trigger fires */
	suspend_until_trigger(device, TRIGGER_SUSPENDTIME + timetrig, TRUE);

	/* then set a timer for it */
	cpuexec_triggertime(device->machine, TRIGGER_SUSPENDTIME + timetrig, duration);
	timetrig = (timetrig + 1) % 256;
}



/***************************************************************************
    TRIGGERS
***************************************************************************/

/*-------------------------------------------------
    cpuexec_trigger - generate a trigger now
-------------------------------------------------*/

void cpuexec_trigger(running_machine *machine, int trigger)
{
	const device_config *cpu;
	
	/* look for suspended CPUs waiting for this trigger and unsuspend them */
	for (cpu = machine->cpu[0]; cpu != NULL; cpu = cpu->typenext)
	{
		cpu_class_data *classdata = cpu_get_class_data(cpu);

		/* if we're executing, for an immediate abort */
		cpu_abort_timeslice(cpu);

		/* see if this is a matching trigger */
		if (classdata->suspend != 0 && classdata->trigger == trigger)
		{
			cpu_resume(cpu, SUSPEND_REASON_TRIGGER);
			classdata->trigger = 0;
		}
	}
}


/*-------------------------------------------------
    cpuexec_triggertime - generate a trigger after a
    specific period of time
-------------------------------------------------*/

void cpuexec_triggertime(running_machine *machine, int trigger, attotime duration)
{
	timer_set(machine, duration, NULL, trigger, triggertime_callback);
}


/*-------------------------------------------------
    cpu_triggerint - generate a trigger
    corresponding to an interrupt on the given CPU
-------------------------------------------------*/

void cpu_triggerint(const device_config *device)
{
	cpu_class_data *classdata = cpu_get_class_data(device);

	/* signal this CPU's interrupt trigger */
	cpuexec_trigger(device->machine, classdata->inttrigger);
}



/***************************************************************************
    INTERRUPTS
***************************************************************************/

/*-------------------------------------------------
    cpu_set_input_line - set the logical state
    (ASSERT_LINE/CLEAR_LINE) of an input line
    on a CPU
-------------------------------------------------*/

void cpu_set_input_line(const device_config *device, int line, int state)
{
	cpu_class_data *classdata = cpu_get_class_data(device);
	int vector = (line >= 0 && line < MAX_INPUT_LINES) ? classdata->input[line].vector : 0xff;
	cpu_set_input_line_and_vector(device, line, state, vector);
}


/*-------------------------------------------------
    cpu_set_input_line_vector - set the vector to
    be returned during a CPU's interrupt
    acknowledge cycle
-------------------------------------------------*/

void cpu_set_input_line_vector(const device_config *device, int line, int vector)
{
	cpu_class_data *classdata = cpu_get_class_data(device);
	if (line >= 0 && line < MAX_INPUT_LINES)
	{
		classdata->input[line].vector = vector;
		return;
	}
	LOG(("cpu_set_input_line_vector CPU '%s' line %d > max input lines\n", device->tag, line));
}


/*-------------------------------------------------
    cpu_set_input_line_and_vector - set the logical
    state (ASSERT_LINE/CLEAR_LINE) of an input
    line on a CPU and its associated vector
-------------------------------------------------*/

void cpu_set_input_line_and_vector(const device_config *device, int line, int state, int vector)
{
	cpu_class_data *classdata = cpu_get_class_data(device);

	/* catch errors where people use PULSE_LINE for CPUs that don't support it */
	if (state == PULSE_LINE && line != INPUT_LINE_NMI && line != INPUT_LINE_RESET)
		fatalerror("CPU %s: PULSE_LINE can only be used for NMI and RESET lines\n", device->tag);

	if (line >= 0 && line < MAX_INPUT_LINES)
	{
		cpu_input_data *inputline = &classdata->input[line];
		INT32 input_event = (state & 0xff) | (vector << 8);
		int event_index = inputline->qindex++;

		LOG(("cpu_set_input_line_and_vector('%s',%d,%d,%02x)\n", device->tag, line, state, vector));

		/* if we're full of events, flush the queue and log a message */
		if (event_index >= ARRAY_LENGTH(inputline->queue))
		{
			inputline->qindex--;
			empty_event_queue(device->machine, (void *)device, line);
			event_index = inputline->qindex++;
			logerror("Exceeded pending input line event queue on CPU '%s'!\n", device->tag);
		}

		/* enqueue the event */
		if (event_index < ARRAY_LENGTH(inputline->queue))
		{
			inputline->queue[event_index] = input_event;

			/* if this is the first one, set the timer */
			if (event_index == 0)
				timer_call_after_resynch(device->machine, (void *)device, line, empty_event_queue);
		}
	}
}


/*-------------------------------------------------
    cpu_set_irq_callback - install a driver-
    specific callback for IRQ acknowledge
-------------------------------------------------*/

void cpu_set_irq_callback(const device_config *device, cpu_irq_callback callback)
{
	cpu_class_data *classdata = cpu_get_class_data(device);
	classdata->driver_irq = callback;
}



/***************************************************************************
    CHEESY FAKE VIDEO TIMING (OBSOLETE)
***************************************************************************/

/*-------------------------------------------------
    cpu_getiloops - return the cheesy VBLANK
    interrupt counter (deprecated)
-------------------------------------------------*/

int cpu_getiloops(const device_config *device)
{
	cpu_class_data *classdata = cpu_get_class_data(device);
	return classdata->iloops;
}



/***************************************************************************
    INTERNAL FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    update_clock_information - recomputes clock
    information for the specified CPU
-------------------------------------------------*/

static void update_clock_information(const device_config *device)
{
	cpu_class_data *classdata = cpu_get_class_data(device);
	INT64 attos;

	/* recompute cps and spc */
	classdata->cycles_per_second = (double)classdata->clock * classdata->clockscale;
	classdata->attoseconds_per_cycle = ATTOSECONDS_PER_SECOND / ((double)classdata->clock * classdata->clockscale);

	/* update the CPU's divisor */
	attos = classdata->attoseconds_per_cycle;
	classdata->divshift = 0;
	while (attos >= (1UL << 31))
	{
		classdata->divshift++;
		attos >>= 1;
	}
	classdata->divisor = attos;

	/* re-compute the perfect interleave factor */
	compute_perfect_interleave(device->machine);
}


/*-------------------------------------------------
    compute_perfect_interleave - compute the
    "perfect" interleave interval
-------------------------------------------------*/

static void compute_perfect_interleave(running_machine *machine)
{
	if (machine->cpu[0] != NULL && machine->cpu[0]->token != NULL)
	{
		cpu_class_data *classdata = cpu_get_class_data(machine->cpu[0]);
		attoseconds_t smallest = classdata->attoseconds_per_cycle * cpu_get_min_cycles(machine->cpu[0]);
		attoseconds_t perfect = ATTOSECONDS_PER_SECOND - 1;
		const device_config *cpu;

		/* start with a huge time factor and find the 2nd smallest cycle time */
		for (cpu = machine->cpu[0]->typenext; cpu != NULL; cpu = cpu->typenext)
			if (cpu->token != NULL)
			{
				cpu_class_data *classdata = cpu_get_class_data(cpu);
				attoseconds_t curtime = classdata->attoseconds_per_cycle * cpu_get_min_cycles(cpu);

				/* find the 2nd smallest cycle interval */
				if (curtime < smallest)
				{
					perfect = smallest;
					smallest = curtime;
				}
				else if (curtime < perfect)
					perfect = classdata->attoseconds_per_cycle;
			}

		/* adjust the final value */
		timer_set_minimum_quantum(machine, perfect);

		LOG(("Perfect interleave = %.9f, smallest = %.9f\n", ATTOSECONDS_TO_DOUBLE(perfect), ATTOSECONDS_TO_DOUBLE(smallest)));
	}
}


/*-------------------------------------------------
    on_vblank - calls any external callbacks
    for this screen
-------------------------------------------------*/

static void on_vblank(const device_config *device, void *param, int vblank_state)
{
	/* VBLANK starting */
	if (vblank_state)
	{
		const device_config *cpu;
		
		/* find any CPUs that have this screen as their VBLANK interrupt source */
		for (cpu = device->machine->cpu[0]; cpu != NULL; cpu = cpu->typenext)
		{
			const cpu_config *config = cpu->inline_config;
			cpu_class_data *classdata = cpu_get_class_data(cpu);
			int cpu_interested;

			/* start the interrupt counter */
			if (!(classdata->suspend & SUSPEND_REASON_DISABLE))
				classdata->iloops = 0;
			else
				classdata->iloops = -1;

			/* the hack style VBLANK decleration always uses the first screen */
			if (config->vblank_interrupts_per_frame > 1)
				cpu_interested = TRUE;

			/* for new style declaration, we need to compare the tags */
			else if (config->vblank_interrupt_screen != NULL)
				cpu_interested = (strcmp(config->vblank_interrupt_screen, device->tag) == 0);

			/* no VBLANK interrupt, not interested */
			else
				cpu_interested = FALSE;

			/* if interested, call the interrupt handler */
			if (cpu_interested)
			{
				if (!(classdata->suspend & (SUSPEND_REASON_HALT | SUSPEND_REASON_RESET | SUSPEND_REASON_DISABLE)))
					(*config->vblank_interrupt)(cpu);

				/* if we have more than one interrupt per frame, start the timer now to trigger the rest of them */
				if (config->vblank_interrupts_per_frame > 1 && !(classdata->suspend & SUSPEND_REASON_DISABLE))
				{
					classdata->partial_frame_period = attotime_div(video_screen_get_frame_period(device->machine->primary_screen), config->vblank_interrupts_per_frame);
					timer_adjust_oneshot(classdata->partial_frame_timer, classdata->partial_frame_period, 0);
				}
			}
		}
	}
}


/*-------------------------------------------------
    trigger_partial_frame_interrupt - called to
    trigger a partial frame interrupt
-------------------------------------------------*/

static TIMER_CALLBACK( trigger_partial_frame_interrupt )
{
	const device_config *device = ptr;
	const cpu_config *config = device->inline_config;
	cpu_class_data *classdata = cpu_get_class_data(device);

	if (classdata->iloops == 0)
		classdata->iloops = config->vblank_interrupts_per_frame;

	classdata->iloops--;

	/* call the interrupt handler */
	if (!cpu_is_suspended(device, SUSPEND_REASON_HALT | SUSPEND_REASON_RESET | SUSPEND_REASON_DISABLE))
		(*config->vblank_interrupt)(device);

	/* more? */
	if (classdata->iloops > 1)
		timer_adjust_oneshot(classdata->partial_frame_timer, classdata->partial_frame_period, 0);
}


/*-------------------------------------------------
    trigger_periodic_interrupt - timer callback for
    timed interrupts
-------------------------------------------------*/

static TIMER_CALLBACK( trigger_periodic_interrupt )
{
	const device_config *device = ptr;
	const cpu_config *config = device->inline_config;

	/* bail if there is no routine */
	if (config->timed_interrupt != NULL && !cpu_is_suspended(device, SUSPEND_REASON_HALT | SUSPEND_REASON_RESET | SUSPEND_REASON_DISABLE))
		(*config->timed_interrupt)(device);
}


/*-------------------------------------------------
    triggertime_callback - signal a global trigger
-------------------------------------------------*/

static TIMER_CALLBACK( triggertime_callback )
{
	cpuexec_trigger(machine, param);
}



/*-------------------------------------------------
    empty_event_queue - empty a CPU's event queue
    for a specific input line
-------------------------------------------------*/

static TIMER_CALLBACK( empty_event_queue )
{
	const device_config *device = ptr;
	cpu_class_data *classdata = cpu_get_class_data(device);
	cpu_input_data *inputline = &classdata->input[param];
	int curevent;

	/* loop over all events */
	for (curevent = 0; curevent < inputline->qindex; curevent++)
	{
		INT32 input_event = inputline->queue[curevent];
		int state = input_event & 0xff;
		int vector = input_event >> 8;

		/* set the input line state and vector */
		inputline->curstate = state;
		inputline->curvector = vector;

		/* special case: RESET */
		if (param == INPUT_LINE_RESET)
		{
			/* if we're asserting the line, just halt the CPU */
			if (state == ASSERT_LINE)
				cpu_suspend(device, SUSPEND_REASON_RESET, 1);
			else
			{
				/* if we're clearing the line that was previously asserted, or if we're just */
				/* pulsing the line, reset the CPU */
				if ((state == CLEAR_LINE && cpu_is_suspended(device, SUSPEND_REASON_RESET)) || state == PULSE_LINE)
					device_reset(device);

				/* if we're clearing the line, make sure the CPU is not halted */
				cpu_resume(device, SUSPEND_REASON_RESET);
			}
		}

		/* special case: HALT */
		else if (param == INPUT_LINE_HALT)
		{
			/* if asserting, halt the CPU */
			if (state == ASSERT_LINE)
				cpu_suspend(device, SUSPEND_REASON_HALT, 1);

			/* if clearing, unhalt the CPU */
			else if (state == CLEAR_LINE)
				cpu_resume(device, SUSPEND_REASON_HALT);
		}

		/* all other cases */
		else
		{
			/* switch off the requested state */
			switch (state)
			{
				case PULSE_LINE:
					device_set_info_int(device, CPUINFO_INT_INPUT_STATE + param, ASSERT_LINE);
					device_set_info_int(device, CPUINFO_INT_INPUT_STATE + param, CLEAR_LINE);
					break;

				case HOLD_LINE:
				case ASSERT_LINE:
					device_set_info_int(device, CPUINFO_INT_INPUT_STATE + param, ASSERT_LINE);
					break;

				case CLEAR_LINE:
					device_set_info_int(device, CPUINFO_INT_INPUT_STATE + param, CLEAR_LINE);
					break;

				default:
					logerror("empty_event_queue cpu '%s', line %d, unknown state %d\n", device->tag, param, state);
					break;
			}

			/* generate a trigger to unsuspend any CPUs waiting on the interrupt */
			if (state != CLEAR_LINE)
				cpu_triggerint(device);
		}
	}

	/* reset counter */
	inputline->qindex = 0;
}


/*-------------------------------------------------
    standard_irq_callback - IRQ acknowledge
    callback; handles HOLD_LINE case and signals
    to the debugger
-------------------------------------------------*/

static IRQ_CALLBACK( standard_irq_callback )
{
	cpu_class_data *classdata = cpu_get_class_data(device);
	cpu_input_data *inputline = &classdata->input[irqline];
	int vector = inputline->curvector;

	LOG(("standard_irq_callback('%s', %d) $%04x\n", device->tag, irqline, vector));

	/* if the IRQ state is HOLD_LINE, clear it */
	if (inputline->curstate == HOLD_LINE)
	{
		LOG(("->set_irq_line('%s',%d,%d)\n", device->tag, irqline, CLEAR_LINE));
		device_set_info_int(device, CPUINFO_INT_INPUT_STATE + irqline, CLEAR_LINE);
		inputline->curstate = CLEAR_LINE;
	}

	/* if there's a driver callback, run it */
	if (classdata->driver_irq != NULL)
		vector = (*classdata->driver_irq)(device, irqline);

	/* notify the debugger */
	debugger_interrupt_hook(device, irqline);

	/* otherwise, just return the current vector */
	return vector;
}


/*-------------------------------------------------
    register_save_states - register for CPU-
    specific save states
-------------------------------------------------*/

static void register_save_states(const device_config *device)
{
	cpu_class_data *classdata = cpu_get_class_data(device);
	int line;

	state_save_register_device_item(device, 0, classdata->suspend);
	state_save_register_device_item(device, 0, classdata->nextsuspend);
	state_save_register_device_item(device, 0, classdata->eatcycles);
	state_save_register_device_item(device, 0, classdata->nexteatcycles);
	state_save_register_device_item(device, 0, classdata->trigger);

	state_save_register_device_item(device, 0, classdata->iloops);

	state_save_register_device_item(device, 0, classdata->totalcycles);
	state_save_register_device_item(device, 0, classdata->localtime.seconds);
	state_save_register_device_item(device, 0, classdata->localtime.attoseconds);
	state_save_register_device_item(device, 0, classdata->clock);
	state_save_register_device_item(device, 0, classdata->clockscale);

	for (line = 0; line < ARRAY_LENGTH(classdata->input); line++)
	{
		cpu_input_data *inputline = &classdata->input[line];
		state_save_register_device_item(device, line, inputline->vector);
		state_save_register_device_item(device, line, inputline->curvector);
		state_save_register_device_item(device, line, inputline->curstate);
	}
}

