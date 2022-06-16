// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    schedule.h

    Core device execution and scheduling engine.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef MAME_EMU_SCHEDULE_H
#define MAME_EMU_SCHEDULE_H


//**************************************************************************
//  MACROS
//**************************************************************************

#define TIMER_CALLBACK_MEMBER(name)     void name(s32 param)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// timer callbacks look like this
typedef named_delegate<void (s32)> timer_expired_delegate;

// ======================> emu_timer

class emu_timer
{
public:
	// getters
	bool enabled() const noexcept { return m_enabled; }
	int param() const noexcept { return m_param; }

	// setters
	bool enable(bool enable = true) noexcept;
	void set_param(int param) noexcept { m_param = param; }

	// control
	void reset(const attotime &duration = attotime::never) noexcept { adjust(duration, m_param, m_period); }
	void adjust(attotime start_delay, s32 param = 0, const attotime &periodicity = attotime::never) noexcept;

	// timing queries
	attotime elapsed() const noexcept;
	attotime remaining() const noexcept;
	attotime start() const noexcept { return m_start; }
	attotime expire() const noexcept { return m_expire; }
	attotime period() const noexcept { return m_period; }

private:
	// construction/destruction
	emu_timer() noexcept;
	~emu_timer();

	// allocation and re-use
	emu_timer &init(
			running_machine &machine,
			timer_expired_delegate &&callback,
			attotime start_delay,
			int param,
			bool temporary);

	// internal helpers
	void register_save(save_manager &manager) ATTR_COLD;
	void schedule_next_period() noexcept;
	void dump() const;

	// internal state
	device_scheduler *  m_scheduler;    // reference to the owning machine
	emu_timer *         m_next;         // next timer in order in the list
	emu_timer *         m_prev;         // previous timer in order in the list
	timer_expired_delegate m_callback;  // callback function
	s32                 m_param;        // integer parameter
	bool                m_enabled;      // is the timer enabled?
	bool                m_temporary;    // is the timer temporary?
	attotime            m_period;       // the repeat frequency of the timer
	attotime            m_start;        // time when the timer was started
	attotime            m_expire;       // time when the timer will expire

	friend class device_scheduler;
	friend class fixed_allocator<emu_timer>;
	friend class simple_list<emu_timer>; // FIXME: fixed_allocator requires this
};


// ======================> device_scheduler

class device_scheduler
{
	friend class device_execute_interface;
	friend class emu_timer;

public:
	// construction/destruction
	device_scheduler(running_machine &machine);
	~device_scheduler();

	// getters
	running_machine &machine() const noexcept { return m_machine; }
	attotime time() const noexcept;
	emu_timer *first_timer() const noexcept { return m_timer_list; }
	device_execute_interface *currently_executing() const noexcept { return m_executing_device; }
	bool can_save() const;

	// execution
	void timeslice();
	void abort_timeslice() noexcept;
	void trigger(int trigid, const attotime &after = attotime::zero);
	void boost_interleave(const attotime &timeslice_time, const attotime &boost_duration);
	void suspend_resume_changed() { m_suspend_changes_pending = true; }

	// timers, specified by callback/name
	emu_timer *timer_alloc(timer_expired_delegate callback);
	[[deprecated("timer_set is deprecated; please avoid anonymous timers. Use TIMER_CALLBACK_MEMBER and an allocated emu_timer instead.")]]
	void timer_set(const attotime &duration, timer_expired_delegate callback, int param = 0);
	void synchronize(timer_expired_delegate callback = timer_expired_delegate(), int param = 0);

	// debugging
	void dump_timers() const;

	// for emergencies only!
	void eat_all_cycles();

private:
	// callbacks
	void timed_trigger(s32 param);
	void presave();
	void postload();

	// scheduling helpers
	void compute_perfect_interleave();
	void rebuild_execute_list();
	void apply_suspend_changes();
	void add_scheduling_quantum(const attotime &quantum, const attotime &duration);

	// timer helpers
	emu_timer &timer_list_insert(emu_timer &timer);
	emu_timer &timer_list_remove(emu_timer &timer);
	void execute_timers();

	// internal state
	running_machine &           m_machine;                  // reference to our machine
	device_execute_interface *  m_executing_device;         // pointer to currently executing device
	device_execute_interface *  m_execute_list;             // list of devices to be executed
	attotime                    m_basetime;                 // global basetime; everything moves forward from here

	// list of active timers
	emu_timer *                 m_timer_list;               // head of the active list
	emu_timer *                 m_inactive_timers;          // head of the inactive timer list
	fixed_allocator<emu_timer>  m_timer_allocator;          // allocator for timers

	// other internal states
	emu_timer *                 m_callback_timer;           // pointer to the current callback timer
	bool                        m_callback_timer_modified;  // true if the current callback timer was modified
	attotime                    m_callback_timer_expire_time; // the original expiration time
	bool                        m_suspend_changes_pending;  // suspend/resume changes are pending

	// scheduling quanta
	class quantum_slot
	{
		friend class simple_list<quantum_slot>;

	public:
		quantum_slot *next() const { return m_next; }

		quantum_slot *          m_next;
		attoseconds_t           m_actual;                   // actual duration of the quantum
		attoseconds_t           m_requested;                // duration of the requested quantum
		attotime                m_expire;                   // absolute expiration time of this quantum
	};
	simple_list<quantum_slot>   m_quantum_list;             // list of active quanta
	fixed_allocator<quantum_slot> m_quantum_allocator;      // allocator for quanta
	attoseconds_t               m_quantum_minimum;          // duration of minimum quantum
};


#endif  // MAME_EMU_SCHEDULE_H
