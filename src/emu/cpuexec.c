/***************************************************************************

    cpuexec.c

    Core multi-CPU execution engine.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include <math.h>
#include "driver.h"
#include "cheat.h"
#include "profiler.h"
#include "debugger.h"

#ifdef MAME_DEBUG
#include "debug/debugcpu.h"
#endif



/*************************************
 *
 *  Debug logging
 *
 *************************************/

#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)



/*************************************
 *
 *  Macros to help verify active CPU
 *
 *************************************/

#define VERIFY_ACTIVECPU(name) \
	int activecpu = cpu_getactivecpu(); \
	assert_always(activecpu >= 0, #name "() called with no active cpu!")

#define VERIFY_EXECUTINGCPU(name) \
	int activecpu = cpu_getexecutingcpu(); \
	assert_always(activecpu >= 0, #name "() called with no executing cpu!")

#define VERIFY_CPUNUM(name) \
	assert_always(cpunum >= 0 && cpunum < cpu_gettotalcpu(), #name "() called for invalid cpu num!")



/*************************************
 *
 *  Triggers for the timer system
 *
 *************************************/

enum
{
	TRIGGER_TIMESLICE 	= -1000,
	TRIGGER_INT 		= -2000,
	TRIGGER_YIELDTIME 	= -3000,
	TRIGGER_SUSPENDTIME = -4000
};



/*************************************
 *
 *  Internal CPU info structure
 *
 *************************************/

typedef struct _cpuexec_data cpuexec_data;
struct _cpuexec_data
{
	UINT8	saveable;				/* true if saveable */

	UINT8	suspend;				/* suspend reason mask (0 = not suspended) */
	UINT8	nextsuspend;			/* pending suspend reason mask */
	UINT8	eatcycles;				/* true if we eat cycles while suspended */
	UINT8	nexteatcycles;			/* pending value */
	INT32	trigger;				/* pending trigger to release a trigger suspension */

	INT32 	iloops; 				/* number of interrupts remaining this frame */

	UINT64 	totalcycles;			/* total CPU cycles executed */
	attotime localtime;				/* local time, relative to the timer system's global time */
	INT32	clock;					/* current active clock */
	double	clockscale;				/* current active clock scale factor */

	INT32	vblankint_countdown;	/* number of vblank callbacks left until we interrupt */
	INT32 	vblankint_multiplier;	/* number of vblank callbacks per interrupt */
	void *	vblankint_timer;		/* reference to elapsed time counter */

	void *	timedint_timer;			/* reference to this CPU's timer */
	attotime timedint_period; 		/* timing period of the timed interrupt */
};



/*************************************
 *
 *  General CPU variables
 *
 *************************************/

static cpuexec_data cpu[MAX_CPU];

static UINT8 vblank;
static UINT32 current_frame;
static INT32 watchdog_counter;

static int cycles_running;
static int cycles_stolen;



/*************************************
 *
 *  Timer variables
 *
 *************************************/

static emu_timer *vblank_timer;
static INT32 vblank_countdown;
static INT32 vblank_multiplier;
static attotime vblank_period;

static emu_timer *update_timer;

emu_timer *refresh_timer;	/* temporarily made non-static (for ccpu) */
static attotime refresh_period;

static emu_timer *timeslice_timer;
static attotime timeslice_period;

static emu_timer *interleave_boost_timer;
static emu_timer *interleave_boost_timer_end;
static attotime perfect_interleave;

static emu_timer *watchdog_timer;



/*************************************
 *
 *  Static prototypes
 *
 *************************************/

static void cpuexec_exit(running_machine *machine);
static void cpuexec_reset(running_machine *machine);
static void cpu_inittimers(running_machine *machine);
static void cpu_vblankreset(void);
static TIMER_CALLBACK( cpu_vblankcallback );
static TIMER_CALLBACK( cpu_updatecallback );
static TIMER_CALLBACK( end_interleave_boost );
static void compute_perfect_interleave(void);
static void watchdog_setup(int alloc_new);



/*************************************
 *
 *  Watchdog Flags
 *
 *************************************/

#define WATCHDOG_IS_STARTED_DISABLED	-1
#define WATCHDOG_IS_DISABLED			-2
#define WATCHDOG_IS_TIMER_BASED			-3
#define WATCHDOG_IS_INVALID				-4
#define WATCHDOG_IS_BEING_STARTED		-5



#if 0
#pragma mark CORE CPU
#endif

/*************************************
 *
 *  Initialize all the CPUs
 *
 *************************************/

void cpuexec_init(running_machine *machine)
{
	int cpunum;

	/* if there has been no VBLANK time specified in the MACHINE_DRIVER, compute it now
       from the visible area */
	if (machine->screen[0].vblank == 0 && !machine->screen[0].oldstyle_vblank_supplied)
		machine->screen[0].vblank = (machine->screen[0].refresh / machine->screen[0].height) * (machine->screen[0].height - (machine->screen[0].visarea.max_y + 1 - machine->screen[0].visarea.min_y));

	/* allocate vblank and refresh timers, and compute the initial timing */
	vblank_timer = timer_alloc(cpu_vblankcallback, NULL);
	refresh_timer = timer_alloc(NULL, NULL);
	cpu_compute_vblank_timing();

	/* loop over all our CPUs */
	for (cpunum = 0; cpunum < MAX_CPU; cpunum++)
	{
		cpu_type cputype = machine->drv->cpu[cpunum].type;
		int num_regs;

		/* if this is a dummy, stop looking */
		if (cputype == CPU_DUMMY)
			break;

		/* initialize the cpuinfo struct */
		memset(&cpu[cpunum], 0, sizeof(cpu[cpunum]));
		cpu[cpunum].suspend = SUSPEND_REASON_RESET;
		cpu[cpunum].clock = machine->drv->cpu[cpunum].clock;
		cpu[cpunum].clockscale = 1.0;
		cpu[cpunum].localtime = attotime_zero;

		/* compute the cycle times */
		cycles_per_second[cpunum] = cpu[cpunum].clockscale * cpu[cpunum].clock;
		attoseconds_per_cycle[cpunum] = ATTOSECONDS_PER_SECOND / (cpu[cpunum].clockscale * cpu[cpunum].clock);

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

		state_save_register_item("cpu", cpunum, cpu[cpunum].vblankint_countdown);

		/* initialize this CPU */
		state_save_push_tag(cpunum + 1);
		num_regs = state_save_get_reg_count();
		if (cpuintrf_init_cpu(cpunum, cputype, cpu[cpunum].clock, machine->drv->cpu[cpunum].reset_param, cpu_irq_callbacks[cpunum]))
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

	/* compute the perfect interleave factor */
	compute_perfect_interleave();

	/* save some stuff in the default tag */
	state_save_push_tag(0);
	state_save_register_item("cpu", 0, vblank);
	state_save_register_item("cpu", 0, current_frame);
	state_save_register_item("cpu", 0, watchdog_counter);
	state_save_register_item("cpu", 0, vblank_countdown);
	state_save_pop_tag();
}



/*************************************
 *
 *  Prepare the system for execution
 *
 *************************************/

static void cpuexec_reset(running_machine *machine)
{
	int cpunum;

	/* initialize the various timers (suspends all CPUs at startup) */
	cpu_inittimers(machine);
	watchdog_counter = WATCHDOG_IS_INVALID;
	watchdog_setup(TRUE);

	/* first pass over CPUs */
	for (cpunum = 0; cpunum < cpu_gettotalcpu(); cpunum++)
	{
		/* enable all CPUs (except for disabled CPUs) */
		if (!(machine->drv->cpu[cpunum].flags & CPU_DISABLE))
			cpunum_resume(cpunum, SUSPEND_ANY_REASON);
		else
			cpunum_suspend(cpunum, SUSPEND_REASON_DISABLE, 1);

		/* reset the total number of cycles */
		cpu[cpunum].totalcycles = 0;

		/* then reset the CPU directly */
		cpunum_reset(cpunum);
	}

	/* reset the globals */
	cpu_vblankreset();
	vblank = 0;
	current_frame = 0;
}



/*************************************
 *
 *  Deinitialize all the CPUs
 *
 *************************************/

static void cpuexec_exit(running_machine *machine)
{
	int cpunum;

	/* shut down the CPU cores */
	for (cpunum = 0; cpunum < cpu_gettotalcpu(); cpunum++)
		cpuintrf_exit_cpu(cpunum);
}




#if 0
#pragma mark -
#pragma mark WATCHDOG
#endif

/*************************************
 *
 *  Watchdog timer callback
 *
 *************************************/

static TIMER_CALLBACK( watchdog_callback )
{
	logerror("reset caused by the (time) watchdog\n");
	mame_schedule_soft_reset(machine);
}



/*************************************
 *
 *  Watchdog setup routine
 *
 *************************************/

static void watchdog_setup(int alloc_new)
{
	if (watchdog_counter != WATCHDOG_IS_DISABLED)
	{
		if (Machine->drv->watchdog_vblank_count)
		{
			/* Start a vblank based watchdog. */
			watchdog_counter = Machine->drv->watchdog_vblank_count;
		}
		else if (attotime_compare(Machine->drv->watchdog_time, attotime_zero) != 0)
		{
			/* Start a time based watchdog. */
			if (alloc_new)
				watchdog_timer = timer_alloc(watchdog_callback, NULL);
			timer_adjust(watchdog_timer, Machine->drv->watchdog_time, 0, attotime_zero);
			watchdog_counter = WATCHDOG_IS_TIMER_BASED;
		}
		else if (watchdog_counter == WATCHDOG_IS_INVALID)
		{
			/* The watchdog was not initialized in the MACHINE_DRIVER,
             * so we will start with it disabled.
             */
			watchdog_counter = WATCHDOG_IS_STARTED_DISABLED;
		}
		else
		{
			/* The watchdog was not initialized in the MACHINE_DRIVER.
             * But it has been manually started, so we will default to
             * using a vblank watchdog.  We will set up a default time
             * of 3 times the refresh rate.  Which is 3 seconds @ 60Hz
             * refresh.

             * The 3 seconds delay is targeted at qzshowby, which otherwise
             * would reset at the start of a game.
             */
			watchdog_counter = 3 * ATTOSECONDS_TO_HZ(Machine->screen[0].refresh);
		}
	}
}



/*************************************
 *
 *  Watchdog reset
 *
 *************************************/

void watchdog_reset(void)
{
	if (watchdog_counter == WATCHDOG_IS_TIMER_BASED)
	{
		timer_reset(watchdog_timer, Machine->drv->watchdog_time);
	}
	else
	{
		if (watchdog_counter == WATCHDOG_IS_STARTED_DISABLED)
		{
			watchdog_counter = WATCHDOG_IS_BEING_STARTED;
			logerror("(vblank) watchdog armed by reset\n");
		}

		watchdog_setup(FALSE);
	}
}



/*************************************
 *
 *  Watchdog enable/disable
 *
 *************************************/

void watchdog_enable(int enable)
{
	if (!enable)
	{
		// Disable all timers
		watchdog_counter = WATCHDOG_IS_DISABLED;
	}
	else
	// Setup only on change from disable to enable.
	// Do not setup if watchdog is disabled from machine init.
	if (watchdog_counter == WATCHDOG_IS_DISABLED)
	{
		watchdog_counter = WATCHDOG_IS_BEING_STARTED;
		watchdog_setup(FALSE);
	}
}



#if 0
#pragma mark -
#pragma mark CPU SCHEDULING
#endif

/*************************************
 *
 *  Execute all the CPUs for one
 *  timeslice
 *
 *************************************/

void cpuexec_timeslice(void)
{
	attotime target = timer_next_fire_time();
	attotime base = timer_get_time();
	int cpunum, ran;

	LOG(("------------------\n"));
	LOG(("cpu_timeslice: target = %s\n", attotime_string(target, 9)));

	/* process any pending suspends */
	for (cpunum = 0; Machine->drv->cpu[cpunum].type != CPU_DUMMY; cpunum++)
	{
		if (cpu[cpunum].suspend != cpu[cpunum].nextsuspend)
			LOG(("--> updated CPU%d suspend from %X to %X\n", cpunum, cpu[cpunum].suspend, cpu[cpunum].nextsuspend));
		cpu[cpunum].suspend = cpu[cpunum].nextsuspend;
		cpu[cpunum].eatcycles = cpu[cpunum].nexteatcycles;
	}

	/* loop over CPUs */
	for (cpunum = 0; Machine->drv->cpu[cpunum].type != CPU_DUMMY; cpunum++)
	{
		/* only process if we're not suspended */
		if (!cpu[cpunum].suspend)
		{
			/* compute how long to run */
			cycles_running = ATTOTIME_TO_CYCLES(cpunum, attotime_sub(target, cpu[cpunum].localtime));
			LOG(("  cpu %d: %d cycles\n", cpunum, cycles_running));

			/* run for the requested number of cycles */
			if (cycles_running > 0)
			{
				profiler_mark(PROFILER_CPU1 + cpunum);

				/* note that this global variable cycles_stolen can be modified */
				/* via the call to the cpunum_execute */
				cycles_stolen = 0;
				ran = cpunum_execute(cpunum, cycles_running);

#ifdef MAME_DEBUG
				if (ran < cycles_stolen)
					fatalerror("Negative CPU cycle count!");
#endif /* MAME_DEBUG */

				ran -= cycles_stolen;
				profiler_mark(PROFILER_END);

				/* account for these cycles */
				cpu[cpunum].totalcycles += ran;
				cpu[cpunum].localtime = attotime_add(cpu[cpunum].localtime, ATTOTIME_IN_CYCLES(ran, cpunum));
				LOG(("         %d ran, %d total, time = %s\n", ran, (INT32)cpu[cpunum].totalcycles, attotime_string(cpu[cpunum].localtime, 9)));

				/* if the new local CPU time is less than our target, move the target up */
				if (attotime_compare(cpu[cpunum].localtime, target) < 0)
				{
					if (attotime_compare(cpu[cpunum].localtime, base) > 0)
						target = cpu[cpunum].localtime;
					else
						target = base;
					LOG(("         (new target)\n"));
				}
			}
		}
	}

	/* update the local times of all CPUs */
	for (cpunum = 0; Machine->drv->cpu[cpunum].type != CPU_DUMMY; cpunum++)
	{
		/* if we're suspended and counting, process */
		if (cpu[cpunum].suspend && cpu[cpunum].eatcycles && attotime_compare(cpu[cpunum].localtime, target) < 0)
		{
			/* compute how long to run */
			cycles_running = ATTOTIME_TO_CYCLES(cpunum, attotime_sub(target, cpu[cpunum].localtime));
			LOG(("  cpu %d: %d cycles (suspended)\n", cpunum, cycles_running));

			cpu[cpunum].totalcycles += cycles_running;
			cpu[cpunum].localtime = attotime_add(cpu[cpunum].localtime, ATTOTIME_IN_CYCLES(cycles_running, cpunum));
			LOG(("         %d skipped, %d total, time = %s\n", cycles_running, (INT32)cpu[cpunum].totalcycles, attotime_string(cpu[cpunum].localtime, 9)));
		}

		/* update the suspend state */
		if (cpu[cpunum].suspend != cpu[cpunum].nextsuspend)
			LOG(("--> updated CPU%d suspend from %X to %X\n", cpunum, cpu[cpunum].suspend, cpu[cpunum].nextsuspend));
		cpu[cpunum].suspend = cpu[cpunum].nextsuspend;
		cpu[cpunum].eatcycles = cpu[cpunum].nexteatcycles;
	}

	/* update the global time */
	timer_set_global_time(target);
}



/*************************************
 *
 *  Abort the timeslice for the
 *  active CPU
 *
 *************************************/

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



/*************************************
 *
 *  Return the current local time for
 *  a CPU, relative to the current
 *  timeslice
 *
 *************************************/

attotime cpunum_get_localtime(int cpunum)
{
	attotime result;

	VERIFY_CPUNUM(cpunum_get_localtime);

	/* if we're active, add in the time from the current slice */
	result = cpu[cpunum].localtime;
	if (cpunum == cpu_getexecutingcpu())
	{
		int cycles = cycles_currently_ran();
		result = attotime_add(result, ATTOTIME_IN_CYCLES(cycles, cpunum));
	}
	return result;
}



/*************************************
 *
 *  Set a suspend reason for the
 *  given CPU
 *
 *************************************/

void cpunum_suspend(int cpunum, int reason, int eatcycles)
{
	VERIFY_CPUNUM(cpunum_suspend);
	LOG(("cpunum_suspend (CPU=%d, r=%X, eat=%d)\n", cpunum, reason, eatcycles));

	/* set the pending suspend bits, and force a resync */
	cpu[cpunum].nextsuspend |= reason;
	cpu[cpunum].nexteatcycles = eatcycles;
	if (cpu_getexecutingcpu() >= 0)
		activecpu_abort_timeslice();
}



/*************************************
 *
 *  Clear a suspend reason for a
 *  given CPU
 *
 *************************************/

void cpunum_resume(int cpunum, int reason)
{
	VERIFY_CPUNUM(cpunum_resume);
	LOG(("cpunum_resume (CPU=%d, r=%X)\n", cpunum, reason));

	/* clear the pending suspend bits, and force a resync */
	cpu[cpunum].nextsuspend &= ~reason;
	if (cpu_getexecutingcpu() >= 0)
		activecpu_abort_timeslice();
}



/*************************************
 *
 *  Return true if a given CPU is
 *  suspended
 *
 *************************************/

int cpunum_is_suspended(int cpunum, int reason)
{
	VERIFY_CPUNUM(cpunum_suspend);
	return ((cpu[cpunum].nextsuspend & reason) != 0);
}



/*************************************
 *
 *  Gets the current CPU's clock speed
 *
 *************************************/

int cpunum_get_clock(int cpunum)
{
	VERIFY_CPUNUM(cpunum_get_clock);
	return cpu[cpunum].clock;
}



/*************************************
 *
 *  Sets the current CPU's clock speed
 *
 *************************************/

void cpunum_set_clock(int cpunum, int clock)
{
	VERIFY_CPUNUM(cpunum_set_clock);

	cpu[cpunum].clock = clock;
	cycles_per_second[cpunum] = (double)clock * cpu[cpunum].clockscale;
	attoseconds_per_cycle[cpunum] = ATTOSECONDS_PER_SECOND / ((double)clock * cpu[cpunum].clockscale);

	/* re-compute the perfect interleave factor */
	compute_perfect_interleave();
}



void cpunum_set_clock_period(int cpunum, attoseconds_t clock_period)
{
	VERIFY_CPUNUM(cpunum_set_clock);

	cpu[cpunum].clock = ATTOSECONDS_PER_SECOND / clock_period;
	cycles_per_second[cpunum] = (double) (ATTOSECONDS_PER_SECOND / clock_period) * cpu[cpunum].clockscale;
	attoseconds_per_cycle[cpunum] = clock_period;

	/* re-compute the perfect interleave factor */
	compute_perfect_interleave();
}



/*************************************
 *
 *  Returns the current scaling factor
 *  for a CPU's clock speed
 *
 *************************************/

double cpunum_get_clockscale(int cpunum)
{
	VERIFY_CPUNUM(cpunum_get_clockscale);
	return cpu[cpunum].clockscale;
}



/*************************************
 *
 *  Sets the current scaling factor
 *  for a CPU's clock speed
 *
 *************************************/

void cpunum_set_clockscale(int cpunum, double clockscale)
{
	VERIFY_CPUNUM(cpunum_set_clockscale);

	cpu[cpunum].clockscale = clockscale;
	cycles_per_second[cpunum] = (double)cpu[cpunum].clock * clockscale;
	attoseconds_per_cycle[cpunum] = ATTOSECONDS_PER_SECOND / ((double)cpu[cpunum].clock * clockscale);

	/* re-compute the perfect interleave factor */
	compute_perfect_interleave();
}



/*************************************
 *
 *  Temporarily boosts the interleave
 *  factor
 *
 *************************************/

void cpu_boost_interleave(attotime timeslice_time, attotime boost_duration)
{
	/* if you pass 0 for the timeslice_time, it means pick something reasonable */
	if (attotime_compare(timeslice_time, perfect_interleave) < 0)
		timeslice_time = perfect_interleave;

	LOG(("cpu_boost_interleave(%s, %s)\n", attotime_string(timeslice_time, 9), attotime_string(boost_duration, 9)));

	/* adjust the interleave timer */
	timer_adjust(interleave_boost_timer, timeslice_time, 0, timeslice_time);

	/* adjust the end timer, but only if we are going to extend it */
	if (!timer_enabled(interleave_boost_timer_end) || attotime_compare(timer_timeleft(interleave_boost_timer_end), boost_duration) < 0)
		timer_adjust(interleave_boost_timer_end, boost_duration, 0, attotime_never);
}



#if 0
#pragma mark -
#pragma mark TIMING HELPERS
#endif

/*************************************
 *
 *  Return cycles ran this iteration
 *
 *************************************/

int cycles_currently_ran(void)
{
	VERIFY_EXECUTINGCPU(cycles_currently_ran);
	return cycles_running - activecpu_get_icount();
}



/*************************************
 *
 *  Return total number of CPU cycles
 *  for the active CPU or for a given CPU.
 *
 *************************************/

/*--------------------------------------------------------------

    IMPORTANT: this value wraps around in a relatively short
    time. For example, for a 6MHz CPU, it will wrap around in
    2^32/6000000 = 716 seconds = 12 minutes.

    Make sure you don't do comparisons between values returned
    by this function, but only use the difference (which will
    be correct regardless of wraparound).

    Alternatively, use the new 64-bit variants instead.

--------------------------------------------------------------*/

UINT32 activecpu_gettotalcycles(void)
{
	VERIFY_ACTIVECPU(activecpu_gettotalcycles);
	if (activecpu == cpu_getexecutingcpu())
		return cpu[activecpu].totalcycles + cycles_currently_ran();
	else
		return cpu[activecpu].totalcycles;
}

UINT32 cpunum_gettotalcycles(int cpunum)
{
	VERIFY_CPUNUM(cpunum_gettotalcycles);
	if (cpunum == cpu_getexecutingcpu())
		return cpu[cpunum].totalcycles + cycles_currently_ran();
	else
		return cpu[cpunum].totalcycles;
}


UINT64 activecpu_gettotalcycles64(void)
{
	VERIFY_ACTIVECPU(activecpu_gettotalcycles64);
	if (activecpu == cpu_getexecutingcpu())
		return cpu[activecpu].totalcycles + cycles_currently_ran();
	else
		return cpu[activecpu].totalcycles;
}

UINT64 cpunum_gettotalcycles64(int cpunum)
{
	VERIFY_CPUNUM(cpunum_gettotalcycles64);
	if (cpunum == cpu_getexecutingcpu())
		return cpu[cpunum].totalcycles + cycles_currently_ran();
	else
		return cpu[cpunum].totalcycles;
}



/*************************************
 *
 *  Safely eats cycles so we don't
 *  cross a timeslice boundary
 *
 *************************************/

void activecpu_eat_cycles(int cycles)
{
	int cyclesleft = activecpu_get_icount();
	if (cycles > cyclesleft)
		cycles = cyclesleft;
	activecpu_adjust_icount(-cycles);
}



/*************************************
 *
 *  Scales a given value by the fraction
 *  of time elapsed between refreshes
 *
 *************************************/

int cpu_scalebyfcount(int value)
{
	attotime refresh_elapsed = timer_timeelapsed(refresh_timer);
	int result;

	/* shift off some bits to ensure no overflow */
	if (value < 65536)
		result = value * (refresh_elapsed.attoseconds >> 16) / (refresh_period.attoseconds >> 16);
	else
		result = value * (refresh_elapsed.attoseconds >> 32) / (refresh_period.attoseconds >> 32);
	if (value >= 0)
		return (result < value) ? result : value;
	else
		return (result > value) ? result : value;
}



#if 0
#pragma mark -
#pragma mark VIDEO TIMING
#endif

/*************************************
 *
 *  Computes the VBLANK timing
 *
 *************************************/

void cpu_compute_vblank_timing(void)
{
	refresh_period = attotime_make(0, Machine->screen[0].refresh);

	/* recompute the vblank period */
	vblank_period = attotime_make(0, Machine->screen[0].refresh / (vblank_multiplier ? vblank_multiplier : 1));
	if (vblank_timer != NULL && timer_enable(vblank_timer, FALSE))
	{
		attotime remaining = timer_timeleft(vblank_timer);
		if (remaining.seconds == 0 && remaining.attoseconds == 0)
			remaining = vblank_period;
		timer_adjust(vblank_timer, remaining, 0, vblank_period);
	}

	LOG(("cpu_compute_vblank_timing: refresh=%s vblank=%s\n", attotime_string(refresh_period, 9), attotime_string(vblank_period, 9)));
}



/*************************************
 *
 *  Returns the VBLANK state
 *
 *************************************/

int cpu_getvblank(void)
{
	return vblank;
}



/*************************************
 *
 *  Returns the current frame count
 *
 *************************************/

int cpu_getcurrentframe(void)
{
	return current_frame;
}



#if 0
#pragma mark -
#pragma mark SYNCHRONIZATION
#endif

/*************************************
 *
 *  Generate a specific trigger
 *
 *************************************/

void cpu_trigger(int trigger)
{
	int cpunum;

	/* cause an immediate resynchronization */
	if (cpu_getexecutingcpu() >= 0)
		activecpu_abort_timeslice();

	/* look for suspended CPUs waiting for this trigger and unsuspend them */
	for (cpunum = 0; cpunum < MAX_CPU; cpunum++)
	{
		/* if this is a dummy, stop looking */
		if (Machine->drv->cpu[cpunum].type == CPU_DUMMY)
			break;

		/* see if this is a matching trigger */
		if (cpu[cpunum].suspend && cpu[cpunum].trigger == trigger)
		{
			cpunum_resume(cpunum, SUSPEND_REASON_TRIGGER);
			cpu[cpunum].trigger = 0;
		}
	}
}



/*************************************
 *
 *  Generate a trigger in the future
 *
 *************************************/

static TIMER_CALLBACK( cpu_triggertime_callback )
{
	cpu_trigger(param);
}


void cpu_triggertime(attotime duration, int trigger)
{
	timer_set(duration, NULL, trigger, cpu_triggertime_callback);
}



/*************************************
 *
 *  Generate a trigger for an int
 *
 *************************************/

void cpu_triggerint(int cpunum)
{
	cpu_trigger(TRIGGER_INT + cpunum);
}



/*************************************
 *
 *  Burn/yield CPU cycles until a trigger
 *
 *************************************/

void cpu_spinuntil_trigger(int trigger)
{
	int cpunum = cpu_getexecutingcpu();

	VERIFY_EXECUTINGCPU(cpu_spinuntil_trigger);

	/* suspend the CPU immediately if it's not already */
	cpunum_suspend(cpunum, SUSPEND_REASON_TRIGGER, 1);

	/* set the trigger */
	cpu[cpunum].trigger = trigger;
}

void cpunum_spinuntil_trigger( int cpunum, int trigger )
{
	VERIFY_CPUNUM(cpunum_spinuntil_trigger);

	/* suspend the CPU immediately if it's not already */
	cpunum_suspend(cpunum, SUSPEND_REASON_TRIGGER, 1);

	/* set the trigger */
	cpu[cpunum].trigger = trigger;
}



/*************************************
 *
 *  Burn/yield CPU cycles until an
 *  interrupt
 *
 *************************************/

void cpu_spinuntil_int(void)
{
	VERIFY_EXECUTINGCPU(cpu_spinuntil_int);
	cpu_spinuntil_trigger(TRIGGER_INT + activecpu);
}



/*************************************
 *
 *  Burn/yield CPU cycles until the
 *  end of the current timeslice
 *
 *************************************/

void cpu_spin(void)
{
	cpu_spinuntil_trigger(TRIGGER_TIMESLICE);
}


void cpu_yield(void)
{
	int cpunum = cpu_getexecutingcpu();

	VERIFY_EXECUTINGCPU(cpu_yielduntil_trigger);

	/* suspend the CPU immediately if it's not already */
	cpunum_suspend(cpunum, SUSPEND_REASON_TRIGGER, 0);

	/* set the trigger */
	cpu[cpunum].trigger = TRIGGER_TIMESLICE;
}



/*************************************
 *
 *  Burn/yield CPU cycles for a
 *  specific period of time
 *
 *************************************/

void cpu_spinuntil_time(attotime duration)
{
	static int timetrig = 0;

	cpu_spinuntil_trigger(TRIGGER_SUSPENDTIME + timetrig);
	cpu_triggertime(duration, TRIGGER_SUSPENDTIME + timetrig);
	timetrig = (timetrig + 1) & 255;
}



#if 0
#pragma mark -
#pragma mark CORE TIMING
#endif

/*************************************
 *
 *  Returns the number of times the
 *  interrupt handler will be called
 *  before the end of the current
 *  video frame.
 *
 *************************************/

/*--------------------------------------------------------------

    This can be useful to interrupt handlers to synchronize
    their operation. If you call this from outside an interrupt
    handler, add 1 to the result, i.e. if it returns 0, it means
    that the interrupt handler will be called once.

--------------------------------------------------------------*/

int cpu_getiloops(void)
{
	VERIFY_ACTIVECPU(cpu_getiloops);
	return cpu[activecpu].iloops;
}



/*************************************
 *
 *  Hook for updating things on the
 *  real VBLANK (once per frame)
 *
 *************************************/

static void cpu_vblankreset(void)
{
	int cpunum;

	/* notify the video system of a VBLANK start */
	video_vblank_start(Machine);

	/* read keyboard & update the status of the input ports */
	input_port_vblank_start();

	/* check the watchdog */
	if (watchdog_counter > 0)
	{
		if (--watchdog_counter == 0)
		{
			logerror("reset caused by the (vblank) watchdog\n");
			mame_schedule_soft_reset(Machine);
		}
	}

	/* reset the cycle counters */
	for (cpunum = 0; cpunum < cpu_gettotalcpu(); cpunum++)
	{
		if (!(cpu[cpunum].suspend & SUSPEND_REASON_DISABLE))
			cpu[cpunum].iloops = Machine->drv->cpu[cpunum].vblank_interrupts_per_frame - 1;
		else
			cpu[cpunum].iloops = -1;
	}
}



/*************************************
 *
 *  First-run callback for VBLANKs
 *
 *************************************/

static TIMER_CALLBACK( cpu_firstvblankcallback )
{
	/* now that we're synced up, pulse from here on out */
	timer_adjust(vblank_timer, vblank_period, param, vblank_period);

	/* but we need to call the standard routine as well */
	cpu_vblankcallback(machine, NULL, param);
}



/*************************************
 *
 *  VBLANK core handler
 *
 *************************************/

static TIMER_CALLBACK( cpu_vblankcallback )
{
	int cpunum;

	if (vblank_countdown == 1)
		vblank = 1;

	/* loop over CPUs */
	for (cpunum = 0; cpunum < cpu_gettotalcpu(); cpunum++)
	{
		/* if the interrupt multiplier is valid */
		if (cpu[cpunum].vblankint_multiplier != -1)
		{
			/* decrement; if we hit zero, generate the interrupt and reset the countdown */
			if (!--cpu[cpunum].vblankint_countdown)
			{
				/* a param of -1 means don't call any callbacks */
				if (param != -1)
				{
					/* if the CPU has a VBLANK handler, call it */
					if (machine->drv->cpu[cpunum].vblank_interrupt && !cpunum_is_suspended(cpunum, SUSPEND_REASON_HALT | SUSPEND_REASON_RESET | SUSPEND_REASON_DISABLE))
					{
						cpuintrf_push_context(cpunum);
						(*machine->drv->cpu[cpunum].vblank_interrupt)();
						cpuintrf_pop_context();
					}

					/* update the counters */
					cpu[cpunum].iloops--;
				}

				/* reset the countdown and timer */
				cpu[cpunum].vblankint_countdown = cpu[cpunum].vblankint_multiplier;
				timer_adjust(cpu[cpunum].vblankint_timer, attotime_never, 0, attotime_never);
			}
		}

		/* else reset the VBLANK timer if this is going to be a real VBLANK */
		else if (vblank_countdown == 1)
			timer_adjust(cpu[cpunum].vblankint_timer, attotime_never, 0, attotime_never);
	}

	/* is it a real VBLANK? */
	if (!--vblank_countdown)
	{
		/* do we update the screen now? */
		if (!(machine->drv->video_attributes & VIDEO_UPDATE_AFTER_VBLANK))
			video_frame_update(FALSE);

		/* Set the timer to update the screen */
		timer_adjust(update_timer, attotime_make(0, machine->screen[0].vblank), 0, attotime_zero);

		/* reset the globals */
		cpu_vblankreset();

		/* reset the counter */
		vblank_countdown = vblank_multiplier;

#ifdef MAME_DEBUG
		/* notify the debugger */
		debug_vblank_hook();
#endif
	}
}



/*************************************
 *
 *  End-of-VBLANK callback
 *
 *************************************/

static TIMER_CALLBACK( cpu_updatecallback )
{
	/* update the screen if we didn't before */
	if (machine->drv->video_attributes & VIDEO_UPDATE_AFTER_VBLANK)
		video_frame_update(FALSE);
	vblank = 0;

	/* update IPT_VBLANK input ports */
	input_port_vblank_end();

	/* track total frames */
	current_frame++;

	/* reset the refresh timer */
	timer_adjust(refresh_timer, attotime_never, 0, attotime_never);
}



/*************************************
 *
 *  Callback for timed interrupts
 *  (not tied to a VBLANK)
 *
 *************************************/

static TIMER_CALLBACK( cpu_timedintcallback )
{
	/* bail if there is no routine */
	if (machine->drv->cpu[param].timed_interrupt && !cpunum_is_suspended(param, SUSPEND_REASON_HALT | SUSPEND_REASON_RESET | SUSPEND_REASON_DISABLE))
	{
		cpuintrf_push_context(param);
		(*machine->drv->cpu[param].timed_interrupt)();
		cpuintrf_pop_context();
	}
}



/*************************************
 *
 *  Callback to force a timeslice
 *
 *************************************/

static TIMER_CALLBACK( cpu_timeslicecallback )
{
	cpu_trigger(TRIGGER_TIMESLICE);
}



/*************************************
 *
 *  Callback to end a temporary
 *  interleave boost
 *
 *************************************/

static TIMER_CALLBACK( end_interleave_boost )
{
	timer_adjust(interleave_boost_timer, attotime_never, 0, attotime_never);
	LOG(("end_interleave_boost\n"));
}



/*************************************
 *
 *  Compute the "perfect" interleave
 *  interval
 *
 *************************************/

static void compute_perfect_interleave(void)
{
	attoseconds_t smallest = attoseconds_per_cycle[0];
	int cpunum;

	/* start with a huge time factor and find the 2nd smallest cycle time */
	perfect_interleave = attotime_zero;
	perfect_interleave.attoseconds = ATTOSECONDS_PER_SECOND - 1;
	for (cpunum = 1; Machine->drv->cpu[cpunum].type != CPU_DUMMY; cpunum++)
	{
		/* find the 2nd smallest cycle interval */
		if (attoseconds_per_cycle[cpunum] < smallest)
		{
			perfect_interleave.attoseconds = smallest;
			smallest = attoseconds_per_cycle[cpunum];
		}
		else if (attoseconds_per_cycle[cpunum] < perfect_interleave.attoseconds)
			perfect_interleave.attoseconds = attoseconds_per_cycle[cpunum];
	}

	/* adjust the final value */
	if (perfect_interleave.attoseconds == ATTOSECONDS_PER_SECOND - 1)
		perfect_interleave.attoseconds = attoseconds_per_cycle[0];

	LOG(("Perfect interleave = %s, smallest = %.9f\n", attotime_string(perfect_interleave, 9), ATTOSECONDS_TO_DOUBLE(smallest)));
}



/*************************************
 *
 *  Setup all the core timers
 *
 *************************************/

static void cpu_inittimers(running_machine *machine)
{
	attotime first_time;
	int cpunum, max, ipf;

	/* allocate a dummy timer at the minimum frequency to break things up */
	ipf = machine->drv->cpu_slices_per_frame;
	if (ipf <= 0)
		ipf = 1;
	timeslice_period = attotime_make(0, machine->screen[0].refresh / ipf);
	timeslice_timer = timer_alloc(cpu_timeslicecallback, NULL);
	timer_adjust(timeslice_timer, timeslice_period, 0, timeslice_period);

	/* allocate timers to handle interleave boosts */
	interleave_boost_timer = timer_alloc(NULL, NULL);
	interleave_boost_timer_end = timer_alloc(end_interleave_boost, NULL);

	/*
     *  The following code finds all the CPUs that are interrupting in sync with the VBLANK
     *  and sets up the VBLANK timer to run at the minimum number of cycles per frame in
     *  order to service all the synced interrupts
     */

	/* find the CPU with the maximum interrupts per frame */
	max = 1;
	for (cpunum = 0; cpunum < cpu_gettotalcpu(); cpunum++)
	{
		ipf = machine->drv->cpu[cpunum].vblank_interrupts_per_frame;
		if (ipf > max)
			max = ipf;
	}

	/* now find the LCD with the rest of the CPUs (brute force - these numbers aren't huge) */
	vblank_multiplier = max;
	while (1)
	{
		for (cpunum = 0; cpunum < cpu_gettotalcpu(); cpunum++)
		{
			ipf = machine->drv->cpu[cpunum].vblank_interrupts_per_frame;
			if (ipf > 0 && (vblank_multiplier % ipf) != 0)
				break;
		}
		if (cpunum == cpu_gettotalcpu())
			break;
		vblank_multiplier += max;
	}

	/* initialize the countdown timers and intervals */
	for (cpunum = 0; cpunum < cpu_gettotalcpu(); cpunum++)
	{
		ipf = machine->drv->cpu[cpunum].vblank_interrupts_per_frame;
		if (ipf > 0)
			cpu[cpunum].vblankint_countdown = cpu[cpunum].vblankint_multiplier = vblank_multiplier / ipf;
		else
			cpu[cpunum].vblankint_countdown = cpu[cpunum].vblankint_multiplier = -1;
	}

	/* allocate a vblank timer at the frame rate * the LCD number of interrupts per frame */
	vblank_period = attotime_make(0, machine->screen[0].refresh / vblank_multiplier);
	vblank_countdown = vblank_multiplier;

	/* allocate an update timer that will be used to time the actual screen updates */
	update_timer = timer_alloc(cpu_updatecallback, NULL);

	/*
     *      The following code creates individual timers for each CPU whose interrupts are not
     *      synced to the VBLANK, and computes the typical number of cycles per interrupt
     */

	/* start the CPU interrupt timers */
	for (cpunum = 0; cpunum < cpu_gettotalcpu(); cpunum++)
	{
		ipf = machine->drv->cpu[cpunum].vblank_interrupts_per_frame;

		/* compute the average number of cycles per interrupt */
		if (ipf <= 0)
			ipf = 1;
		cpu[cpunum].vblankint_timer = timer_alloc(NULL, NULL);

		/* see if we need to allocate a CPU timer */
		if (machine->drv->cpu[cpunum].timed_interrupt_period != 0)
		{
			cpu[cpunum].timedint_period = attotime_make(0, machine->drv->cpu[cpunum].timed_interrupt_period);
			cpu[cpunum].timedint_timer = timer_alloc(cpu_timedintcallback, NULL);
			timer_adjust(cpu[cpunum].timedint_timer, cpu[cpunum].timedint_period, cpunum, cpu[cpunum].timedint_period);
		}
	}

	/* note that since we start the first frame on the refresh, we can't pulse starting
       immediately; instead, we back up one VBLANK period, and inch forward until we hit
       positive time. That time will be the time of the first VBLANK timer callback */
	first_time = attotime_sub_attoseconds(vblank_period, machine->screen[0].vblank);
	while (attotime_compare(first_time, attotime_zero) < 0)
	{
		cpu_vblankcallback(machine, NULL, -1);
		first_time = attotime_add(first_time, vblank_period);
	}
	timer_set(first_time, NULL, 0, cpu_firstvblankcallback);

	/* reset the refresh timer to get ourself back in sync */
	timer_adjust(refresh_timer, attotime_never, 0, attotime_never);
}

