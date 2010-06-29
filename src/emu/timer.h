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

#include "devlegcy.h"


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
#define TIMER_DEVICE_CALLBACK(name)		void name(timer_device &timer, void *ptr, INT32 param)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// forward declarations
class emu_timer;
class timer_device;


// a timer callback looks like this
typedef void (*timer_fired_func)(running_machine *machine, void *ptr, INT32 param);
typedef void (*timer_device_fired_func)(timer_device &timer, void *ptr, INT32 param);




struct timer_execution_state
{
	attotime				nextfire;		/* time that the head of the timer list will fire */
	attotime				basetime;		/* global basetime; everything moves forward from here */
	attoseconds_t			curquantum;		/* current quantum of execution */
};



//**************************************************************************
//  TIMER DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MDRV_TIMER_ADD(_tag, _callback) \
	MDRV_DEVICE_ADD(_tag, TIMER, 0) \
	MDRV_DEVICE_INLINE_DATA16(timer_device_config::INLINE_TYPE, timer_device_config::TIMER_TYPE_GENERIC) \
	MDRV_DEVICE_INLINE_DATAPTR(timer_device_config::INLINE_CALLBACK, _callback)

#define MDRV_TIMER_ADD_PERIODIC(_tag, _callback, _period) \
	MDRV_DEVICE_ADD(_tag, TIMER, 0) \
	MDRV_DEVICE_INLINE_DATA16(timer_device_config::INLINE_TYPE, timer_device_config::TIMER_TYPE_PERIODIC) \
	MDRV_DEVICE_INLINE_DATAPTR(timer_device_config::INLINE_CALLBACK, _callback) \
	MDRV_DEVICE_INLINE_DATA64(timer_device_config::INLINE_PERIOD, UINT64_ATTOTIME_IN_##_period)

#define MDRV_TIMER_ADD_SCANLINE(_tag, _callback, _screen, _first_vpos, _increment) \
	MDRV_DEVICE_ADD(_tag, TIMER, 0) \
	MDRV_DEVICE_INLINE_DATA16(timer_device_config::INLINE_TYPE, timer_device_config::TIMER_TYPE_SCANLINE) \
	MDRV_DEVICE_INLINE_DATAPTR(timer_device_config::INLINE_CALLBACK, _callback) \
	MDRV_DEVICE_INLINE_DATAPTR(timer_device_config::INLINE_SCREEN, _screen) \
	MDRV_DEVICE_INLINE_DATA16(timer_device_config::INLINE_FIRST_VPOS, _first_vpos) \
	MDRV_DEVICE_INLINE_DATA16(timer_device_config::INLINE_INCREMENT, _increment)

#define MDRV_TIMER_MODIFY(_tag) \
	MDRV_DEVICE_MODIFY(_tag)

#define MDRV_TIMER_CALLBACK(_callback) \
	MDRV_DEVICE_INLINE_DATA32(timer_device_config::INLINE_CALLBACK, _callback)

#define MDRV_TIMER_START_DELAY(_start_delay) \
	MDRV_DEVICE_INLINE_DATA64(timer_device_config::INLINE_DELAY, UINT64_ATTOTIME_IN_##_start_delay)

#define MDRV_TIMER_PARAM(_param) \
	MDRV_DEVICE_INLINE_DATA32(timer_device_config::INLINE_PARAM, _param)

#define MDRV_TIMER_PTR(_ptr) \
	MDRV_DEVICE_INLINE_DATAPTR(timer_device_config::INLINE_PTR, _ptr)



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

/* adjust the time when this timer will fire and specify a period for subsequent firings */
void timer_adjust_periodic(emu_timer *which, attotime start_delay, INT32 param, attotime period);



/* ----- anonymous timer management ----- */

/* allocate a one-shot timer, which calls the callback after the given duration */
void _timer_set_internal(running_machine *machine, attotime duration, void *ptr, INT32 param, timer_fired_func callback, const char *file, int line, const char *func);

/* allocate a pulse timer, which repeatedly calls the callback using the given period */
void _timer_pulse_internal(running_machine *machine, attotime period, void *ptr, INT32 param, timer_fired_func callback, const char *file, int line, const char *func);



/* ----- miscellaneous controls ----- */

/* reset the timing on a timer */
void timer_reset(emu_timer *which, attotime duration);

/* enable/disable a timer */
int timer_enable(emu_timer *which, int enable);

/* determine if a timer is enabled */
int timer_enabled(emu_timer *which);

/* returns the callback parameter of a timer */
int timer_get_param(emu_timer *which);

/* changes the callback parameter of a timer */
void timer_set_param(emu_timer *which, int param);

/* returns the callback pointer of a timer */
void *timer_get_ptr(emu_timer *which);

/* changes the callback pointer of a timer */
void timer_set_ptr(emu_timer *which, void *ptr);



/* ----- timing functions ----- */

/* return the time since the last trigger */
attotime timer_timeelapsed(emu_timer *which);

/* return the time until the next trigger */
attotime timer_timeleft(emu_timer *which);

/* return the current time */
attotime timer_get_time(running_machine *machine);

/* return the time when this timer started counting */
attotime timer_starttime(emu_timer *which);

/* return the time when this timer will fire next */
attotime timer_firetime(emu_timer *which);



// ======================> timer_device_config

class timer_device_config : public device_config
{
	friend class timer_device;

	// construction/destruction
	timer_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
	// allocators
	static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
	virtual device_t *alloc_device(running_machine &machine) const;

	// indexes to inline data
	enum
	{
		INLINE_TYPE,
		INLINE_CALLBACK,
		INLINE_PERIOD,
		INLINE_SCREEN,
		INLINE_FIRST_VPOS,
		INLINE_INCREMENT,
		INLINE_DELAY,
		INLINE_PARAM,
		INLINE_PTR
	};

	// timer types
	enum timer_type
	{
		TIMER_TYPE_PERIODIC,
		TIMER_TYPE_SCANLINE,
		TIMER_TYPE_GENERIC
	};

private:
	// device_config overrides
	virtual void device_config_complete();
	virtual bool device_validity_check(const game_driver &driver) const;

	// configuration data
	timer_type				m_type;				// type of timer
	timer_device_fired_func	m_callback;			// the timer's callback function
	void *					m_ptr;				// the pointer parameter passed to the timer callback

	// periodic timers only
	UINT64					m_start_delay;		// delay before the timer fires for the first time
	UINT64					m_period;			// period of repeated timer firings
	INT32					m_param;			// the integer parameter passed to the timer callback

	// scanline timers only
	const char *			m_screen;			// the name of the screen this timer tracks
	UINT32					m_first_vpos;		// the first vertical scanline position the timer fires on
	UINT32					m_increment;		// the number of scanlines between firings
};



// ======================> timer_device

class timer_device : public device_t
{
	friend class timer_device_config;

	// construction/destruction
	timer_device(running_machine &_machine, const timer_device_config &config);

public:
	// property getters
	int param() const { return timer_get_param(m_timer); }
	void *ptr() const { return m_ptr; }
	bool enabled() const { return (timer_enabled(m_timer) != 0); }

	// property setters
	void set_param(int param) { assert(m_config.m_type == timer_device_config::TIMER_TYPE_GENERIC); timer_set_param(m_timer, param); }
	void set_ptr(void *ptr) { m_ptr = ptr; }
	void enable(bool enable = true) { timer_enable(m_timer, enable); }

	// adjustments
	void reset() { adjust(attotime_never, 0, attotime_never); }
	void adjust(attotime duration, INT32 param = 0, attotime period = attotime_never) { assert(m_config.m_type == timer_device_config::TIMER_TYPE_GENERIC); timer_adjust_periodic(m_timer, duration, param, period); }

	// timing information
	attotime time_elapsed() const { return timer_timeelapsed(m_timer); }
	attotime time_left() const { return timer_timeleft(m_timer); }
	attotime start_time() const { return timer_starttime(m_timer); }
	attotime fire_time() const { return timer_firetime(m_timer); }

private:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// internal helpers
	static TIMER_CALLBACK( static_periodic_timer_callback ) { reinterpret_cast<timer_device *>(ptr)->periodic_timer_callback(param); }
	void periodic_timer_callback(int param);

	static TIMER_CALLBACK( static_scanline_timer_callback ) { reinterpret_cast<timer_device *>(ptr)->scanline_timer_callback(param); }
	void scanline_timer_callback(int scanline);

	// internal state
	const timer_device_config &m_config;
	emu_timer *		m_timer;			// the backing timer
	void *			m_ptr;				// the pointer parameter passed to the timer callback

	// scanline timers only
	screen_device *m_screen;		// pointer to the screen
	bool			m_first_time;		// indicates that the system is starting
};


// device type definition
extern const device_type TIMER;


#endif	/* __TIMER_H__ */
