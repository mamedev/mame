/***************************************************************************

    timer.h

    Functions needed to generate timing and synchronization between several
    CPUs.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __TIMER_H__
#define __TIMER_H__

#include "mamecore.h"
#include "attotime.h"


/***************************************************************************
    MACROS
***************************************************************************/

/* convert cycles on a given CPU to/from attotime */
#define ATTOTIME_TO_CYCLES(cpu,t)		((t).seconds * cycles_per_second[cpu] + (t).attoseconds / attoseconds_per_cycle[cpu])
#define ATTOTIME_IN_CYCLES(c,cpu)		(attotime_make((c) / cycles_per_second[cpu], (c) * attoseconds_per_cycle[cpu]))

/* macro for the RC time constant on a 74LS123 with C > 1000pF */
/* R is in ohms, C is in farads */
#define TIME_OF_74LS123(r,c)			(0.45 * (double)(r) * (double)(c))

/* macros for the RC time constant on a 555 timer IC */
/* R is in ohms, C is in farads */
#define PERIOD_OF_555_MONOSTABLE(r,c)	ATTOTIME_IN_NSEC((attoseconds_t)(1100000000 * (double)(r) * (double)(c)))
#define PERIOD_OF_555_ASTABLE(r1,r2,c)	ATTOTIME_IN_NSEC((attoseconds_t)( 693000000 * ((double)(r1) + 2.0 * (double)(r2)) * (double)(c)))

/* macros that map all allocations to provide file/line/functions to the callee */
#define timer_alloc(c)					_timer_alloc(c, __FILE__, __LINE__, #c)
#define timer_alloc_ptr(c,p)			_timer_alloc_ptr(c, p, __FILE__, __LINE__, #c)
#define timer_pulse(e,p,c)				_timer_pulse(e, p, c, __FILE__, __LINE__, #c)
#define timer_pulse_ptr(e,p,c)			_timer_pulse_ptr(e, p, c, __FILE__, __LINE__, #c)
#define timer_set(d,p,c)				_timer_set(d, p, c, __FILE__, __LINE__, #c)
#define timer_set_ptr(d,p,c)			_timer_set_ptr(d, p, c, __FILE__, __LINE__, #c)

/* macros for a timer callback functions */
#define TIMER_CALLBACK(name)			void name(running_machine *machine, int param)
#define TIMER_CALLBACK_PTR(name)		void name(running_machine *machine, void *param)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* opaque type for representing a timer */
typedef struct _emu_timer emu_timer;



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* arrays containing mappings between CPU cycle times and timer values */
extern attoseconds_t attoseconds_per_cycle[];
extern UINT32 cycles_per_second[];



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- initialization ----- */

/* initialize the timer system */
void timer_init(running_machine *machine);

/* destruct a timer from a pool callback */
void timer_destructor(void *ptr, size_t size);



/* ----- scheduling helpers ----- */

/* return the time when the next timer will fire */
attotime timer_next_fire_time(void);

/* adjust the global time; this is also where we fire the timers */
void timer_set_global_time(attotime newbase);



/* ----- save/restore helpers ----- */

/* count the number of anonymous (non-saveable) timers */
int timer_count_anonymous(void);



/* ----- core timer management ----- */

/* allocate a permament timer that isn't primed yet */
emu_timer *_timer_alloc(void (*callback)(running_machine *, int), const char *file, int line, const char *func);
emu_timer *_timer_alloc_ptr(void (*callback)(running_machine *, void *), void *param, const char *file, int line, const char *func);

/* adjust the time when this timer will fire, and whether or not it will fire periodically */
void timer_adjust(emu_timer *which, attotime duration, INT32 param, attotime period);
void timer_adjust_ptr(emu_timer *which, attotime duration, attotime period);



/* ----- anonymous timer management ----- */

/* allocate a pulse timer, which repeatedly calls the callback using the given period */
void _timer_pulse(attotime period, INT32 param, void (*callback)(running_machine *, int), const char *file, int line, const char *func);
void _timer_pulse_ptr(attotime period, void *param, void (*callback)(running_machine *, void *), const char *file, int line, const char *func);

/* allocate a one-shot timer, which calls the callback after the given duration */
void _timer_set(attotime duration, INT32 param, void (*callback)(running_machine *, int), const char *file, int line, const char *func);
void _timer_set_ptr(attotime duration, void *param, void (*callback)(running_machine *, void *), const char *file, int line, const char *func);



/* ----- miscellaneous controls ----- */

/* reset the timing on a timer */
void timer_reset(emu_timer *which, attotime duration);

/* enable/disable a timer */
int timer_enable(emu_timer *which, int enable);

/* determine if a timer is enabled */
int timer_enabled(emu_timer *which);

/* returns the callback parameter of a timer */
int timer_get_param(emu_timer *which);
void *timer_get_param_ptr(emu_timer *which);



/* ----- timing functions ----- */

/* return the time since the last trigger */
attotime timer_timeelapsed(emu_timer *which);

/* return the time until the next trigger */
attotime timer_timeleft(emu_timer *which);

/* return the current time */
attotime timer_get_time(void);

/* return the time when this timer started counting */
attotime timer_starttime(emu_timer *which);

/* return the time when this timer will fire next */
attotime timer_firetime(emu_timer *which);




/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    timer_call_after_resynch - synchs the CPUs
    and calls the callback immediately
-------------------------------------------------*/

INLINE void timer_call_after_resynch(INT32 param, void (*callback)(running_machine *, int))
{
	timer_set(attotime_zero, param, callback);
}

INLINE void timer_call_after_resynch_ptr(void *param, void (*callback)(running_machine *, void *))
{
	timer_set_ptr(attotime_zero, param, callback);
}


#endif	/* __TIMER_H__ */
