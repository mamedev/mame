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
typedef struct _cpuinput_data cpuinput_data;
struct _cpuinput_data
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
	/* this item must remain first */
	cpu_class_header header;				/* header containing public data */
	
	/* core interface */
	int *			icount;					/* pointer to the icount */

	/* input states and IRQ callbacks */
	cpu_irq_callback driver_irq;			/* driver-specific IRQ callback */
	cpuinput_data	input[MAX_INPUT_LINES]; /* data about inputs */

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

	/* these below are hacks to support multiple interrupts per frame */
	INT32 			iloops; 				/* number of interrupts remaining this frame */
	emu_timer *		partial_frame_timer;	/* the timer that triggers partial frame interrupts */
	attotime		partial_frame_period;	/* the length of one partial frame for interrupt purposes */
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* general CPU variables */
static int cycles_running;
static int cycles_stolen;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void cpuexec_exit(running_machine *machine);
static void cpuexec_reset(running_machine *machine);

static void cpu_timers_init(running_machine *machine);
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
    get_safe_classtoken - makes sure that the 
    passed in device is, in fact, a CPU, and
    return the class token
-------------------------------------------------*/

INLINE cpu_class_data *get_safe_classtoken(const device_config *device)
{
	assert(device != NULL);
	assert(device->classtoken != NULL);
	assert(device->class == DEVICE_CLASS_CPU_CHIP);

	return (cpu_class_data *)device->classtoken;
}


/*-------------------------------------------------
    suspend_until_trigger - suspend execution
    until the given trigger fires
-------------------------------------------------*/

INLINE void suspend_until_trigger(const device_config *device, int trigger, int eatcycles)
{
	cpu_class_data *classdata = get_safe_classtoken(device);

	/* suspend the CPU immediately if it's not already */
	cpu_suspend(device, SUSPEND_REASON_TRIGGER, eatcycles);

	/* set the trigger */
	classdata->trigger = trigger;
}



/***************************************************************************
    CORE CPU EXECUTION
***************************************************************************/

/*-------------------------------------------------
    cpuexec_create_cpu_device - temporary
    function to allocate a fake CPU device for
    each CPU
-------------------------------------------------*/

const device_config *cpuexec_create_cpu_device(const cpu_config *config)
{
	device_config *device;

	/* create a fake device for the CPU -- this will be done automatically in the future */
	device = malloc_or_die(sizeof(*device) + strlen(config->tag));
	memset(device, 0, sizeof(*device));
	strcpy(device->tag, config->tag);
	device->type = (device_type)config->type;
	device->class = DEVICE_CLASS_CPU_CHIP;
	device->inline_config = (void *)config;
	device->static_config = config->reset_param;
	
	return device;
}


/*-------------------------------------------------
    cpuexec_init - initialize internal states of
    all CPUs
-------------------------------------------------*/

void cpuexec_init(running_machine *machine)
{
	int cpunum;

	/* loop over all our CPUs */
	for (cpunum = 0; cpunum < ARRAY_LENGTH(machine->config->cpu); cpunum++)
		if (machine->cpu[cpunum] != NULL)
		{
			device_config *device = (device_config *)machine->cpu[cpunum];
			const cpu_config *config = device->inline_config;
			cpu_type cputype = config->type;
			cpu_class_data *classdata;
			int num_regs;
			int line;

			/* allocate memory for our class state */
			classdata = auto_malloc(sizeof(*classdata));
			memset(classdata, 0, sizeof(*classdata));
			
			/* fill in the header */
			classdata->header = *cputype_get_header_template(cputype);
			
			/* make the device run */
			device->started = FALSE;
			device->machine = machine;
			device->region = memory_region(machine, device->tag);
			device->regionbytes = memory_region_length(machine, device->tag);
			device->token = auto_malloc(cputype_get_context_size(cputype));
			memset(device->token, 0, cputype_get_context_size(cputype));
			device->classtoken = classdata;
			
			/* fill in the input states and IRQ callback information */
			for (line = 0; line < ARRAY_LENGTH(classdata->input); line++)
			{
				cpuinput_data *inputline = &classdata->input[line];
				/* vector and curvector are initialized later */
				inputline->curstate = CLEAR_LINE;
				inputline->qindex = 0;
			}
			
			/* fill in the suspend states */
			classdata->suspend = SUSPEND_REASON_RESET;
			classdata->inttrigger = cpunum + TRIGGER_INT;
			
			/* fill in the clock and timing information */
			classdata->clock = (UINT64)config->clock * classdata->header.clock_multiplier / classdata->header.clock_divider;
			classdata->clockscale = 1.0;
			
			/* allocate timers if we need them */
			if (config->vblank_interrupts_per_frame > 1)
				classdata->partial_frame_timer = timer_alloc(trigger_partial_frame_interrupt, device);
			if (config->timed_interrupt_period != 0)
				classdata->timedint_timer = timer_alloc(trigger_periodic_interrupt, device);

			/* initialize this CPU */
			state_save_push_tag(cpunum + 1);
			num_regs = state_save_get_reg_count();
			cpu_init(device, cpunum, classdata->clock, standard_irq_callback);
			num_regs = state_save_get_reg_count() - num_regs;
			state_save_pop_tag();

			/* fetch post-initialization data */
			classdata->icount = cpu_get_icount_ptr(device);
			for (line = 0; line < ARRAY_LENGTH(classdata->input); line++)
			{
				cpuinput_data *inputline = &classdata->input[line];
				inputline->vector = cpu_get_default_irq_vector(device);
				inputline->curvector = inputline->vector;
			}
			update_clock_information(device);
			
			/* if no state registered for saving, we can't save */
			if (num_regs == 0)
			{
				logerror("CPU #%d (%s) did not register any state to save!\n", cpunum, cputype_get_name(cputype));
				if (machine->gamedrv->flags & GAME_SUPPORTS_SAVE)
					fatalerror("CPU #%d (%s) did not register any state to save!", cpunum, cputype_get_name(cputype));
			}

			/* register some internal states as well */
			register_save_states(device);
		}

	add_reset_callback(machine, cpuexec_reset);
	add_exit_callback(machine, cpuexec_exit);
}


/*-------------------------------------------------
    cpuexec_reset - reset CPU states on a soft
    reset
-------------------------------------------------*/

static void cpuexec_reset(running_machine *machine)
{
	int cpunum;

	/* initialize the various timers */
	cpu_timers_init(machine);

	/* first pass over CPUs */
	for (cpunum = 0; cpunum < ARRAY_LENGTH(machine->cpu); cpunum++)
		if (machine->cpu[cpunum] != NULL)
		{
			const device_config *device = machine->cpu[cpunum];
			const cpu_config *config = device->inline_config;
			cpu_class_data *classdata = device->classtoken;
			int line;

			/* enable all CPUs (except for disabled CPUs) */
			if (!(config->flags & CPU_DISABLE))
				cpu_resume(device, SUSPEND_ANY_REASON);
			else
				cpu_suspend(device, SUSPEND_REASON_DISABLE, 1);

			/* reset the total number of cycles */
			classdata->totalcycles = 0;

			/* then reset the CPU directly */
			cpu_reset(device);

			/* reset the interrupt vectors and queues */
			for (line = 0; line < ARRAY_LENGTH(classdata->input); line++)
			{
				cpuinput_data *inputline = &classdata->input[line];
				inputline->vector = cpu_get_default_irq_vector(device);
				inputline->qindex = 0;
			}
		}
}


/*-------------------------------------------------
    cpuexec_exit - cleanup all CPUs on exit
-------------------------------------------------*/

static void cpuexec_exit(running_machine *machine)
{
	int cpunum;

	/* shut down the CPU cores */
	for (cpunum = 0; cpunum < ARRAY_LENGTH(machine->cpu); cpunum++)
		if (machine->cpu[cpunum] != NULL)
			cpu_exit(machine->cpu[cpunum]);
}


/*-------------------------------------------------
    cpuexec_timeslice - execute all CPUs for a
    single timeslice
-------------------------------------------------*/

void cpuexec_timeslice(running_machine *machine)
{
	int call_debugger = ((machine->debug_flags & DEBUG_FLAG_ENABLED) != 0);
	attotime target = timer_next_fire_time();
	attotime base = timer_get_time();
	int cpunum, ran;

	LOG(("------------------\n"));
	LOG(("cpu_timeslice: target = %s\n", attotime_string(target, 9)));

	/* apply pending suspension changes */
	for (cpunum = 0; machine->cpu[cpunum] != NULL; cpunum++)
	{
		cpu_class_data *classdata = machine->cpu[cpunum]->classtoken;
		classdata->suspend = classdata->nextsuspend;
		classdata->nextsuspend &= ~SUSPEND_REASON_TIMESLICE;
		classdata->eatcycles = classdata->nexteatcycles;
	}

	/* loop over non-suspended CPUs */
	for (cpunum = 0; machine->cpu[cpunum] != NULL; cpunum++)
	{
		cpu_class_data *classdata = machine->cpu[cpunum]->classtoken;
		if (classdata->suspend == 0)
		{
			attotime delta = attotime_sub(target, classdata->localtime);
			if (delta.seconds >= 0 && delta.attoseconds >= attoseconds_per_cycle[cpunum])
			{
				/* compute how long to run */
				cycles_running = div_64x32(delta.attoseconds >> classdata->divshift, classdata->divisor);
				LOG(("  cpu %d: %d cycles\n", cpunum, cycles_running));

				profiler_mark(PROFILER_CPU1 + cpunum);

				/* note that this global variable cycles_stolen can be modified */
				/* via the call to the cpunum_execute */
				cycles_stolen = 0;
				if (!call_debugger)
					ran = cpu_execute(machine->cpu[cpunum], cycles_running);
				else
				{
					debugger_start_cpu_hook(machine, cpunum, target);
					ran = cpu_execute(machine->cpu[cpunum], cycles_running);
					debugger_stop_cpu_hook(machine, cpunum);
				}

#ifdef MAME_DEBUG
				if (ran < cycles_stolen)
					fatalerror("Negative CPU cycle count!");
#endif /* MAME_DEBUG */

				ran -= cycles_stolen;
				profiler_mark(PROFILER_END);

				/* account for these cycles */
				classdata->totalcycles += ran;
				classdata->localtime = attotime_add_attoseconds(classdata->localtime, ran * attoseconds_per_cycle[cpunum]);
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

	/* update the local times of all CPUs */
	for (cpunum = 0; cpunum < ARRAY_LENGTH(machine->cpu) && machine->cpu[cpunum] != NULL; cpunum++)
	{
		cpu_class_data *classdata = machine->cpu[cpunum]->classtoken;

		/* if we're suspended and counting, process */
		if (classdata->suspend != 0 && classdata->eatcycles && attotime_compare(classdata->localtime, target) < 0)
		{
			attotime delta = attotime_sub(target, classdata->localtime);

			/* compute how long to run */
			cycles_running = div_64x32(delta.attoseconds >> classdata->divshift, classdata->divisor);
			LOG(("  cpu %d: %d cycles (suspended)\n", cpunum, cycles_running));

			classdata->totalcycles += cycles_running;
			classdata->localtime = attotime_add_attoseconds(classdata->localtime, cycles_running * attoseconds_per_cycle[cpunum]);
			LOG(("         %d skipped, %d total, time = %s\n", cycles_running, (INT32)classdata->totalcycles, attotime_string(classdata->localtime, 9)));
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


/*-------------------------------------------------
    cputag_get_cpu - return a pointer to the given 
    CPU by tag
-------------------------------------------------*/

const device_config *cputag_get_cpu(running_machine *machine, const char *tag)
{
	int cpunum;
	
	for (cpunum = 0; cpunum < ARRAY_LENGTH(machine->cpu); cpunum++)
		if (machine->cpu[cpunum] != NULL && strcmp(tag, machine->cpu[cpunum]->tag) == 0)
			return machine->cpu[cpunum];
	
	return NULL;
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
	cpu_class_data *classdata = get_safe_classtoken(device);
	
	/* set the suspend reason and eat cycles flag */
	classdata->nextsuspend |= reason;
	classdata->nexteatcycles = eatcycles;
	
	/* if we're active, synchronize */
	if (device == device->machine->activecpu)
		cpu_abort_timeslice(device);
}


/*-------------------------------------------------
    cpu_resume - clear a suspend reason for the
    given CPU
-------------------------------------------------*/

void cpu_resume(const device_config *device, int reason)
{
	cpu_class_data *classdata = get_safe_classtoken(device);

	/* clear the suspend reason and eat cycles flag */
	classdata->nextsuspend &= ~reason;

	/* if we're active, synchronize */
	if (device == device->machine->activecpu)
		cpu_abort_timeslice(device);
}


/*-------------------------------------------------
    cpu_is_suspended - returns true if the
    given CPU is suspended for any of the given
    reasons
-------------------------------------------------*/

int cpu_is_suspended(const device_config *device, int reason)
{
	cpu_class_data *classdata = get_safe_classtoken(device);
	
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
	cpu_class_data *classdata = get_safe_classtoken(device);

	/* return the current clock value */
	return classdata->clock;
}


/*-------------------------------------------------
    cpu_set_clock - sets the given CPU's
    clock speed
-------------------------------------------------*/

void cpu_set_clock(const device_config *device, int clock)
{
	cpu_class_data *classdata = get_safe_classtoken(device);

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
	cpu_class_data *classdata = get_safe_classtoken(device);

	/* return the current clock scale factor */
	return classdata->clockscale;
}


/*-------------------------------------------------
    cpu_set_clockscale - sets the current
    scaling factor for a CPU's clock speed
-------------------------------------------------*/

void cpu_set_clockscale(const device_config *device, double clockscale)
{
	cpu_class_data *classdata = get_safe_classtoken(device);

	/* set the scale factor and update the information */
	classdata->clockscale = clockscale;
	update_clock_information(device);
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
	cpu_class_data *classdata = get_safe_classtoken(device);
	attotime result;

	/* if we're active, add in the time from the current slice */
	result = classdata->localtime;
	if (device == device->machine->activecpu && classdata->icount != NULL)
	{
		int cycles = cycles_running - *classdata->icount;
		result = attotime_add(result, ATTOTIME_IN_CYCLES(cycles, classdata->header.index));
	}
	return result;
}


/*-------------------------------------------------
    cpu_get_total_cycles - return the total
    number of CPU cycles executed on the active
    CPU
-------------------------------------------------*/

UINT64 cpu_get_total_cycles(const device_config *device)
{
	cpu_class_data *classdata = get_safe_classtoken(device);

	if (device == device->machine->activecpu && classdata->icount != NULL)
		return classdata->totalcycles + cycles_running - *classdata->icount;
	else
		return classdata->totalcycles;
}


/*-------------------------------------------------
    cpu_eat_cycles - safely eats cycles so
    we don't cross a timeslice boundary
-------------------------------------------------*/

void cpu_eat_cycles(const device_config *device, int cycles)
{
	cpu_class_data *classdata = get_safe_classtoken(device);

	assert(device == device->machine->activecpu);

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
	cpu_class_data *classdata = get_safe_classtoken(device);

	assert(device == device->machine->activecpu);

	*classdata->icount += delta;
}


/*-------------------------------------------------
    cpu_abort_timeslice - abort execution
    for the current timeslice, allowing other
    CPUs to run before we run again
-------------------------------------------------*/

void cpu_abort_timeslice(const device_config *device)
{
	cpu_class_data *classdata = get_safe_classtoken(device);
	int delta;

	assert(device == device->machine->activecpu);

	/* swallow the remaining cycles */
	if (classdata->icount != NULL)
	{
		delta = *classdata->icount + 1;
		cycles_stolen += delta;
		cycles_running -= delta;
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
	cpu_class_data *classdata = get_safe_classtoken(device);

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
	int cpunum;

	/* cause an immediate resynchronization */
	if (machine->activecpu != NULL)
		cpu_abort_timeslice(machine->activecpu);

	/* look for suspended CPUs waiting for this trigger and unsuspend them */
	for (cpunum = 0; cpunum < ARRAY_LENGTH(machine->cpu) && machine->cpu[cpunum] != NULL; cpunum++)
	{
		cpu_class_data *classdata = machine->cpu[cpunum]->classtoken;
		
		/* see if this is a matching trigger */
		if (classdata->suspend != 0 && classdata->trigger == trigger)
		{
			cpu_resume(machine->cpu[cpunum], SUSPEND_REASON_TRIGGER);
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
	timer_set(duration, NULL, trigger, triggertime_callback);
}


/*-------------------------------------------------
    cpu_triggerint - generate a trigger
    corresponding to an interrupt on the given CPU
-------------------------------------------------*/

void cpu_triggerint(const device_config *device)
{
	cpu_class_data *classdata = get_safe_classtoken(device);
	
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
	cpu_class_data *classdata = get_safe_classtoken(device);
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
	cpu_class_data *classdata = get_safe_classtoken(device);
	if (line >= 0 && line < MAX_INPUT_LINES)
	{
		classdata->input[line].vector = vector;
		return;
	}
	LOG(("cpunum_set_input_line_vector CPU '%s' line %d > max input lines\n", device->tag, line));
}


/*-------------------------------------------------
    cpu_set_input_line_and_vector - set the logical 
    state (ASSERT_LINE/CLEAR_LINE) of an input 
    line on a CPU and its associated vector
-------------------------------------------------*/

void cpu_set_input_line_and_vector(const device_config *device, int line, int state, int vector)
{
	cpu_class_data *classdata = get_safe_classtoken(device);

#ifdef MAME_DEBUG
	/* catch errors where people use PULSE_LINE for CPUs that don't support it */
	if (state == PULSE_LINE && line != INPUT_LINE_NMI && line != INPUT_LINE_RESET)
	{
		switch ((cpu_type)device->type)
		{
			case CPU_Z80:
			case CPU_Z180:
			case CPU_M68000:
			case CPU_M68008:
			case CPU_M68010:
			case CPU_M68EC020:
			case CPU_M68020:
			case CPU_M68040:
			case CPU_R4600BE:
			case CPU_R4600LE:
			case CPU_R4650BE:
			case CPU_R4650LE:
			case CPU_R4700BE:
			case CPU_R4700LE:
			case CPU_R5000BE:
			case CPU_R5000LE:
			case CPU_QED5271BE:
			case CPU_QED5271LE:
			case CPU_RM7000BE:
			case CPU_RM7000LE:
			case CPU_PPC403GA:
			case CPU_PPC403GCX:
			case CPU_PPC601:
			case CPU_PPC602:
			case CPU_PPC603:
			case CPU_PPC603E:
			case CPU_PPC603R:
			case CPU_PPC604:
			case CPU_I8035:
			case CPU_I8041:
			case CPU_I8048:
			case CPU_I8648:
			case CPU_I8748:
			case CPU_MB8884:
			case CPU_N7751:
			case CPU_TMS34010:
			case CPU_TMS34020:
			case CPU_TMS32010:
			case CPU_TMS32025:
			case CPU_TMS32026:
				fatalerror("CPU %s: PULSE_LINE used with level-detected IRQ %d\n", device->tag, line);
				break;

			default:
				break;
		}
	}
#endif

	if (line >= 0 && line < MAX_INPUT_LINES)
	{
		cpuinput_data *inputline = &classdata->input[line];
		INT32 input_event = (state & 0xff) | (vector << 8);
		int event_index = inputline->qindex++;

		LOG(("cpunum_set_input_line_and_vector('%s',%d,%d,%02x)\n", device->tag, line, state, vector));

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
				timer_call_after_resynch((void *)device, line, empty_event_queue);
		}
	}
}


/*-------------------------------------------------
    cpu_set_irq_callback - install a driver-
    specific callback for IRQ acknowledge
-------------------------------------------------*/

void cpu_set_irq_callback(const device_config *device, cpu_irq_callback callback)
{
	cpu_class_data *classdata = get_safe_classtoken(device);
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
	cpu_class_data *classdata = get_safe_classtoken(device);
	return classdata->iloops;
}



/***************************************************************************
    INTERNAL FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    cpu_timers_init - set up all the core timers
-------------------------------------------------*/

static void cpu_timers_init(running_machine *machine)
{
	int numscreens = video_screen_count(machine->config);
	attoseconds_t refresh_attosecs;
	int cpunum, ipf;

	/* set the core scheduling quantum */
	ipf = machine->config->cpu_slices_per_frame;
	if (ipf <= 0)
		ipf = 1;
	refresh_attosecs = (numscreens == 0) ? HZ_TO_ATTOSECONDS(60) : video_screen_get_frame_period(machine->primary_screen).attoseconds;
	timer_add_scheduling_quantum(machine, refresh_attosecs / ipf, attotime_never);

	/* register the interrupt handler callbacks */
	for (cpunum = 0; cpunum < ARRAY_LENGTH(machine->cpu); cpunum++)
		if (machine->cpu[cpunum] != NULL)
		{
			const device_config *device = machine->cpu[cpunum];
			const cpu_config *config = device->inline_config;
			cpu_class_data *classdata = device->classtoken;

			/* VBLANK interrupts */
			if (config->vblank_interrupts_per_frame > 0)
			{
				const device_config *screen;

				/* get the screen that will trigger the VBLANK */

				/* new style - use screen tag directly */
				if (config->vblank_interrupts_per_frame == 1)
					screen = device_list_find_by_tag(machine->config->devicelist, VIDEO_SCREEN, config->vblank_interrupt_screen);

				/* old style 'hack' setup - use screen #0 */
				else
					screen = device_list_first(machine->config->devicelist, VIDEO_SCREEN);

				assert(screen != NULL);
				video_screen_register_vblank_callback(screen, on_vblank, NULL);
			}

			/* periodic interrupts */
			if (config->timed_interrupt_period != 0)
			{
				attotime timedint_period = attotime_make(0, config->timed_interrupt_period);
				assert(classdata->timedint_timer != NULL);
				timer_adjust_periodic(classdata->timedint_timer, timedint_period, cpunum, timedint_period);
			}
		}
}


/*-------------------------------------------------
    update_clock_information - recomputes clock
    information for the specified CPU
-------------------------------------------------*/

static void update_clock_information(const device_config *device)
{
	cpu_class_data *classdata = device->classtoken;
	INT64 attos;

	/* recompute cps and spc */
	cycles_per_second[classdata->header.index] = (double)classdata->clock * classdata->clockscale;
	attoseconds_per_cycle[classdata->header.index] = ATTOSECONDS_PER_SECOND / ((double)classdata->clock * classdata->clockscale);

	/* update the CPU's divisor */
	attos = attoseconds_per_cycle[classdata->header.index];
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
	if (attoseconds_per_cycle[0] != 0)
	{
		attoseconds_t smallest = attoseconds_per_cycle[0] * cputype_get_min_cycles(machine->config->cpu[0].type);
		attoseconds_t perfect = ATTOSECONDS_PER_SECOND - 1;
		int cpunum;

		/* start with a huge time factor and find the 2nd smallest cycle time */
		for (cpunum = 1; cpunum < ARRAY_LENGTH(machine->cpu) && machine->cpu[cpunum] != NULL; cpunum++)
			if (attoseconds_per_cycle[cpunum] != 0)
			{
				attoseconds_t curtime = attoseconds_per_cycle[cpunum] * cputype_get_min_cycles(machine->config->cpu[cpunum].type);

				/* find the 2nd smallest cycle interval */
				if (curtime < smallest)
				{
					perfect = smallest;
					smallest = curtime;
				}
				else if (curtime < perfect)
					perfect = attoseconds_per_cycle[cpunum];
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
		int cpunum;

		/* find any CPUs that have this screen as their VBLANK interrupt source */
		for (cpunum = 0; cpunum < ARRAY_LENGTH(device->machine->cpu); cpunum++)
			if (device->machine->cpu[cpunum] != NULL)
			{
				const device_config *cpudevice = device->machine->cpu[cpunum];
				const cpu_config *config = cpudevice->inline_config;
				cpu_class_data *classdata = cpudevice->classtoken;
				int cpu_interested;

				/* start the interrupt counter */
				if (!(classdata->suspend & SUSPEND_REASON_DISABLE))
					classdata->iloops = 0;
				else
					classdata->iloops = -1;

				/* the hack style VBLANK decleration always uses the first screen */
				if (config->vblank_interrupts_per_frame > 1)
					cpu_interested = TRUE;

				/* for new style decleration, we need to compare the tags */
				else if (config->vblank_interrupts_per_frame == 1)
					cpu_interested = (strcmp(config->vblank_interrupt_screen, device->tag) == 0);

				/* no VBLANK interrupt, not interested */
				else
					cpu_interested = FALSE;

				/* if interested, call the interrupt handler */
				if (cpu_interested)
				{
					if (!(classdata->suspend & (SUSPEND_REASON_HALT | SUSPEND_REASON_RESET | SUSPEND_REASON_DISABLE)))
					{
						cpu_push_context(cpudevice);
						(*config->vblank_interrupt)(cpudevice);
						cpu_pop_context();
					}

					/* if we have more than one interrupt per frame, start the timer now to trigger the rest of them */
					if (config->vblank_interrupts_per_frame > 1 && !(classdata->suspend & SUSPEND_REASON_DISABLE))
					{
						classdata->partial_frame_period = attotime_div(video_screen_get_frame_period(device->machine->primary_screen), config->vblank_interrupts_per_frame);
						timer_adjust_oneshot(classdata->partial_frame_timer, classdata->partial_frame_period, cpunum);
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
	cpu_class_data *classdata = device->classtoken;

	if (classdata->iloops == 0)
		classdata->iloops = config->vblank_interrupts_per_frame;

	classdata->iloops--;

	/* call the interrupt handler */
	if (!cpu_is_suspended(device, SUSPEND_REASON_HALT | SUSPEND_REASON_RESET | SUSPEND_REASON_DISABLE))
	{
		cpu_push_context(device);
		(*config->vblank_interrupt)(device);
		cpu_pop_context();
	}

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
	{
		cpu_push_context(device);
		(*config->timed_interrupt)(device);
		cpu_pop_context();
	}
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
	cpu_class_data *classdata = device->classtoken;
	cpuinput_data *inputline = &classdata->input[param];
	int curevent;

	/* swap to the CPU's context */
	cpu_push_context(device);

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
					cpu_reset(device);

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
					/* temporary: PULSE_LINE only makes sense for NMI lines on Z80 */
					assert((cpu_type)device->type != CPU_Z80 || param == INPUT_LINE_NMI);
					cpu_set_info_int(device, CPUINFO_INT_INPUT_STATE + param, ASSERT_LINE);
					cpu_set_info_int(device, CPUINFO_INT_INPUT_STATE + param, CLEAR_LINE);
					break;

				case HOLD_LINE:
				case ASSERT_LINE:
					cpu_set_info_int(device, CPUINFO_INT_INPUT_STATE + param, ASSERT_LINE);
					break;

				case CLEAR_LINE:
					cpu_set_info_int(device, CPUINFO_INT_INPUT_STATE + param, CLEAR_LINE);
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

	/* swap back */
	cpu_pop_context();

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
	cpu_class_data *classdata = device->classtoken;
	cpuinput_data *inputline = &classdata->input[irqline];
	int vector = inputline->curvector;

	LOG(("standard_irq_callback('%s', %d) $%04x\n", device->tag, irqline, vector));

	/* if the IRQ state is HOLD_LINE, clear it */
	if (inputline->curstate == HOLD_LINE)
	{
		LOG(("->set_irq_line('%s',%d,%d)\n", device->tag, irqline, CLEAR_LINE));
		cpu_set_info_int(device, CPUINFO_INT_INPUT_STATE + irqline, CLEAR_LINE);
		inputline->curstate = CLEAR_LINE;
	}

	/* if there's a driver callback, run it */
	if (classdata->driver_irq != NULL)
		vector = (*classdata->driver_irq)(device, irqline);

	/* notify the debugger */
	debug_cpu_interrupt_hook(device->machine, classdata->header.index, irqline);

	/* otherwise, just return the current vector */
	return vector;
}


/*-------------------------------------------------
    register_save_states - register for CPU-
    specific save states
-------------------------------------------------*/

static void register_save_states(const device_config *device)
{
	cpu_class_data *classdata = device->classtoken;
	int line;
	
	state_save_register_item("cpu", classdata->header.index, classdata->suspend);
	state_save_register_item("cpu", classdata->header.index, classdata->nextsuspend);
	state_save_register_item("cpu", classdata->header.index, classdata->eatcycles);
	state_save_register_item("cpu", classdata->header.index, classdata->nexteatcycles);
	state_save_register_item("cpu", classdata->header.index, classdata->trigger);

	state_save_register_item("cpu", classdata->header.index, classdata->iloops);

	state_save_register_item("cpu", classdata->header.index, classdata->totalcycles);
	state_save_register_item("cpu", classdata->header.index, classdata->localtime.seconds);
	state_save_register_item("cpu", classdata->header.index, classdata->localtime.attoseconds);
	state_save_register_item("cpu", classdata->header.index, classdata->clock);
	state_save_register_item("cpu", classdata->header.index, classdata->clockscale);

	for (line = 0; line < ARRAY_LENGTH(classdata->input); line++)
	{
		cpuinput_data *inputline = &classdata->input[line];
		int index = classdata->header.index * ARRAY_LENGTH(classdata->input) + line;
		state_save_register_item("cpu", index, inputline->vector);
		state_save_register_item("cpu", index, inputline->curvector);
		state_save_register_item("cpu", index, inputline->curstate);
	}
}

