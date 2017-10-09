// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    timer.h

    Timer devices.

***************************************************************************/

#pragma once

#ifndef MAME_MACHINE_TIMER_H
#define MAME_MACHINE_TIMER_H



//**************************************************************************
//  MACROS
//**************************************************************************

// macros for a timer callback functions
#define TIMER_DEVICE_CALLBACK_MEMBER(name)  void name(timer_device &timer, void *ptr, s32 param)

//**************************************************************************
//  TIMER DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_TIMER_ADD_NONE(_tag) \
	MCFG_DEVICE_ADD(_tag, TIMER, 0) \
	timer_device::static_configure_generic(*device, timer_device::expired_delegate());
#define MCFG_TIMER_DRIVER_ADD(_tag, _class, _callback) \
	MCFG_DEVICE_ADD(_tag, TIMER, 0) \
	timer_device::static_configure_generic(*device, timer_device::expired_delegate(&_class::_callback, #_class "::" #_callback, nullptr, (_class *)nullptr));
#define MCFG_TIMER_DEVICE_ADD(_tag, _devtag, _class, _callback) \
	MCFG_DEVICE_ADD(_tag, TIMER, 0) \
	timer_device::static_configure_generic(*device, timer_device::expired_delegate(&_class::_callback, #_class "::" #_callback, _devtag, (_class *)nullptr));
#define MCFG_TIMER_DRIVER_ADD_PERIODIC(_tag, _class, _callback, _period) \
	MCFG_DEVICE_ADD(_tag, TIMER, 0) \
	timer_device::static_configure_periodic(*device, timer_device::expired_delegate(&_class::_callback, #_class "::" #_callback, nullptr, (_class *)nullptr), _period);
#define MCFG_TIMER_DEVICE_ADD_PERIODIC(_tag, _devtag, _class, _callback, _period) \
	MCFG_DEVICE_ADD(_tag, TIMER, 0) \
	timer_device::static_configure_periodic(*device, timer_device::expired_delegate(&_class::_callback, #_class "::" #_callback, _devtag, (_class *)nullptr), _period);
#define MCFG_TIMER_DRIVER_ADD_SCANLINE(_tag, _class, _callback, _screen, _first_vpos, _increment) \
	MCFG_DEVICE_ADD(_tag, TIMER, 0) \
	timer_device::static_configure_scanline(*device, timer_device::expired_delegate(&_class::_callback, #_class "::" #_callback, nullptr, (_class *)nullptr), _screen, _first_vpos, _increment);
#define MCFG_TIMER_DEVICE_ADD_SCANLINE(_tag, _devtag, _class, _callback, _screen, _first_vpos, _increment) \
	MCFG_DEVICE_ADD(_tag, TIMER, 0) \
	timer_device::static_configure_scanline(*device, timer_device::expired_delegate(&_class::_callback, #_class "::" #_callback, _devtag, (_class *)nullptr), _screen, _first_vpos, _increment);
#define MCFG_TIMER_MODIFY(_tag) \
	MCFG_DEVICE_MODIFY(_tag)

#define MCFG_TIMER_DRIVER_CALLBACK(_class, _callback) \
	timer_device::static_set_callback(*device, timer_device::expired_delegate(&_class::_callback, #_class "::" #_callback, nullptr, (_class *)nullptr));
#define MCFG_TIMER_START_DELAY(_start_delay) \
	timer_device::static_set_start_delay(*device, _start_delay);
#define MCFG_TIMER_PARAM(_param) \
	timer_device::static_set_param(*device, _param);
#define MCFG_TIMER_PTR(_ptr) \
	timer_device::static_set_ptr(*device, (void *)(_ptr));


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> timer_device

class timer_device : public device_t
{
public:
	// a timer callbacks look like this
	typedef device_delegate<void (timer_device &, void *, s32)> expired_delegate;

	// construction/destruction
	timer_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// inline configuration helpers
	static void static_configure_generic(device_t &device, expired_delegate callback);
	static void static_configure_periodic(device_t &device, expired_delegate callback, const attotime &period);
	static void static_configure_scanline(device_t &device, expired_delegate callback, const char *screen, int first_vpos, int increment);
	static void static_set_callback(device_t &device, expired_delegate callback);
	static void static_set_start_delay(device_t &device, const attotime &delay);
	static void static_set_param(device_t &device, int param);
	static void static_set_ptr(device_t &device, void *ptr);

	// property getters
	int param() const { return m_timer->param(); }
	void *ptr() const { return m_ptr; }
	bool enabled() const { return m_timer->enabled(); }

	// property setters
	void set_param(int param) const { assert(m_type == TIMER_TYPE_GENERIC); m_timer->set_param(param); }
	void set_ptr(void *ptr) { m_ptr = ptr; }
	void enable(bool enable = true) const { m_timer->enable(enable); }

	// adjustments
	void reset() { adjust(attotime::never, 0, attotime::never); }
	void adjust(const attotime &duration, s32 param = 0, const attotime &period = attotime::never) const { assert(m_type == TIMER_TYPE_GENERIC); m_timer->adjust(duration, param, period); }

	// timing information
	attotime time_elapsed() const { return m_timer->elapsed(); }
	attotime time_left() const { return m_timer->remaining(); }
	attotime start_time() const { return m_timer->start(); }
	attotime fire_time() const { return m_timer->expire(); }

private:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// timer types
	enum timer_type
	{
		TIMER_TYPE_PERIODIC,
		TIMER_TYPE_SCANLINE,
		TIMER_TYPE_GENERIC
	};

	// configuration data
	timer_type              m_type;             // type of timer
	expired_delegate   m_callback;         // the timer's callback function
	void *                  m_ptr;              // the pointer parameter passed to the timer callback

	// periodic timers only
	attotime                m_start_delay;      // delay before the timer fires for the first time
	attotime                m_period;           // period of repeated timer firings
	s32                     m_param;            // the integer parameter passed to the timer callback

	// scanline timers only
	const char *            m_screen_tag;       // the tag of the screen this timer tracks
	screen_device *         m_screen;           // pointer to the screen device
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
