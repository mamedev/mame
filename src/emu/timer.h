/***************************************************************************

    timer.h

    Functions needed to generate timing and synchronization between several
    CPUs.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __TIMER_H__
#define __TIMER_H__

#include "mamecore.h"
#include "devintrf.h"
#include "attotime.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* timer types */
enum
{
	TIMER_TYPE_PERIODIC = 0
};


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
#define PERIOD_OF_555_MONOSTABLE_NSEC(r,c)	((attoseconds_t)(1100000000 * (double)(r) * (double)(c)))
#define PERIOD_OF_555_ASTABLE_NSEC(r1,r2,c)	((attoseconds_t)( 693000000 * ((double)(r1) + 2.0 * (double)(r2)) * (double)(c)))
#define PERIOD_OF_555_MONOSTABLE(r,c)		ATTOTIME_IN_NSEC(PERIOD_OF_555_MONOSTABLE_NSEC(r,c))
#define PERIOD_OF_555_ASTABLE(r1,r2,c)		ATTOTIME_IN_NSEC(PERIOD_OF_555_ASTABLE_NSEC(r1,r2,c))

/* macros that map all allocations to provide file/line/functions to the callee */
#define timer_alloc(c,ptr)				_timer_alloc_internal(c, ptr, __FILE__, __LINE__, #c)
#define timer_pulse(e,ptr,p,c)			_timer_pulse_internal(e, ptr, p, c, __FILE__, __LINE__, #c)
#define timer_set(d,ptr,p,c)			_timer_set_internal(d, ptr, p, c, __FILE__, __LINE__, #c)
#define timer_call_after_resynch(ptr,p,c) _timer_set_internal(attotime_zero, ptr, p, c, __FILE__, __LINE__, #c)

/* macros for a timer callback functions */
#define TIMER_CALLBACK(name)			void name(running_machine *machine, void *ptr, int param)
#define TIMER_DEVICE_CALLBACK(name)		void name(const device_config *timer, void *ptr, INT32 param)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* a timer callback looks like this */
typedef void (*timer_fired_func)(running_machine *machine, void *ptr, INT32 param);
typedef void (*timer_device_fired_func)(const device_config *timer, void *ptr, INT32 param);


/*-------------------------------------------------
    timer_config - configuration of a single
    timer
-------------------------------------------------*/

typedef struct _timer_config timer_config;
struct _timer_config
{
	int						type;			/* type of timer */
	timer_device_fired_func	callback;		/* the timer's callback function */
	UINT64					duration;		/* duration before the timer fires */
	UINT64					period;			/* period of repeated timer firings */
	INT32					param;			/* the integer parameter passed to the timer callback */
	void 					*ptr;			/* the pointer parameter passed to the timer callback */
};


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
emu_timer *_timer_alloc_internal(timer_fired_func callback, void *param, const char *file, int line, const char *func);

/* adjust the time when this timer will fire and disable any periodic firings */
void timer_adjust_oneshot(emu_timer *which, attotime duration, INT32 param);
void timer_device_adjust_oneshot(const device_config *timer, attotime duration, INT32 param);

/* adjust the time when this timer will fire and specify a period for subsequent firings */
void timer_adjust_periodic(emu_timer *which, attotime duration, INT32 param, attotime period);
void timer_device_adjust_periodic(const device_config *timer, attotime duration, INT32 param, attotime period);



/* ----- anonymous timer management ----- */

/* allocate a one-shot timer, which calls the callback after the given duration */
void _timer_set_internal(attotime druation, void *ptr, INT32 param, timer_fired_func callback, const char *file, int line, const char *func);

/* allocate a pulse timer, which repeatedly calls the callback using the given period */
void _timer_pulse_internal(attotime period, void *ptr, INT32 param, timer_fired_func callback, const char *file, int line, const char *func);



/* ----- miscellaneous controls ----- */

/* reset the timing on a timer */
void timer_reset(emu_timer *which, attotime duration);
void timer_device_reset(const device_config *timer, attotime duration);

/* enable/disable a timer */
int timer_enable(emu_timer *which, int enable);
int timer_device_enable(const device_config *timer, int enable);

/* determine if a timer is enabled */
int timer_enabled(emu_timer *which);
int timer_device_enabled(const device_config *timer);

/* returns the callback parameter of a timer */
int timer_get_param(emu_timer *which);
int timer_device_get_param(const device_config *timer);

void *timer_get_param_ptr(emu_timer *which);
void *timer_device_get_param_ptr(const device_config *timer);



/* ----- timing functions ----- */

/* return the time since the last trigger */
attotime timer_timeelapsed(emu_timer *which);
attotime timer_device_timeelapsed(const device_config *timer);

/* return the time until the next trigger */
attotime timer_timeleft(emu_timer *which);
attotime timer_device_timeleft(const device_config *timer);

/* return the current time */
attotime timer_get_time(void);

/* return the time when this timer started counting */
attotime timer_starttime(emu_timer *which);
attotime timer_device_starttime(const device_config *timer);

/* return the time when this timer will fire next */
attotime timer_firetime(emu_timer *which);
attotime timer_device_firetime(const device_config *timer);


/* ----- timer device interface ----- */

/* device get info callback */
#define TIMER DEVICE_GET_INFO_NAME(timer)
DEVICE_GET_INFO( timer );


#endif	/* __TIMER_H__ */
