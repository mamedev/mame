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

// timer IDs for devices
using device_timer_id = u32;

// timer callbacks look like this
using timer_expired_delegate_native = named_delegate<void (emu_timer const &timer)>;
using timer_expired_delegate_form1 = named_delegate<void ()>;
using timer_expired_delegate_form2 = named_delegate<void (emu_timer &, device_timer_id, int, void *)>;
template<typename IntType> using timer_expired_delegate_form3 = named_delegate<void (IntType)>;
template<typename IntType> using timer_expired_delegate_form4 = named_delegate<void (void *, IntType)>;
template<typename IntType, typename IntType2> using timer_expired_delegate_form5 = named_delegate<void (IntType, IntType2)>;
template<typename IntType, typename IntType2, typename IntType3> using timer_expired_delegate_form6 = named_delegate<void (IntType, IntType2, IntType3)>;


// ======================> timer_expired_delegate

class timer_expired_delegate : public timer_expired_delegate_native
{
	// this is just a substitute for an arbitrary delegate; it presumes that
	// all delegates are equivalent from a size/copy/move perspective
	using generic_delegate = named_delegate<void ()>;

public:
	// import direct constructors for native callbacks
	using timer_expired_delegate_native::timer_expired_delegate_native;

	// copy constructor
	timer_expired_delegate(timer_expired_delegate const &src) :
		timer_expired_delegate_native(src),
		m_sub_delegate(src.m_sub_delegate)
	{
		// if the delegate is bound to the source object, rebind it to the copy
		if (src.has_sub_delegate())
			bind(reinterpret_cast<delegate_generic_class *>(this));
	}

	// copy assignment
	timer_expired_delegate &operator=(timer_expired_delegate const &src)
	{
		*(timer_expired_delegate_native *)this = src;
		m_sub_delegate = src.m_sub_delegate;

		// if the delegate is bound to the source object, rebind it to the copy
		if (src.has_sub_delegate())
			bind(reinterpret_cast<delegate_generic_class *>(this));
		return *this;
	}

	// equality
	bool operator==(const timer_expired_delegate &rhs) const
	{
		if (has_sub_delegate())
			return rhs.has_sub_delegate() ? (m_sub_delegate == rhs.m_sub_delegate) : false;
		else
			return rhs.has_sub_delegate() ? false : timer_expired_delegate_native::operator==(rhs);
	}
	bool operator!=(const timer_expired_delegate &rhs) const
	{
		if (has_sub_delegate())
			return rhs.has_sub_delegate() ? (m_sub_delegate != rhs.m_sub_delegate) : false;
		else
			return rhs.has_sub_delegate() ? false : timer_expired_delegate_native::operator!=(rhs);
	}

	// form 1 constructor
	template<typename FuncDeviceType, typename DeviceType>
	timer_expired_delegate(void (FuncDeviceType::*cb)(), char const *name, DeviceType *bindto) :
		timer_expired_delegate_native(FUNC(timer_expired_delegate::form1_callback), this)
	{
		static_assert(sizeof(timer_expired_delegate_form1) == sizeof(generic_delegate));
		reinterpret_cast<timer_expired_delegate_form1 &>(m_sub_delegate) = timer_expired_delegate_form1(cb, name, bindto);
	}

	// form 2 constructor
	template<typename FuncDeviceType, typename DeviceType>
	timer_expired_delegate(void (FuncDeviceType::*cb)(emu_timer &, device_timer_id, int, void *), char const *name, DeviceType *bindto) :
		timer_expired_delegate_native(FUNC(timer_expired_delegate::form2_callback), this)
	{
		static_assert(sizeof(timer_expired_delegate_form2) == sizeof(generic_delegate));
		reinterpret_cast<timer_expired_delegate_form2 &>(m_sub_delegate) = timer_expired_delegate_form2(cb, name, bindto);
	}

	// form 3 constructor
	template<typename FuncDeviceType, typename DeviceType, typename IntType, std::enable_if_t<std::is_integral<IntType>::value, bool> = true>
	timer_expired_delegate(void (FuncDeviceType::*cb)(IntType), char const *name, DeviceType *bindto) :
		timer_expired_delegate_native(FUNC(timer_expired_delegate::form3_callback<IntType>), this)
	{
		static_assert(sizeof(timer_expired_delegate_form3<IntType>) == sizeof(generic_delegate));
		reinterpret_cast<timer_expired_delegate_form3<IntType> &>(m_sub_delegate) = timer_expired_delegate_form3<IntType>(cb, name, bindto);
	}

	// form 4 constructor
	template<typename FuncDeviceType, typename DeviceType, typename IntType, std::enable_if_t<std::is_integral<IntType>::value, bool> = true>
	timer_expired_delegate(void (FuncDeviceType::*cb)(void *ptr, IntType), char const *name, DeviceType *bindto) :
		timer_expired_delegate_native(FUNC(timer_expired_delegate::form4_callback<IntType>), this)
	{
		static_assert(sizeof(timer_expired_delegate_form4<IntType>) == sizeof(generic_delegate));
		reinterpret_cast<timer_expired_delegate_form4<IntType> &>(m_sub_delegate) = timer_expired_delegate_form4<IntType>(cb, name, bindto);
	}

	// form 5 constructor
	template<typename FuncDeviceType, typename DeviceType, typename IntType, typename IntType2, std::enable_if_t<std::is_integral<IntType>::value && std::is_integral<IntType2>::value, bool> = true>
	timer_expired_delegate(void (FuncDeviceType::*cb)(u32, IntType), char const *name, DeviceType *bindto) :
		timer_expired_delegate_native(FUNC((timer_expired_delegate::form5_callback<IntType, IntType2>)), this)
	{
		static_assert(sizeof(timer_expired_delegate_form5<IntType, IntType2>) == sizeof(generic_delegate));
		reinterpret_cast<timer_expired_delegate_form5<IntType, IntType2> &>(m_sub_delegate) = timer_expired_delegate_form5<IntType, IntType2>(cb, name, bindto);
	}

	// form 6 constructor
	template<typename FuncDeviceType, typename DeviceType, typename IntType, typename IntType2, typename IntType3, std::enable_if_t<std::is_integral<IntType>::value && std::is_integral<IntType2>::value && std::is_integral<IntType3>::value, bool> = true>
	timer_expired_delegate(void (FuncDeviceType::*cb)(u32, IntType, IntType), char const *name, DeviceType *bindto) :
		timer_expired_delegate_native(FUNC((timer_expired_delegate::form6_callback<IntType, IntType2, IntType3>)), this)
	{
		static_assert(sizeof(timer_expired_delegate_form6<IntType, IntType2, IntType3>) == sizeof(generic_delegate));
		reinterpret_cast<timer_expired_delegate_form6<IntType, IntType2, IntType3> &>(m_sub_delegate) = timer_expired_delegate_form6<IntType, IntType2, IntType3>(cb, name, bindto);
	}

	// return the name
	char const *name() const { return has_sub_delegate() ? m_sub_delegate.name() : timer_expired_delegate_native::name(); }

private:
	// helper: true if this uses a subdelegate
	bool has_sub_delegate() const
	{
		return (object() == const_cast<delegate_generic_class *>(reinterpret_cast<delegate_generic_class const *>(this)));
	}

	// callbacks for various forms
	void form1_callback(emu_timer const &timer);
	void form2_callback(emu_timer const &timer);
	template<typename IntType> void form3_callback(emu_timer const &timer);
	template<typename IntType> void form4_callback(emu_timer const &timer);
	template<typename IntType, typename IntType2> void form5_callback(emu_timer const &timer);
	template<typename IntType, typename IntType2, typename IntType3> void form6_callback(emu_timer const &timer);

	// buffer to hold an arbitrary secondary delegate
	generic_delegate m_sub_delegate;
};


// ======================> emu_timer_cb

class emu_timer_cb
{
	friend class device_scheduler;
	friend class emu_timer;

public:
	// construction/destruction
	emu_timer_cb();

	// registration
	void enregister(device_scheduler &scheduler, char const *unique, timer_expired_delegate const &callback);
	void enregister(device_t &device, char const *unique, timer_expired_delegate const &callback);
	void interface_enregister(device_interface &device, char const *unique, timer_expired_delegate const &callback);

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
	emu_timer &init_persistent(running_machine &machine, timer_expired_delegate const &callback, void *ptr);
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
	emu_timer *timer_alloc(timer_expired_delegate const &callback, void *ptr = nullptr);
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
	void timed_trigger(emu_timer const &timer);
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
	void empty_timer_cb(emu_timer const &timer);

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


inline void timer_expired_delegate::form1_callback(emu_timer const &timer)
{
	reinterpret_cast<timer_expired_delegate_form1 &>(m_sub_delegate)();
}

inline void timer_expired_delegate::form2_callback(emu_timer const &timer)
{
	reinterpret_cast<timer_expired_delegate_form2 &>(m_sub_delegate)(const_cast<emu_timer &>(timer), timer.id(), timer.param(), timer.ptr());
}

template<typename IntType>
inline void timer_expired_delegate::form3_callback(emu_timer const &timer)
{
	reinterpret_cast<timer_expired_delegate_form3<IntType> &>(m_sub_delegate)(IntType(timer.param()));
}

template<typename IntType>
inline void timer_expired_delegate::form4_callback(emu_timer const &timer)
{
	reinterpret_cast<timer_expired_delegate_form4<IntType> &>(m_sub_delegate)(timer.ptr(), IntType(timer.param()));
}

template<typename IntType, typename IntType2>
inline void timer_expired_delegate::form5_callback(emu_timer const &timer)
{
	reinterpret_cast<timer_expired_delegate_form5<IntType, IntType2> &>(m_sub_delegate)(IntType(timer.param()), IntType2(timer.param2()));
}

template<typename IntType, typename IntType2, typename IntType3>
inline void timer_expired_delegate::form6_callback(emu_timer const &timer)
{
	reinterpret_cast<timer_expired_delegate_form6<IntType, IntType2, IntType3> &>(m_sub_delegate)(IntType(timer.param()), IntType2(timer.param2()), IntType3(timer.param3()));
}


#endif  // MAME_EMU_SCHEDULE_H
