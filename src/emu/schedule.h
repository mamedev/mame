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
using timer_expired_delegate_native = named_delegate<void (timer_instance const &)>;
using timer_expired_delegate_form1 = named_delegate<void ()>;
using timer_expired_delegate_form2 = named_delegate<void (timer_instance const &, device_timer_id, int, void *)>;
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
		*static_cast<timer_expired_delegate_native *>(this) = src;
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

	// form 1 constructor: void timer_callback()
	template<typename FuncDeviceType, typename DeviceType>
	timer_expired_delegate(void (FuncDeviceType::*cb)(), char const *name, DeviceType *bindto) :
		timer_expired_delegate_native(FUNC(timer_expired_delegate::form1_callback), this)
	{
		static_assert(sizeof(timer_expired_delegate_form1) == sizeof(generic_delegate));
		reinterpret_cast<timer_expired_delegate_form1 &>(m_sub_delegate) = timer_expired_delegate_form1(cb, name, bindto);
	}

	// form 2 constructor: void timer_callback(timer_instance const &timer, device_timer_id id, int param, void *ptr) -- device_timer style
	template<typename FuncDeviceType, typename DeviceType>
	timer_expired_delegate(void (FuncDeviceType::*cb)(timer_instance const &, device_timer_id, int, void *), char const *name, DeviceType *bindto) :
		timer_expired_delegate_native(FUNC(timer_expired_delegate::form2_callback), this)
	{
		static_assert(sizeof(timer_expired_delegate_form2) == sizeof(generic_delegate));
		reinterpret_cast<timer_expired_delegate_form2 &>(m_sub_delegate) = timer_expired_delegate_form2(cb, name, bindto);
	}

	// form 3 constructor: void timer_callback(int param)
	template<typename FuncDeviceType, typename DeviceType, typename IntType, std::enable_if_t<std::is_integral<IntType>::value, bool> = true>
	timer_expired_delegate(void (FuncDeviceType::*cb)(IntType), char const *name, DeviceType *bindto) :
		timer_expired_delegate_native(FUNC(timer_expired_delegate::form3_callback<IntType>), this)
	{
		static_assert(sizeof(timer_expired_delegate_form3<IntType>) == sizeof(generic_delegate));
		reinterpret_cast<timer_expired_delegate_form3<IntType> &>(m_sub_delegate) = timer_expired_delegate_form3<IntType>(cb, name, bindto);
	}

	// form 4 constructor: void timer_callback(void *ptr, int param) -- legacy style
	template<typename FuncDeviceType, typename DeviceType, typename IntType, std::enable_if_t<std::is_integral<IntType>::value, bool> = true>
	timer_expired_delegate(void (FuncDeviceType::*cb)(void *ptr, IntType), char const *name, DeviceType *bindto) :
		timer_expired_delegate_native(FUNC(timer_expired_delegate::form4_callback<IntType>), this)
	{
		static_assert(sizeof(timer_expired_delegate_form4<IntType>) == sizeof(generic_delegate));
		reinterpret_cast<timer_expired_delegate_form4<IntType> &>(m_sub_delegate) = timer_expired_delegate_form4<IntType>(cb, name, bindto);
	}

	// form 5 constructor: void timer_callback(int param, int param2)
	template<typename FuncDeviceType, typename DeviceType, typename IntType, typename IntType2, std::enable_if_t<std::is_integral<IntType>::value && std::is_integral<IntType2>::value, bool> = true>
	timer_expired_delegate(void (FuncDeviceType::*cb)(u32, IntType), char const *name, DeviceType *bindto) :
		timer_expired_delegate_native(FUNC((timer_expired_delegate::form5_callback<IntType, IntType2>)), this)
	{
		static_assert(sizeof(timer_expired_delegate_form5<IntType, IntType2>) == sizeof(generic_delegate));
		reinterpret_cast<timer_expired_delegate_form5<IntType, IntType2> &>(m_sub_delegate) = timer_expired_delegate_form5<IntType, IntType2>(cb, name, bindto);
	}

	// form 6 constructor: void timer_callback(int param, int param2, int param3)
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
	void form1_callback(timer_instance const &timer);
	void form2_callback(timer_instance const &timer);
	template<typename IntType> void form3_callback(timer_instance const &timer);
	template<typename IntType> void form4_callback(timer_instance const &timer);
	template<typename IntType, typename IntType2> void form5_callback(timer_instance const &timer);
	template<typename IntType, typename IntType2, typename IntType3> void form6_callback(timer_instance const &timer);

	// secondary delegate, which may be of a number of forms
	generic_delegate m_sub_delegate;
};


// ======================> timer_callback

class timer_callback
{
	friend class device_scheduler;
	friend class timer_instance;
	friend class transient_timer_factory;

public:
	// construction/destruction
	timer_callback();
	~timer_callback();

	// copy constructor
	timer_callback(timer_callback const &src);

	// copy assignment
	timer_callback &operator=(timer_callback const &src);

	// registration of a delegate directly
	timer_callback &enregister_base(device_scheduler &scheduler, timer_expired_delegate const &callback, char const *unique);

	// registration of a delegate directly, adding device tag to unique id
	timer_callback &enregister_device(device_t &device, timer_expired_delegate const &callback, char const *unique);

	// registration of an arbitrary member function bound to an arbitrary object; requires the
	// device_scheduler as the first parameter since we don't know how to get one
	template<typename ObjectType, typename FuncType>
	timer_callback &enregister(device_scheduler &scheduler, ObjectType &object, FuncType callback, char const *string, char const *unique = nullptr)
	{
		return enregister_base(scheduler, timer_expired_delegate(callback, string, &object), unique);
	}

	// registration of a device member function bound to that device
	template<typename DeviceType, typename FuncType, std::enable_if_t<std::is_base_of<device_t, DeviceType>::value, bool> = true>
	timer_callback &enregister(DeviceType &device, FuncType callback, char const *string, char const *unique = nullptr)
	{
		return enregister_device(device, timer_expired_delegate(callback, string, &device), unique);
	}

	// registration of a device interface member function bound to the interface
	template<typename IntfType, typename FuncType, std::enable_if_t<std::is_base_of<device_interface, IntfType>::value && !std::is_base_of<device_t, IntfType>::value, bool> = true>
	timer_callback &enregister(IntfType &intf, FuncType callback, char const *string, char const *unique = nullptr)
	{
		return enregister_device(intf.device(), timer_expired_delegate(callback, string, &intf), unique);
	}

	// getters
	device_scheduler &scheduler() const { assert(m_scheduler != nullptr); return *m_scheduler; }
	char const *name() const { return m_callback.name(); }
	void *ptr() const { return m_ptr; }
	device_t *device() const { return m_device; }

	// setters
	timer_callback &set_ptr(void *ptr) { m_ptr = ptr; return *this; }
	timer_callback &set_device(device_t &device) { m_device = &device; return *this; }

private:
	// internal state
	timer_expired_delegate m_callback;    // the full delegate
	void *m_ptr;                          // user-supplied pointer
	device_t *m_device;                   // pointer to device, if there is one
	device_scheduler *m_scheduler;        // pointer to the scheduler
	timer_callback *m_next;                 // link to the next registered item
	u32 m_unique_hash;                    // hash of the unique ID
	std::string m_unique_id;              // a unique ID string
};


// ======================> timer_instance

class timer_instance
{
	friend class device_scheduler;
	friend class persistent_timer;
	friend class transient_timer_factory;
	friend class timer_list;

	// allocation and re-use
	timer_instance &init_transient(timer_callback &callback, attotime const &duration);
	timer_instance &init_persistent(timer_callback &callback);

public:
	// construction/destruction
	timer_instance();
	~timer_instance();

	// getters
	running_machine &machine() const noexcept;
	device_scheduler &scheduler() const noexcept;
	timer_instance *prev() const { return m_prev; }
	timer_instance *next() const { return m_next; }
	bool enabled() const { return m_enabled; }
	bool active() const { return m_enabled && !m_expire.is_never(); }
	u64 param() const { return m_param; }
	u64 param2() const { return m_param2; }
	u64 param3() const { return m_param3; }
	device_timer_id id() const { return m_id; }
	void *ptr() const { return m_callback->ptr(); }

	// setters
	bool enable(bool enable = true);
	timer_instance &set_param(u64 param) { m_param = param; return *this; }
	timer_instance &set_param2(u64 param2) { m_param2 = param2; return *this; }
	timer_instance &set_param3(u64 param3) { m_param3 = param3; return *this; }
	timer_instance &set_id(device_timer_id id) { m_id = id; return *this; }

	// control
	timer_instance &reset(attotime const &duration = attotime::never) { return adjust(duration, m_param, m_period); }
	timer_instance &adjust(attotime const &start_delay, s32 param = 0, attotime const &periodicity = attotime::never);

	// timing queries
	attotime elapsed() const noexcept;
	attotime remaining() const noexcept;
	attotime const &start() const { return m_start; }
	attotime const &expire() const { return m_expire; }
	attotime const &period() const { return m_period; }

private:
	// internal helpers
	void register_save();
	void dump() const;

	// internal state
	timer_instance *    m_next;         // next timer in order in the list
	timer_instance *    m_prev;         // previous timer in order in the list
	attotime            m_start;        // time when the timer was started
	attotime            m_expire;       // time when the timer will expire
	attotime            m_period;       // the repeat frequency of the timer
	timer_callback *    m_callback;     // pointer to an external callback
	u64                 m_param;        // integer parameter
	u64                 m_param2;       // larger integer parameter
	u64                 m_param3;       // larger integer parameter
	device_timer_id     m_id;           // for device timers, the ID of the timer
	bool                m_enabled;      // is the timer enabled?
	bool                m_transient;    // is this a transient timer?
};


// ======================> transient_timer_factory

class transient_timer_factory
{
	friend class timer_instance;

public:
	// constructor
	transient_timer_factory();

	// initialization
	template<typename... T>
	void init(T &&... args)
	{
		m_callback.enregister(std::forward<T>(args)...);
	}

	// create a new timer_instance that will fire after the given duration
	void call_after(attotime const &duration, u64 param = 0, u64 param2 = 0, u64 param3 = 0);

	// same as above, but accepting a timer ID up front as well
	void call_after(device_timer_id id, attotime const &duration, u64 param = 0, u64 param2 = 0, u64 param3 = 0);

	// create a new timer_instance that will fire as soon as possible
	void synchronize(u64 param = 0, u64 param2 = 0, u64 param3 = 0)
	{
		call_after(attotime::zero, param, param2, param3);
	}

	// getters
	timer_callback const &callback() const { return m_callback; }

private:
	// internal state
	timer_callback m_callback;
};


// ======================> persistent_timer

class persistent_timer
{
public:
	// constructor
	persistent_timer();
	~persistent_timer();

	// generic initialization; matches any of the enregister forms; note that we test the
	// second parameter for int/enum/timer_expired_delegate to allow for the specialized
	// cases below
	template<typename T1, typename T2, typename... Tn, std::enable_if_t<!std::is_integral<T2>::value && !std::is_enum<T2>::value && !std::is_base_of<timer_expired_delegate, T2>::value, bool> = true>
	persistent_timer &init(T1 &&arg1, T2 &&arg2, Tn &&... args)
	{
		m_callback.enregister(std::forward<T1>(arg1), std::forward<T2>(arg2), std::forward<Tn>(args)...);
		m_instance.init_persistent(m_callback);
		return *this;
	}

	// initialize with the scheduler and a pre-formed timer_expired_delegate
	persistent_timer &init(device_scheduler &scheduler, timer_expired_delegate const &callback, char const *unique = nullptr)
	{
		m_callback.enregister_base(scheduler, callback, unique);
		m_instance.init_persistent(m_callback);
		return *this;
	}

	// initialize with a device and ID; implicitly calls the device's device_timer method
	persistent_timer &init(device_t &device, device_timer_id id);

	// getters
	timer_instance const &instance() const { return m_instance; }
	timer_callback const &callback() const { return m_callback; }
	bool enabled() const { return m_instance.enabled(); }
	bool active() const { return m_instance.active(); }
	u64 param() const { return m_instance.param(); }
	u64 param2() const { return m_instance.param2(); }
	u64 param3() const { return m_instance.param3(); }
	void *ptr() const { return m_callback.ptr(); }
	device_timer_id id() const { return m_instance.id(); }

	// setters
	bool enable(bool newenable = true) { return m_instance.enable(newenable); }
	persistent_timer &set_param(u64 param) { m_instance.set_param(param); return *this; }
	persistent_timer &set_param2(u64 param2) { m_instance.set_param2(param2); return *this; }
	persistent_timer &set_param3(u64 param3) { m_instance.set_param3(param3); return *this; }
	persistent_timer &set_ptr(void *ptr) { m_callback.set_ptr(ptr); return *this; }

	// control
	persistent_timer &reset(attotime const &duration = attotime::never) { return adjust(duration, m_instance.param(), m_instance.period()); }
	persistent_timer &adjust(attotime const &start_delay, s32 param = 0, attotime const &periodicity = attotime::never) { m_instance.adjust(start_delay, param, periodicity); return *this; }

	// timing queries
	attotime elapsed() const noexcept { return m_instance.elapsed(); }
	attotime remaining() const noexcept { return m_instance.remaining(); }
	attotime const &start() const { return m_instance.start(); }
	attotime const &expire() const { return m_instance.expire(); }
	attotime const &period() const { return m_instance.period(); }

private:
	// internal state
	timer_instance m_instance;
	timer_callback m_callback;
};

using emu_timer = persistent_timer;


// ======================> device_scheduler

class device_scheduler
{
	friend class device_execute_interface;
	friend class transient_timer_factory;
	friend class basetime_relative;
	friend class timer_callback;
	friend class timer_instance;
	friend class device_t; // for access to timer_alloc/timer_set device forms

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

		timer_instance *head() const { return m_head; }
		timer_instance *tail() const { return m_tail; }

		bool insert_sorted(timer_instance &timer);
		timer_instance &insert_head(timer_instance &timer);
		timer_instance &insert_tail(timer_instance &timer);

		timer_instance *remove_head();
		timer_instance &remove(timer_instance &timer);

	private:
		timer_instance *m_head;
		timer_instance *m_tail;
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

	// timer callback registration
	void register_callback(timer_callback &callback);
	void deregister_callback(timer_callback &callback);

	// timers, specified by callback/name; using persistent_timer is preferred
	persistent_timer *timer_alloc(timer_expired_delegate const &callback, void *ptr = nullptr);
	void synchronize() { m_empty_timer.synchronize(); }

	// pointer to the current callback timer, if live
	timer_instance *callback_timer() const { return m_callback_timer; }

	// debugging
	void dump_timers() const;

	// for emergencies only!
	void eat_all_cycles();

private:
	// timers, specified by device/id; generally devices should use the device_t methods instead
	persistent_timer *timer_alloc(device_t &device, device_timer_id id = 0, void *ptr = nullptr);
	void timer_set(attotime const &duration, device_t &device, device_timer_id id = 0, s32 param = 0, u64 param2 = 0, u64 param3 = 0);

	// callbacks
	void presave();
	void postload();

	// execution helpers
	void execute_timers();
	void update_first_timer_expire();
	void update_basetime();

	// scheduling helpers
	void compute_perfect_interleave();
	void rebuild_execute_list();
	void apply_suspend_changes();
	void add_scheduling_quantum(attotime const &quantum, attotime const &duration);

	// helpers for other friends
	timer_instance &insert_active(timer_instance &instance);
	timer_instance &insert_inactive(timer_instance &instance);

	// timer helpers
	timer_instance &instance_alloc();
	void instance_reclaim(timer_instance &timer);
	void instance_move(timer_instance &timer, attotime const &new_expire, bool new_enable);

	// internal timers
	void empty_timer(timer_instance const &timer);
	void timed_trigger(timer_instance const &timer);

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
	timer_instance *            m_free_timers;              // simple list of free timers
	timer_callback *              m_registered_callbacks;     // list of registered callbacks
	transient_timer_factory     m_empty_timer;              // empty timer factory
	transient_timer_factory     m_timed_trigger;            // timed trigger factory
	std::vector<std::unique_ptr<persistent_timer>> m_allocated_persistents;
	std::vector<std::unique_ptr<timer_instance>> m_allocated_instances;

	// other internal states
	timer_instance *            m_callback_timer;           // pointer to the current callback timer
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



inline running_machine &timer_instance::machine() const noexcept
{
	return m_callback->m_scheduler->machine();
}

inline device_scheduler &timer_instance::scheduler() const noexcept
{
	return *m_callback->m_scheduler;
}



inline void timer_expired_delegate::form1_callback(timer_instance const &timer)
{
	reinterpret_cast<timer_expired_delegate_form1 &>(m_sub_delegate)();
}

inline void timer_expired_delegate::form2_callback(timer_instance const &timer)
{
	reinterpret_cast<timer_expired_delegate_form2 &>(m_sub_delegate)(timer, timer.id(), timer.param(), timer.ptr());
}

template<typename IntType>
inline void timer_expired_delegate::form3_callback(timer_instance const &timer)
{
	reinterpret_cast<timer_expired_delegate_form3<IntType> &>(m_sub_delegate)(IntType(timer.param()));
}

template<typename IntType>
inline void timer_expired_delegate::form4_callback(timer_instance const &timer)
{
	reinterpret_cast<timer_expired_delegate_form4<IntType> &>(m_sub_delegate)(timer.ptr(), IntType(timer.param()));
}

template<typename IntType, typename IntType2>
inline void timer_expired_delegate::form5_callback(timer_instance const &timer)
{
	reinterpret_cast<timer_expired_delegate_form5<IntType, IntType2> &>(m_sub_delegate)(IntType(timer.param()), IntType2(timer.param2()));
}

template<typename IntType, typename IntType2, typename IntType3>
inline void timer_expired_delegate::form6_callback(timer_instance const &timer)
{
	reinterpret_cast<timer_expired_delegate_form6<IntType, IntType2, IntType3> &>(m_sub_delegate)(IntType(timer.param()), IntType2(timer.param2()), IntType3(timer.param3()));
}


#endif  // MAME_EMU_SCHEDULE_H
