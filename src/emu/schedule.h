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

#define TIMER_CALLBACK_MEMBER(name)     void name(void *ptr, s32 param)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// timer callbacks look like this
typedef named_delegate<void (void *, s32)> timer_expired_delegate;


// ======================> emu_timer_cb

class emu_timer_cb
{
	friend class device_scheduler;

public:
	// construction/destruction
	emu_timer_cb();

	// registration
	void enregister(device_scheduler &scheduler, timer_expired_delegate callback);
	void enregister(device_t &device, timer_expired_delegate callback);
	void interface_enregister(device_interface &device, timer_expired_delegate callback);

	// direct registration for device's and device interfaces
	template<typename DeviceType, typename FuncType>
	void enregister(DeviceType &device, FuncType callback, char const *string)
	{
		return enregister(device, timer_expired_delegate(callback, string, &device));
	}
	template<typename IntfType, typename FuncType>
	void interface_enregister(IntfType &intf, FuncType callback, char const *string)
	{
		return enregister(intf, timer_expired_delegate(callback, string, &intf));
	}

	// call the delegate after a set amount of time, with the given parameter
	void call_after(const attotime &duration, s32 param = 0, u64 param2 = 0, u64 param3 = 0) const;

	// synchronize; essentially, call immediately
	void synchronize(s32 param = 0, u64 param2 = 0, u64 param3 = 0) const { call_after(attotime::zero, param, param2, param3); }

private:
	// internal state
	timer_expired_delegate m_callback;         // the full delegate
	class device_scheduler *m_scheduler;       // pointer to the scheduler
	emu_timer_cb *m_next; // link to the next registered item
	std::string m_unique_id;                   // a unique ID string
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
	emu_timer &init_temporary(running_machine &machine, timer_expired_delegate callback, attotime const &duration, s32 param, u64 param2, u64 param3);
	emu_timer &init_temporary(device_t &device, device_timer_id id, attotime const &duration, s32 param, u64 param2, u64 param3);

public:
	// getters
	emu_timer *prev() const { return m_prev; }
	emu_timer *next() const { return m_next; }
	running_machine &machine() const noexcept { assert(m_machine != nullptr); return *m_machine; }
	bool enabled() const { return m_enabled; }
	bool active() const { return m_enabled && !m_expire.is_never(); }
	int param() const { return m_param; }
	void *ptr() const { return m_ptr; }

	// setters
	bool enable(bool enable = true);
	void set_param(s32 param) { m_param = param; }
	void set_ptr(void *ptr) { m_ptr = ptr; }

	// control
	void reset(const attotime &duration = attotime::never) { adjust(duration, m_param, m_period); }
	void adjust(attotime start_delay, s32 param = 0, const attotime &periodicity = attotime::never);

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
	static void device_timer_expired(emu_timer &timer, void *ptr, s32 param);

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
	bool                m_temporary;    // is the timer temporary?
	timer_expired_delegate m_callback;  // callback function
	device_t *          m_device;       // for device timers, a pointer to the device
	device_timer_id     m_id;           // for device timers, the ID of the timer
};


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
	void trigger(int trigid, const attotime &after = attotime::zero);
	void boost_interleave(const attotime &timeslice_time, const attotime &boost_duration);
	void suspend_resume_changed() { m_suspend_changes_pending = true; }

	// register timer callback
	void register_timer_expired(emu_timer_cb &callback);

	// timers, specified by callback/name
	emu_timer *timer_alloc(timer_expired_delegate callback, void *ptr = nullptr);
	void synchronize() { synchronize(m_empty_timer_cb); }
	void synchronize(timer_expired_delegate callback, s32 param = 0, u64 param2 = 0, u64 param3 = 0) { timer_set(attotime::zero, callback, param, param2, param3); }
	void synchronize(emu_timer_cb const &callback, s32 param = 0, u64 param2 = 0, u64 param3 = 0) { timer_set(attotime::zero, callback, param, param2, param3); }

	// timers, specified by device/id; generally devices should use the device_t methods instead
	emu_timer *timer_alloc(device_t &device, device_timer_id id = 0, void *ptr = nullptr);
	void timer_set(const attotime &duration, device_t &device, device_timer_id id = 0, s32 param = 0, u64 param2 = 0, u64 param3 = 0);

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
	void add_scheduling_quantum(const attotime &quantum, const attotime &duration);

	// deprecated timer interfaces
	void timer_set(const attotime &duration, timer_expired_delegate callback, s32 param = 0, u64 param2 = 0, u64 param3 = 0);
	void timer_set(const attotime &duration, emu_timer_cb const &callback, s32 param = 0, u64 param2 = 0, u64 param3 = 0);

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
