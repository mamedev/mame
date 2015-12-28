// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    schedule.c

    Core device execution and scheduling engine.

***************************************************************************/

#include "emu.h"
#include "debugger.h"

//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define VERBOSE 0

#define LOG(x)  do { if (VERBOSE) machine().logerror x; } while (0)
#define PRECISION



//**************************************************************************
//  CONSTANTS
//**************************************************************************

// internal trigger IDs
enum
{
	TRIGGER_INT         = -2000,
	TRIGGER_YIELDTIME   = -3000,
	TRIGGER_SUSPENDTIME = -4000
};



//**************************************************************************
//  EMU TIMER
//**************************************************************************

//-------------------------------------------------
//  emu_timer - constructor
//-------------------------------------------------

emu_timer::emu_timer()
	: m_machine(nullptr),
		m_next(nullptr),
		m_prev(nullptr),
		m_param(0),
		m_ptr(nullptr),
		m_enabled(false),
		m_temporary(false),
		m_period(attotime::zero),
		m_start(attotime::zero),
		m_expire(attotime::never),
		m_device(nullptr),
		m_id(0)
{
}


//-------------------------------------------------
//  ~emu_timer - destructor
//-------------------------------------------------

emu_timer::~emu_timer()
{
}


//-------------------------------------------------
//  init - completely initialize the state when
//  re-allocated as a non-device timer
//-------------------------------------------------

emu_timer &emu_timer::init(running_machine &machine, timer_expired_delegate callback, void *ptr, bool temporary)
{
	// ensure the entire timer state is clean
	m_machine = &machine;
	m_next = nullptr;
	m_prev = nullptr;
	m_callback = callback;
	m_param = 0;
	m_ptr = ptr;
	m_enabled = false;
	m_temporary = temporary;
	m_period = attotime::never;
	m_start = machine.time();
	m_expire = attotime::never;
	m_device = nullptr;
	m_id = 0;

	// if we're not temporary, register ourselves with the save state system
	if (!m_temporary)
		register_save();

	// insert into the list
	machine.scheduler().timer_list_insert(*this);
	return *this;
}


//-------------------------------------------------
//  init - completely initialize the state when
//  re-allocated as a device timer
//-------------------------------------------------

emu_timer &emu_timer::init(device_t &device, device_timer_id id, void *ptr, bool temporary)
{
	// ensure the entire timer state is clean
	m_machine = &device.machine();
	m_next = nullptr;
	m_prev = nullptr;
	m_callback = timer_expired_delegate();
	m_param = 0;
	m_ptr = ptr;
	m_enabled = false;
	m_temporary = temporary;
	m_period = attotime::never;
	m_start = machine().time();
	m_expire = attotime::never;
	m_device = &device;
	m_id = id;

	// if we're not temporary, register ourselves with the save state system
	if (!m_temporary)
		register_save();

	// insert into the list
	machine().scheduler().timer_list_insert(*this);
	return *this;
}


//-------------------------------------------------
//  release - release us from the global list
//  management when deallocating
//-------------------------------------------------

emu_timer &emu_timer::release()
{
	// unhook us from the global list
	machine().scheduler().timer_list_remove(*this);
	return *this;
}


//-------------------------------------------------
//  enable - enable/disable a timer
//-------------------------------------------------

bool emu_timer::enable(bool enable)
{
	// reschedule only if the state has changed
	bool old = m_enabled;
	if (old != enable)
	{
		// set the enable flag
		m_enabled = enable;

		// remove the timer and insert back into the list
		machine().scheduler().timer_list_remove(*this);
		machine().scheduler().timer_list_insert(*this);
	}
	return old;
}


//-------------------------------------------------
//  adjust - adjust the time when this timer will
//  fire and specify a period for subsequent
//  firings
//-------------------------------------------------

void emu_timer::adjust(attotime start_delay, INT32 param, const attotime &period)
{
	// if this is the callback timer, mark it modified
	device_scheduler &scheduler = machine().scheduler();
	if (scheduler.m_callback_timer == this)
		scheduler.m_callback_timer_modified = true;

	// compute the time of the next firing and insert into the list
	m_param = param;
	m_enabled = true;

	// clamp negative times to 0
	if (start_delay.seconds() < 0)
		start_delay = attotime::zero;

	// set the start and expire times
	m_start = scheduler.time();
	m_expire = m_start + start_delay;
	m_period = period;

	// remove and re-insert the timer in its new order
	scheduler.timer_list_remove(*this);
	scheduler.timer_list_insert(*this);

	// if this was inserted as the head, abort the current timeslice and resync
	if (this == scheduler.first_timer())
		scheduler.abort_timeslice();
}


//-------------------------------------------------
//  elapsed - return the amount of time since the
//  timer was started
//-------------------------------------------------

attotime emu_timer::elapsed() const
{
	return machine().time() - m_start;
}


//-------------------------------------------------
//  remaining - return the amount of time
//  remaining until the timer expires
//-------------------------------------------------

attotime emu_timer::remaining() const
{
	attotime curtime = machine().time();
	if (curtime >= m_expire)
		return attotime::zero;
	return m_expire - curtime;
}


//-------------------------------------------------
//  register_save - register ourself with the save
//  state system
//-------------------------------------------------

void emu_timer::register_save()
{
	// determine our instance number and name
	int index = 0;
	std::string name;

	// for non-device timers, it is an index based on the callback function name
	if (m_device == nullptr)
	{
		name = m_callback.name();
		for (emu_timer *curtimer = machine().scheduler().first_timer(); curtimer != nullptr; curtimer = curtimer->next())
			if (!curtimer->m_temporary && curtimer->m_device == nullptr && strcmp(curtimer->m_callback.name(), m_callback.name()) == 0)
				index++;
	}

	// for device timers, it is an index based on the device and timer ID
	else
	{
		strprintf(name,"%s/%d", m_device->tag(), m_id);
		for (emu_timer *curtimer = machine().scheduler().first_timer(); curtimer != nullptr; curtimer = curtimer->next())
			if (!curtimer->m_temporary && curtimer->m_device != nullptr && curtimer->m_device == m_device && curtimer->m_id == m_id)
				index++;
	}

	// save the bits
	machine().save().save_item(m_device, "timer", name.c_str(), index, NAME(m_param));
	machine().save().save_item(m_device, "timer", name.c_str(), index, NAME(m_enabled));
	machine().save().save_item(m_device, "timer", name.c_str(), index, NAME(m_period));
	machine().save().save_item(m_device, "timer", name.c_str(), index, NAME(m_start));
	machine().save().save_item(m_device, "timer", name.c_str(), index, NAME(m_expire));
}


//-------------------------------------------------
//  schedule_next_period - schedule the next
//  period
//-------------------------------------------------

inline void emu_timer::schedule_next_period()
{
	// advance by one period
	m_start = m_expire;
	m_expire += m_period;

	// remove and re-insert us
	device_scheduler &scheduler = machine().scheduler();
	scheduler.timer_list_remove(*this);
	scheduler.timer_list_insert(*this);
}


//-------------------------------------------------
//  dump - dump internal state to a single output
//  line in the error log
//-------------------------------------------------

void emu_timer::dump() const
{
	machine().logerror("%p: en=%d temp=%d exp=%15s start=%15s per=%15s param=%d ptr=%p", this, m_enabled, m_temporary, m_expire.as_string(PRECISION), m_start.as_string(PRECISION), m_period.as_string(PRECISION), m_param, m_ptr);
	if (m_device == nullptr)
		machine().logerror(" cb=%s\n", m_callback.name());
	else
		machine().logerror(" dev=%s id=%d\n", m_device->tag(), m_id);
}



//**************************************************************************
//  DEVICE SCHEDULER
//**************************************************************************

//-------------------------------------------------
//  device_scheduler - constructor
//-------------------------------------------------

device_scheduler::device_scheduler(running_machine &machine) :
	m_machine(machine),
	m_executing_device(nullptr),
	m_execute_list(nullptr),
	m_basetime(attotime::zero),
	m_timer_list(nullptr),
	m_callback_timer(nullptr),
	m_callback_timer_modified(false),
	m_callback_timer_expire_time(attotime::zero),
	m_suspend_changes_pending(true),
	m_quantum_minimum(ATTOSECONDS_IN_NSEC(1) / 1000)
{
	// append a single never-expiring timer so there is always one in the list
	m_timer_list = &m_timer_allocator.alloc()->init(machine, timer_expired_delegate(), nullptr, true);
	m_timer_list->adjust(attotime::never);

	// register global states
	machine.save().save_item(NAME(m_basetime));
	machine.save().register_presave(save_prepost_delegate(FUNC(device_scheduler::presave), this));
	machine.save().register_postload(save_prepost_delegate(FUNC(device_scheduler::postload), this));
}


//-------------------------------------------------
//  device_scheduler - destructor
//-------------------------------------------------

device_scheduler::~device_scheduler()
{
	// remove all timers
	while (m_timer_list != nullptr)
		m_timer_allocator.reclaim(m_timer_list->release());
}


//-------------------------------------------------
//  time - return the current time
//-------------------------------------------------

attotime device_scheduler::time() const
{
	// if we're currently in a callback, use the timer's expiration time as a base
	if (m_callback_timer != nullptr)
		return m_callback_timer_expire_time;

	// if we're executing as a particular CPU, use its local time as a base
	// otherwise, return the global base time
	return (m_executing_device != nullptr) ? m_executing_device->local_time() : m_basetime;
}


//-------------------------------------------------
//  can_save - return true if it's safe to save
//  (i.e., no temporary timers outstanding)
//-------------------------------------------------

bool device_scheduler::can_save() const
{
	// if any live temporary timers exit, fail
	for (emu_timer *timer = m_timer_list; timer != nullptr; timer = timer->next())
		if (timer->m_temporary && !timer->expire().is_never())
		{
			machine().logerror("Failed save state attempt due to anonymous timers:\n");
			dump_timers();
			return false;
		}

	// otherwise, we're good
	return true;
}


//-------------------------------------------------
//  apply_suspend_changes - applies suspend/resume
//  changes to all device_execute_interfaces
//-------------------------------------------------

inline void device_scheduler::apply_suspend_changes()
{
	UINT32 suspendchanged = 0;
	for (device_execute_interface *exec = m_execute_list; exec != nullptr; exec = exec->m_nextexec)
	{
		suspendchanged |= exec->m_suspend ^ exec->m_nextsuspend;
		exec->m_suspend = exec->m_nextsuspend;
		exec->m_nextsuspend &= ~SUSPEND_REASON_TIMESLICE;
		exec->m_eatcycles = exec->m_nexteatcycles;
	}

	// recompute the execute list if any CPUs changed their suspension state
	if (suspendchanged != 0)
		rebuild_execute_list();
	else
		m_suspend_changes_pending = false;
}


//-------------------------------------------------
//  timeslice - execute all devices for a single
//  timeslice
//-------------------------------------------------

void device_scheduler::timeslice()
{
	bool call_debugger = ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0);

	// build the execution list if we don't have one yet
	if (UNEXPECTED(m_execute_list == nullptr))
		rebuild_execute_list();

	// if the current quantum has expired, find a new one
	while (m_basetime >= m_quantum_list.first()->m_expire)
		m_quantum_allocator.reclaim(m_quantum_list.detach_head());

	// loop until we hit the next timer
	while (m_basetime < m_timer_list->m_expire)
	{
		// by default, assume our target is the end of the next quantum
		attotime target(m_basetime + attotime(0, m_quantum_list.first()->m_actual));

		// however, if the next timer is going to fire before then, override
		if (m_timer_list->m_expire < target)
			target = m_timer_list->m_expire;

		LOG(("------------------\n"));
		LOG(("cpu_timeslice: target = %s\n", target.as_string(PRECISION)));

		// do we have pending suspension changes?
		if (m_suspend_changes_pending)
			apply_suspend_changes();

		// loop over all CPUs
		for (device_execute_interface *exec = m_execute_list; exec != nullptr; exec = exec->m_nextexec)
		{
			// only process if this CPU is executing or truly halted (not yielding)
			// and if our target is later than the CPU's current time (coarse check)
			if (EXPECTED((exec->m_suspend == 0 || exec->m_eatcycles) && target.seconds() >= exec->m_localtime.seconds()))
			{
				// compute how many attoseconds to execute this CPU
				attoseconds_t delta = target.attoseconds() - exec->m_localtime.attoseconds();
				if (delta < 0 && target.seconds() > exec->m_localtime.seconds())
					delta += ATTOSECONDS_PER_SECOND;
				assert(delta == (target - exec->m_localtime).as_attoseconds());

				// if we have enough for at least 1 cycle, do the math
				if (delta >= exec->m_attoseconds_per_cycle)
				{
					// compute how many cycles we want to execute
					int ran = exec->m_cycles_running = divu_64x32((UINT64)delta >> exec->m_divshift, exec->m_divisor);
					LOG(("  cpu '%s': %" I64FMT"d (%d cycles)\n", exec->device().tag(), delta, exec->m_cycles_running));

					// if we're not suspended, actually execute
					if (exec->m_suspend == 0)
					{
						g_profiler.start(exec->m_profiler);

						// note that this global variable cycles_stolen can be modified
						// via the call to cpu_execute
						exec->m_cycles_stolen = 0;
						m_executing_device = exec;
						*exec->m_icountptr = exec->m_cycles_running;
						if (!call_debugger)
							exec->run();
						else
						{
							debugger_start_cpu_hook(&exec->device(), target);
							exec->run();
							debugger_stop_cpu_hook(&exec->device());
						}

						// adjust for any cycles we took back
						assert(ran >= *exec->m_icountptr);
						ran -= *exec->m_icountptr;
						assert(ran >= exec->m_cycles_stolen);
						ran -= exec->m_cycles_stolen;
						g_profiler.stop();
					}

					// account for these cycles
					exec->m_totalcycles += ran;

					// update the local time for this CPU
					attotime deltatime(0, exec->m_attoseconds_per_cycle * ran);
					assert(deltatime >= attotime::zero);
					exec->m_localtime += deltatime;
					LOG(("         %d ran, %d total, time = %s\n", ran, (INT32)exec->m_totalcycles, exec->m_localtime.as_string(PRECISION)));

					// if the new local CPU time is less than our target, move the target up, but not before the base
					if (exec->m_localtime < target)
					{
						target = max(exec->m_localtime, m_basetime);
						LOG(("         (new target)\n"));
					}
				}
			}
		}
		m_executing_device = nullptr;

		// update the base time
		m_basetime = target;
	}

	// execute timers
	execute_timers();
}


//-------------------------------------------------
//  abort_timeslice - abort execution for the
//  current timeslice
//-------------------------------------------------

void device_scheduler::abort_timeslice()
{
	if (m_executing_device != nullptr)
		m_executing_device->abort_timeslice();
}


//-------------------------------------------------
//  trigger - generate a global trigger
//-------------------------------------------------

void device_scheduler::trigger(int trigid, const attotime &after)
{
	// ensure we have a list of executing devices
	if (m_execute_list == nullptr)
		rebuild_execute_list();

	// if we have a non-zero time, schedule a timer
	if (after != attotime::zero)
		timer_set(after, timer_expired_delegate(FUNC(device_scheduler::timed_trigger), this), trigid);

	// send the trigger to everyone who cares
	else
		for (device_execute_interface *exec = m_execute_list; exec != nullptr; exec = exec->m_nextexec)
			exec->trigger(trigid);
}


//-------------------------------------------------
//  boost_interleave - temporarily boosts the
//  interleave factor
//-------------------------------------------------

void device_scheduler::boost_interleave(const attotime &timeslice_time, const attotime &boost_duration)
{
	// ignore timeslices > 1 second
	if (timeslice_time.seconds() > 0)
		return;
	add_scheduling_quantum(timeslice_time, boost_duration);
}


//-------------------------------------------------
//  timer_alloc - allocate a global non-device
//  timer and return a pointer
//-------------------------------------------------

emu_timer *device_scheduler::timer_alloc(timer_expired_delegate callback, void *ptr)
{
	return &m_timer_allocator.alloc()->init(machine(), callback, ptr, false);
}


//-------------------------------------------------
//  timer_set - allocate an anonymous non-device
//  timer and set it to go off after the given
//  amount of time
//-------------------------------------------------

void device_scheduler::timer_set(const attotime &duration, timer_expired_delegate callback, int param, void *ptr)
{
	m_timer_allocator.alloc()->init(machine(), callback, ptr, true).adjust(duration, param);
}


//-------------------------------------------------
//  timer_pulse - allocate an anonymous non-device
//  timer and set it to go off at the given
//  frequency
//-------------------------------------------------

void device_scheduler::timer_pulse(const attotime &period, timer_expired_delegate callback, int param, void *ptr)
{
	m_timer_allocator.alloc()->init(machine(), callback, ptr, false).adjust(period, param, period);
}


//-------------------------------------------------
//  timer_alloc - allocate a global device timer
//  and return a pointer
//-------------------------------------------------

emu_timer *device_scheduler::timer_alloc(device_t &device, device_timer_id id, void *ptr)
{
	return &m_timer_allocator.alloc()->init(device, id, ptr, false);
}


//-------------------------------------------------
//  timer_set - allocate an anonymous device timer
//  and set it to go off after the given amount of
//  time
//-------------------------------------------------

void device_scheduler::timer_set(const attotime &duration, device_t &device, device_timer_id id, int param, void *ptr)
{
	m_timer_allocator.alloc()->init(device, id, ptr, true).adjust(duration, param);
}


//-------------------------------------------------
//  eat_all_cycles - eat a ton of cycles on all
//  CPUs to force a quick exit
//-------------------------------------------------

void device_scheduler::eat_all_cycles()
{
	for (device_execute_interface *exec = m_execute_list; exec != nullptr; exec = exec->m_nextexec)
		exec->eat_cycles(1000000000);
}


//-------------------------------------------------
//  timed_trigger - generate a trigger after a
//  given amount of time
//-------------------------------------------------

void device_scheduler::timed_trigger(void *ptr, INT32 param)
{
	trigger(param);
}


//-------------------------------------------------
//  presave - before creating a save state
//-------------------------------------------------

void device_scheduler::presave()
{
	// report the timer state after a log
	machine().logerror("Prior to saving state:\n");
	dump_timers();
}


//-------------------------------------------------
//  postload - after loading a save state
//-------------------------------------------------

void device_scheduler::postload()
{
	// remove all timers and make a private list of permanent ones
	simple_list<emu_timer> private_list;
	while (m_timer_list != nullptr)
	{
		emu_timer &timer = *m_timer_list;

		// temporary timers go away entirely (except our special never-expiring one)
		if (timer.m_temporary && !timer.expire().is_never())
			m_timer_allocator.reclaim(timer.release());

		// permanent ones get added to our private list
		else
			private_list.append(timer_list_remove(timer));
	}

	// now re-insert them; this effectively re-sorts them by time
	emu_timer *timer;
	while ((timer = private_list.detach_head()) != nullptr)
		timer_list_insert(*timer);

	m_suspend_changes_pending = true;
	rebuild_execute_list();

	// report the timer state after a log
	machine().logerror("After resetting/reordering timers:\n");
	dump_timers();
}


//-------------------------------------------------
//  compute_perfect_interleave - compute the
//  "perfect" interleave interval
//-------------------------------------------------

void device_scheduler::compute_perfect_interleave()
{
	// ensure we have a list of executing devices
	if (m_execute_list == nullptr)
		rebuild_execute_list();

	// start with the first one
	device_execute_interface *first = m_execute_list;
	if (first != nullptr)
	{
		// start with a huge time factor and find the 2nd smallest cycle time
		attoseconds_t smallest = first->minimum_quantum();
		attoseconds_t perfect = ATTOSECONDS_PER_SECOND - 1;
		for (device_execute_interface *exec = first->m_nextexec; exec != nullptr; exec = exec->m_nextexec)
		{
			// find the 2nd smallest cycle interval
			attoseconds_t curquantum = exec->minimum_quantum();
			if (curquantum < smallest)
			{
				perfect = smallest;
				smallest = curquantum;
			}
			else if (curquantum < perfect)
				perfect = curquantum;
		}

		// if this is a new minimum quantum, apply it
		if (m_quantum_minimum != perfect)
		{
			// adjust all the actuals; this doesn't affect the current
			m_quantum_minimum = perfect;
			for (quantum_slot *quant = m_quantum_list.first(); quant != nullptr; quant = quant->next())
				quant->m_actual = MAX(quant->m_requested, m_quantum_minimum);
		}
	}
}


//-------------------------------------------------
//  rebuild_execute_list - rebuild the list of
//  executing CPUs, moving suspended CPUs to the
//  end
//-------------------------------------------------

void device_scheduler::rebuild_execute_list()
{
	// if we haven't yet set a scheduling quantum, do it now
	if (m_quantum_list.first() == nullptr)
	{
		// set the core scheduling quantum
		attotime min_quantum = machine().config().m_minimum_quantum;

		// if none specified default to 60Hz
		if (min_quantum.is_zero())
			min_quantum = attotime::from_hz(60);

		// if the configuration specifies a device to make perfect, pick that as the minimum
		if (!machine().config().m_perfect_cpu_quantum.empty())
		{
			device_t *device = machine().device(machine().config().m_perfect_cpu_quantum.c_str());
			if (device == nullptr)
				fatalerror("Device '%s' specified for perfect interleave is not present!\n", machine().config().m_perfect_cpu_quantum.c_str());

			device_execute_interface *exec;
			if (!device->interface(exec))
				fatalerror("Device '%s' specified for perfect interleave is not an executing device!\n", machine().config().m_perfect_cpu_quantum.c_str());

			min_quantum = min(attotime(0, exec->minimum_quantum()), min_quantum);
		}

		// make sure it's no higher than 60Hz
		min_quantum = min(min_quantum, attotime::from_hz(60));

		// inform the timer system of our decision
		add_scheduling_quantum(min_quantum, attotime::never);
	}

	// start with an empty list
	device_execute_interface **active_tailptr = &m_execute_list;
	*active_tailptr = nullptr;

	// also make an empty list of suspended devices
	device_execute_interface *suspend_list = nullptr;
	device_execute_interface **suspend_tailptr = &suspend_list;

	// iterate over all devices
	execute_interface_iterator iter(machine().root_device());
	for (device_execute_interface *exec = iter.first(); exec != nullptr; exec = iter.next())
	{
		// append to the appropriate list
		exec->m_nextexec = nullptr;
		if (exec->m_suspend == 0)
		{
			*active_tailptr = exec;
			active_tailptr = &exec->m_nextexec;
		}
		else
		{
			*suspend_tailptr = exec;
			suspend_tailptr = &exec->m_nextexec;
		}
	}

	// append the suspend list to the end of the active list
	*active_tailptr = suspend_list;
}


//-------------------------------------------------
//  timer_list_insert - insert a new timer into
//  the list at the appropriate location
//-------------------------------------------------

emu_timer &device_scheduler::timer_list_insert(emu_timer &timer)
{
	// disabled timers sort to the end
	const attotime &expire = timer.m_enabled ? timer.m_expire : attotime::never;

	// loop over the timer list
	emu_timer *prevtimer = nullptr;
	for (emu_timer *curtimer = m_timer_list; curtimer != nullptr; prevtimer = curtimer, curtimer = curtimer->next())
	{
		// if the current list entry expires after us, we should be inserted before it
		if (curtimer->m_expire > expire)
		{
			// link the new guy in before the current list entry
			timer.m_prev = curtimer->m_prev;
			timer.m_next = curtimer;

			if (curtimer->m_prev != nullptr)
				curtimer->m_prev->m_next = &timer;
			else
				m_timer_list = &timer;

			curtimer->m_prev = &timer;
			return timer;
		}
	}

	// need to insert after the last one
	if (prevtimer != nullptr)
		prevtimer->m_next = &timer;
	else
		m_timer_list = &timer;

	timer.m_prev = prevtimer;
	timer.m_next = nullptr;
	return timer;
}


//-------------------------------------------------
//  timer_list_remove - remove a timer from the
//  linked list
//-------------------------------------------------

emu_timer &device_scheduler::timer_list_remove(emu_timer &timer)
{
	// remove it from the list
	if (timer.m_prev != nullptr)
		timer.m_prev->m_next = timer.m_next;
	else
		m_timer_list = timer.m_next;

	if (timer.m_next != nullptr)
		timer.m_next->m_prev = timer.m_prev;

	return timer;
}


//-------------------------------------------------
//  execute_timers - execute timers that are due
//-------------------------------------------------

inline void device_scheduler::execute_timers()
{
	LOG(("execute_timers: new=%s head->expire=%s\n", m_basetime.as_string(PRECISION), m_timer_list->m_expire.as_string(PRECISION)));

	// now process any timers that are overdue
	while (m_timer_list->m_expire <= m_basetime)
	{
		// if this is a one-shot timer, disable it now
		emu_timer &timer = *m_timer_list;
		bool was_enabled = timer.m_enabled;
		if (timer.m_period.is_zero() || timer.m_period.is_never())
			timer.m_enabled = false;

		// set the global state of which callback we're in
		m_callback_timer_modified = false;
		m_callback_timer = &timer;
		m_callback_timer_expire_time = timer.m_expire;

		// call the callback
		if (was_enabled)
		{
			g_profiler.start(PROFILER_TIMER_CALLBACK);

			if (timer.m_device != nullptr)
			{
				LOG(("execute_timers: timer device %s timer %d\n", timer.m_device->tag(), timer.m_id));
				timer.m_device->timer_expired(timer, timer.m_id, timer.m_param, timer.m_ptr);
			}
			else if (!timer.m_callback.isnull())
			{
				LOG(("execute_timers: timer callback %s\n", timer.m_callback.name()));
				timer.m_callback(timer.m_ptr, timer.m_param);
			}

			g_profiler.stop();
		}

		// clear the callback timer global
		m_callback_timer = nullptr;

		// reset or remove the timer, but only if it wasn't modified during the callback
		if (!m_callback_timer_modified)
		{
			// if the timer is temporary, remove it now
			if (timer.m_temporary)
				m_timer_allocator.reclaim(timer.release());

			// otherwise, reschedule it
			else
				timer.schedule_next_period();
		}
	}
}


//-------------------------------------------------
//  add_scheduling_quantum - add a scheduling
//  quantum; the smallest active one is the one
//  that is in use
//-------------------------------------------------

void device_scheduler::add_scheduling_quantum(const attotime &quantum, const attotime &duration)
{
	assert(quantum.seconds() == 0);

	attotime curtime = time();
	attotime expire = curtime + duration;
	const attoseconds_t quantum_attos = quantum.attoseconds();

	// figure out where to insert ourselves, expiring any quanta that are out-of-date
	quantum_slot *insert_after = nullptr;
	quantum_slot *next;
	for (quantum_slot *quant = m_quantum_list.first(); quant != nullptr; quant = next)
	{
		// if this quantum is expired, nuke it
		next = quant->next();
		if (curtime >= quant->m_expire)
			m_quantum_allocator.reclaim(m_quantum_list.detach(*quant));

		// if this quantum is shorter than us, we need to be inserted afterwards
		else if (quant->m_requested <= quantum_attos)
			insert_after = quant;
	}

	// if we found an exact match, just take the maximum expiry time
	if (insert_after != nullptr && insert_after->m_requested == quantum_attos)
		insert_after->m_expire = max(insert_after->m_expire, expire);

	// otherwise, allocate a new quantum and insert it after the one we picked
	else
	{
		quantum_slot &quant = *m_quantum_allocator.alloc();
		quant.m_requested = quantum_attos;
		quant.m_actual = MAX(quantum_attos, m_quantum_minimum);
		quant.m_expire = expire;
		m_quantum_list.insert_after(quant, insert_after);
	}
}


//-------------------------------------------------
//  dump_timers - dump the current timer state
//-------------------------------------------------

void device_scheduler::dump_timers() const
{
	machine().logerror("=============================================\n");
	machine().logerror("Timer Dump: Time = %15s\n", time().as_string(PRECISION));
	for (emu_timer *timer = first_timer(); timer != nullptr; timer = timer->next())
		timer->dump();
	machine().logerror("=============================================\n");
}
