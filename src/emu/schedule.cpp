// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    schedule.cpp

    Core device execution and scheduling engine.

***************************************************************************/

#include "emu.h"
#include "debugger.h"


//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define VERBOSE 0

#define LOG(...)  do { if (VERBOSE) machine().logerror(__VA_ARGS__); } while (0)
#define PRECISION


//**************************************************************************
//  GLOBAL HELPERS
//**************************************************************************

//-------------------------------------------------
//  dummy_delegate - a global empty delegate that
//  can substitute for a null function pointer, if
//  one is provided; this saves us a check in the
//  innermost timer expiration loop
//-------------------------------------------------

static void dummy_delegate(void *, s32)
{
}

static timer_expired_delegate const s_dummy_delegate(FUNC(dummy_delegate));



//**************************************************************************
//  EMU TIMER CB
//**************************************************************************

emu_timer_cb::emu_timer_cb() :
	m_scheduler(nullptr),
	m_next(nullptr)
{
}

void emu_timer_cb::enregister(device_scheduler &scheduler, timer_expired_delegate callback)
{
	if (m_next != nullptr)
		throw emu_fatalerror("Attempted to re-enregister a emu_timer_cb");
	m_callback = callback;
	m_scheduler = &scheduler;
	m_unique_id = m_callback.name();
	m_scheduler->register_timer_expired(*this);
}

void emu_timer_cb::enregister(device_t &device, timer_expired_delegate callback)
{
	if (m_next != nullptr)
		throw emu_fatalerror("Attempted to re-enregister a emu_timer_cb");
	m_callback = callback;
	m_scheduler = &device.machine().scheduler();
	m_unique_id = device.tag();
	m_unique_id += "/";
	m_unique_id += m_callback.name();
	m_scheduler->register_timer_expired(*this);
}

void emu_timer_cb::interface_enregister(device_interface &intf, timer_expired_delegate callback)
{
	if (m_next != nullptr)
		throw emu_fatalerror("Attempted to re-enregister a emu_timer_cb");
	m_callback = callback;
	m_scheduler = &intf.device().machine().scheduler();
	m_unique_id = intf.device().tag();
	m_unique_id += "/";
	m_unique_id += m_callback.name();
	m_scheduler->register_timer_expired(*this);
}

void emu_timer_cb::call_after(const attotime &duration, int param) const
{
	m_scheduler->timer_set(duration, *this, param);
}



//**************************************************************************
//  EMU TIMER
//**************************************************************************

//-------------------------------------------------
//  emu_timer - constructor
//-------------------------------------------------

emu_timer::emu_timer()
{
	// nothing is initialized by default because we explicitly
	// do that in the init_* calls
}


//-------------------------------------------------
//  ~emu_timer - destructor
//-------------------------------------------------

emu_timer::~emu_timer()
{
}


//-------------------------------------------------
//  init_persistent - initialize a persistent
//  system or device timer; persistent timers can
//  be saved and start off in a disabled state
//-------------------------------------------------

inline emu_timer &emu_timer::init_persistent(running_machine &machine, timer_expired_delegate callback, void *ptr)
{
	// ensure the entire timer state is clean
	m_machine = &machine;
	m_next = nullptr;
	m_prev = nullptr;
	m_start = machine.time();
	m_expire = attotime::never;
	m_period = attotime::never;
	m_ptr = ptr;
	m_param = 0;
	m_enabled = false;
	m_temporary = false;
	m_callback = callback.isnull() ? s_dummy_delegate : callback;
	m_device = nullptr;
	m_id = 0;

	// register ourselves with the save state system
	register_save();
	return *this;
}

inline emu_timer &emu_timer::init_persistent(device_t &device, device_timer_id id, void *ptr)
{
	// ensure the entire timer state is clean
	m_machine = &device.machine();
	m_next = nullptr;
	m_prev = nullptr;
	m_start = m_machine->time();
	m_expire = attotime::never;
	m_period = attotime::never;
	m_ptr = ptr;
	m_param = 0;
	m_enabled = false;
	m_temporary = false;
	m_callback = timer_expired_delegate(FUNC(emu_timer::device_timer_expired), this);
	m_device = &device;
	m_id = id;

	// register ourselves with the save state system
	register_save();
	return *this;
}


//-------------------------------------------------
//  init_temporary - initialize a temporary
//  system timer; temporary timers have a parameter
//  and expiration time from the outset
//-------------------------------------------------

inline emu_timer &emu_timer::init_temporary(running_machine &machine, timer_expired_delegate callback, int param, attotime const &duration)
{
	// ensure the entire timer state is clean
	m_machine = &machine;
	m_next = nullptr;
	m_prev = nullptr;
	m_start = machine.time();
	m_expire = m_start + duration;
	m_period = attotime::never;
	m_ptr = nullptr;
	m_param = param;
	m_enabled = true;
	m_temporary = true;
	m_callback = callback.isnull() ? s_dummy_delegate : callback;
	m_device = nullptr;
	m_id = 0;

	return *this;
}

inline emu_timer &emu_timer::init_temporary(device_t &device, device_timer_id id, int param, attotime const &duration)
{
	// ensure the entire timer state is clean
	m_machine = &device.machine();
	m_next = nullptr;
	m_prev = nullptr;
	m_start = m_machine->time();
	m_expire = m_start + duration;
	m_period = attotime::never;
	m_ptr = nullptr;
	m_param = param;
	m_enabled = true;
	m_temporary = true;
	m_callback = timer_expired_delegate(FUNC(emu_timer::device_timer_expired), this);
	m_device = &device;
	m_id = id;

	return *this;
}


//-------------------------------------------------
//  enable - enable/disable a timer
//-------------------------------------------------

bool emu_timer::enable(bool enable)
{
	// reschedule only if the state has changed
	const bool old = m_enabled;
	if (old != enable)
		machine().scheduler().timer_list_move(*this, m_expire, enable);
	return old;
}


//-------------------------------------------------
//  adjust - adjust the time when this timer will
//  fire and specify a period for subsequent
//  firings
//-------------------------------------------------

void emu_timer::adjust(attotime start_delay, s32 param, const attotime &period)
{
	// clamp negative times to 0
	if (start_delay.seconds() < 0)
		start_delay = attotime::zero;

	// set the start and expire times
	device_scheduler &scheduler = machine().scheduler();
	m_start = scheduler.time();
	m_period = period.is_zero() ? attotime::never : period;
	m_param = param;

	// move it into place
	scheduler.timer_list_move(*this, m_start + start_delay, true);
}


//-------------------------------------------------
//  elapsed - return the amount of time since the
//  timer was started
//-------------------------------------------------

attotime emu_timer::elapsed() const noexcept
{
	return machine().time() - m_start;
}


//-------------------------------------------------
//  remaining - return the amount of time
//  remaining until the timer expires
//-------------------------------------------------

attotime emu_timer::remaining() const noexcept
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

	if (m_device == nullptr)
	{
		// for non-device timers, it is an index based on the callback function name
		name = m_callback.name() ? m_callback.name() : "unnamed";
		for (emu_timer *curtimer = machine().scheduler().m_active_timers.head(); curtimer != nullptr; curtimer = curtimer->next())
			if (!curtimer->m_temporary && curtimer->m_device == nullptr)
			{
				if (curtimer->m_callback.name() != nullptr && m_callback.name() != nullptr && strcmp(curtimer->m_callback.name(), m_callback.name()) == 0)
					index++;
				else if (curtimer->m_callback.name() == nullptr && m_callback.name() == nullptr)
					index++;
			}

		for (emu_timer *curtimer = machine().scheduler().m_inactive_timers.head(); curtimer != nullptr; curtimer = curtimer->next())
			if (!curtimer->m_temporary && curtimer->m_device == nullptr)
			{
				if (curtimer->m_callback.name() != nullptr && m_callback.name() != nullptr && strcmp(curtimer->m_callback.name(), m_callback.name()) == 0)
					index++;
				else if (curtimer->m_callback.name() == nullptr && m_callback.name() == nullptr)
					index++;
			}
	}
	else
	{
		// for device timers, it is an index based on the device and timer ID
		name = string_format("%s/%d", m_device->tag(), m_id);
		for (emu_timer *curtimer = machine().scheduler().m_active_timers.head(); curtimer != nullptr; curtimer = curtimer->next())
			if (!curtimer->m_temporary && curtimer->m_device == m_device && curtimer->m_id == m_id)
				index++;

		for (emu_timer *curtimer = machine().scheduler().m_inactive_timers.head(); curtimer != nullptr; curtimer = curtimer->next())
			if (!curtimer->m_temporary && curtimer->m_device == m_device && curtimer->m_id == m_id)
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
//  dump - dump internal state to a single output
//  line in the error log
//-------------------------------------------------

void emu_timer::dump() const
{
	machine().logerror("%p: en=%d temp=%d exp=%15s start=%15s per=%15s param=%d ptr=%p", this, m_enabled, m_temporary, m_expire.as_string(PRECISION), m_start.as_string(PRECISION), m_period.as_string(PRECISION), m_param, m_ptr);
	if (m_device == nullptr)
		if (m_callback.name() == nullptr)
			machine().logerror(" cb=NULL\n");
		else
			machine().logerror(" cb=%s\n", m_callback.name());
	else
		machine().logerror(" dev=%s id=%d\n", m_device->tag(), m_id);
}


//-------------------------------------------------
//  device_timer_expired - trampoline to avoid a
//  conditional jump on the hot path
//-------------------------------------------------

void emu_timer::device_timer_expired(emu_timer &timer, void *ptr, s32 param)
{
	timer.m_device->timer_expired(timer, timer.m_id, param, ptr);
}



//**************************************************************************
//  BASETIME-RELATIVE
//**************************************************************************

//-------------------------------------------------
//  basetime_relative - constructor
//-------------------------------------------------

device_scheduler::basetime_relative::basetime_relative() :
	m_relative(0),
	m_absolute(attotime::zero),
	m_absolute_dirty(false),
	m_base_seconds(0)
{
}


//-------------------------------------------------
//  set - set an absolute time, updating the
//  base-relative time as well
//-------------------------------------------------

void device_scheduler::basetime_relative::set(attotime const &src)
{
	m_absolute = src;
	m_absolute_dirty = false;
	update_relative();
}


//-------------------------------------------------
//  add - add attoseconds to the base-relative
//  time, marking the absolute time dirty for
//  later conversion if needed
//-------------------------------------------------

void device_scheduler::basetime_relative::add(attoseconds_t src)
{
	m_relative += src;
	m_absolute_dirty = true;
}


//-------------------------------------------------
//  set_base_seconds - set the base seconds value
//-------------------------------------------------

void device_scheduler::basetime_relative::set_base_seconds(seconds_t base)
{
	// update the absolute time if dirty first
	if (m_absolute_dirty)
		update_absolute();

	// then set and recompute the relative from the absolute time
	m_base_seconds = base;
	update_relative();
}


//-------------------------------------------------
//  update_relative - update the relative time
//  from the absolute time
//-------------------------------------------------

void device_scheduler::basetime_relative::update_relative()
{
	seconds_t delta = m_absolute.seconds() - m_base_seconds;

	// if the seconds match, then the relative time is fine as-is
	m_relative = m_absolute.attoseconds();
	if (delta == 0)
		return;

	// if the absolute time is ahead/behind, we need to add/subtract
	// ATTOSECONDS_PER_SECOND; but only do it once
	if (delta > 0)
	{
		if (delta == 1)
			m_relative += ATTOSECONDS_PER_SECOND;
		else
			m_relative = MAX_RELATIVE;
	}
	else
	{
		if (delta == -1)
			m_relative -= ATTOSECONDS_PER_SECOND;
		else
			m_relative = MIN_RELATIVE;
	}
}


//-------------------------------------------------
//  update_absolute - update the absolute time
//  from the relative time
//-------------------------------------------------

void device_scheduler::basetime_relative::update_absolute()
{
	seconds_t secs = m_base_seconds;
	attoseconds_t attos = m_relative;

	// if relative is outside of range, adjust the seconds
	if (attos >= ATTOSECONDS_PER_SECOND)
	{
		attos -= ATTOSECONDS_PER_SECOND;
		secs++;
	}
	else if (attos < 0)
	{
		attos += ATTOSECONDS_PER_SECOND;
		secs--;
	}

	// set the new value and clear any dirtiness
	m_absolute.set_seconds(secs);
	m_absolute.set_attoseconds(attos);
	m_absolute_dirty = false;
}



//**************************************************************************
//  TIMER LIST
//**************************************************************************

//-------------------------------------------------
//  timer_list - constructor
//-------------------------------------------------

device_scheduler::timer_list::timer_list() :
	m_head(nullptr),
	m_tail(nullptr)
{
}


//-------------------------------------------------
//  ~timer_list - destructor
//-------------------------------------------------

device_scheduler::timer_list::~timer_list()
{
	// we own the timers, so delete them when we go away
	while (m_head != nullptr)
		delete remove_head();
}


//-------------------------------------------------
//  insert_sorted - insert a timer into the list,
//  sorted by expiration time
//-------------------------------------------------

bool device_scheduler::timer_list::insert_sorted(emu_timer &timer)
{
	// special case insert at start
	if (m_head == nullptr || timer.m_expire < m_head->m_expire)
	{
		insert_head(timer);
		return true;
	}

	// special case insert at end
	if (timer.m_expire >= m_tail->m_expire)
	{
		insert_tail(timer);
		return false;
	}

	// scan to find out where we go
	for (emu_timer *scan = m_head->m_next; scan != nullptr; scan = scan->m_next)
		if (timer.m_expire < scan->m_expire)
		{
			timer.m_prev = scan->m_prev;
			timer.m_next = scan;
			scan->m_prev->m_next = &timer;
			scan->m_prev = &timer;
			return false;
		}

	// should never get here
	assert(false);
	return false;
}


//-------------------------------------------------
//  insert_head - insert a timer at the head of
//  the list
//-------------------------------------------------

emu_timer &device_scheduler::timer_list::insert_head(emu_timer &timer)
{
	// no previous, next is the head
	timer.m_prev = nullptr;
	timer.m_next = m_head;

	// if we had a head, link us as the previous and make us head;
	// otherwise, the list was empty, so we're head and tail
	if (m_head != nullptr)
	{
		m_head->m_prev = &timer;
		m_head = &timer;
	}
	else
		m_head = m_tail = &timer;
	return timer;
}


//-------------------------------------------------
//  insert_tail - insert a timer at the tail of
//  the list
//-------------------------------------------------

emu_timer &device_scheduler::timer_list::insert_tail(emu_timer &timer)
{
	// no next, previous is the tail
	timer.m_prev = m_tail;
	timer.m_next = nullptr;

	// if we had a tail, link us as the next and make us tail;
	// otherwise, the list was empty, so we're head and tail
	if (m_tail != nullptr)
	{
		m_tail->m_next = &timer;
		m_tail = &timer;
	}
	else
		m_tail = m_head = &timer;
	return timer;
}


//-------------------------------------------------
//  remove - remove a timer from the list
//-------------------------------------------------

emu_timer &device_scheduler::timer_list::remove(emu_timer &timer)
{
	// link the previous to us; if no previous, we're the head
	if (timer.m_prev != nullptr)
		timer.m_prev->m_next = timer.m_next;
	else
		m_head = timer.m_next;

	// link the next to us; if no next, we're the tail
	if (timer.m_next != nullptr)
		timer.m_next->m_prev = timer.m_prev;
	else
		m_tail = timer.m_prev;

	// return the timer back for chaining
	return timer;
}


//-------------------------------------------------
//  remove_head - remove the head item from the
//  list, returning it or nullptr if the list was
//  empty
//-------------------------------------------------

emu_timer *device_scheduler::timer_list::remove_head()
{
	// no head, empty result
	if (m_head == nullptr)
		return nullptr;

	// advance the head and fix up the previous pointer
	emu_timer *result = m_head;
	m_head = m_head->m_next;
	if (m_head != nullptr)
		m_head->m_prev = nullptr;
	else
		m_tail = nullptr;

	return result;
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
	m_free_timers(nullptr),
	m_registered_timers(nullptr),
	m_callback_timer(nullptr),
	m_callback_timer_modified(false),
	m_callback_timer_expire_time(attotime::zero),
	m_suspend_changes_pending(true),
	m_quantum_minimum(ATTOSECONDS_IN_NSEC(1) / 1000)
{
	// register global states
	machine.save().save_item(NAME(m_basetime));
	machine.save().register_presave(save_prepost_delegate(FUNC(device_scheduler::presave), this));
	machine.save().register_postload(save_prepost_delegate(FUNC(device_scheduler::postload), this));

	m_empty_timer_cb.enregister(*this, FUNC(device_scheduler::empty_timer_cb));
}


//-------------------------------------------------
//  device_scheduler - destructor
//-------------------------------------------------

device_scheduler::~device_scheduler()
{
	emu_timer *head;
	while ((head = m_free_timers) != nullptr)
	{
		m_free_timers = head->m_next;
		delete head;
	}
}


//-------------------------------------------------
//  time - return the current time
//-------------------------------------------------

attotime device_scheduler::time() const noexcept
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
	// if any live temporary timers exist, fail (temporary timers are
	// always active, so only need to scan the active list)
	for (emu_timer *timer = m_active_timers.head(); timer != nullptr; timer = timer->next())
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
	u32 suspendchanged = 0;
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

template<bool Debugging>
void device_scheduler::timeslice()
{
	// build the execution list if we don't have one yet
	if (UNEXPECTED(m_execute_list == nullptr))
		rebuild_execute_list();

	// if the current quantum has expired, find a new one
	while (m_basetime >= m_quantum_list.first()->m_expire)
		m_quantum_allocator.reclaim(m_quantum_list.detach_head());

	// loop until we hit the next timer
	attoseconds_t basetime = m_basetime.attoseconds();
	while (basetime < m_first_timer_expire.relative())
	{
		// by default, assume our target is the end of the next quantum
		attoseconds_t target = basetime + m_quantum_list.first()->m_actual;
		assert(target < basetime_relative::MAX_RELATIVE);

		// however, if the next timer is going to fire before then, override
		if (m_first_timer_expire.relative() < target)
			target = m_first_timer_expire.relative();

		LOG("------------------\n");
		LOG("cpu_timeslice: target = %lld\n", target);

		// do we have pending suspension changes?
		if (m_suspend_changes_pending)
			apply_suspend_changes();

		// loop over all executing devices
		for (device_execute_interface *exec = m_execute_list; exec != nullptr; exec = exec->m_nextexec)
		{
			// compute how many attoseconds to execute this device
			attoseconds_t delta = target - exec->m_localtime.relative() - 1;
			assert(delta < basetime_relative::MAX_RELATIVE);

			// if we're already ahead, do nothing; in theory we should do this 0 as
			// well, but it's a rare case and the compiler tends to be able to
			// optimize a strict < 0 check better than <= 0
			if (delta < 0)
				continue;

			// if not suspended, execute normally
			if (EXPECTED(exec->m_suspend == 0))
			{
				// precache the CPU timing information
				u64 attoseconds_per_cycle = exec->m_attoseconds_per_cycle;

				// compute how many cycles we want to execute, rounding up
				u32 ran = u64(delta) / attoseconds_per_cycle + 1;
				LOG("  cpu '%s': %d (%d cycles)\n", exec->device().tag(), delta, exec->m_cycles_running);

				g_profiler.start(exec->m_profiler);

				// store the number of cycles we've requested in the executing
				// device
				// TODO: do we need to do this?
				exec->m_cycles_running = ran;

				// set the device's icount value to the number of cycles we want
				// the fact that we have a direct point to this is an artifact of
				// the original MAME design
				auto *icountptr = exec->m_icountptr;
				*icountptr = ran;

				// clear m_cycles_stolen, which gets updated if the timeslice
				// is aborted (due to synchronization or setting a new timer to
				// expire before the original timeslice end)
				exec->m_cycles_stolen = 0;

				// store a pointer to the executing device so that we know the
				// relevant active context
				m_executing_device = exec;

				// tell the debugger we're going to start executing; the funny math
				// below uses the relative target as attoseconds, but it may be
				// larger than the the max attoseconds allowed, so we add zero to
				// it, which forces a normalization
				if (Debugging)
					exec->debugger_start_cpu_hook(attotime(m_basetime.seconds(), target) + attotime::zero);

				// now run the device for the number of cycles
				exec->run();

				// tell the debugger we're done executing
				if (Debugging)
					exec->debugger_stop_cpu_hook();

				// now let's see how many cycles we actually ran; if the device's
				// icount is negative, then we ran more than requested (this is both
				// allowed and expected), so the subtract here typically will
				// increase ran
				assert(ran >= *icountptr);
				ran -= *icountptr;

				// if cycles were stolen (i.e., icount was artificially decremented)
				// then ran isn't actually correct, so remove the number of cycles
				// that we did that for
				assert(ran >= exec->m_cycles_stolen);
				ran -= exec->m_cycles_stolen;

				// time should never go backwards, nor should we ever attempt to
				// execute more than a full second (minimum quantum prevents that)
				assert(ran >= 0 && ran < exec->m_cycles_per_second);

				g_profiler.stop();

				// update the device's count of total cycles executed with the
				// true number of cycles
				exec->m_totalcycles += ran;

				// update the local time for the device so that it represents an
				// integral number of cycles
				attoseconds_t deltatime = attoseconds_per_cycle * ran;
				exec->m_localtime.add(deltatime);

				LOG("         %d ran, %d total, time = %s\n", ran, s32(exec->m_totalcycles), exec->m_localtime.absolute().as_string(PRECISION));

				// if the new local device time is less than our target, move the
				// target up, but not before the base
				if (exec->m_localtime.relative() < target)
				{
					target = std::max(exec->m_localtime.relative(), basetime);
					LOG("         (new target)\n");
				}
			}

			// if suspended, eat cycles efficiently
			else if (exec->m_eatcycles)
			{
				// this is just the minimal logic from above to consume the cycles;
				// we don't check to see if the new local time is less than the
				// target because the calculation below guarantees it won't happen
				u32 ran = u64(delta) / u64(exec->m_attoseconds_per_cycle) + 1;
				exec->m_totalcycles += ran;
				exec->m_localtime.add(exec->m_attoseconds_per_cycle * ran);
			}
		}

		// set the executing device to null, which indicates that there is
		// no active context; this is used by machine.time() to return the
		// context-appropriate value
		m_executing_device = nullptr;

		// our final target becomes our new base time
		basetime = target;
	}

	// if basetime remained within the current second, we just have to
	// update the attoseconds part; however, it if did overflow, we need to
	// update all the basetime_relative structures in the system
	if (basetime < ATTOSECONDS_PER_SECOND)
		m_basetime.set_attoseconds(basetime);
	else
	{
		basetime -= ATTOSECONDS_PER_SECOND;
		assert(basetime < ATTOSECONDS_PER_SECOND);
		m_basetime.set_attoseconds(basetime);
		m_basetime.set_seconds(m_basetime.seconds() + 1);
		update_basetime();
	}

	// now that we've reached the expiration time of the first timer in the
	// queue, execute pending ones
	execute_timers();
}

// explicitly instatiate both versions of the scheduler
template void device_scheduler::timeslice<true>();
template void device_scheduler::timeslice<false>();


//-------------------------------------------------
//  update_basetime - update all the
//  basetime_relative times now that the basetime
//  has ticked over another second
//-------------------------------------------------

void device_scheduler::update_basetime()
{
	seconds_t base_seconds = m_basetime.seconds();

	// update execute devices
	for (device_execute_interface &exec : execute_interface_enumerator(machine().root_device()))
		exec.m_localtime.set_base_seconds(base_seconds);

	// move timers from future list into current list
	m_first_timer_expire.set_base_seconds(base_seconds);
}


//-------------------------------------------------
//  empty_timer_cb - empty callback stub when
//  timers provide nothing
//-------------------------------------------------

void device_scheduler::empty_timer_cb(void *ptr, int param)
{
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
//  timer_alloc_object - allocate memory for a new
//  timer, either by reclaiming a freed one, or
//  allocating memory for a new one
//-------------------------------------------------

emu_timer &device_scheduler::timer_alloc_object()
{
	emu_timer *result = m_free_timers;
	if (result != nullptr)
	{
		m_free_timers = result->m_next;
		return *result;
	}
	return *new emu_timer;
}


//-------------------------------------------------
//  timer_reclaim_object - reclaim memory for a
//  timer by adding it to the free list
//-------------------------------------------------

void device_scheduler::timer_reclaim_object(emu_timer &timer)
{
	timer.m_next = m_free_timers;
	m_free_timers = &timer;
}


//-------------------------------------------------
//  register_timer_expired - register a timer
//  expired callback
//-------------------------------------------------

void device_scheduler::register_timer_expired(emu_timer_cb &callback)
{
	callback.m_next = m_registered_timers;
	m_registered_timers = &callback;
}


//-------------------------------------------------
//  timer_alloc - allocate a global non-device
//  timer and return a pointer
//-------------------------------------------------

emu_timer *device_scheduler::timer_alloc(timer_expired_delegate callback, void *ptr)
{
	// persistent timers are implicitly inactive, so they go on the
	// inactive list once created
	return &m_inactive_timers.insert_tail(timer_alloc_object().init_persistent(machine(), callback, ptr));
}


//-------------------------------------------------
//  timer_set - allocate an anonymous non-device
//  timer and set it to go off after the given
//  amount of time
//-------------------------------------------------

void device_scheduler::timer_set(const attotime &duration, timer_expired_delegate callback, int param)
{
	// temporary timers are implicitly active, so add them directly
	// to the active list after creation
	emu_timer &timer = timer_alloc_object().init_temporary(machine(), callback, param, duration);
	if (m_active_timers.insert_sorted(timer))
	{
		update_first_timer_expire();
		abort_timeslice();
	}
}


//-------------------------------------------------
//  timer_set - allocate an anonymous non-device
//  timer and set it to go off after the given
//  amount of time
//-------------------------------------------------

void device_scheduler::timer_set(const attotime &duration, emu_timer_cb const &callback, int param)
{
	// temporary timers are implicitly active, so add them directly
	// to the active list after creation
	emu_timer &timer = timer_alloc_object().init_temporary(machine(), callback.m_callback, param, duration);
	if (m_active_timers.insert_sorted(timer))
	{
		update_first_timer_expire();
		abort_timeslice();
	}
}


//-------------------------------------------------
//  timer_alloc - allocate a global device timer
//  and return a pointer
//-------------------------------------------------

emu_timer *device_scheduler::timer_alloc(device_t &device, device_timer_id id, void *ptr)
{
	// persistent timers are implicitly inactive, so they go on the
	// inactive list once created
	return &m_inactive_timers.insert_tail(timer_alloc_object().init_persistent(device, id, ptr));
}


//-------------------------------------------------
//  timer_set - allocate an anonymous device timer
//  and set it to go off after the given amount of
//  time
//-------------------------------------------------

void device_scheduler::timer_set(const attotime &duration, device_t &device, device_timer_id id, int param)
{
	// temporary timers are implicitly active, so add them directly
	// to the active list after creation
	emu_timer &timer = timer_alloc_object().init_temporary(device, id, param, duration);
	if (m_active_timers.insert_sorted(timer))
	{
		update_first_timer_expire();
		abort_timeslice();
	}
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

void device_scheduler::timed_trigger(void *ptr, s32 param)
{
	trigger(param);
}


//-------------------------------------------------
//  presave - before creating a save state
//-------------------------------------------------

void device_scheduler::presave()
{
	// report the timer state after a log
	LOG("Prior to saving state:\n");
#if VERBOSE
	dump_timers();
#endif
}


//-------------------------------------------------
//  postload - after loading a save state
//-------------------------------------------------

void device_scheduler::postload()
{
	// upon reloading, all the timers' parameters and expiration times
	// will be different; because we rely on this information to tell us
	// which category (active/inactive) they belong to, we have to be
	// careful in how we maniuplate the timers

	// our approach here is to remove all the timers in each list directly,
	// discarding any temporary timers, and appending persistent ones to
	// a local list; once all timers have been rescued this way, we
	// reassemble the final list

	timer_list private_list;
	emu_timer *timer;

	// first discard or capture active timers
	while ((timer = m_active_timers.remove_head()) != nullptr)
		if (timer->m_temporary)
			timer_reclaim_object(*timer);
		else
			private_list.insert_tail(*timer);

	// then discard or capture inactive timers
	while ((timer = m_inactive_timers.remove_head()) != nullptr)
		if (timer->m_temporary)
			timer_reclaim_object(*timer);
		else
			private_list.insert_tail(*timer);

	// now re-insert them; this effectively re-sorts them by time
	while ((timer = private_list.remove_head()) != nullptr)
		if (timer->active())
			m_active_timers.insert_sorted(*timer);
		else
			m_inactive_timers.insert_tail(*timer);

	// force a refresh of things that are lazily updated
	update_first_timer_expire();
	m_suspend_changes_pending = true;
	rebuild_execute_list();

	// report the timer state after a log
	LOG("After resetting/reordering timers:\n");
#if VERBOSE
	dump_timers();
#endif
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
			for (quantum_slot &quant : m_quantum_list)
				quant.m_actual = std::max(quant.m_requested, m_quantum_minimum);
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
	if (m_quantum_list.empty())
	{
		// set the core scheduling quantum, ensuring it's no longer than 60Hz
		attotime min_quantum = machine().config().maximum_quantum(attotime::from_hz(60));

		// if the configuration specifies a device to make perfect, pick that as the minimum
		device_execute_interface *const exec(machine().config().perfect_quantum_device());
		if (exec)
			min_quantum = (std::min)(attotime(0, exec->minimum_quantum()), min_quantum);

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
	for (device_execute_interface &exec : execute_interface_enumerator(machine().root_device()))
	{
		// append to the appropriate list
		exec.m_nextexec = nullptr;
		if (exec.m_suspend == 0)
		{
			*active_tailptr = &exec;
			active_tailptr = &exec.m_nextexec;
		}
		else
		{
			*suspend_tailptr = &exec;
			suspend_tailptr = &exec.m_nextexec;
		}
	}

	// append the suspend list to the end of the active list
	*active_tailptr = suspend_list;
}


//-------------------------------------------------
//  timer_list_move - move an existing timer into
//  the appropriate list at the appropriate
//  location
//-------------------------------------------------

inline void device_scheduler::timer_list_move(emu_timer &timer, attotime const &new_expire, bool new_enable)
{
	// track the before/after active state to see if it changed
	bool was_active = timer.active();
	timer.m_expire = new_expire;
	timer.m_enabled = new_enable;

	// if this is the active timer, don't move anything, just mark it modified
	if (m_callback_timer == &timer)
	{
		m_callback_timer_modified = true;
		return;
	}

	// most common case is becoming/remaining active
	if (timer.active())
	{
		// ok, we're active now; remove us from the previous list
		(was_active ? m_active_timers : m_inactive_timers).remove(timer);

		// if we were inserted at the head, update the first timer expiration; also
		// abort the current timeslice since the first expire time was moved up
		if (m_active_timers.insert_sorted(timer))
		{
			update_first_timer_expire();
			abort_timeslice();
		}
	}
	else if (was_active)
	{
		bool was_first = (timer.m_prev == nullptr);
		m_inactive_timers.insert_tail(m_active_timers.remove(timer));

		// if we were previously the head, update the first timer expiration;
		// no need to abort the current timeslice because the new expiration
		// time will be later than before
		if (was_first)
			update_first_timer_expire();
	}
}


//-------------------------------------------------
//  update_first_timer_expire - updated the
//  m_first_timer_expire field based on the head
//  of the active timers list
//-------------------------------------------------

void device_scheduler::update_first_timer_expire()
{
	emu_timer *timer = m_active_timers.head();
	if (timer != nullptr)
		m_first_timer_expire.set(timer->m_expire);
	else
		m_first_timer_expire.set(attotime(m_basetime.seconds() + 1, m_basetime.attoseconds()));
}


//-------------------------------------------------
//  execute_timers - execute timers that are due
//-------------------------------------------------

inline void device_scheduler::execute_timers()
{
	LOG("execute_timers: new=%s head->expire=%s\n", m_basetime.as_string(PRECISION), m_first_timer_expire.absolute().as_string(PRECISION));

	// now process any timers that are overdue
	while (m_active_timers.head() != nullptr && m_active_timers.head()->m_expire <= m_basetime)
	{
		// if this is a one-shot timer, disable it now
		emu_timer &timer = *m_active_timers.remove_head();

		// handle temporary timers specially
		if (timer.m_temporary)
		{
			// set the global state of which callback we're in
			m_callback_timer = &timer;
			m_callback_timer_expire_time = timer.m_expire;

			// call the callback
			g_profiler.start(PROFILER_TIMER_CALLBACK);
			{
				if (timer.m_device != nullptr)
					LOG("execute_timers: timer device %s timer %d\n", timer.m_device->tag(), timer.m_id);
				else
					LOG("execute_timers: timer callback %s\n", timer.m_callback.name());
				timer.m_callback(timer.m_ptr, timer.m_param);
			}
			g_profiler.stop();

			// reclaim the timer now that we're done with it
			timer_reclaim_object(timer);
		}
		else
		{
			// if not repeating, mark the timer disabled now
			if (timer.m_period.is_never())
				timer.m_enabled = false;

			// set the global state of which callback we're in
			m_callback_timer_modified = false;
			m_callback_timer = &timer;
			m_callback_timer_expire_time = timer.m_expire;

			// call the callback
			g_profiler.start(PROFILER_TIMER_CALLBACK);
			{
				if (timer.m_device != nullptr)
					LOG("execute_timers: timer device %s timer %d\n", timer.m_device->tag(), timer.m_id);
				else
					LOG("execute_timers: timer callback %s\n", timer.m_callback.name());
				timer.m_callback(timer.m_ptr, timer.m_param);
			}
			g_profiler.stop();

			// if the timer wasn't modified during the callback, advance by one period
			if (!m_callback_timer_modified)
			{
				timer.m_start = timer.m_expire;
				timer.m_expire += timer.m_period;
			}

			// insert into the appropriate list
			if (timer.active())
				m_active_timers.insert_sorted(timer);
			else
				m_inactive_timers.insert_tail(timer);
		}
	}

	// update the expiration time of the first timer
	update_first_timer_expire();

	// clear the callback timer global
	m_callback_timer = nullptr;
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
		insert_after->m_expire = std::max(insert_after->m_expire, expire);

	// otherwise, allocate a new quantum and insert it after the one we picked
	else
	{
		quantum_slot &quant = *m_quantum_allocator.alloc();
		quant.m_requested = quantum_attos;
		quant.m_actual = std::max(quantum_attos, m_quantum_minimum);
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
	for (emu_timer *timer = m_active_timers.head(); timer != nullptr; timer = timer->next())
		timer->dump();
	for (emu_timer *timer = m_inactive_timers.head(); timer != nullptr; timer = timer->next())
		timer->dump();
	machine().logerror("=============================================\n");
}
