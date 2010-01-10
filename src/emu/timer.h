/***************************************************************************

    timer.h

    Functions needed to generate timing and synchronization between several
    CPUs.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __TIMER_H__
#define __TIMER_H__


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* timer types */
enum
{
	TIMER_TYPE_PERIODIC = 0,
	TIMER_TYPE_SCANLINE,
	TIMER_TYPE_GENERIC
};



/***************************************************************************
    MACROS
***************************************************************************/

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
#define timer_alloc(m,c,ptr)			_timer_alloc_internal(m, c, ptr, __FILE__, __LINE__, #c)
#define timer_pulse(m,e,ptr,p,c)		_timer_pulse_internal(m, e, ptr, p, c, __FILE__, __LINE__, #c)
#define timer_set(m,d,ptr,p,c)			_timer_set_internal(m, d, ptr, p, c, __FILE__, __LINE__, #c)
#define timer_call_after_resynch(m,ptr,p,c) _timer_set_internal(m, attotime_zero, ptr, p, c, __FILE__, __LINE__, #c)

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
	void					*ptr;			/* the pointer parameter passed to the timer callback */

	/* periodic timers only */
	UINT64					start_delay;	/* delay before the timer fires for the first time */
	UINT64					period;			/* period of repeated timer firings */
	INT32					param;			/* the integer parameter passed to the timer callback */

	/* scanline timers only */
	const char				*screen;		/* the name of the screen this timer tracks */
	UINT32					first_vpos;		/* the first vertical scanline position the timer fires on */
	UINT32					increment;		/* the number of scanlines between firings */
};


/* opaque type for representing a timer */
typedef struct _emu_timer emu_timer;


typedef struct _timer_execution_state timer_execution_state;
struct _timer_execution_state
{
	attotime				nextfire;		/* time that the head of the timer list will fire */
	attotime				basetime;		/* global basetime; everything moves forward from here */
	attoseconds_t			curquantum;		/* current quantum of execution */
};



/***************************************************************************
    TIMER DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_TIMER_ADD(_tag, _callback) \
	MDRV_DEVICE_ADD(_tag, TIMER, 0) \
	MDRV_DEVICE_CONFIG_DATA32(timer_config, type, TIMER_TYPE_GENERIC) \
	MDRV_DEVICE_CONFIG_DATAPTR(timer_config, callback, _callback) \

#define MDRV_TIMER_ADD_PERIODIC(_tag, _callback, _period) \
	MDRV_DEVICE_ADD(_tag, TIMER, 0) \
	MDRV_DEVICE_CONFIG_DATA32(timer_config, type, TIMER_TYPE_PERIODIC) \
	MDRV_DEVICE_CONFIG_DATAPTR(timer_config, callback, _callback) \
	MDRV_DEVICE_CONFIG_DATA64(timer_config, period, UINT64_ATTOTIME_IN_##_period)

#define MDRV_TIMER_ADD_SCANLINE(_tag, _callback, _screen, _first_vpos, _increment) \
	MDRV_DEVICE_ADD(_tag, TIMER, 0) \
	MDRV_DEVICE_CONFIG_DATA32(timer_config, type, TIMER_TYPE_SCANLINE) \
	MDRV_DEVICE_CONFIG_DATAPTR(timer_config, callback, _callback) \
	MDRV_DEVICE_CONFIG_DATAPTR(timer_config, screen, _screen) \
	MDRV_DEVICE_CONFIG_DATA32(timer_config, first_vpos, _first_vpos) \
	MDRV_DEVICE_CONFIG_DATA32(timer_config, increment, _increment)

#define MDRV_TIMER_MODIFY(_tag) \
	MDRV_DEVICE_MODIFY(_tag)

#define MDRV_TIMER_CALLBACK(_callback) \
	MDRV_DEVICE_CONFIG_DATAPTR(timer_config, callback, _callback)

#define MDRV_TIMER_START_DELAY(_start_delay) \
	MDRV_DEVICE_CONFIG_DATA64(timer_config, start_delay, UINT64_ATTOTIME_IN_##_start_delay)

#define MDRV_TIMER_PARAM(_param) \
	MDRV_DEVICE_CONFIG_DATA32(timer_config, param, _param)

#define MDRV_TIMER_PTR(_ptr) \
	MDRV_DEVICE_CONFIG_DATAPTR(timer_config, ptr, _ptr)



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- initialization ----- */

/* initialize the timer system */
void timer_init(running_machine *machine);

/* destruct a timer from a pool callback */
void timer_destructor(void *ptr, size_t size);



/* ----- scheduling helpers ----- */

/* return a pointer to the execution state */
timer_execution_state *timer_get_execution_state(running_machine *machine);

/* execute timers and update scheduling quanta */
void timer_execute_timers(running_machine *machine);

/* add a scheduling quantum; the smallest active one is the one that is in use */
void timer_add_scheduling_quantum(running_machine *machine, attoseconds_t quantum, attotime duration);

/* control the minimum useful quantum (used by cpuexec only) */
void timer_set_minimum_quantum(running_machine *machine, attoseconds_t quantum);



/* ----- save/restore helpers ----- */

/* count the number of anonymous (non-saveable) timers */
int timer_count_anonymous(running_machine *machine);



/* ----- core timer management ----- */

/* allocate a permament timer that isn't primed yet */
emu_timer *_timer_alloc_internal(running_machine *machine, timer_fired_func callback, void *param, const char *file, int line, const char *func);

/* adjust the time when this timer will fire and disable any periodic firings */
void timer_adjust_oneshot(emu_timer *which, attotime duration, INT32 param);
void timer_device_adjust_oneshot(const device_config *timer, attotime duration, INT32 param);

/* adjust the time when this timer will fire and specify a period for subsequent firings */
void timer_adjust_periodic(emu_timer *which, attotime start_delay, INT32 param, attotime period);
void timer_device_adjust_periodic(const device_config *timer, attotime start_delay, INT32 param, attotime period);



/* ----- anonymous timer management ----- */

/* allocate a one-shot timer, which calls the callback after the given duration */
void _timer_set_internal(running_machine *machine, attotime duration, void *ptr, INT32 param, timer_fired_func callback, const char *file, int line, const char *func);

/* allocate a pulse timer, which repeatedly calls the callback using the given period */
void _timer_pulse_internal(running_machine *machine, attotime period, void *ptr, INT32 param, timer_fired_func callback, const char *file, int line, const char *func);



/* ----- miscellaneous controls ----- */

/* reset the timing on a timer */
void timer_reset(emu_timer *which, attotime duration);
void timer_device_reset(const device_config *timer);

/* enable/disable a timer */
int timer_enable(emu_timer *which, int enable);
int timer_device_enable(const device_config *timer, int enable);

/* determine if a timer is enabled */
int timer_enabled(emu_timer *which);
int timer_device_enabled(const device_config *timer);

/* returns the callback parameter of a timer */
int timer_get_param(emu_timer *which);
int timer_device_get_param(const device_config *timer);

/* changes the callback parameter of a timer */
void timer_set_param(emu_timer *which, int param);
void timer_device_set_param(const device_config *timer, int param);

/* returns the callback pointer of a timer */
void *timer_get_ptr(emu_timer *which);
void *timer_device_get_ptr(const device_config *timer);

/* changes the callback pointer of a timer */
void timer_set_ptr(emu_timer *which, void *ptr);
void timer_device_set_ptr(const device_config *timer, void *ptr);



/* ----- timing functions ----- */

/* return the time since the last trigger */
attotime timer_timeelapsed(emu_timer *which);
attotime timer_device_timeelapsed(const device_config *timer);

/* return the time until the next trigger */
attotime timer_timeleft(emu_timer *which);
attotime timer_device_timeleft(const device_config *timer);

/* return the current time */
attotime timer_get_time(running_machine *machine);

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
