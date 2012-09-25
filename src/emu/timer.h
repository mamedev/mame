/***************************************************************************

    timer.h

    Timer devices.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __TIMER_H__
#define __TIMER_H__



//**************************************************************************
//  MACROS
//**************************************************************************

// macros for a timer callback functions
#define TIMER_DEVICE_CALLBACK(name)		void name(device_t *, timer_device &timer, void *ptr, INT32 param)
#define TIMER_DEVICE_CALLBACK_MEMBER(name)	void name(timer_device &timer, void *ptr, INT32 param)

//**************************************************************************
//  TIMER DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_TIMER_ADD(_tag, _callback) \
	MCFG_DEVICE_ADD(_tag, TIMER, 0) \
	timer_device::static_configure_generic(*device, timer_device_expired_delegate(&_callback, #_callback)); \

#define MCFG_TIMER_ADD_NONE(_tag) \
	MCFG_DEVICE_ADD(_tag, TIMER, 0) \
	timer_device::static_configure_generic(*device, timer_device_expired_delegate()); \

#define MCFG_TIMER_ADD_PERIODIC(_tag, _callback, _period) \
	MCFG_DEVICE_ADD(_tag, TIMER, 0) \
	timer_device::static_configure_periodic(*device, timer_device_expired_delegate(&_callback, #_callback), _period); \

#define MCFG_TIMER_ADD_SCANLINE(_tag, _callback, _screen, _first_vpos, _increment) \
	MCFG_DEVICE_ADD(_tag, TIMER, 0) \
	timer_device::static_configure_scanline(*device, timer_device_expired_delegate(&_callback, #_callback), _screen, _first_vpos, _increment); \

#define MCFG_TIMER_DRIVER_ADD(_tag, _class, _callback) \
	MCFG_DEVICE_ADD(_tag, TIMER, 0) \
	timer_device::static_configure_generic(*device, timer_device_expired_delegate(&_class::_callback, #_class "::" #_callback, NULL, (_class *)0)); \

#define MCFG_TIMER_DRIVER_ADD_PERIODIC(_tag, _class, _callback, _period) \
	MCFG_DEVICE_ADD(_tag, TIMER, 0) \
	timer_device::static_configure_periodic(*device, timer_device_expired_delegate(&_class::_callback, #_class "::" #_callback, NULL, (_class *)0), _period); \

#define MCFG_TIMER_DRIVER_ADD_SCANLINE(_tag, _class, _callback, _screen, _first_vpos, _increment) \
	MCFG_DEVICE_ADD(_tag, TIMER, 0) \
	timer_device::static_configure_scanline(*device, timer_device_expired_delegate(&_class::_callback, #_class "::" #_callback, NULL, (_class *)0), _screen, _first_vpos, _increment); \

#define MCFG_TIMER_MODIFY(_tag) \
	MCFG_DEVICE_MODIFY(_tag)

#define MCFG_TIMER_CALLBACK(_callback) \
	timer_device::static_set_callback(*device, timer_device_expired_delegate(&_callback, #_callback)); \

#define MCFG_TIMER_DRIVER_CALLBACK(_class, _callback) \
	timer_device::static_set_callback(*device, timer_device_expired_delegate(&_class::_callback, #_class "::" #_callback, NULL, (_class *)0)); \

#define MCFG_TIMER_START_DELAY(_start_delay) \
	timer_device::static_set_start_delay(*device, _start_delay); \

#define MCFG_TIMER_PARAM(_param) \
	timer_device::static_set_param(*device, _param); \

#define MCFG_TIMER_PTR(_ptr) \
	timer_device::static_set_ptr(*device, (void *)(_ptr)); \



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declarations
class emu_timer;
class timer_device;

// a timer callbacks look like this
typedef device_delegate<void (timer_device &, void *, INT32)> timer_device_expired_delegate;

// ======================> timer_device

class timer_device : public device_t
{
public:
	// construction/destruction
	timer_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// inline configuration helpers
	static void static_configure_generic(device_t &device, timer_device_expired_delegate callback);
	static void static_configure_periodic(device_t &device, timer_device_expired_delegate callback, attotime period);
	static void static_configure_scanline(device_t &device, timer_device_expired_delegate callback, const char *screen, int first_vpos, int increment);
	static void static_set_callback(device_t &device, timer_device_expired_delegate callback);
	static void static_set_start_delay(device_t &device, attotime delay);
	static void static_set_param(device_t &device, int param);
	static void static_set_ptr(device_t &device, void *ptr);

	// property getters
	int param() const { return m_timer->param(); }
	void *ptr() const { return m_ptr; }
	bool enabled() const { return m_timer->enabled(); }

	// property setters
	void set_param(int param) { assert(m_type == TIMER_TYPE_GENERIC); m_timer->set_param(param); }
	void set_ptr(void *ptr) { m_ptr = ptr; }
	void enable(bool enable = true) { m_timer->enable(enable); }

	// adjustments
	void reset() { adjust(attotime::never, 0, attotime::never); }
	void adjust(attotime duration, INT32 param = 0, attotime period = attotime::never) { assert(m_type == TIMER_TYPE_GENERIC); m_timer->adjust(duration, param, period); }

	// timing information
	attotime time_elapsed() const { return m_timer->elapsed(); }
	attotime time_left() const { return m_timer->remaining(); }
	attotime start_time() const { return m_timer->start(); }
	attotime fire_time() const { return m_timer->expire(); }

private:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// timer types
	enum timer_type
	{
		TIMER_TYPE_PERIODIC,
		TIMER_TYPE_SCANLINE,
		TIMER_TYPE_GENERIC
	};

	// configuration data
	timer_type				m_type;				// type of timer
	timer_device_expired_delegate	m_callback;			// the timer's callback function
	void *					m_ptr;				// the pointer parameter passed to the timer callback

	// periodic timers only
	attotime				m_start_delay;		// delay before the timer fires for the first time
	attotime				m_period;			// period of repeated timer firings
	INT32					m_param;			// the integer parameter passed to the timer callback

	// scanline timers only
	const char *			m_screen_tag;		// the tag of the screen this timer tracks
	screen_device *			m_screen;			// pointer to the screen device
	UINT32					m_first_vpos;		// the first vertical scanline position the timer fires on
	UINT32					m_increment;		// the number of scanlines between firings

	// internal state
	emu_timer *				m_timer;			// the backing timer
	bool					m_first_time;		// indicates that the system is starting (scanline timers only)
};



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

extern const device_type TIMER;


#endif	/* __TIMER_H__ */
