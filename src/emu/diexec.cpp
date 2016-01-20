// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    diexec.c

    Device execution interfaces.

***************************************************************************/

#include "emu.h"
#include "debugger.h"


//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define VERBOSE 0

#define LOG(x)  do { if (VERBOSE) m_execute->device().logerror x; } while (0)

#define TEMPLOG 0



//**************************************************************************
//  CONSTANTS
//**************************************************************************

const int TRIGGER_INT           = -2000;
const int TRIGGER_SUSPENDTIME   = -4000;



//**************************************************************************
//  EXECUTING DEVICE CONFIG
//**************************************************************************

//-------------------------------------------------
//  device_execute_interface - constructor
//-------------------------------------------------

device_execute_interface::device_execute_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "execute"),
		m_disabled(false),
		m_timed_interrupt_period(attotime::zero),
		m_is_octal(false),
		m_nextexec(nullptr),
		m_timedint_timer(nullptr),
		m_profiler(PROFILER_IDLE),
		m_icountptr(nullptr),
		m_cycles_running(0),
		m_cycles_stolen(0),
		m_suspend(0),
		m_nextsuspend(0),
		m_eatcycles(0),
		m_nexteatcycles(0),
		m_trigger(0),
		m_inttrigger(0),
		m_totalcycles(0),
		m_divisor(0),
		m_divshift(0),
		m_cycles_per_second(0),
		m_attoseconds_per_cycle(0)
{
	memset(&m_localtime, 0, sizeof(m_localtime));

	// configure the fast accessor
	device.m_execute = this;
}


//-------------------------------------------------
//  device_execute_interface - destructor
//-------------------------------------------------

device_execute_interface::~device_execute_interface()
{
}


//-------------------------------------------------
//  static_set_disable - configuration helper to
//  set the disabled state of a device
//-------------------------------------------------

void device_execute_interface::static_set_disable(device_t &device)
{
	device_execute_interface *exec;
	if (!device.interface(exec))
		throw emu_fatalerror("MCFG_DEVICE_DISABLE called on device '%s' with no execute interface", device.tag().c_str());
	exec->m_disabled = true;
}


//-------------------------------------------------
//  static_set_vblank_int - configuration helper
//  to set up VBLANK interrupts on the device
//-------------------------------------------------

void device_execute_interface::static_set_vblank_int(device_t &device, device_interrupt_delegate function, std::string tag, int rate)
{
	device_execute_interface *exec;
	if (!device.interface(exec))
		throw emu_fatalerror("MCFG_DEVICE_VBLANK_INT called on device '%s' with no execute interface", device.tag().c_str());
	exec->m_vblank_interrupt = function;
	exec->m_vblank_interrupt_screen = tag;
}


//-------------------------------------------------
//  static_set_periodic_int - configuration helper
//  to set up periodic interrupts on the device
//-------------------------------------------------

void device_execute_interface::static_set_periodic_int(device_t &device, device_interrupt_delegate function, const attotime &rate)
{
	device_execute_interface *exec;
	if (!device.interface(exec))
		throw emu_fatalerror("MCFG_DEVICE_PERIODIC_INT called on device '%s' with no execute interface", device.tag().c_str());
	exec->m_timed_interrupt = function;
	exec->m_timed_interrupt_period = rate;
}


//-------------------------------------------------
//  set_irq_acknowledge_callback - configuration helper
//  to setup callback for IRQ acknowledge
//-------------------------------------------------

void device_execute_interface::static_set_irq_acknowledge_callback(device_t &device, device_irq_acknowledge_delegate callback)
{
	device_execute_interface *exec;
	if (!device.interface(exec))
		throw emu_fatalerror("MCFG_DEVICE_IRQ_ACKNOWLEDGE called on device '%s' with no execute interface", device.tag().c_str());
	exec->m_driver_irq = callback;
}


//-------------------------------------------------
//  executing - return true if this device is
//  within its execute function
//-------------------------------------------------

bool device_execute_interface::executing() const
{
	return (this == m_scheduler->currently_executing());
}


//-------------------------------------------------
//  cycles_remaining - return the number of cycles
//  remaining in this timeslice
//-------------------------------------------------

INT32 device_execute_interface::cycles_remaining() const
{
	return executing() ? *m_icountptr : 0;
}


//-------------------------------------------------
//  eat_cycles - safely eats cycles so we don't
//  cross a timeslice boundary
//-------------------------------------------------

void device_execute_interface::eat_cycles(int cycles)
{
	// ignore if not the executing device
	if (!executing())
		return;

	// clamp cycles to the icount and update
	if (cycles > *m_icountptr)
		cycles = *m_icountptr;
	*m_icountptr -= cycles;
}


//-------------------------------------------------
//  adjust_icount - apply a +/- to the current
//  icount
//-------------------------------------------------

void device_execute_interface::adjust_icount(int delta)
{
	// ignore if not the executing device
	if (!executing())
		return;

	// apply the delta directly
	*m_icountptr += delta;
}


//-------------------------------------------------
//  abort_timeslice - abort execution for the
//  current timeslice, allowing other devices to
//  run before we run again
//-------------------------------------------------

void device_execute_interface::abort_timeslice()
{
	// ignore if not the executing device
	if (!executing())
		return;

	// swallow the remaining cycles
	if (m_icountptr != nullptr)
	{
		int delta = *m_icountptr;
		m_cycles_stolen += delta;
		m_cycles_running -= delta;
		*m_icountptr -= delta;
	}
}


//-------------------------------------------------
//  suspend_resume_changed
//-------------------------------------------------

void device_execute_interface::suspend_resume_changed()
{
	// inform the scheduler
	m_scheduler->suspend_resume_changed();

	// if we're active, synchronize
	abort_timeslice();
}


//-------------------------------------------------
//  suspend - set a suspend reason for this device
//-------------------------------------------------

void device_execute_interface::suspend(UINT32 reason, bool eatcycles)
{
if (TEMPLOG) printf("suspend %s (%X)\n", device().tag().c_str(), reason);
	// set the suspend reason and eat cycles flag
	m_nextsuspend |= reason;
	m_nexteatcycles = eatcycles;
	suspend_resume_changed();
}


//-------------------------------------------------
//  resume - clear a suspend reason for this
//  device
//-------------------------------------------------

void device_execute_interface::resume(UINT32 reason)
{
if (TEMPLOG) printf("resume %s (%X)\n", device().tag().c_str(), reason);
	// clear the suspend reason and eat cycles flag
	m_nextsuspend &= ~reason;
	suspend_resume_changed();
}


//-------------------------------------------------
//  spin_until_time - burn cycles for a specific
//  period of time
//-------------------------------------------------

void device_execute_interface::spin_until_time(const attotime &duration)
{
	static int timetrig = 0;

	// suspend until the given trigger fires
	suspend_until_trigger(TRIGGER_SUSPENDTIME + timetrig, true);

	// then set a timer for it
	m_scheduler->timer_set(duration, FUNC(static_timed_trigger_callback), TRIGGER_SUSPENDTIME + timetrig, this);
	timetrig = (timetrig + 1) % 256;
}


//-------------------------------------------------
//  suspend_until_trigger - suspend execution
//  until the given trigger fires
//-------------------------------------------------

void device_execute_interface::suspend_until_trigger(int trigid, bool eatcycles)
{
	// suspend the device immediately if it's not already
	suspend(SUSPEND_REASON_TRIGGER, eatcycles);

	// set the trigger
	m_trigger = trigid;
}


//-------------------------------------------------
//  trigger - respond to a trigger event
//-------------------------------------------------

void device_execute_interface::trigger(int trigid)
{
	// if we're executing, for an immediate abort
	abort_timeslice();

	// see if this is a matching trigger
	if ((m_nextsuspend & SUSPEND_REASON_TRIGGER) != 0 && m_trigger == trigid)
	{
		resume(SUSPEND_REASON_TRIGGER);
		m_trigger = 0;
	}
}


//-------------------------------------------------
//  local_time - returns the current local time
//  for a device
//-------------------------------------------------

attotime device_execute_interface::local_time() const
{
	// if we're active, add in the time from the current slice
	if (executing())
	{
		assert(m_cycles_running >= *m_icountptr);
		int cycles = m_cycles_running - *m_icountptr;
		return m_localtime + cycles_to_attotime(cycles);
	}
	return m_localtime;
}


//-------------------------------------------------
//  total_cycles - return the total number of
//  cycles executed on this device
//-------------------------------------------------

UINT64 device_execute_interface::total_cycles() const
{
	if (executing())
	{
		assert(m_cycles_running >= *m_icountptr);
		return m_totalcycles + m_cycles_running - *m_icountptr;
	}
	else
		return m_totalcycles;
}


//-------------------------------------------------
//  execute_clocks_to_cycles - convert the number
//  of clocks to cycles, rounding down if necessary
//-------------------------------------------------

UINT64 device_execute_interface::execute_clocks_to_cycles(UINT64 clocks) const
{
	return clocks;
}


//-------------------------------------------------
//  execute_cycles_to_clocks - convert the number
//  of cycles to clocks, rounding down if necessary
//-------------------------------------------------

UINT64 device_execute_interface::execute_cycles_to_clocks(UINT64 cycles) const
{
	return cycles;
}


//-------------------------------------------------
//  execute_min_cycles - return the smallest number
//  of cycles that a single instruction or
//  operation can take
//-------------------------------------------------

UINT32 device_execute_interface::execute_min_cycles() const
{
	return 1;
}


//-------------------------------------------------
//  execute_max_cycles - return the maximum number
//  of cycles that a single instruction or
//  operation can take
//-------------------------------------------------

UINT32 device_execute_interface::execute_max_cycles() const
{
	return 1;
}


//-------------------------------------------------
//  execute_input_lines - return the total number
//  of input lines for the device
//-------------------------------------------------

UINT32 device_execute_interface::execute_input_lines() const
{
	return 0;
}


//-------------------------------------------------
//  execute_default_irq_vector - return the default
//  IRQ vector when an acknowledge is processed
//-------------------------------------------------

UINT32 device_execute_interface::execute_default_irq_vector() const
{
	return 0;
}


//-------------------------------------------------
//  execute_burn - called after we consume a bunch
//  of cycles for artifical reasons (such as
//  spinning devices for performance optimization)
//-------------------------------------------------

void device_execute_interface::execute_burn(INT32 cycles)
{
	// by default, do nothing
}


//-------------------------------------------------
//  execute_set_input - called when a synchronized
//  input is changed
//-------------------------------------------------

void device_execute_interface::execute_set_input(int linenum, int state)
{
	// by default, do nothing
}


//-------------------------------------------------
//  interface_validity_check - validation for a
//  device after the configuration has been
//  constructed
//-------------------------------------------------

void device_execute_interface::interface_validity_check(validity_checker &valid) const
{
	// validate the interrupts
	if (!m_vblank_interrupt.isnull())
	{
		screen_device_iterator iter(device().mconfig().root_device());
		if (iter.first() == nullptr)
			osd_printf_error("VBLANK interrupt specified, but the driver is screenless\n");
		else if (!m_vblank_interrupt_screen.empty() && device().siblingdevice(m_vblank_interrupt_screen) == nullptr)
			osd_printf_error("VBLANK interrupt references a non-existant screen tag '%s'\n", m_vblank_interrupt_screen.c_str());
	}

	if (!m_timed_interrupt.isnull() && m_timed_interrupt_period == attotime::zero)
		osd_printf_error("Timed interrupt handler specified with 0 period\n");
	else if (m_timed_interrupt.isnull() && m_timed_interrupt_period != attotime::zero)
		osd_printf_error("No timer interrupt handler specified, but has a non-0 period given\n");
}


//-------------------------------------------------
//  interface_pre_start - work to be done prior to
//  actually starting a device
//-------------------------------------------------

void device_execute_interface::interface_pre_start()
{
	m_scheduler = &device().machine().scheduler();

	// bind delegates
	m_vblank_interrupt.bind_relative_to(*device().owner());
	m_timed_interrupt.bind_relative_to(*device().owner());
	m_driver_irq.bind_relative_to(*device().owner());

	// fill in the initial states
	device_iterator iter(device().machine().root_device());
	int index = iter.indexof(*this);
	m_suspend = SUSPEND_REASON_RESET;
	m_profiler = profile_type(index + PROFILER_DEVICE_FIRST);
	m_inttrigger = index + TRIGGER_INT;

	// allocate timers if we need them
	if (m_timed_interrupt_period != attotime::zero)
		m_timedint_timer = m_scheduler->timer_alloc(FUNC(static_trigger_periodic_interrupt), (void *)this);
}


//-------------------------------------------------
//  interface_post_start - work to be done after
//  actually starting a device
//-------------------------------------------------

void device_execute_interface::interface_post_start()
{
	// make sure somebody set us up the icount
	assert_always(m_icountptr != nullptr, "m_icountptr never initialized!");

	// register for save states
	device().save_item(NAME(m_suspend));
	device().save_item(NAME(m_nextsuspend));
	device().save_item(NAME(m_eatcycles));
	device().save_item(NAME(m_nexteatcycles));
	device().save_item(NAME(m_trigger));
	device().save_item(NAME(m_totalcycles));
	device().save_item(NAME(m_localtime));

	// fill in the input states and IRQ callback information
	for (int line = 0; line < ARRAY_LENGTH(m_input); line++)
		m_input[line].start(this, line);
}


//-------------------------------------------------
//  interface_pre_reset - work to be done prior to
//  actually resetting a device
//-------------------------------------------------

void device_execute_interface::interface_pre_reset()
{
	// reset the total number of cycles
	m_totalcycles = 0;

	// enable all devices (except for disabled devices)
	if (!disabled())
		resume(SUSPEND_ANY_REASON);
	else
		suspend(SUSPEND_REASON_DISABLE, true);
}


//-------------------------------------------------
//  interface_post_reset - work to be done after a
//  device is reset
//-------------------------------------------------

void device_execute_interface::interface_post_reset()
{
	// reset the interrupt vectors and queues
	for (auto & elem : m_input)
		elem.reset();

	// reconfingure VBLANK interrupts
	if (!m_vblank_interrupt_screen.empty())
	{
		// get the screen that will trigger the VBLANK
		screen_device *screen = downcast<screen_device *>(device().machine().device(device().siblingtag(m_vblank_interrupt_screen)));

		assert(screen != nullptr);
		screen->register_vblank_callback(vblank_state_delegate(FUNC(device_execute_interface::on_vblank), this));
	}

	// reconfigure periodic interrupts
	if (m_timed_interrupt_period != attotime::zero)
	{
		attotime timedint_period = m_timed_interrupt_period;
		assert(m_timedint_timer != nullptr);
		m_timedint_timer->adjust(timedint_period, 0, timedint_period);
	}
}


//-------------------------------------------------
//  interface_clock_changed - recomputes clock
//  information for this device
//-------------------------------------------------

void device_execute_interface::interface_clock_changed()
{
	// a clock of zero disables the device
	if (device().clock() == 0)
	{
		suspend(SUSPEND_REASON_CLOCK, true);
		return;
	}

	// if we were suspended because we had no clock, enable us now
	if (suspended(SUSPEND_REASON_CLOCK))
		resume(SUSPEND_REASON_CLOCK);

	// recompute cps and spc
	m_cycles_per_second = clocks_to_cycles(device().clock());
	m_attoseconds_per_cycle = HZ_TO_ATTOSECONDS(m_cycles_per_second);

	// update the device's divisor
	INT64 attos = m_attoseconds_per_cycle;
	m_divshift = 0;
	while (attos >= (1UL << 31))
	{
		m_divshift++;
		attos >>= 1;
	}
	m_divisor = attos;

	// re-compute the perfect interleave factor
	m_scheduler->compute_perfect_interleave();
}


//-------------------------------------------------
//  standard_irq_callback_member - IRQ acknowledge
//  callback; handles HOLD_LINE case and signals
//  to the debugger
//-------------------------------------------------

IRQ_CALLBACK_MEMBER( device_execute_interface::standard_irq_callback_member )
{
	return device.execute().standard_irq_callback(irqline);
}

int device_execute_interface::standard_irq_callback(int irqline)
{
	// get the default vector and acknowledge the interrupt if needed
	int vector = m_input[irqline].default_irq_callback();

	if (VERBOSE) device().logerror("standard_irq_callback('%s', %d) $%04x\n", device().tag().c_str(), irqline, vector);

	// if there's a driver callback, run it to get the vector
	if (!m_driver_irq.isnull())
		vector = m_driver_irq(device(),irqline);

	// notify the debugger
	debugger_interrupt_hook(&device(), irqline);
	return vector;
}


//-------------------------------------------------
//  minimum_quantum - return the minimum quantum
//  required for this device
//-------------------------------------------------

attoseconds_t device_execute_interface::minimum_quantum() const
{
	// if we don't have a clock, return a huge factor
	if (device().clock() == 0)
		return ATTOSECONDS_PER_SECOND - 1;

	// if we don't have the quantum time, compute it
	attoseconds_t basetick = m_attoseconds_per_cycle;
	if (basetick == 0)
		basetick = HZ_TO_ATTOSECONDS(clocks_to_cycles(device().clock()));

	// apply the minimum cycle count
	return basetick * min_cycles();
}


//-------------------------------------------------
//  static_timed_trigger_callback - signal a timed
//  trigger
//-------------------------------------------------

TIMER_CALLBACK( device_execute_interface::static_timed_trigger_callback )
{
	device_execute_interface *device = reinterpret_cast<device_execute_interface *>(ptr);
	device->trigger(param);
}


//-------------------------------------------------
//  on_vblank - calls any external callbacks
//  for this screen
//-------------------------------------------------

void device_execute_interface::on_vblank(screen_device &screen, bool vblank_state)
{
	// ignore VBLANK end
	if (!vblank_state)
		return;

	// generate the interrupt callback
	if (!suspended(SUSPEND_REASON_HALT | SUSPEND_REASON_RESET | SUSPEND_REASON_DISABLE | SUSPEND_REASON_CLOCK))
	{
		if (!m_vblank_interrupt.isnull())
			m_vblank_interrupt(device());
	}
}


//-------------------------------------------------
//  static_trigger_periodic_interrupt - timer
//  callback for timed interrupts
//-------------------------------------------------

TIMER_CALLBACK( device_execute_interface::static_trigger_periodic_interrupt )
{
	reinterpret_cast<device_execute_interface *>(ptr)->trigger_periodic_interrupt();
}

void device_execute_interface::trigger_periodic_interrupt()
{
	// bail if there is no routine
	if (!suspended(SUSPEND_REASON_HALT | SUSPEND_REASON_RESET | SUSPEND_REASON_DISABLE | SUSPEND_REASON_CLOCK))
	{
		if (!m_timed_interrupt.isnull())
			m_timed_interrupt(device());
	}
}



//**************************************************************************
//  DEVICE INPUT
//**************************************************************************

//-------------------------------------------------
//  device_input - constructor
//-------------------------------------------------

device_execute_interface::device_input::device_input()
	: m_execute(nullptr),
		m_linenum(0),
		m_stored_vector(0),
		m_curvector(0),
		m_curstate(CLEAR_LINE),
		m_qindex(0)
{
	memset(m_queue, 0, sizeof(m_queue));
}


//-------------------------------------------------
//  start - called by interface_pre_start so we
//  can set ourselves up
//-------------------------------------------------

void device_execute_interface::device_input::start(device_execute_interface *execute, int linenum)
{
	m_execute = execute;
	m_linenum = linenum;

	reset();

	device_t &device = m_execute->device();
	device.save_item(NAME(m_stored_vector), m_linenum);
	device.save_item(NAME(m_curvector), m_linenum);
	device.save_item(NAME(m_curstate), m_linenum);
}


//-------------------------------------------------
//  reset - reset our input states
//-------------------------------------------------

void device_execute_interface::device_input::reset()
{
	m_curvector = m_stored_vector = m_execute->default_irq_vector();
	m_qindex = 0;
}


//-------------------------------------------------
//  set_state_synced - enqueue an event for later
//  execution via timer
//-------------------------------------------------

void device_execute_interface::device_input::set_state_synced(int state, int vector)
{
	LOG(("set_state_synced('%s',%d,%d,%02x)\n", m_execute->device().tag().c_str(), m_linenum, state, vector));

if (TEMPLOG) printf("setline(%s,%d,%d,%d)\n", m_execute->device().tag().c_str(), m_linenum, state, (vector == USE_STORED_VECTOR) ? 0 : vector);
	assert(state == ASSERT_LINE || state == HOLD_LINE || state == CLEAR_LINE || state == PULSE_LINE);

	// treat PULSE_LINE as ASSERT+CLEAR
	if (state == PULSE_LINE)
	{
		// catch errors where people use PULSE_LINE for devices that don't support it
		if (m_linenum != INPUT_LINE_NMI && m_linenum != INPUT_LINE_RESET)
			throw emu_fatalerror("device '%s': PULSE_LINE can only be used for NMI and RESET lines\n", m_execute->device().tag().c_str());

		set_state_synced(ASSERT_LINE, vector);
		set_state_synced(CLEAR_LINE, vector);
		return;
	}

	// if we're full of events, flush the queue and log a message
	int event_index = m_qindex++;
	if (event_index >= ARRAY_LENGTH(m_queue))
	{
		m_qindex--;
		empty_event_queue();
		event_index = m_qindex++;
		m_execute->device().logerror("Exceeded pending input line event queue on device '%s'!\n", m_execute->device().tag().c_str());
	}

	// enqueue the event
	if (event_index < ARRAY_LENGTH(m_queue))
	{
		if (vector == USE_STORED_VECTOR)
			vector = m_stored_vector;
		m_queue[event_index] = (state & 0xff) | (vector << 8);

		// if this is the first one, set the timer
		if (event_index == 0)
			m_execute->scheduler().synchronize(FUNC(static_empty_event_queue), 0, (void *)this);
	}
}


//-------------------------------------------------
//  empty_event_queue - empty our event queue
//-------------------------------------------------

TIMER_CALLBACK( device_execute_interface::device_input::static_empty_event_queue )
{
	reinterpret_cast<device_input *>(ptr)->empty_event_queue();
}

void device_execute_interface::device_input::empty_event_queue()
{
if (TEMPLOG) printf("empty_queue(%s,%d,%d)\n", m_execute->device().tag().c_str(), m_linenum, m_qindex);
	// loop over all events
	for (int curevent = 0; curevent < m_qindex; curevent++)
	{
		INT32 input_event = m_queue[curevent];

		// set the input line state and vector
		m_curstate = input_event & 0xff;
		m_curvector = input_event >> 8;
if (TEMPLOG) printf(" (%d,%d)\n", m_curstate, m_curvector);

		assert(m_curstate == ASSERT_LINE || m_curstate == HOLD_LINE || m_curstate == CLEAR_LINE);

		// special case: RESET
		if (m_linenum == INPUT_LINE_RESET)
		{
			// if we're asserting the line, just halt the device
			if (m_curstate == ASSERT_LINE)
				m_execute->suspend(SUSPEND_REASON_RESET, true);

			// if we're clearing the line that was previously asserted, reset the device
			else if (m_execute->suspended(SUSPEND_REASON_RESET))
			{
				m_execute->device().reset();
				m_execute->resume(SUSPEND_REASON_RESET);
			}
		}

		// special case: HALT
		else if (m_linenum == INPUT_LINE_HALT)
		{
			// if asserting, halt the device
			if (m_curstate == ASSERT_LINE)
				m_execute->suspend(SUSPEND_REASON_HALT, true);

			// if clearing, unhalt the device
			else if (m_curstate == CLEAR_LINE)
				m_execute->resume(SUSPEND_REASON_HALT);
		}

		// all other cases
		else
		{
			// switch off the requested state
			switch (m_curstate)
			{
				case HOLD_LINE:
				case ASSERT_LINE:
					m_execute->execute_set_input(m_linenum, ASSERT_LINE);
					break;

				case CLEAR_LINE:
					m_execute->execute_set_input(m_linenum, CLEAR_LINE);
					break;

				default:
					m_execute->device().logerror("empty_event_queue device '%s', line %d, unknown state %d\n", m_execute->device().tag().c_str(), m_linenum, m_curstate);
					break;
			}

			// generate a trigger to unsuspend any devices waiting on the interrupt
			if (m_curstate != CLEAR_LINE)
				m_execute->signal_interrupt_trigger();
		}
	}

	// reset counter
	m_qindex = 0;
}


//-------------------------------------------------
//  default_irq_callback - the default IRQ
//  callback for this input line
//-------------------------------------------------

int device_execute_interface::device_input::default_irq_callback()
{
	int vector = m_curvector;

	// if the IRQ state is HOLD_LINE, clear it
	if (m_curstate == HOLD_LINE)
	{
		LOG(("->set_irq_line('%s',%d,%d)\n", m_execute->device().tag().c_str(), m_linenum, CLEAR_LINE));
		m_execute->execute_set_input(m_linenum, CLEAR_LINE);
		m_curstate = CLEAR_LINE;
	}
	return vector;
}
