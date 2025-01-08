// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    timer.h

    Timer devices.

***************************************************************************/
#ifndef MAME_MACHINE_TIMER_H
#define MAME_MACHINE_TIMER_H

#pragma once

#include "screen.h"


//**************************************************************************
//  MACROS
//**************************************************************************

// macros for a timer callback functions
#define TIMER_DEVICE_CALLBACK_MEMBER(name)  void name(timer_device &timer, s32 param)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> timer_device

class timer_device : public device_t
{
public:
	// a timer callbacks look like this
	typedef device_delegate<void (timer_device &, s32)> expired_delegate;

	// construction/destruction
	timer_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// inline configuration helpers
	template <typename... T> void configure_generic(T &&... args)
	{
		m_type = TIMER_TYPE_GENERIC;
		m_callback.set(std::forward<T>(args)...);
	}

	template <typename F> void configure_periodic(F &&callback, const char *name, const attotime &period)
	{
		m_type = TIMER_TYPE_PERIODIC;
		m_callback.set(std::forward<F>(callback), name);
		m_period = period;
	}
	template <typename T, typename F> void configure_periodic(T &&target, F &&callback, const char *name, const attotime &period)
	{
		m_type = TIMER_TYPE_PERIODIC;
		m_callback.set(std::forward<T>(target), std::forward<F>(callback), name);
		m_period = period;
	}

	template <typename F, typename U> void configure_scanline(F &&callback, const char *name, U &&screen, int first_vpos, int increment)
	{
		m_type = TIMER_TYPE_SCANLINE;
		m_callback.set(std::forward<F>(callback), name);
		m_screen.set_tag(std::forward<U>(screen));
		m_first_vpos = first_vpos;
		m_increment = increment;
	}
	template <typename T, typename F, typename U> void configure_scanline(T &&target, F &&callback, const char *name, U &&screen, int first_vpos, int increment)
	{
		m_type = TIMER_TYPE_SCANLINE;
		m_callback.set(std::forward<T>(target), std::forward<F>(callback), name);
		m_screen.set_tag(std::forward<U>(screen));
		m_first_vpos = first_vpos;
		m_increment = increment;
	}

	template <typename... T> void set_callback(T &&... args) { m_callback.set(std::forward<T>(args)...); }

	void set_start_delay(const attotime &delay) { m_start_delay = delay; }
	void config_param(s32 param) { m_param = param; }

	// property getters
	s32 param() const { return m_timer->param(); }
	bool enabled() const { return m_timer->enabled(); }

	// property setters
	void set_param(s32 param) const { if(m_type != TIMER_TYPE_GENERIC) fatalerror("Cannot change parameter on a non-generic timer.\n"); m_timer->set_param(param); }
	void enable(bool enable = true) const { m_timer->enable(enable); }

	// adjustments
	void reset() { adjust(attotime::never, 0, attotime::never); }
	void adjust(const attotime &duration, s32 param = 0, const attotime &period = attotime::never) const
	{
		if(m_type != TIMER_TYPE_GENERIC)
			fatalerror("Cannot adjust a non-generic timer.\n");
		m_timer->adjust(duration, param, period);
	}

	// timing information
	attotime elapsed() const { return m_timer->elapsed(); }
	attotime remaining() const { return m_timer->remaining(); }
	attotime start() const { return m_timer->start(); }
	attotime expire() const { return m_timer->expire(); }
	attotime period() const { return m_timer ? m_timer->period() : m_period; }

private:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(generic_tick);
	TIMER_CALLBACK_MEMBER(scanline_tick);

	// timer types
	enum timer_type
	{
		TIMER_TYPE_PERIODIC,
		TIMER_TYPE_SCANLINE,
		TIMER_TYPE_GENERIC
	};

	// configuration data
	timer_type              m_type;             // type of timer
	expired_delegate        m_callback;         // the timer's callback function

	// periodic timers only
	attotime                m_start_delay;      // delay before the timer fires for the first time
	attotime                m_period;           // period of repeated timer firings
	s32                     m_param;            // the integer parameter passed to the timer callback

	// scanline timers only
	optional_device<screen_device> m_screen;    // pointer to the screen device
	u32                     m_first_vpos;       // the first vertical scanline position the timer fires on
	u32                     m_increment;        // the number of scanlines between firings

	// internal state
	emu_timer *             m_timer;            // the backing timer
	bool                    m_first_time;       // indicates that the system is starting (scanline timers only)
};



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DECLARE_DEVICE_TYPE(TIMER, timer_device)


#endif // MAME_MACHINE_TIMER_H
