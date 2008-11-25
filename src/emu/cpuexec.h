/***************************************************************************

    cpuexec.h

    Core multi-CPU execution engine.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __CPUEXEC_H__
#define __CPUEXEC_H__

#include "cpuintrf.h"
#include "timer.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* flags for MDRV_CPU_FLAGS */
enum
{
	/* set this flag to disable execution of a CPU (if one is there for documentation */
	/* purposes only, for example */
	CPU_DISABLE = 0x0001
};


/* suspension reasons for cpunum_suspend */
enum
{
	SUSPEND_REASON_HALT 	= 0x0001,
	SUSPEND_REASON_RESET 	= 0x0002,
	SUSPEND_REASON_SPIN 	= 0x0004,
	SUSPEND_REASON_TRIGGER 	= 0x0008,
	SUSPEND_REASON_DISABLE 	= 0x0010,
	SUSPEND_REASON_TIMESLICE= 0x0020,
	SUSPEND_ANY_REASON 		= ~0
};



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* CPU description for drivers */
typedef struct _cpu_config cpu_config;
struct _cpu_config
{
	cpu_type		type;						/* index for the CPU type */
	int				flags;						/* flags; see #defines below */
	int				clock;						/* in Hertz */
	const addrmap_token *address_map[ADDRESS_SPACES][2]; /* 2 memory maps per address space */
	void 			(*vblank_interrupt)(const device_config *device);	/* for interrupts tied to VBLANK */
	int 			vblank_interrupts_per_frame;/* usually 1 */
	const char *	vblank_interrupt_screen;	/* the screen that causes the VBLANK interrupt */
	void 			(*timed_interrupt)(const device_config *device);	/* for interrupts not tied to VBLANK */
	attoseconds_t 	timed_interrupt_period;		/* period for periodic interrupts */
	const void *	reset_param;				/* parameter for cpu_reset */
	const char *	tag;
};



/***************************************************************************
    MACROS
***************************************************************************/

#define INTERRUPT_GEN(func)		void func(const device_config *device)



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- core CPU execution ----- */

/* temporary function to allocate a fake CPU device for each CPU */
const device_config *cpuexec_create_cpu_device(const cpu_config *config);

/* prepare CPUs for execution */
void cpuexec_init(running_machine *machine);

/* execute for a single timeslice */
void cpuexec_timeslice(running_machine *machine);

/* temporarily boosts the interleave factor */
void cpuexec_boost_interleave(running_machine *machine, attotime timeslice_time, attotime boost_duration);

/* return a pointer to the given CPU by tag */
const device_config *cputag_get_cpu(running_machine *machine, const char *tag);



/* ----- CPU scheduling----- */

/* suspend the given CPU for a specific reason */
void cpu_suspend(const device_config *device, int reason, int eatcycles);

/* resume the given CPU for a specific reason */
void cpu_resume(const device_config *device, int reason);

/* returns true if the given CPU is suspended for any of the given reasons */
int cpu_is_suspended(const device_config *device, int reason);



/* ----- CPU clock management ----- */

/* returns the current CPU's unscaled running clock speed */
int cpu_get_clock(const device_config *device);

/* sets the current CPU's clock speed and then adjusts for scaling */
void cpu_set_clock(const device_config *device, int clock);

/* returns the current scaling factor for a CPU's clock speed */
double cpu_get_clockscale(const device_config *device);

/* sets the current scaling factor for a CPU's clock speed */
void cpu_set_clockscale(const device_config *device, double clockscale);

/* converts a number of clock ticks to an attotime */
attotime cpu_clocks_to_attotime(const device_config *device, UINT32 clocks);

/* converts a duration as attotime to CPU clock ticks */
UINT32 cpu_attotime_to_clocks(const device_config *device, attotime duration);



/* ----- CPU timing ----- */

/* returns the current local time for a CPU */
attotime cpu_get_local_time(const device_config *device);

/* returns the total number of CPU cycles for a given CPU */
UINT64 cpu_get_total_cycles(const device_config *device);

/* safely eats cycles so we don't cross a timeslice boundary */
void cpu_eat_cycles(const device_config *device, int cycles);

/* apply a +/- to the current icount */
void cpu_adjust_icount(const device_config *device, int delta);

/* aborts the timeslice for the active CPU */
void cpu_abort_timeslice(const device_config *device);



/* ----- synchronization helpers ----- */

/* yield the given CPU until the end of the current timeslice */
void cpu_yield(const device_config *device);

/* burn CPU cycles until the end of the current timeslice */
void cpu_spin(const device_config *device);

/* burn specified CPU cycles until a trigger */
void cpu_spinuntil_trigger(const device_config *device, int trigger);

/* burn CPU cycles until the next interrupt */
void cpu_spinuntil_int(const device_config *device);

/* burn CPU cycles for a specific period of time */
void cpu_spinuntil_time(const device_config *device, attotime duration);



/* ----- triggers ----- */

/* generate a global trigger now */
void cpuexec_trigger(running_machine *machine, int trigger);

/* generate a global trigger after a specific period of time */
void cpuexec_triggertime(running_machine *machine, int trigger, attotime duration);

/* generate a trigger corresponding to an interrupt on the given CPU */
void cpu_triggerint(const device_config *device);



/* ----- interrupts ----- */

/* set the logical state (ASSERT_LINE/CLEAR_LINE) of the an input line on a CPU */
void cpu_set_input_line(const device_config *cpu, int line, int state);

/* set the vector to be returned during a CPU's interrupt acknowledge cycle */
void cpu_set_input_line_vector(const device_config *cpu, int irqline, int vector);

/* set the logical state (ASSERT_LINE/CLEAR_LINE) of the an input line on a CPU and its associated vector */
void cpu_set_input_line_and_vector(const device_config *cpu, int line, int state, int vector);

/* install a driver-specific callback for IRQ acknowledge */
void cpu_set_irq_callback(const device_config *cpu, cpu_irq_callback callback);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    cputag_set_input_line - set the logical state
    (ASSERT_LINE/CLEAR_LINE) of an input line
    on a CPU specified by machine/tag
-------------------------------------------------*/

INLINE void cputag_set_input_line(running_machine *machine, const char *tag, int line, int state)
{
	cpu_set_input_line(cputag_get_cpu(machine, tag), line, state);
}


/*-------------------------------------------------
    cputag_set_input_line_and_vector - set the
    logical state (ASSERT_LINE/CLEAR_LINE) of an
    input line on a CPU and its associated vector
-------------------------------------------------*/

INLINE void cputag_set_input_line_and_vector(running_machine *machine, const char *tag, int line, int state, int vector)
{
	cpu_set_input_line_and_vector(cputag_get_cpu(machine, tag), line, state, vector);
}



#endif	/* __CPUEXEC_H__ */
