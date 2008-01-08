/***************************************************************************

    cpuexec.h

    Core multi-CPU execution engine.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __CPUEXEC_H__
#define __CPUEXEC_H__

#include "memory.h"
#include "timer.h"


/* Enum listing all the CPUs */
typedef enum _cpu_type cpu_type;
enum _cpu_type
{
	CPU_DUMMY,
	CPU_Z80,
	CPU_Z180,
	CPU_8080,
	CPU_8085A,
	CPU_M6502,
	CPU_M65C02,
	CPU_M65SC02,
	CPU_M65CE02,
	CPU_M6509,
	CPU_M6510,
	CPU_M6510T,
	CPU_M7501,
	CPU_M8502,
	CPU_N2A03,
	CPU_DECO16,
	CPU_M4510,
	CPU_H6280,
	CPU_I8086,
	CPU_I8088,
	CPU_I80186,
	CPU_I80188,
	CPU_I80286,
	CPU_V20,
	CPU_V25,
	CPU_V30,
	CPU_V33,
	CPU_V35,
	CPU_V60,
	CPU_V70,
	CPU_I8035,
	CPU_I8039,
	CPU_I8048,
	CPU_N7751,
	CPU_I8X41,
	CPU_I8051,
	CPU_I8052,
	CPU_I8751,
	CPU_I8752,
	CPU_M6800,
	CPU_M6801,
	CPU_M6802,
	CPU_M6803,
	CPU_M6808,
	CPU_HD63701,
	CPU_NSC8105,
	CPU_M6805,
	CPU_M68705,
	CPU_HD63705,
	CPU_HD6309,
	CPU_M6809,
	CPU_M6809E,
	CPU_KONAMI,
	CPU_M68000,
	CPU_M68008,
	CPU_M68010,
	CPU_M68EC020,
	CPU_M68020,
	CPU_M68040,
	CPU_T11,
	CPU_S2650,
	CPU_TMS34010,
	CPU_TMS34020,
	CPU_TI990_10,
	CPU_TMS9900,
	CPU_TMS9940,
	CPU_TMS9980,
	CPU_TMS9985,
	CPU_TMS9989,
	CPU_TMS9995,
	CPU_TMS99100,
	CPU_TMS99105A,
	CPU_TMS99110A,
	CPU_Z8000,
	CPU_TMS32010,
	CPU_TMS32025,
	CPU_TMS32026,
	CPU_TMS32031,
	CPU_TMS32032,
	CPU_TMS32051,
	CPU_CCPU,
	CPU_ADSP2100,
 	CPU_ADSP2101,
	CPU_ADSP2104,
	CPU_ADSP2105,
	CPU_ADSP2115,
	CPU_ADSP2181,
	CPU_PSXCPU,
	CPU_ASAP,
	CPU_UPD7810,
	CPU_UPD7807,
	CPU_JAGUARGPU,
	CPU_JAGUARDSP,
	CPU_R3000BE,
	CPU_R3000LE,
	CPU_R4600BE,
	CPU_R4600LE,
	CPU_R4650BE,
	CPU_R4650LE,
	CPU_R4700BE,
	CPU_R4700LE,
	CPU_R5000BE,
	CPU_R5000LE,
	CPU_QED5271BE,
	CPU_QED5271LE,
	CPU_RM7000BE,
	CPU_RM7000LE,
	CPU_ARM,
	CPU_ARM7,
	CPU_SH2,
	CPU_SH4,
	CPU_DSP32C,
	CPU_PIC16C54,
	CPU_PIC16C55,
	CPU_PIC16C56,
	CPU_PIC16C57,
	CPU_PIC16C58,
	CPU_G65816,
	CPU_SPC700,
	CPU_E116T,
	CPU_E116XT,
	CPU_E116XS,
	CPU_E116XSR,
	CPU_E132N,
	CPU_E132T,
	CPU_E132XN,
	CPU_E132XT,
	CPU_E132XS,
	CPU_E132XSR,
	CPU_GMS30C2116,
	CPU_GMS30C2132,
	CPU_GMS30C2216,
	CPU_GMS30C2232,
	CPU_I386,
	CPU_I486,
	CPU_PENTIUM,
	CPU_MEDIAGX,
	CPU_I960,
	CPU_H83002,
	CPU_V810,
	CPU_M37702,
	CPU_M37710,
	CPU_PPC403,
	CPU_PPC601,
	CPU_PPC602,
	CPU_PPC603,
	CPU_PPC604,
	CPU_MPC8240,
	CPU_SE3208,
	CPU_MC68HC11,
	CPU_ADSP21062,
	CPU_DSP56156,
	CPU_RSP,
	CPU_ALPHA8201,
	CPU_ALPHA8301,
	CPU_CDP1802,
	CPU_COP420,
	CPU_COP410,
	CPU_COP411,
	CPU_TMP90840,
	CPU_TMP90841,
	CPU_TMP91640,
	CPU_TMP91641,
	CPU_APEXC,
	CPU_CP1610,
	CPU_F8,
	CPU_LH5801,
	CPU_PDP1,
	CPU_SATURN,
	CPU_SC61860,
	CPU_TX0_64KW,
	CPU_TX0_8KW,
	CPU_Z80GB,
	CPU_TMS7000,
	CPU_TMS7000_EXL,
	CPU_SM8500,
	CPU_V30MZ,
	CPU_MB8841,
	CPU_MB8842,
	CPU_MB8843,
	CPU_MB8844,
	CPU_MB86233,
	CPU_SSP1610,
	CPU_MINX,
    CPU_COUNT
};


/*************************************
 *
 *  CPU description for drivers
 *
 *************************************/

typedef struct _cpu_config cpu_config;
struct _cpu_config
{
	cpu_type		type;					/* index for the CPU type */
	int			flags;					/* flags; see #defines below */
	int			clock;					/* in Hertz */
	construct_map_t construct_map[ADDRESS_SPACES][2]; /* 2 memory maps per address space */
	void 		(*vblank_interrupt)(running_machine *machine, int cpunum);	/* for interrupts tied to VBLANK */
	int 		vblank_interrupts_per_frame;/* usually 1 */
	void 		(*timed_interrupt)(running_machine *machine, int cpunum);	/* for interrupts not tied to VBLANK */
	attoseconds_t timed_interrupt_period;	/* period for periodic interrupts */
	const void *reset_param;				/* parameter for cpu_reset */
	const char *tag;
};



/*************************************
 *
 *  CPU flag constants
 *
 *************************************/

enum
{
	/* set this flag to disable execution of a CPU (if one is there for documentation */
	/* purposes only, for example */
	CPU_DISABLE = 0x0001
};




/*************************************
 *
 *  Core CPU execution
 *
 *************************************/

/* Prepare CPUs for execution */
void cpuexec_init(running_machine *machine);

/* Execute for a single timeslice */
void cpuexec_timeslice(void);



/*************************************
 *
 *  Optional watchdog
 *
 *************************************/

/* bang on the watchdog */
void watchdog_reset(void);

/* watchdog enabled when TRUE */
/* timer is set to reset state when going from disable to enable */
void watchdog_enable(int enable);



/*************************************
 *
 *  CPU scheduling
 *
 *************************************/

/* Suspension reasons */
enum
{
	SUSPEND_REASON_HALT 	= 0x0001,
	SUSPEND_REASON_RESET 	= 0x0002,
	SUSPEND_REASON_SPIN 	= 0x0004,
	SUSPEND_REASON_TRIGGER 	= 0x0008,
	SUSPEND_REASON_DISABLE 	= 0x0010,
	SUSPEND_ANY_REASON 		= ~0
};

/* Suspend the given CPU for a specific reason */
void cpunum_suspend(int cpunum, int reason, int eatcycles);

/* Suspend the given CPU for a specific reason */
void cpunum_resume(int cpunum, int reason);

/* Returns true if the given CPU is suspended for any of the given reasons */
int cpunum_is_suspended(int cpunum, int reason);

/* Aborts the timeslice for the active CPU */
void activecpu_abort_timeslice(void);

/* Returns the current local time for a CPU */
attotime cpunum_get_localtime(int cpunum);

/* Returns the current CPU's unscaled running clock speed */
/* If you want to know the current effective running clock
 * after scaling it is just the double sec_to_cycles[cpunum] */
int cpunum_get_clock(int cpunum);

/* Sets the current CPU's clock speed and then adjusts for scaling */
void cpunum_set_clock(int cpunum, int clock);
void cpunum_set_clock_period(int cpunum, attoseconds_t clock_period);

/* Returns the current scaling factor for a CPU's clock speed */
double cpunum_get_clockscale(int cpunum);

/* Sets the current scaling factor for a CPU's clock speed */
void cpunum_set_clockscale(int cpunum, double clockscale);

/* Temporarily boosts the interleave factor */
void cpu_boost_interleave(attotime timeslice_time, attotime boost_duration);



/*************************************
 *
 *  Timing helpers
 *
 *************************************/

/* Returns the number of cycles run so far this timeslice */
int cycles_currently_ran(void);

/* Returns the total number of CPU cycles */
UINT32 activecpu_gettotalcycles(void);
UINT64 activecpu_gettotalcycles64(void);

/* Returns the total number of CPU cycles for a given CPU */
UINT32 cpunum_gettotalcycles(int cpunum);
UINT64 cpunum_gettotalcycles64(int cpunum);

/* Safely eats cycles so we don't cross a timeslice boundary */
void activecpu_eat_cycles(int cycles);

/* Scales a given value by the ratio of fcount / fperiod */
int cpu_scalebyfcount(int value);



/*************************************
 *
 *  Video timing
 *
 *************************************/

/***** OBSOLETE NOTICE: these functions are no longer considered */
/***** to be the authority on scanline timing. Please use the */
/***** video_screen_* functions in video.c for newer drivers. */
/***** These functions may eventually go away. */

/* Recomputes the VBLANK timing after, e.g., a visible area change */
void cpu_compute_vblank_timing(void);

/* Returns the number of the video frame we are currently playing */
int cpu_getcurrentframe(void);



/*************************************
 *
 *  Synchronization
 *
 *************************************/

/* generate a trigger now */
void cpu_trigger(int trigger);

/* generate a trigger after a specific period of time */
void cpu_triggertime(attotime duration, int trigger);

/* generate a trigger corresponding to an interrupt on the given CPU */
void cpu_triggerint(int cpunum);

/* burn CPU cycles until a timer trigger */
void cpu_spinuntil_trigger(int trigger);

/* burn specified CPU cycles until a timer trigger */
void cpunum_spinuntil_trigger(int cpunum, int trigger);

/* burn CPU cycles until the next interrupt */
void cpu_spinuntil_int(void);

/* burn CPU cycles until our timeslice is up */
void cpu_spin(void);

/* yield our current timeslice */
void cpu_yield(void);

/* burn CPU cycles for a specific period of time */
void cpu_spinuntil_time(attotime duration);



/*************************************
 *
 *  Core timing
 *
 *************************************/

/* Returns the number of times the interrupt handler will be called before
   the end of the current video frame. This is can be useful to interrupt
   handlers to synchronize their operation. If you call this from outside
   an interrupt handler, add 1 to the result, i.e. if it returns 0, it means
   that the interrupt handler will be called once. */
int cpu_getiloops(void);


#endif	/* __CPUEXEC_H__ */
