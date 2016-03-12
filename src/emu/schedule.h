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

#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__


//**************************************************************************
//  MACROS
//**************************************************************************

// macro for the RC time constant on a 74LS123 with C > 1000pF
// R is in ohms, C is in farads
#define TIME_OF_74LS123(r,c)            (0.45 * (double)(r) * (double)(c))

// macros for the RC time constant on a 555 timer IC
// R is in ohms, C is in farads
#define PERIOD_OF_555_MONOSTABLE_NSEC(r,c)  ((attoseconds_t)(1100000000 * (double)(r) * (double)(c)))
#define PERIOD_OF_555_ASTABLE_NSEC(r1,r2,c) ((attoseconds_t)( 693000000 * ((double)(r1) + 2.0 * (double)(r2)) * (double)(c)))
#define PERIOD_OF_555_MONOSTABLE(r,c)       attotime::from_nsec(PERIOD_OF_555_MONOSTABLE_NSEC(r,c))
#define PERIOD_OF_555_ASTABLE(r1,r2,c)      attotime::from_nsec(PERIOD_OF_555_ASTABLE_NSEC(r1,r2,c))


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// timer callbacks look like this
typedef delegate<void (void *, INT32)> timer_expired_delegate;

// old-skool callbacks are like this
typedef void (*timer_expired_func)(running_machine &machine, void *ptr, INT32 param);


// ======================> emu_timer

class emu_timer
{
	friend class device_scheduler;
	friend class simple_list<emu_timer>;
	friend class fixed_allocator<emu_timer>;
	friend class resource_pool_object<emu_timer>;

	// construction/destruction
	emu_timer();
	~emu_timer();

	// allocation and re-use
	emu_timer &init(running_machine &machine, timer_expired_delegate callback, void *ptr, bool temporary);
	emu_timer &init(device_t &device, device_timer_id id, void *ptr, bool temporary);
	emu_timer &release();

public:
	// getters
	emu_timer *next() const { return m_next; }
	running_machine &machine() const { assert(m_machine != nullptr); return *m_machine; }
	bool enabled() const { return m_enabled; }
	int param() const { return m_param; }
	void *ptr() const { return m_ptr; }

	// setters
	bool enable(bool enable = true);
	void set_param(int param) { m_param = param; }
	void set_ptr(void *ptr) { m_ptr = ptr; }

	// control
	void reset(const attotime &duration = attotime::never) { adjust(duration, m_param, m_period); }
	void adjust(attotime start_delay, INT32 param = 0, const attotime &periodicity = attotime::never);

	// timing queries
	attotime elapsed() const;
	attotime remaining() const;
	attotime start() const { return m_start; }
	attotime expire() const { return m_expire; }

private:
	// internal helpers
	void register_save();
	void schedule_next_period();
	void dump() const;

	// internal state
	running_machine *   m_machine;      // reference to the owning machine
	emu_timer *         m_next;         // next timer in order in the list
	emu_timer *         m_prev;         // previous timer in order in the list
	timer_expired_delegate m_callback;  // callback function
	INT32               m_param;        // integer parameter
	void *              m_ptr;          // pointer parameter
	bool                m_enabled;      // is the timer enabled?
	bool                m_temporary;    // is the timer temporary?
	attotime            m_period;       // the repeat frequency of the timer
	attotime            m_start;        // time when the timer was started
	attotime            m_expire;       // time when the timer will expire
	device_t *          m_device;       // for device timers, a pointer to the device
	device_timer_id     m_id;           // for device timers, the ID of the timer
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
	running_machine &machine() const { return m_machine; }
	attotime time() const;
	emu_timer *first_timer() const { return m_timer_list; }
	device_execute_interface *currently_executing() const { return m_executing_device; }
	bool can_save() const;

	// execution
	void timeslice();
	void abort_timeslice();
	void trigger(int trigid, const attotime &after = attotime::zero);
	void boost_interleave(const attotime &timeslice_time, const attotime &boost_duration);
	void suspend_resume_changed() { m_suspend_changes_pending = true; }

	// timers, specified by callback/name
	emu_timer *timer_alloc(timer_expired_delegate callback, void *ptr = nullptr);
	void timer_set(const attotime &duration, timer_expired_delegate callback, int param = 0, void *ptr = nullptr);
	void timer_pulse(const attotime &period, timer_expired_delegate callback, int param = 0, void *ptr = nullptr);
	void synchronize(timer_expired_delegate callback = timer_expired_delegate(), int param = 0, void *ptr = nullptr) { timer_set(attotime::zero, callback, param, ptr); }

	// timers with old-skool callbacks
#ifdef USE_STATIC_DELEGATE
	emu_timer *timer_alloc(timer_expired_func callback, const char *name, void *ptr = nullptr) { return timer_alloc(timer_expired_delegate(callback, name, &machine()), ptr); }
	void timer_set(const attotime &duration, timer_expired_func callback, const char *name, int param = 0, void *ptr = nullptr) { timer_set(duration, timer_expired_delegate(callback, name, &machine()), param, ptr); }
	void timer_pulse(const attotime &period, timer_expired_func callback, const char *name, int param = 0, void *ptr = nullptr) { timer_pulse(period, timer_expired_delegate(callback, name, &machine()), param, ptr); }
	void synchronize(timer_expired_func callback, const char *name = nullptr, int param = 0, void *ptr = nullptr) { timer_set(attotime::zero, callback, name, param, ptr); }
#endif

	// timers, specified by device/id; generally devices should use the device_t methods instead
	emu_timer *timer_alloc(device_t &device, device_timer_id id = 0, void *ptr = nullptr);
	void timer_set(const attotime &duration, device_t &device, device_timer_id id = 0, int param = 0, void *ptr = nullptr);

	// debugging
	void dump_timers() const;

	// for emergencies only!
	void eat_all_cycles();

private:
	// callbacks
	void timed_trigger(void *ptr, INT32 param);
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


#endif  // __SCHEDULE_H__ */
