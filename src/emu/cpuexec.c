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
	TRIGGER_YIELDTIME	= -3000,
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
	/* execution lists */
	const device_config *device;			/* pointer back to our device */
	cpu_class_data *next;					/* pointer to the next CPU to execute, in order */
	cpu_execute_func execute;				/* execute function pointer */

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
	UINT64			totalcycles;			/* total CPU cycles executed */
	attotime		localtime;				/* local time, relative to the timer system's global time */
	INT32			clock;					/* current active clock */
	double			clockscale;				/* current active clock scale factor */
	INT32			divisor;				/* 32-bit attoseconds_per_cycle divisor */
	UINT8			divshift;				/* right shift amount to fit the divisor into 32 bits */
	emu_timer *		timedint_timer;			/* reference to this CPU's periodic interrupt timer */
	UINT32			cycles_per_second;		/* cycles per second, adjusted for multipliers */
	attoseconds_t	attoseconds_per_cycle;	/* attoseconds per adjusted clock cycle */

	/* internal state reflection */
	const cpu_state_table *state;			/* pointer to the base table */
	const cpu_state_entry *regstate[MAX_REGS];/* pointer to the state entry for each register */

	/* these below are hacks to support multiple interrupts per frame */
	INT32			iloops; 				/* number of interrupts remaining this frame */
	emu_timer *		partial_frame_timer;	/* the timer that triggers partial frame interrupts */
	attotime		partial_frame_period;	/* the length of one partial frame for interrupt purposes */
};


/* global data stored in the machine */
/* In mame.h: typedef struct _cpuexec_private cpuexec_private; */
struct _cpuexec_private
{
	const device_config *executingcpu;		/* pointer to the currently executing CPU */
	cpu_class_data *executelist;			/* execution list; suspended CPUs are at the back */
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
static void rebuild_execute_list(running_machine *machine);
static UINT64 get_register_value(const device_config *device, void *baseptr, const cpu_state_entry *entry);
static void set_register_value(const device_config *device, void *baseptr, const cpu_state_entry *entry, UINT64 value);
static void get_register_string_value(const device_config *device, void *baseptr, const cpu_state_entry *entry, char *dest);
#ifdef UNUSED_FUNCTION
static int get_register_string_max_width(const device_config *device, void *baseptr, const cpu_state_entry *entry);
#endif



/***************************************************************************
    MACROS
***************************************************************************/

/* these are macros to ensure inlining in cpuexec_timeslice */
#define ATTOTIME_LT(a,b)		((a).seconds < (b).seconds || ((a).seconds == (b).seconds && (a).attoseconds < (b).attoseconds))
#define ATTOTIME_NORMALIZE(a)	do { if ((a).attoseconds >= ATTOSECONDS_PER_SECOND) { (a).seconds++; (a).attoseconds -= ATTOSECONDS_PER_SECOND; } } while (0)



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    get_class_data - return a pointer to the
    class data
-------------------------------------------------*/

INLINE cpu_class_data *get_class_data(const device_config *device)
{
	assert(device != NULL);
	assert(device->devclass == DEVICE_CLASS_CPU_CHIP);
	assert(device->token != NULL);
	return (cpu_class_data *)cpu_get_class_header(device) - 1;
}


/*-------------------------------------------------
    get_minimum_quantum - return the minimum
    quantum required for a given CPU device
-------------------------------------------------*/

INLINE attoseconds_t get_minimum_quantum(const device_config *device)
{
	attoseconds_t basetick = 0;

	/* fetch the base clock from the classdata if present */
	if (device->token != NULL)
		basetick = get_class_data(device)->attoseconds_per_cycle;

	/* otherwise compute it from the raw data */
	if (basetick == 0)
	{
		UINT32 baseclock = (UINT64)device->clock * cpu_get_clock_multiplier(device) / cpu_get_clock_divider(device);
		basetick = HZ_TO_ATTOSECONDS(baseclock);
	}

	/* apply the minimum cycle count */
	return basetick * cpu_get_min_cycles(device);
}


/*-------------------------------------------------
    suspend_until_trigger - suspend execution
    until the given trigger fires
-------------------------------------------------*/

INLINE void suspend_until_trigger(const device_config *device, int trigger, int eatcycles)
{
	cpu_class_data *classdata = get_class_data(device);

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
	attotime min_quantum;

	/* allocate global state */
	machine->cpuexec_data = auto_alloc_clear(machine, cpuexec_private);

	/* set the core scheduling quantum */
	min_quantum = machine->config->minimum_quantum;
	if (attotime_compare(min_quantum, attotime_zero) == 0)
		min_quantum = ATTOTIME_IN_HZ(60);
	if (machine->config->perfect_cpu_quantum != NULL)
	{
		const device_config *cpu = cputag_get_cpu(machine, machine->config->perfect_cpu_quantum);
		attotime cpu_quantum;

		if (cpu == NULL)
			fatalerror("CPU '%s' specified for perfect interleave is not present!", machine->config->perfect_cpu_quantum);
		cpu_quantum = attotime_make(0, get_minimum_quantum(cpu));
		min_quantum = attotime_min(cpu_quantum, min_quantum);
	}
	assert(min_quantum.seconds == 0);
	timer_add_scheduling_quantum(machine, min_quantum.attoseconds, attotime_never);
}


/*-------------------------------------------------
    cpuexec_timeslice - execute all CPUs for a
    single timeslice
-------------------------------------------------*/

void cpuexec_timeslice(running_machine *machine)
{
	int call_debugger = ((machine->debug_flags & DEBUG_FLAG_ENABLED) != 0);
	timer_execution_state *timerexec = timer_get_execution_state(machine);
	cpuexec_private *global = machine->cpuexec_data;
	int ran;

	/* build the execution list if we don't have one yet */
	if (global->executelist == NULL)
		rebuild_execute_list(machine);

	/* loop until we hit the next timer */
	while (ATTOTIME_LT(timerexec->basetime, timerexec->nextfire))
	{
		cpu_class_data *classdata;
		UINT32 suspendchanged;
		attotime target;

		/* by default, assume our target is the end of the next quantum */
		target.seconds = timerexec->basetime.seconds;
		target.attoseconds = timerexec->basetime.attoseconds + timerexec->curquantum;
		ATTOTIME_NORMALIZE(target);

		/* however, if the next timer is going to fire before then, override */
		assert(attotime_sub(timerexec->nextfire, target).seconds <= 0);
		if (ATTOTIME_LT(timerexec->nextfire, target))
			target = timerexec->nextfire;

		LOG(("------------------\n"));
		LOG(("cpu_timeslice: target = %s\n", attotime_string(target, 9)));

		/* apply pending suspension changes */
		suspendchanged = 0;
		for (classdata = global->executelist; classdata != NULL; classdata = classdata->next)
		{
			suspendchanged |= (classdata->suspend ^ classdata->nextsuspend);
			classdata->suspend = classdata->nextsuspend;
			classdata->nextsuspend &= ~SUSPEND_REASON_TIMESLICE;
			classdata->eatcycles = classdata->nexteatcycles;
		}

		/* recompute the execute list if any CPUs changed their suspension state */
		if (suspendchanged != 0)
			rebuild_execute_list(machine);

		/* loop over non-suspended CPUs */
		for (classdata = global->executelist; classdata != NULL; classdata = classdata->next)
		{
			/* only process if our target is later than the CPU's current time (coarse check) */
			if (target.seconds >= classdata->localtime.seconds)
			{
				attoseconds_t delta, actualdelta;

				/* compute how many attoseconds to execute this CPU */
				delta = target.attoseconds - classdata->localtime.attoseconds;
				if (delta < 0 && target.seconds > classdata->localtime.seconds)
					delta += ATTOSECONDS_PER_SECOND;
				assert(delta == attotime_to_attoseconds(attotime_sub(target, classdata->localtime)));

				/* if we have enough for at least 1 cycle, do the math */
				if (delta >= classdata->attoseconds_per_cycle)
				{
					/* compute how many cycles we want to execute */
					ran = classdata->cycles_running = divu_64x32((UINT64)delta >> classdata->divshift, classdata->divisor);
					LOG(("  cpu '%s': %d cycles\n", classdata->device->tag, classdata->cycles_running));

					/* if we're not suspended, actually execute */
					if (classdata->suspend == 0)
					{
						profiler_mark_start(classdata->profiler);

						/* note that this global variable cycles_stolen can be modified */
						/* via the call to cpu_execute */
						classdata->cycles_stolen = 0;
						global->executingcpu = classdata->device;
						*classdata->icount = classdata->cycles_running;
						if (!call_debugger)
							ran = (*classdata->execute)(classdata->device, classdata->cycles_running);
						else
						{
							debugger_start_cpu_hook(classdata->device, target);
							ran = (*classdata->execute)(classdata->device, classdata->cycles_running);
							debugger_stop_cpu_hook(classdata->device);
						}

						/* adjust for any cycles we took back */
						assert(ran >= classdata->cycles_stolen);
						ran -= classdata->cycles_stolen;
						profiler_mark_end();
					}

					/* account for these cycles */
					classdata->totalcycles += ran;

					/* update the local time for this CPU */
					actualdelta = classdata->attoseconds_per_cycle * ran;
					classdata->localtime.attoseconds += actualdelta;
					ATTOTIME_NORMALIZE(classdata->localtime);
					LOG(("         %d ran, %d total, time = %s\n", ran, (INT32)classdata->totalcycles, attotime_string(classdata->localtime, 9)));

					/* if the new local CPU time is less than our target, move the target up */
					if (ATTOTIME_LT(classdata->localtime, target))
					{
						assert(attotime_compare(classdata->localtime, target) < 0);
						target = classdata->localtime;

						/* however, if this puts us before the base, clamp to the base as a minimum */
						if (ATTOTIME_LT(target, timerexec->basetime))
						{
							assert(attotime_compare(target, timerexec->basetime) < 0);
							target = timerexec->basetime;
						}
						LOG(("         (new target)\n"));
					}
				}
			}
		}
		global->executingcpu = NULL;

		/* update the base time */
		timerexec->basetime = target;
	}

	/* execute timers */
	timer_execute_timers(machine);
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
	{
		const address_space *space = cpu_get_address_space(executingcpu, ADDRESS_SPACE_PROGRAM);
		sprintf(global->statebuf, "'%s' (%s)", executingcpu->tag, core_i64_hex_format(cpu_get_pc(executingcpu), space->logaddrchars));
	}
	else
		strcpy(global->statebuf, "(no context)");

	return global->statebuf;
}



/***************************************************************************
    CPU DEVICE INTERFACE
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
	int num_regs;
	int line;

	/* validate some basic stuff */
	assert(device != NULL);
	assert(device->inline_config != NULL);
	assert(device->machine != NULL);
	assert(device->machine->config != NULL);

	/* get pointers to our data */
	config = (const cpu_config *)device->inline_config;
	header = cpu_get_class_header(device);
	classdata = get_class_data(device);

	/* build the header */
	header->debug = NULL;
	header->set_info = (cpu_set_info_func)device_get_info_fct(device, CPUINFO_FCT_SET_INFO);

	/* fill in the input states and IRQ callback information */
	for (line = 0; line < ARRAY_LENGTH(classdata->input); line++)
	{
		cpu_input_data *inputline = &classdata->input[line];
		/* vector and curvector are initialized later */
		inputline->curstate = CLEAR_LINE;
		inputline->qindex = 0;
	}

	/* fill in the suspend states */
	classdata->device = device;
	classdata->execute = (cpu_execute_func)device_get_info_fct(device, CPUINFO_FCT_EXECUTE);
	classdata->profiler = index + PROFILER_CPU_FIRST;
	classdata->suspend = SUSPEND_REASON_RESET;
	classdata->inttrigger = index + TRIGGER_INT;

	/* fill in the clock and timing information */
	classdata->clock = (UINT64)device->clock * cpu_get_clock_multiplier(device) / cpu_get_clock_divider(device);
	classdata->clockscale = 1.0;

	/* allocate timers if we need them */
	if (config->vblank_interrupts_per_frame > 1)
		classdata->partial_frame_timer = timer_alloc(device->machine, trigger_partial_frame_interrupt, (void *)device);
	if (config->timed_interrupt_period != 0)
		classdata->timedint_timer = timer_alloc(device->machine, trigger_periodic_interrupt, (void *)device);

	/* initialize this CPU */
	num_regs = state_save_get_reg_count(device->machine);
	init = (cpu_init_func)device_get_info_fct(device, CPUINFO_FCT_INIT);
	(*init)(device, standard_irq_callback);
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

	/* fetch information about the CPU states */
	classdata->state = cpu_get_state_table(device);
	if (classdata->state != NULL)
	{
		int stateindex;

		/* loop over all states specified, and work with any that apply */
		for (stateindex = 0; stateindex < classdata->state->entrycount; stateindex++)
		{
			const cpu_state_entry *entry = &classdata->state->entrylist[stateindex];
			if (entry->validmask == 0 || (entry->validmask & classdata->state->subtypemask) != 0)
			{
				assert(entry->index < MAX_REGS);
				assert(classdata->regstate[entry->index] == NULL);
				classdata->regstate[entry->index] = entry;
			}
		}
	}

	/* if no state registered for saving, we can't save */
	if (num_regs == 0)
	{
		logerror("CPU '%s' did not register any state to save!\n", device->tag);
		if (device->machine->gamedrv->flags & GAME_SUPPORTS_SAVE)
			fatalerror("CPU '%s' did not register any state to save!", device->tag);
	}

	/* register some internal states as well */
	register_save_states(device);
}


/*-------------------------------------------------
    device_reset_cpu - device reset callback
-------------------------------------------------*/

static DEVICE_RESET( cpu )
{
	cpu_class_data *classdata = get_class_data(device);
	const cpu_config *config = (const cpu_config *)device->inline_config;
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
	reset = (cpu_reset_func)device_get_info_fct(device, CPUINFO_FCT_RESET);
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
			screen = devtag_get_device(device->machine, config->vblank_interrupt_screen);

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
	exit = (cpu_exit_func)device_get_info_fct(device, CPUINFO_FCT_EXIT);
	if (exit != NULL)
		(*exit)(device);
}


/*-------------------------------------------------
    cpu_set_info - device set info callback
-------------------------------------------------*/

void cpu_set_info(const device_config *device, UINT32 state, UINT64 value)
{
	cpu_class_header *header = cpu_get_class_header(device);
	cpu_set_info_func set_info;
	cpuinfo cinfo;

	/* if we are live and have a header, save ourself a call */
	if (header != NULL)
		set_info = header->set_info;
	else
		set_info = (cpu_set_info_func)device_get_info_fct(device, CPUINFO_FCT_SET_INFO);

	switch (state)
	{
		/* no parameters to set */

		default:
			/* if we have a state pointer, we can handle some stuff for free */
			if (device->token != NULL)
			{
				const cpu_class_data *classdata = get_class_data(device);
				if (classdata->state != NULL)
				{
					if (state >= CPUINFO_INT_REGISTER && state <= CPUINFO_INT_REGISTER_LAST)
					{
						set_register_value(device, classdata->state->baseptr, classdata->regstate[state - CPUINFO_INT_REGISTER], value);
						return;
					}
				}
			}

			/* integer data */
			assert(state >= DEVINFO_INT_FIRST && state <= DEVINFO_INT_LAST);
			if (state >= DEVINFO_INT_FIRST && state <= DEVINFO_INT_LAST)
			{
				cinfo.i = value;
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
	const cpu_config *config = (device != NULL) ? (const cpu_config *)device->inline_config : NULL;
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
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(cpu);		break;
		case DEVINFO_FCT_STOP:					info->stop = DEVICE_STOP_NAME(cpu); 		break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(cpu);		break;

		default:
			/* if we don't have a device pointer, ignore everything else */
			if (device == NULL)
				break;

			/* if we have a state pointer, we can handle some stuff for free */
			if (device->token != NULL)
			{
				const cpu_class_data *classdata = get_class_data(device);
				if (classdata->state != NULL)
				{
					if (state >= CPUINFO_INT_REGISTER && state <= CPUINFO_INT_REGISTER_LAST)
					{
						info->i = get_register_value(device, classdata->state->baseptr, classdata->regstate[state - CPUINFO_INT_REGISTER]);
						return;
					}
					else if (state >= CPUINFO_STR_REGISTER && state <= CPUINFO_STR_REGISTER_LAST)
					{
						cinfo.s = info->s;
						get_register_string_value(device, classdata->state->baseptr, classdata->regstate[state - CPUINFO_STR_REGISTER], cinfo.s);
						info->s = cinfo.s;
						return;
					}
				}
			}

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
	cpu_class_data *classdata = get_class_data(device);

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
	cpu_class_data *classdata = get_class_data(device);

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
	cpu_class_data *classdata = get_class_data(device);

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
	cpu_class_data *classdata;

	/* if we haven't been started yet, compute it manually */
	if (device->token == NULL)
		return (UINT64)device->clock * cpu_get_clock_multiplier(device) / cpu_get_clock_divider(device);

	/* return the current clock value */
	classdata = get_class_data(device);
	return classdata->clock;
}


/*-------------------------------------------------
    cpu_set_clock - sets the given CPU's
    clock speed
-------------------------------------------------*/

void cpu_set_clock(const device_config *device, int clock)
{
	cpu_class_data *classdata = get_class_data(device);

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
	cpu_class_data *classdata = get_class_data(device);

	/* return the current clock scale factor */
	return classdata->clockscale;
}


/*-------------------------------------------------
    cpu_set_clockscale - sets the current
    scaling factor for a CPU's clock speed
-------------------------------------------------*/

void cpu_set_clockscale(const device_config *device, double clockscale)
{
	cpu_class_data *classdata = get_class_data(device);

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
	cpu_class_data *classdata = get_class_data(device);
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
	cpu_class_data *classdata = get_class_data(device);
	return mulu_32x32(duration.seconds, classdata->cycles_per_second) + (UINT64)duration.attoseconds / (UINT64)classdata->attoseconds_per_cycle;
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
	cpu_class_data *classdata = get_class_data(device);
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
	cpu_class_data *classdata = get_class_data(device);

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
	cpu_class_data *classdata = get_class_data(device);

	/* ignore if not the executing CPU */
	if (device != device->machine->cpuexec_data->executingcpu)
		return;

	if (cycles > *classdata->icount)
		cycles = *classdata->icount;
	*classdata->icount -= cycles;
}


/*-------------------------------------------------
    cpu_adjust_icount - apply a +/- to the current
    icount
-------------------------------------------------*/

void cpu_adjust_icount(const device_config *device, int delta)
{
	cpu_class_data *classdata = get_class_data(device);

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
	cpu_class_data *classdata = get_class_data(device);
	int delta;

	/* ignore if not the executing CPU */
	if (device != device->machine->cpuexec_data->executingcpu)
		return;

	/* swallow the remaining cycles */
	if (classdata->icount != NULL)
	{
		delta = *classdata->icount;
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
	cpu_class_data *classdata = get_class_data(device);

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
	for (cpu = machine->firstcpu; cpu != NULL; cpu = cpu_next(cpu))
	{
		cpu_class_data *classdata = get_class_data(cpu);

		/* if we're executing, for an immediate abort */
		cpu_abort_timeslice(cpu);

		/* see if this is a matching trigger */
		if ((classdata->nextsuspend & SUSPEND_REASON_TRIGGER) != 0 && classdata->trigger == trigger)
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
	cpu_class_data *classdata = get_class_data(device);

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
	cpu_class_data *classdata = get_class_data(device);
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
	cpu_class_data *classdata = get_class_data(device);
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
	cpu_class_data *classdata = get_class_data(device);

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
	cpu_class_data *classdata = get_class_data(device);
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
	cpu_class_data *classdata = get_class_data(device);
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
	cpu_class_data *classdata = get_class_data(device);
	INT64 attos;

	/* recompute cps and spc */
	classdata->cycles_per_second = (double)classdata->clock * classdata->clockscale;
	classdata->attoseconds_per_cycle = HZ_TO_ATTOSECONDS((double)classdata->clock * classdata->clockscale);

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
	const device_config *firstcpu = machine->firstcpu;
	if (firstcpu != NULL)
	{
		attoseconds_t smallest = get_minimum_quantum(firstcpu);
		attoseconds_t perfect = ATTOSECONDS_PER_SECOND - 1;
		const device_config *cpu;

		/* start with a huge time factor and find the 2nd smallest cycle time */
		for (cpu = cpu_next(firstcpu); cpu != NULL; cpu = cpu_next(cpu))
		{
			attoseconds_t curquantum = get_minimum_quantum(cpu);

			/* find the 2nd smallest cycle interval */
			if (curquantum < smallest)
			{
				perfect = smallest;
				smallest = curquantum;
			}
			else if (curquantum < perfect)
				perfect = curquantum;
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
		for (cpu = device->machine->firstcpu; cpu != NULL; cpu = cpu_next(cpu))
		{
			const cpu_config *config = (const cpu_config *)cpu->inline_config;
			cpu_class_data *classdata = get_class_data(cpu);
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
	const device_config *device = (const device_config *)ptr;
	const cpu_config *config = (const cpu_config *)device->inline_config;
	cpu_class_data *classdata = get_class_data(device);

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
	const device_config *device = (const device_config *)ptr;
	const cpu_config *config = (const cpu_config *)device->inline_config;

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
	const device_config *device = (const device_config *)ptr;
	cpu_class_data *classdata = get_class_data(device);
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
					cpu_set_info(device, CPUINFO_INT_INPUT_STATE + param, ASSERT_LINE);
					cpu_set_info(device, CPUINFO_INT_INPUT_STATE + param, CLEAR_LINE);
					break;

				case HOLD_LINE:
				case ASSERT_LINE:
					cpu_set_info(device, CPUINFO_INT_INPUT_STATE + param, ASSERT_LINE);
					break;

				case CLEAR_LINE:
					cpu_set_info(device, CPUINFO_INT_INPUT_STATE + param, CLEAR_LINE);
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
	cpu_class_data *classdata = get_class_data(device);
	cpu_input_data *inputline = &classdata->input[irqline];
	int vector = inputline->curvector;

	LOG(("standard_irq_callback('%s', %d) $%04x\n", device->tag, irqline, vector));

	/* if the IRQ state is HOLD_LINE, clear it */
	if (inputline->curstate == HOLD_LINE)
	{
		LOG(("->set_irq_line('%s',%d,%d)\n", device->tag, irqline, CLEAR_LINE));
		cpu_set_info(device, CPUINFO_INT_INPUT_STATE + irqline, CLEAR_LINE);
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
	cpu_class_data *classdata = get_class_data(device);
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


/*-------------------------------------------------
    rebuild_execute_list - rebuild the list of
    executing CPUs, moving suspended CPUs to the
    end
-------------------------------------------------*/

static void rebuild_execute_list(running_machine *machine)
{
	cpuexec_private *global = machine->cpuexec_data;
	const device_config *curcpu;
	cpu_class_data **tailptr;

	/* start with an empty list */
	tailptr = &global->executelist;
	*tailptr = NULL;

	/* first iterate over non-suspended CPUs */
	for (curcpu = machine->firstcpu; curcpu != NULL; curcpu = cpu_next(curcpu))
	{
		cpu_class_data *classdata = get_class_data(curcpu);
		if (classdata->suspend == 0)
		{
			*tailptr = classdata;
			tailptr = &classdata->next;
			classdata->next = NULL;
		}
	}

	/* then add the suspended CPUs */
	for (curcpu = machine->firstcpu; curcpu != NULL; curcpu = cpu_next(curcpu))
	{
		cpu_class_data *classdata = get_class_data(curcpu);
		if (classdata->suspend != 0)
		{
			*tailptr = classdata;
			tailptr = &classdata->next;
			classdata->next = NULL;
		}
	}
}


/*-------------------------------------------------
    get_register_value - return a register value
    of a CPU using the state table
-------------------------------------------------*/

static UINT64 get_register_value(const device_config *device, void *baseptr, const cpu_state_entry *entry)
{
	void *dataptr;
	UINT64 result;

	/* NULL entry returns 0 */
	if (entry == NULL || baseptr == NULL)
		return 0;

	/* if we have an exporter, call it now */
	if ((entry->flags & CPUSTATE_EXPORT) != 0)
	{
		cpu_state_io_func exportcb = (cpu_state_io_func)device_get_info_fct(device, CPUINFO_FCT_EXPORT_STATE);
		assert(exportcb != NULL);
		(*exportcb)(device, baseptr, entry);
	}

	/* pick up the value */
	dataptr = (UINT8 *)baseptr + entry->dataoffs;
	switch (entry->datasize)
	{
		default:
		case 1:	result = *(UINT8 *)dataptr;		break;
		case 2:	result = *(UINT16 *)dataptr;	break;
		case 4:	result = *(UINT32 *)dataptr;	break;
		case 8:	result = *(UINT64 *)dataptr;	break;
	}
	return result & entry->mask;
}


/*-------------------------------------------------
    set_register_value - set the value of a
    CPU register using the state table
-------------------------------------------------*/

static void set_register_value(const device_config *device, void *baseptr, const cpu_state_entry *entry, UINT64 value)
{
	void *dataptr;

	/* NULL entry is a no-op */
	if (entry == NULL || baseptr == NULL)
		return;

	/* apply the mask */
	value &= entry->mask;

	/* sign-extend if necessary */
	if ((entry->flags & CPUSTATE_IMPORT_SEXT) != 0 && value > (entry->mask >> 1))
		value |= ~entry->mask;

	/* store the value */
	dataptr = (UINT8 *)baseptr + entry->dataoffs;
	switch (entry->datasize)
	{
		default:
		case 1:	*(UINT8 *)dataptr = value;		break;
		case 2:	*(UINT16 *)dataptr = value;		break;
		case 4:	*(UINT32 *)dataptr = value;		break;
		case 8:	*(UINT64 *)dataptr = value;		break;
	}

	/* if we have an importer, call it now */
	if ((entry->flags & CPUSTATE_IMPORT) != 0)
	{
		cpu_state_io_func importcb = (cpu_state_io_func)device_get_info_fct(device, CPUINFO_FCT_IMPORT_STATE);
		assert(importcb != NULL);
		(*importcb)(device, baseptr, entry);
	}
}


/*-------------------------------------------------
    get_register_string_value - return a string
    representation of a CPU register using the
    state table
-------------------------------------------------*/

static void get_register_string_value(const device_config *device, void *baseptr, const cpu_state_entry *entry, char *dest)
{
	static const UINT64 decdivisor[] = {
		1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000,
		U64(10000000000),			U64(100000000000),			U64(1000000000000),
		U64(10000000000000),		U64(100000000000000),		U64(1000000000000000),
		U64(10000000000000000), 	U64(100000000000000000),	U64(1000000000000000000),
		U64(10000000000000000000)
	};
	static const char hexchars[] = "0123456789ABCDEF";
	int leadzero = 0, width = 0, percent = 0, explicitsign = 0, hitnonzero = 0, reset;
	const char *fptr;
	UINT64 result;

	/* NULL entry does nothing */
	if (entry == NULL || entry->symbol == NULL || entry->format == NULL)
		return;

	/* fetch the data */
	result = get_register_value(device, baseptr, entry);

	/* start with the basics */
	dest += sprintf(dest, "%s%s:", (entry->flags & CPUSTATE_NOSHOW) ? "~" : "", entry->symbol);

	/* parse the format */
	reset = TRUE;
	for (fptr = entry->format; *fptr != 0; fptr++)
	{
		int digitnum;

		/* reset any accumulated state */
		if (reset)
			leadzero = width = percent = explicitsign = reset = 0;

		/* if we're not within a format, then anything other than a % outputs directly */
		if (!percent && *fptr != '%')
		{
			*dest++ = *fptr;
			continue;
		}

		/* handle each character in turn */
		switch (*fptr)
		{
			/* % starts a format; %% outputs a single % */
			case '%':
				if (!percent)
					percent = TRUE;
				else
				{
					*dest++ = *fptr;
					percent = FALSE;
				}
				break;

			/* 0 means insert leading 0s, unless it follows another width digit */
			case '0':
				if (width == 0)
					leadzero = TRUE;
				else
					width *= 10;
				break;

			/* 1-9 accumulate into the width */
			case '1':	case '2':	case '3':	case '4':	case '5':
			case '6':	case '7':	case '8':	case '9':
				width = width * 10 + (*fptr - '0');
				break;

			/* + means explicit sign */
			case '+':
				explicitsign = TRUE;
				break;

			/* X outputs as hexadecimal */
			case 'X':
				if (width == 0)
					fatalerror("Width required for %%X formats\n");
				hitnonzero = FALSE;
				while (leadzero && width > 16)
				{
					*dest++ = ' ';
					width--;
				}
				for (digitnum = 15; digitnum >= 0; digitnum--)
				{
					int digit = (result >> (4 * digitnum)) & 0x0f;
					if (digit != 0)
						*dest++ = hexchars[digit];
					else if (hitnonzero || (leadzero && digitnum < width) || digitnum == 0)
						*dest++ = '0';
					hitnonzero |= digit;
				}
				reset = TRUE;
				break;

			/* d outputs as signed decimal */
			case 'd':
				if (width == 0)
					fatalerror("Width required for %%d formats\n");
				if ((result & entry->mask) > (entry->mask >> 1))
				{
					result = -result & entry->mask;
					*dest++ = '-';
					width--;
				}
				else if (explicitsign)
				{
					*dest++ = '+';
					width--;
				}
				/* fall through to unsigned case */

			/* u outputs as unsigned decimal */
			case 'u':
				if (width == 0)
					fatalerror("Width required for %%u formats\n");
				hitnonzero = FALSE;
				while (leadzero && width > ARRAY_LENGTH(decdivisor))
				{
					*dest++ = ' ';
					width--;
				}
				for (digitnum = ARRAY_LENGTH(decdivisor); digitnum >= 0; digitnum--)
				{
					int digit = (result >= decdivisor[digitnum]) ? (result / decdivisor[digitnum]) % 10 : 0;
					if (digit != 0)
						*dest++ = '0' + digit;
					else if (hitnonzero || (leadzero && digitnum < width) || digitnum == 0)
						*dest++ = '0';
					hitnonzero |= digit;
				}
				reset = TRUE;
				break;

			/* s is a custom format */
			case 's':
			{
				cpu_string_io_func exportstring = (cpu_string_io_func)device_get_info_fct(device, CPUINFO_FCT_EXPORT_STRING);
				assert(exportstring != NULL);
				(*exportstring)(device, baseptr, entry, dest);
				dest += strlen(dest);
				break;
			}

			/* other formats unknown */
			default:
				fatalerror("Unknown format character '%c'\n", *fptr);
				break;
		}
	}
	*dest = 0;
}


/*-------------------------------------------------
    get_register_string_max_width - return the
    maximum width of a string described by a
    format
-------------------------------------------------*/

#ifdef UNUSED_FUNCTION
static int get_register_string_max_width(const device_config *device, void *baseptr, const cpu_state_entry *entry)
{
	int leadzero = 0, width = 0, percent = 0, explicitsign = 0, reset;
	int totalwidth = 0;
	const char *fptr;

	/* NULL entry does nothing */
	if (entry == NULL || entry->symbol == NULL || entry->format == NULL)
		return 0;

	/* parse the format */
	reset = TRUE;
	for (fptr = entry->format; *fptr != 0; fptr++)
	{
		/* reset any accumulated state */
		if (reset)
			leadzero = width = percent = explicitsign = reset = 0;

		/* if we're not within a format, then anything other than a % outputs directly */
		if (!percent && *fptr != '%')
		{
			totalwidth++;
			continue;
		}

		/* handle each character in turn */
		switch (*fptr)
		{
			/* % starts a format; %% outputs a single % */
			case '%':
				if (!percent)
					percent = TRUE;
				else
				{
					totalwidth++;
					percent = FALSE;
				}
				break;

			/* 0 means insert leading 0s, unless it follows another width digit */
			case '0':
				if (width == 0)
					leadzero = TRUE;
				else
					width *= 10;
				break;

			/* 1-9 accumulate into the width */
			case '1':	case '2':	case '3':	case '4':	case '5':
			case '6':	case '7':	case '8':	case '9':
				width = width * 10 + (*fptr - '0');
				break;

			/* + means explicit sign */
			case '+':
				explicitsign = TRUE;
				break;

			/* X outputs as hexadecimal */
			/* d outputs as signed decimal */
			/* u outputs as unsigned decimal */
			/* s outputs as custom format */
			case 'X':
			case 'd':
			case 'u':
			case 's':
				totalwidth += width;
				reset = TRUE;
				break;

				if (width == 0)
					fatalerror("Width required for %%d formats\n");
				totalwidth += width;
				break;

			/* other formats unknown */
			default:
				fatalerror("Unknown format character '%c'\n", *fptr);
				break;
		}
	}
	return totalwidth;
}
#endif
