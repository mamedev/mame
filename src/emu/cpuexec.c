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
    MACROS
***************************************************************************/

#define VERIFY_ACTIVECPU(name) \
	int activecpu = cpu_getactivecpu(); \
	assert_always(activecpu >= 0, #name "() called with no active cpu!")

#define VERIFY_EXECUTINGCPU(name) \
	int activecpu = cpu_getexecutingcpu(); \
	assert_always(activecpu >= 0, #name "() called with no executing cpu!")

#define VERIFY_CPUNUM(name) \
	assert_always(cpunum >= 0 && cpunum < cpu_gettotalcpu(), #name "() called for invalid cpu num!")



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

/* Internal CPU info structure */
typedef struct _cpuexec_data cpuexec_data;
struct _cpuexec_data
{
	UINT8		suspend;				/* suspend reason mask (0 = not suspended) */
	UINT8		nextsuspend;			/* pending suspend reason mask */
	UINT8		eatcycles;				/* true if we eat cycles while suspended */
	UINT8		nexteatcycles;			/* pending value */
	INT32		trigger;				/* pending trigger to release a trigger suspension */

	UINT64 		totalcycles;			/* total CPU cycles executed */
	attotime 	localtime;				/* local time, relative to the timer system's global time */
	INT32		clock;					/* current active clock */
	double		clockscale;				/* current active clock scale factor */
	INT32		divisor;				/* 32-bit attoseconds_per_cycle divisor */
	UINT8		divshift;				/* right shift amount to fit the divisor into 32 bits */

	emu_timer *	timedint_timer;			/* reference to this CPU's periodic interrupt timer */

	/* these below are hacks to support multiple interrupts per frame */
	INT32 		iloops; 				/* number of interrupts remaining this frame */
	emu_timer *	partial_frame_timer;	/* the timer that triggers partial frame interrupts */
	attotime	partial_frame_period;	/* the length of one partial frame for interrupt purposes */
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* general CPU variables */
static cpuexec_data cpu[MAX_CPU];

static int cycles_running;
static int cycles_stolen;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void cpuexec_exit(running_machine *machine);
static void cpuexec_reset(running_machine *machine);
static void cpu_inittimers(running_machine *machine);
static void update_clock_information(running_machine *machine, int cpunum);
static TIMER_CALLBACK( trigger_partial_frame_interrupt );
static void compute_perfect_interleave(running_machine *machine);



/***************************************************************************
    CORE CPU EXECUTION
***************************************************************************/

/*-------------------------------------------------
    cpuexec_init - initialize internal states of
    all CPUs
-------------------------------------------------*/

void cpuexec_init(running_machine *machine)
{
	int cpunum;

	/* loop over all our CPUs */
	memset(cpu, 0, sizeof(cpu));
	for (cpunum = 0; cpunum < MAX_CPU; cpunum++)
	{
		cpu_type cputype = machine->config->cpu[cpunum].type;
		int num_regs;

		/* if this is a dummy, stop looking */
		if (cputype == CPU_DUMMY)
			break;

		/* initialize the cpuinfo struct */
		cpu[cpunum].suspend = SUSPEND_REASON_RESET;
		cpu[cpunum].clock = (UINT64)machine->config->cpu[cpunum].clock * cputype_clock_multiplier(cputype) / cputype_clock_divider(cputype);
		cpu[cpunum].clockscale = 1.0;
		cpu[cpunum].localtime = attotime_zero;

		/* compute the cycle times */
		update_clock_information(machine, cpunum);

		/* register some of our variables for later */
		state_save_register_item("cpu", cpunum, cpu[cpunum].suspend);
		state_save_register_item("cpu", cpunum, cpu[cpunum].nextsuspend);
		state_save_register_item("cpu", cpunum, cpu[cpunum].eatcycles);
		state_save_register_item("cpu", cpunum, cpu[cpunum].nexteatcycles);
		state_save_register_item("cpu", cpunum, cpu[cpunum].trigger);

		state_save_register_item("cpu", cpunum, cpu[cpunum].iloops);

		state_save_register_item("cpu", cpunum, cpu[cpunum].totalcycles);
		state_save_register_item("cpu", cpunum, cpu[cpunum].localtime.seconds);
		state_save_register_item("cpu", cpunum, cpu[cpunum].localtime.attoseconds);
		state_save_register_item("cpu", cpunum, cpu[cpunum].clock);
		state_save_register_item("cpu", cpunum, cpu[cpunum].clockscale);

		/* initialize this CPU */
		state_save_push_tag(cpunum + 1);
		num_regs = state_save_get_reg_count();
		if (cpuintrf_init_cpu(cpunum, cputype, cpu[cpunum].clock, machine->config->cpu[cpunum].reset_param, cpu_irq_callbacks[cpunum]))
			fatalerror("Unable to initialize CPU #%d (%s)", cpunum, cputype_name(cputype));
		num_regs = state_save_get_reg_count() - num_regs;
		state_save_pop_tag();

		/* if no state registered for saving, we can't save */
		if (num_regs == 0)
		{
			logerror("CPU #%d (%s) did not register any state to save!\n", cpunum, cputype_name(cputype));
			if (machine->gamedrv->flags & GAME_SUPPORTS_SAVE)
				fatalerror("CPU #%d (%s) did not register any state to save!", cpunum, cputype_name(cputype));
		}
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

	/* initialize the various timers (suspends all CPUs at startup) */
	cpu_inittimers(machine);

	/* first pass over CPUs */
	for (cpunum = 0; cpunum < cpu_gettotalcpu(); cpunum++)
	{
		/* enable all CPUs (except for disabled CPUs) */
		if (!(machine->config->cpu[cpunum].flags & CPU_DISABLE))
			cpunum_resume(cpunum, SUSPEND_ANY_REASON);
		else
			cpunum_suspend(cpunum, SUSPEND_REASON_DISABLE, 1);

		/* reset the total number of cycles */
		cpu[cpunum].totalcycles = 0;

		/* then reset the CPU directly */
		cpunum_reset(cpunum);
	}
}


/*-------------------------------------------------
    cpuexec_exit - cleanup all CPUs on exit
-------------------------------------------------*/

static void cpuexec_exit(running_machine *machine)
{
	int cpunum;

	/* shut down the CPU cores */
	for (cpunum = 0; cpunum < cpu_gettotalcpu(); cpunum++)
		cpuintrf_exit_cpu(cpunum);
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
	for (cpunum = 0; machine->config->cpu[cpunum].type != CPU_DUMMY; cpunum++)
	{
		cpuexec_data *cpudata = &cpu[cpunum];
		cpudata->suspend = cpudata->nextsuspend;
		cpudata->nextsuspend &= ~SUSPEND_REASON_TIMESLICE;
		cpudata->eatcycles = cpudata->nexteatcycles;
	}

	/* loop over non-suspended CPUs */
	for (cpunum = 0; machine->config->cpu[cpunum].type != CPU_DUMMY; cpunum++)
	{
		cpuexec_data *cpudata = &cpu[cpunum];
		if (cpudata->suspend == 0)
		{
			attotime delta = attotime_sub(target, cpudata->localtime);
			if (delta.seconds >= 0 && delta.attoseconds >= attoseconds_per_cycle[cpunum])
			{
				/* compute how long to run */
				cycles_running = div_64x32(delta.attoseconds >> cpudata->divshift, cpudata->divisor);
				LOG(("  cpu %d: %d cycles\n", cpunum, cycles_running));

				profiler_mark(PROFILER_CPU1 + cpunum);

				/* note that this global variable cycles_stolen can be modified */
				/* via the call to the cpunum_execute */
				cycles_stolen = 0;
				if (!call_debugger)
					ran = cpunum_execute(cpunum, cycles_running);
				else
				{
					debugger_start_cpu_hook(machine, cpunum, target);
					ran = cpunum_execute(cpunum, cycles_running);
					debugger_stop_cpu_hook(machine, cpunum);
				}

#ifdef MAME_DEBUG
				if (ran < cycles_stolen)
					fatalerror("Negative CPU cycle count!");
#endif /* MAME_DEBUG */

				ran -= cycles_stolen;
				profiler_mark(PROFILER_END);

				/* account for these cycles */
				cpudata->totalcycles += ran;
				cpudata->localtime = attotime_add_attoseconds(cpudata->localtime, ran * attoseconds_per_cycle[cpunum]);
				LOG(("         %d ran, %d total, time = %s\n", ran, (INT32)cpudata->totalcycles, attotime_string(cpudata->localtime, 9)));

				/* if the new local CPU time is less than our target, move the target up */
				if (attotime_compare(cpudata->localtime, target) < 0)
				{
					target = attotime_max(cpudata->localtime, base);
					LOG(("         (new target)\n"));
				}
			}
		}
	}

	/* update the local times of all CPUs */
	for (cpunum = 0; machine->config->cpu[cpunum].type != CPU_DUMMY; cpunum++)
	{
		cpuexec_data *cpudata = &cpu[cpunum];

		/* if we're suspended and counting, process */
		if (cpudata->suspend != 0 && cpudata->eatcycles && attotime_compare(cpudata->localtime, target) < 0)
		{
			attotime delta = attotime_sub(target, cpudata->localtime);

			/* compute how long to run */
			cycles_running = div_64x32(delta.attoseconds >> cpudata->divshift, cpudata->divisor);
			LOG(("  cpu %d: %d cycles (suspended)\n", cpunum, cycles_running));

			cpudata->totalcycles += cycles_running;
			cpudata->localtime = attotime_add_attoseconds(cpudata->localtime, cycles_running * attoseconds_per_cycle[cpunum]);
			LOG(("         %d skipped, %d total, time = %s\n", cycles_running, (INT32)cpudata->totalcycles, attotime_string(cpudata->localtime, 9)));
		}
		
		/* update the suspend state (breaks steeltal if we don't) */
		cpudata->suspend = cpudata->nextsuspend;
		cpudata->eatcycles = cpudata->nexteatcycles;
	}

	/* update the global time */
	timer_set_global_time(machine, target);
}



/***************************************************************************
    CPU SCHEDULING
***************************************************************************/

/*-------------------------------------------------
    cpu_boost_interleave - temporarily boosts the
    interleave factor
-------------------------------------------------*/

void cpu_boost_interleave(running_machine *machine, attotime timeslice_time, attotime boost_duration)
{
	/* ignore timeslices > 1 second */
	if (timeslice_time.seconds > 0)
		return;
	timer_add_scheduling_quantum(machine, timeslice_time.attoseconds, boost_duration);
}


/*-------------------------------------------------
    activecpu_abort_timeslice - abort execution
    for the current timeslice, allowing other
    CPUs to run before we run again
-------------------------------------------------*/

void activecpu_abort_timeslice(void)
{
	int current_icount;

	VERIFY_EXECUTINGCPU(activecpu_abort_timeslice);
	LOG(("activecpu_abort_timeslice (CPU=%d, cycles_left=%d)\n", cpu_getexecutingcpu(), activecpu_get_icount() + 1));

	/* swallow the remaining cycles */
	current_icount = activecpu_get_icount() + 1;
	cycles_stolen += current_icount;
	cycles_running -= current_icount;
	activecpu_adjust_icount(-current_icount);
}


/*-------------------------------------------------
    cpunum_suspend - set a suspend reason for the
    given CPU
-------------------------------------------------*/

void cpunum_suspend(int cpunum, int reason, int eatcycles)
{
	VERIFY_CPUNUM(cpunum_suspend);
	LOG(("cpunum_suspend (CPU=%d, r=%X, eat=%d)\n", cpunum, reason, eatcycles));
	cpu[cpunum].nextsuspend |= reason;
	cpu[cpunum].nexteatcycles = eatcycles;
	if (cpu_getexecutingcpu() >= 0)
		activecpu_abort_timeslice();
}


/*-------------------------------------------------
    cpunum_resume - clear a suspend reason for the
    given CPU
-------------------------------------------------*/

void cpunum_resume(int cpunum, int reason)
{
	VERIFY_CPUNUM(cpunum_resume);
	LOG(("cpunum_resume (CPU=%d, r=%X)\n", cpunum, reason));
	cpu[cpunum].nextsuspend &= ~reason;
	if (cpu_getexecutingcpu() >= 0)
		activecpu_abort_timeslice();
}


/*-------------------------------------------------
    cpunum_is_suspended - returns true if the
    given CPU is suspended for any of the given
    reasons
-------------------------------------------------*/

int cpunum_is_suspended(int cpunum, int reason)
{
	VERIFY_CPUNUM(cpunum_suspend);
	return ((cpu[cpunum].nextsuspend & reason) != 0);
}



/***************************************************************************
    CPU CLOCK MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    update_clock_information - recomputes clock
    information for the specified CPU
-------------------------------------------------*/

static void update_clock_information(running_machine *machine, int cpunum)
{
	INT64 attos;
	
	/* recompute cps and spc */
	cycles_per_second[cpunum] = (double)cpu[cpunum].clock * cpu[cpunum].clockscale;
	attoseconds_per_cycle[cpunum] = ATTOSECONDS_PER_SECOND / ((double)cpu[cpunum].clock * cpu[cpunum].clockscale);

	/* update the CPU's divisor */
	attos = attoseconds_per_cycle[cpunum];
	cpu[cpunum].divshift = 0;
	while (attos >= (1UL << 31))
	{
		cpu[cpunum].divshift++;
		attos >>= 1;
	}
	cpu[cpunum].divisor = attos;
	
	/* re-compute the perfect interleave factor */
	compute_perfect_interleave(machine);
}


/*-------------------------------------------------
    cpunum_get_clock - gets the given CPU's
    clock speed
-------------------------------------------------*/

int cpunum_get_clock(int cpunum)
{
	VERIFY_CPUNUM(cpunum_get_clock);
	return cpu[cpunum].clock;
}


/*-------------------------------------------------
    cpunum_set_clock - sets the given CPU's
    clock speed
-------------------------------------------------*/

void cpunum_set_clock(running_machine *machine, int cpunum, int clock)
{
	VERIFY_CPUNUM(cpunum_set_clock);

	cpu[cpunum].clock = clock;
	update_clock_information(machine, cpunum);
}


/*-------------------------------------------------
    cpunum_get_clockscale - returns the current
    scaling factor for a CPU's clock speed
-------------------------------------------------*/

double cpunum_get_clockscale(int cpunum)
{
	VERIFY_CPUNUM(cpunum_get_clockscale);
	return cpu[cpunum].clockscale;
}


/*-------------------------------------------------
    cpunum_set_clockscale - sets the current
    scaling factor for a CPU's clock speed
-------------------------------------------------*/

void cpunum_set_clockscale(running_machine *machine, int cpunum, double clockscale)
{
	VERIFY_CPUNUM(cpunum_set_clockscale);

	cpu[cpunum].clockscale = clockscale;
	update_clock_information(machine, cpunum);
}



/***************************************************************************
    CPU TIMING
***************************************************************************/

/*-------------------------------------------------
    cpunum_get_localtime - returns the current
    local time for a CPU
-------------------------------------------------*/

attotime cpunum_get_localtime(int cpunum)
{
	attotime result;

	VERIFY_CPUNUM(cpunum_get_localtime);

	/* if we're active, add in the time from the current slice */
	result = cpu[cpunum].localtime;
	if (cpunum == cpu_getexecutingcpu())
	{
		int cycles = cycles_running - activecpu_get_icount();
		result = attotime_add(result, ATTOTIME_IN_CYCLES(cycles, cpunum));
	}
	return result;
}


/*-------------------------------------------------
    activecpu_gettotalcycles - return the total
    number of CPU cycles executed on the active
    CPU
-------------------------------------------------*/

UINT64 activecpu_gettotalcycles(void)
{
	VERIFY_ACTIVECPU(activecpu_gettotalcycles);
	if (activecpu == cpu_getexecutingcpu())
		return cpu[activecpu].totalcycles + cycles_running - activecpu_get_icount();
	else
		return cpu[activecpu].totalcycles;
}


/*-------------------------------------------------
    cpunum_gettotalcycles - return the total
    number of CPU cycles executed on the
    specified CPU
-------------------------------------------------*/

UINT64 cpunum_gettotalcycles(int cpunum)
{
	VERIFY_CPUNUM(cpunum_gettotalcycles);
	if (cpunum == cpu_getexecutingcpu())
		return cpu[cpunum].totalcycles + cycles_running - activecpu_get_icount();
	else
		return cpu[cpunum].totalcycles;
}


/*-------------------------------------------------
    activecpu_eat_cycles - safely eats cycles so
    we don't cross a timeslice boundary
-------------------------------------------------*/

void activecpu_eat_cycles(int cycles)
{
	int cyclesleft = activecpu_get_icount();
	if (cycles > cyclesleft)
		cycles = cyclesleft;
	activecpu_adjust_icount(-cycles);
}



/***************************************************************************
    SYNCHRONIZATION HELPERS
***************************************************************************/

/*-------------------------------------------------
    cpu_suspend_until_trigger - suspend execution
    until the given trigger fires
-------------------------------------------------*/

static void cpunum_suspend_until_trigger(int cpunum, int trigger, int eatcycles)
{
	/* suspend the CPU immediately if it's not already */
	cpunum_suspend(cpunum, SUSPEND_REASON_TRIGGER, eatcycles);

	/* set the trigger */
	cpu[cpunum].trigger = trigger;
}


/*-------------------------------------------------
    cpu_yield - yield our current timeslice
-------------------------------------------------*/

void cpu_yield(void)
{
	int cpunum = cpu_getexecutingcpu();
	VERIFY_EXECUTINGCPU(cpu_yield);
	cpunum_suspend(cpunum, SUSPEND_REASON_TIMESLICE, FALSE);
}


/*-------------------------------------------------
    cpu_spin - burn CPU cycles until our timeslice
    is up
-------------------------------------------------*/

void cpu_spin(void)
{
	int cpunum = cpu_getexecutingcpu();
	VERIFY_EXECUTINGCPU(cpu_spin);
	cpunum_suspend(cpunum, SUSPEND_REASON_TIMESLICE, TRUE);
}


/*-------------------------------------------------
    cpu_spinuntil_trigger - burn CPU cycles until
    a timer trigger
-------------------------------------------------*/

void cpu_spinuntil_trigger(int trigger)
{
	int cpunum = cpu_getexecutingcpu();
	VERIFY_EXECUTINGCPU(cpu_yielduntil_trigger);
	cpunum_suspend_until_trigger(cpunum, trigger, TRUE);
}


/*-------------------------------------------------
    cpunum_spinuntil_trigger - burn specified CPU
    cycles until a timer trigger
-------------------------------------------------*/

void cpunum_spinuntil_trigger(int cpunum, int trigger)
{
	VERIFY_CPUNUM(cpunum_spinuntil_trigger);
	cpunum_suspend_until_trigger(cpunum, trigger, TRUE);
}


/*-------------------------------------------------
    cpu_spinuntil_int - burn CPU cycles until the
    next interrupt
-------------------------------------------------*/

void cpu_spinuntil_int(void)
{
	int cpunum = cpu_getexecutingcpu();
	VERIFY_EXECUTINGCPU(cpu_spinuntil_int);
	cpunum_suspend_until_trigger(cpunum, TRIGGER_INT + cpunum, TRUE);
}


/*-------------------------------------------------
    cpu_spinuntil_time - burn CPU cycles for a
    specific period of time
-------------------------------------------------*/

void cpu_spinuntil_time(attotime duration)
{
	static int timetrig = 0;
	int cpunum = cpu_getexecutingcpu();
	VERIFY_EXECUTINGCPU(cpu_spinuntil_time);
	cpunum_suspend_until_trigger(cpunum, TRIGGER_SUSPENDTIME + timetrig, TRUE);
	cpu_triggertime(duration, TRIGGER_SUSPENDTIME + timetrig);
	timetrig = (timetrig + 1) % 256;
}



/***************************************************************************
    TRIGGERS
***************************************************************************/

/*-------------------------------------------------
    cpu_trigger - generate a trigger now
-------------------------------------------------*/

void cpu_trigger(running_machine *machine, int trigger)
{
	int cpunum;

	/* cause an immediate resynchronization */
	if (cpu_getexecutingcpu() >= 0)
		activecpu_abort_timeslice();

	/* look for suspended CPUs waiting for this trigger and unsuspend them */
	for (cpunum = 0; cpunum < MAX_CPU; cpunum++)
	{
		/* if this is a dummy, stop looking */
		if (machine->config->cpu[cpunum].type == CPU_DUMMY)
			break;

		/* see if this is a matching trigger */
		if (cpu[cpunum].suspend && cpu[cpunum].trigger == trigger)
		{
			cpunum_resume(cpunum, SUSPEND_REASON_TRIGGER);
			cpu[cpunum].trigger = 0;
		}
	}
}


/*-------------------------------------------------
    cpu_triggertime - generate a trigger after a
    specific period of time
-------------------------------------------------*/

static TIMER_CALLBACK( cpu_triggertime_callback )
{
	cpu_trigger(machine, param);
}


void cpu_triggertime(attotime duration, int trigger)
{
	timer_set(duration, NULL, trigger, cpu_triggertime_callback);
}


/*-------------------------------------------------
    cpu_triggerint - generate a trigger
    corresponding to an interrupt on the given CPU
-------------------------------------------------*/

void cpu_triggerint(running_machine *machine, int cpunum)
{
	cpu_trigger(machine, TRIGGER_INT + cpunum);
}



/***************************************************************************
    CHEESY FAKE VIDEO TIMING
***************************************************************************/

/*-------------------------------------------------
    cpu_getiloops - return the cheesy VBLANK
    interrupt counter (deprecated)
-------------------------------------------------*/

int cpu_getiloops(void)
{
	VERIFY_ACTIVECPU(cpu_getiloops);
	return cpu[activecpu].iloops;
}


/***************************************************************************
    INTERNAL TIMING
***************************************************************************/

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
		for (cpunum = 0; cpunum < cpu_gettotalcpu(); cpunum++)
		{
			int cpu_interested;
			const cpu_config *config = device->machine->config->cpu + cpunum;

			/* start the interrupt counter */
			if (!(cpu[cpunum].suspend & SUSPEND_REASON_DISABLE))
				cpu[cpunum].iloops = 0;
			else
				cpu[cpunum].iloops = -1;

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
				if (!cpunum_is_suspended(cpunum, SUSPEND_REASON_HALT | SUSPEND_REASON_RESET | SUSPEND_REASON_DISABLE))
				{
					cpuintrf_push_context(cpunum);
					(*config->vblank_interrupt)(device->machine, cpunum);
					cpuintrf_pop_context();
				}

				/* if we have more than one interrupt per frame, start the timer now to trigger the rest of them */
				if ((config->vblank_interrupts_per_frame > 1) &&
					!(cpu[cpunum].suspend & SUSPEND_REASON_DISABLE))
				{
					cpu[cpunum].partial_frame_period = attotime_div(video_screen_get_frame_period(device->machine->primary_screen), config->vblank_interrupts_per_frame);
					timer_adjust_oneshot(cpu[cpunum].partial_frame_timer, cpu[cpunum].partial_frame_period, cpunum);
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
	int cpunum = param;
	const cpu_config *config = machine->config->cpu + cpunum;

	if (cpu[cpunum].iloops == 0)
		cpu[cpunum].iloops = config->vblank_interrupts_per_frame;

	cpu[cpunum].iloops--;

	/* call the interrupt handler */
	if (!cpunum_is_suspended(cpunum, SUSPEND_REASON_HALT | SUSPEND_REASON_RESET | SUSPEND_REASON_DISABLE))
	{
		cpuintrf_push_context(cpunum);
		(*config->vblank_interrupt)(machine, cpunum);
		cpuintrf_pop_context();
	}

	/* more? */
	if (cpu[cpunum].iloops > 1)
		timer_adjust_oneshot(cpu[cpunum].partial_frame_timer, cpu[cpunum].partial_frame_period, cpunum);
}


/*-------------------------------------------------
    cpu_timedintcallback - timer callback for
    timed interrupts
-------------------------------------------------*/

static TIMER_CALLBACK( cpu_timedintcallback )
{
	/* bail if there is no routine */
	if (machine->config->cpu[param].timed_interrupt != NULL && !cpunum_is_suspended(param, SUSPEND_REASON_HALT | SUSPEND_REASON_RESET | SUSPEND_REASON_DISABLE))
	{
		cpuintrf_push_context(param);
		(*machine->config->cpu[param].timed_interrupt)(machine, param);
		cpuintrf_pop_context();
	}
}


/*-------------------------------------------------
    compute_perfect_interleave - compute the
    "perfect" interleave interval
-------------------------------------------------*/

static void compute_perfect_interleave(running_machine *machine)
{
	if (attoseconds_per_cycle[0] != 0)
	{
		attoseconds_t smallest = attoseconds_per_cycle[0] * cputype_min_cycles(machine->config->cpu[0].type);
		attoseconds_t perfect = ATTOSECONDS_PER_SECOND - 1;
		int cpunum;

		/* start with a huge time factor and find the 2nd smallest cycle time */
		for (cpunum = 1; machine->config->cpu[cpunum].type != CPU_DUMMY; cpunum++)
			if (attoseconds_per_cycle[cpunum] != 0)
			{
				attoseconds_t curtime = attoseconds_per_cycle[cpunum] * cputype_min_cycles(machine->config->cpu[cpunum].type);
				
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
    cpu_inittimers - set up all the core timers
-------------------------------------------------*/

static void cpu_inittimers(running_machine *machine)
{
	int numscreens = video_screen_count(machine->config);
	attoseconds_t refresh_attosecs;
	int cpunum, ipf;

	/* allocate a dummy timer at the minimum frequency to break things up */
	ipf = machine->config->cpu_slices_per_frame;
	if (ipf <= 0)
		ipf = 1;
	refresh_attosecs = (numscreens == 0) ? HZ_TO_ATTOSECONDS(60) : video_screen_get_frame_period(machine->primary_screen).attoseconds;
	timer_add_scheduling_quantum(machine, refresh_attosecs / ipf, attotime_never);

	/* register the interrupt handler callbacks */
	for (cpunum = 0; cpunum < cpu_gettotalcpu(); cpunum++)
	{
		const cpu_config *config = machine->config->cpu + cpunum;

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
			{
				screen = device_list_first(machine->config->devicelist, VIDEO_SCREEN);

				/* allocate timer that will trigger the partial frame updates */
				cpu[cpunum].partial_frame_timer = timer_alloc(trigger_partial_frame_interrupt, 0);
			}

			assert(screen != NULL);

			video_screen_register_vblank_callback(screen, on_vblank, NULL);
		}

		/* periodic interrupts */
		if (config->timed_interrupt_period != 0)
		{
			attotime timedint_period = attotime_make(0, config->timed_interrupt_period);
			cpu[cpunum].timedint_timer = timer_alloc(cpu_timedintcallback, NULL);
			timer_adjust_periodic(cpu[cpunum].timedint_timer, timedint_period, cpunum, timedint_period);
		}
	}
}
