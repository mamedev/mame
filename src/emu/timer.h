// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    timer.h

    Core timer objects and helpers.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef MAME_EMU_TIMER_H
#define MAME_EMU_TIMER_H


//**************************************************************************
//  MACROS
//**************************************************************************

#define TIMER_CALLBACK_MEMBER(name)     void name(void *ptr, s32 param)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// timer callbacks look like this
using timer_expired_delegate = named_delegate<void (void *, s32)>;

// timer IDs for devices
using device_timer_id = u32;


// ======================> emu_timer_cb

class emu_timer_cb
{
	friend class device_scheduler;
	friend class emu_timer;

public:
	// construction/destruction
	emu_timer_cb();

	// registration
	void enregister(device_scheduler &scheduler, char const *unique, timer_expired_delegate callback);
	void enregister(device_t &device, char const *unique, timer_expired_delegate callback);
	void interface_enregister(device_interface &device, char const *unique, timer_expired_delegate callback);

	// direct registration for device's and device interfaces
	template<typename DeviceType, typename FuncType>
	void enregister(DeviceType &device, FuncType callback, char const *string, char const *unique = nullptr)
	{
		enregister(device, unique, timer_expired_delegate(callback, string, &device));
	}
	template<typename IntfType, typename FuncType>
	void interface_enregister(IntfType &intf, FuncType callback, char const *string, char const *unique = nullptr)
	{
		interface_enregister(intf, unique, timer_expired_delegate(callback, string, &intf));
	}

	// call the delegate after a set amount of time, with the given parameter
	void call_after(attotime const &duration, s32 param = 0, u64 param2 = 0, u64 param3 = 0) const;

	// synchronize; essentially, call immediately
	void synchronize(s32 param = 0, u64 param2 = 0, u64 param3 = 0) const
	{
		call_after(attotime::zero, param, param2, param3);
	}

private:
	// internal state
	timer_expired_delegate m_callback;    // the full delegate
	device_scheduler *m_scheduler;        // pointer to the scheduler
	emu_timer_cb *m_next;                 // link to the next registered item
	std::string m_unique_id;              // a unique ID string
};


// ======================> emu_timer

class emu_timer
{
	friend class device_scheduler;
	friend class timer_list;

	// construction/destruction
	emu_timer();
	~emu_timer();

	// allocation and re-use
	emu_timer &init_persistent(running_machine &machine, timer_expired_delegate callback, void *ptr);
	emu_timer &init_persistent(device_t &device, device_timer_id id, void *ptr);
	emu_timer &init_temporary(running_machine &machine, emu_timer_cb const &callback, attotime const &duration, s32 param, u64 param2, u64 param3);
	emu_timer &init_temporary(device_t &device, device_timer_id id, attotime const &duration, s32 param, u64 param2, u64 param3);

public:
	// getters
	emu_timer *prev() const { return m_prev; }
	emu_timer *next() const { return m_next; }
	running_machine &machine() const noexcept { assert(m_machine != nullptr); return *m_machine; }
	bool enabled() const { return m_enabled; }
	bool active() const { return m_enabled && !m_expire.is_never(); }
	s32 param() const { return m_param; }
	u64 param2() const { return m_param2; }
	u64 param3() const { return m_param3; }
	void *ptr() const { return m_ptr; }
	u32 id() const { return m_id; }

	// setters
	bool enable(bool enable = true);
	void set_param(s32 param) { m_param = param; }
	void set_param2(u64 param2) { m_param2 = param2; }
	void set_param3(u64 param3) { m_param3 = param3; }
	void set_ptr(void *ptr) { m_ptr = ptr; }

	// control
	void reset(attotime const &duration = attotime::never) { adjust(duration, m_param, m_period); }
	void adjust(attotime start_delay, s32 param = 0, attotime const &periodicity = attotime::never);

	// timing queries
	attotime elapsed() const noexcept;
	attotime remaining() const noexcept;
	attotime start() const { return m_start; }
	attotime expire() const { return m_expire; }
	attotime period() const { return m_period; }

private:
	// internal helpers
	void register_save();
	void dump() const;

	// internal state
	running_machine *   m_machine;      // reference to the owning machine
	emu_timer *         m_next;         // next timer in order in the list
	emu_timer *         m_prev;         // previous timer in order in the list
	attotime            m_expire;       // time when the timer will expire
	attotime            m_period;       // the repeat frequency of the timer
	attotime            m_start;        // time when the timer was started
	void *              m_ptr;          // pointer parameter
	s32                 m_param;        // integer parameter
	u64                 m_param2;       // larger integer parameter
	u64                 m_param3;       // larger integer parameter
	bool                m_enabled;      // is the timer enabled?
	bool                m_temporary;    // is this a temporary timer?
	emu_timer_cb        m_timer_cb;     // embedded callback
	timer_expired_delegate const *m_callback; // pointer to an external callback
	device_t *          m_device;       // for device timers, a pointer to the device
	device_timer_id     m_id;           // for device timers, the ID of the timer
};

#endif  // MAME_EMU_TIMER_H
