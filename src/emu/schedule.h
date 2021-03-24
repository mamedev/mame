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
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> device_scheduler

class device_scheduler
{
	friend class device_execute_interface;
	friend class basetime_relative;
	friend class emu_timer_cb;
	friend class emu_timer;

	class basetime_relative
	{
	public:
		// minima/maxima
		static constexpr attoseconds_t MAX_RELATIVE = 2 * ATTOSECONDS_PER_SECOND;
		static constexpr attoseconds_t MIN_RELATIVE = -MAX_RELATIVE;

		// construction/destruction
		basetime_relative();

		// set an absolute time
		void set(attotime const &src);

		// add a number of attoseconds to the relative time
		void add(attoseconds_t src);

		// set the base for the relative time
		void set_base_seconds(seconds_t base);

		// return the relative time
		attoseconds_t relative() const { return m_relative; }

		// return the absolute time, updating if dirty
		attotime const &absolute() { if (m_absolute_dirty) update_absolute(); return m_absolute; }

	private:
		// internal helpers
		void update_relative();
		void update_absolute();

		// internal state
		attoseconds_t m_relative;
		attotime m_absolute;
		bool m_absolute_dirty;
		seconds_t m_base_seconds;
	};

	class timer_list
	{
	public:
		timer_list();
		~timer_list();

		emu_timer *head() const { return m_head; }
		emu_timer *tail() const { return m_tail; }

		bool insert_sorted(emu_timer &timer);
		emu_timer &insert_head(emu_timer &timer);
		emu_timer &insert_tail(emu_timer &timer);

		emu_timer *remove_head();
		emu_timer &remove(emu_timer &timer);

	private:
		emu_timer *m_head;
		emu_timer *m_tail;
	};

public:
	// construction/destruction
	device_scheduler(running_machine &machine);
	~device_scheduler();

	// getters
	running_machine &machine() const noexcept { return m_machine; }
	attotime time() const noexcept;
	device_execute_interface *currently_executing() const noexcept { return m_executing_device; }
	bool can_save() const;

	// execution
	template<bool Debugging> void timeslice();
	void abort_timeslice();
	void trigger(int trigid, attotime const &after = attotime::zero);
	void boost_interleave(attotime const &timeslice_time, attotime const &boost_duration);
	void suspend_resume_changed() { m_suspend_changes_pending = true; }

	// register timer callback
	void register_timer_expired(emu_timer_cb &callback);

	// timers, specified by callback/name
	emu_timer *timer_alloc(timer_expired_delegate callback, void *ptr = nullptr);
	void synchronize() { m_empty_timer_cb.synchronize(); }

	// timers, specified by device/id; generally devices should use the device_t methods instead
	emu_timer *timer_alloc(device_t &device, device_timer_id id = 0, void *ptr = nullptr);
	void timer_set(attotime const &duration, device_t &device, device_timer_id id = 0, s32 param = 0, u64 param2 = 0, u64 param3 = 0);

	// pointer to the current callback timer, if live
	emu_timer *callback_timer() const { return m_callback_timer; }

	// debugging
	void dump_timers() const;

	// for emergencies only!
	void eat_all_cycles();

private:
	// callbacks
	void timed_trigger(void *ptr, s32 param);
	void presave();
	void postload();

	// scheduling helpers
	void compute_perfect_interleave();
	void rebuild_execute_list();
	void apply_suspend_changes();
	void add_scheduling_quantum(attotime const &quantum, attotime const &duration);

	// deprecated timer interfaces
	void timer_set(attotime const &duration, emu_timer_cb const &callback, s32 param = 0, u64 param2 = 0, u64 param3 = 0);

	// timer helpers
	emu_timer &timer_alloc_object();
	void timer_reclaim_object(emu_timer &timer);
	void timer_list_move(emu_timer &timer, attotime const &new_expire, bool new_enable);
	void update_first_timer_expire();
	void execute_timers();
	void update_basetime();
	void empty_timer_cb(void *ptr, int param);

	// basetime_relative helpers
	attotime const &basetime() const { return m_basetime; }

	// internal state
	running_machine &           m_machine;                  // reference to our machine
	device_execute_interface *  m_executing_device;         // pointer to currently executing device
	device_execute_interface *  m_execute_list;             // list of devices to be executed
	attotime                    m_basetime;                 // global basetime; everything moves forward from here

	// list of active timers
	basetime_relative           m_first_timer_expire;       // time of the first timer expiration
	timer_list                  m_active_timers;            // sorted list of active timers
	timer_list                  m_inactive_timers;          // unsorted list of inactive timers
	emu_timer *                 m_free_timers;              // simple list of free timers
	emu_timer_cb *              m_registered_timers;        // list of registered timers
	emu_timer_cb                m_empty_timer_cb;           // empty callback
	emu_timer_cb                m_timed_trigger;            // timed trigger callback

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
