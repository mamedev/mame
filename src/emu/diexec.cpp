// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    diexec.cpp

    Device execution interfaces.

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "screen.h"


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
	: device_interface(device, "execute")
	, m_scheduler(nullptr)
	, m_disabled(false)
	, m_vblank_interrupt(device)
	, m_vblank_interrupt_screen(nullptr)
	, m_timed_interrupt(device)
	, m_timed_interrupt_period(attotime::zero)
	, m_nextexec(nullptr)
	, m_driver_irq(device)
	, m_timedint_timer(nullptr)
	, m_profiler(PROFILER_IDLE)
	, m_icountptr(nullptr)
	, m_cycles_running(0)
	, m_cycles_stolen(0)
	, m_suspend(0)
	, m_nextsuspend(0)
	, m_eatcycles(0)
	, m_nexteatcycles(0)
	, m_trigger(0)
	, m_inttrigger(0)
	, m_totalcycles(0)
	, m_divisor(0)
	, m_divshift(0)
	, m_cycles_per_second(0)
	, m_attoseconds_per_cycle(0)
	, m_spin_end_timer(nullptr)
{
	// configure the fast accessor
	assert(!device.interfaces().m_execute);
	device.interfaces().m_execute = this;
}


//-------------------------------------------------
//  device_execute_interface - destructor
//-------------------------------------------------

device_execute_interface::~device_execute_interface()
{
}


//-------------------------------------------------
//  abort_timeslice - abort execution for the
//  current timeslice, allowing other devices to
//  run before we run again
//-------------------------------------------------

void device_execute_interface::abort_timeslice() noexcept
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

void device_execute_interface::suspend(u32 reason, bool eatcycles)
{
if (TEMPLOG) printf("suspend %s (%X)\n", device().tag(), reason);
	// set the suspend reason and eat cycles flag
	m_nextsuspend |= reason;
	m_nexteatcycles = eatcycles;
	suspend_resume_changed();
}


//-------------------------------------------------
//  resume - clear a suspend reason for this
//  device
//-------------------------------------------------

void device_execute_interface::resume(u32 reason)
{
if (TEMPLOG) printf("resume %s (%X)\n", device().tag(), reason);
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
	m_spin_end_timer->adjust(duration, TRIGGER_SUSPENDTIME + timetrig);
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

attotime device_execute_interface::local_time() const noexcept
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

u64 device_execute_interface::total_cycles() const noexcept
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

u64 device_execute_interface::execute_clocks_to_cycles(u64 clocks) const noexcept
{
	return clocks;
}


//-------------------------------------------------
//  execute_cycles_to_clocks - convert the number
//  of cycles to clocks, rounding down if necessary
//-------------------------------------------------

u64 device_execute_interface::execute_cycles_to_clocks(u64 cycles) const noexcept
{
	return cycles;
}


//-------------------------------------------------
//  execute_min_cycles - return the smallest number
//  of cycles that a single instruction or
//  operation can take
//-------------------------------------------------

u32 device_execute_interface::execute_min_cycles() const noexcept
{
	return 1;
}


//-------------------------------------------------
//  execute_max_cycles - return the maximum number
//  of cycles that a single instruction or
//  operation can take
//-------------------------------------------------

u32 device_execute_interface::execute_max_cycles() const noexcept
{
	return 1;
}


//-------------------------------------------------
//  execute_default_irq_vector - return the default
//  IRQ vector when an acknowledge is processed
//-------------------------------------------------

u32 device_execute_interface::execute_default_irq_vector(int linenum) const noexcept
{
	return 0;
}


//-------------------------------------------------
//  execute_input_edge_triggered - return true if
//  the input line has an asynchronous edge trigger
//-------------------------------------------------

bool device_execute_interface::execute_input_edge_triggered(int linenum) const noexcept
{
	return false;
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
		screen_device_enumerator iter(device().mconfig().root_device());
		if (iter.first() == nullptr)
			osd_printf_error("VBLANK interrupt specified, but the driver is screenless\n");
		else if (m_vblank_interrupt_screen != nullptr && device().siblingdevice(m_vblank_interrupt_screen) == nullptr)
			osd_printf_error("VBLANK interrupt references a nonexistent screen tag '%s'\n", m_vblank_interrupt_screen);
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
	m_vblank_interrupt.resolve();
	m_timed_interrupt.resolve();
	m_driver_irq.resolve();

	// fill in the initial states
	int const index = device_enumerator(device().machine().root_device()).indexof(*this);
	m_suspend = SUSPEND_REASON_RESET;
	m_profiler = profile_type(index + PROFILER_DEVICE_FIRST);
	m_inttrigger = index + TRIGGER_INT;

	// allocate a timed-interrupt timer if we need it
	if (m_timed_interrupt_period != attotime::zero)
		m_timedint_timer = m_scheduler->timer_alloc(timer_expired_delegate(FUNC(device_execute_interface::trigger_periodic_interrupt), this));

	// allocate a timer for triggering the end of spin-until conditions
	m_spin_end_timer = m_scheduler->timer_alloc(timer_expired_delegate(FUNC(device_execute_interface::timed_trigger_callback), this));

	// allocate input-pulse timers if we have input lines
	for (u32 i = 0; i < MAX_INPUT_LINES; i++)
		m_pulse_end_timers[i] = m_scheduler->timer_alloc(timer_expired_delegate(FUNC(device_execute_interface::irq_pulse_clear), this));
}


//-------------------------------------------------
//  interface_post_start - work to be done after
//  actually starting a device
//-------------------------------------------------

void device_execute_interface::interface_post_start()
{
	// make sure somebody set us up the icount
	if (!m_icountptr)
		throw emu_fatalerror("m_icountptr never initialized!");

	// register for save states
	device().save_item(NAME(m_suspend));
	device().save_item(NAME(m_nextsuspend));
	device().save_item(NAME(m_eatcycles));
	device().save_item(NAME(m_nexteatcycles));
	device().save_item(NAME(m_trigger));
	device().save_item(NAME(m_totalcycles));
	device().save_item(NAME(m_localtime));

	// it's more efficient and causes less clutter to save these this way
	device().save_item(STRUCT_MEMBER(m_input, m_stored_vector));
	device().save_item(STRUCT_MEMBER(m_input, m_curvector));
	device().save_item(STRUCT_MEMBER(m_input, m_curstate));

	// fill in the input states and IRQ callback information
	for (int line = 0; line < std::size(m_input); line++)
		m_input[line].start(*this, line);
}


//-------------------------------------------------
//  interface_pre_reset - work to be done prior to
//  actually resetting a device
//-------------------------------------------------

void device_execute_interface::interface_pre_reset()
{
	// reset the total number of cycles
	m_totalcycles = 0;

	// enable all devices (except for disabled and unclocked devices)
	if (disabled())
		suspend(SUSPEND_REASON_DISABLE, true);
	else if (device().clock() != 0)
		resume(SUSPEND_ANY_REASON);
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
	if (m_vblank_interrupt_screen != nullptr)
	{
		// get the screen that will trigger the VBLANK
		screen_device * screen = device().siblingdevice<screen_device>(m_vblank_interrupt_screen);

		assert(screen != nullptr);
		screen->register_vblank_callback(vblank_state_delegate(&device_execute_interface::on_vblank, this));
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

void device_execute_interface::interface_clock_changed(bool sync_on_new_clock_domain)
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

	// resynchronize the localtime to the clock domain when asked to
	if (sync_on_new_clock_domain)
		m_localtime = attotime::from_ticks(m_localtime.as_ticks(device().clock())+1, device().clock());

	// update the device's divisor
	s64 attos = m_attoseconds_per_cycle;
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
//  standard_irq_callback - IRQ acknowledge
//  callback; handles HOLD_LINE case and signals
//  to the debugger
//-------------------------------------------------

int device_execute_interface::standard_irq_callback(int irqline, offs_t pc)
{
	// get the default vector and acknowledge the interrupt if needed
	int vector = m_input[irqline].default_irq_callback();

	if (VERBOSE) device().logerror("standard_irq_callback('%s', %d) $%04x\n", device().tag(), irqline, vector);

	// if there's a driver callback, run it to get the vector
	if (!m_driver_irq.isnull())
		vector = m_driver_irq(device(), irqline);

	// notify the debugger
	if (device().machine().debug_flags & DEBUG_FLAG_ENABLED)
		device().debug()->interrupt_hook(irqline, pc);

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
//  trigger_periodic_interrupt - timer
//  callback for timed interrupts
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(device_execute_interface::trigger_periodic_interrupt)
{
	// bail if there is no routine
	if (!suspended(SUSPEND_REASON_HALT | SUSPEND_REASON_RESET | SUSPEND_REASON_DISABLE | SUSPEND_REASON_CLOCK))
	{
		if (!m_timed_interrupt.isnull())
			m_timed_interrupt(device());
	}
}


//-------------------------------------------------
//  pulse_input_line - "pulse" an input line by
//  asserting it and then clearing it later
//-------------------------------------------------

void device_execute_interface::pulse_input_line(int irqline, const attotime &duration)
{
	// treat instantaneous pulses as ASSERT+CLEAR
	if (duration == attotime::zero)
	{
		if (irqline != INPUT_LINE_RESET && !input_edge_triggered(irqline))
			throw emu_fatalerror("device '%s': zero-width pulse is not allowed for input line %d\n", device().tag(), irqline);

		set_input_line(irqline, ASSERT_LINE);
		set_input_line(irqline, CLEAR_LINE);
	}
	else
	{
		set_input_line(irqline, ASSERT_LINE);

		attotime target_time = local_time() + duration;
		m_pulse_end_timers[irqline]->adjust(target_time - m_scheduler->time(), irqline);
	}
}



//**************************************************************************
//  DEVICE INPUT
//**************************************************************************

//-------------------------------------------------
//  device_input - constructor
//-------------------------------------------------

device_execute_interface::device_input::device_input()
	: m_execute(nullptr)
	, m_linenum(0)
	, m_stored_vector(0)
	, m_curvector(0)
	, m_curstate(CLEAR_LINE)
	, m_qindex(0)
{
	std::fill(std::begin(m_queue), std::end(m_queue), 0);
}


//-------------------------------------------------
//  start - called by interface_pre_start so we
//  can set ourselves up
//-------------------------------------------------

void device_execute_interface::device_input::start(device_execute_interface &execute, int linenum)
{
	m_execute = &execute;
	m_linenum = linenum;

	reset();
}


//-------------------------------------------------
//  reset - reset our input states
//-------------------------------------------------

void device_execute_interface::device_input::reset()
{
	m_curvector = m_stored_vector = m_execute->default_irq_vector(m_linenum);
}


//-------------------------------------------------
//  set_state_synced - enqueue an event for later
//  execution via timer
//-------------------------------------------------

void device_execute_interface::device_input::set_state_synced(int state, int vector)
{
	LOG(("set_state_synced('%s',%d,%d,%02x)\n", m_execute->device().tag(), m_linenum, state, vector));

if (TEMPLOG) printf("setline(%s,%d,%d,%d)\n", m_execute->device().tag(), m_linenum, state, (vector == USE_STORED_VECTOR) ? 0 : vector);
	assert(state == ASSERT_LINE || state == HOLD_LINE || state == CLEAR_LINE);

	// if we're full of events, flush the queue and log a message
	int event_index = m_qindex++;
	if (event_index >= std::size(m_queue))
	{
		m_qindex--;
		empty_event_queue(0);
		event_index = m_qindex++;
		m_execute->device().logerror("Exceeded pending input line event queue on device '%s'!\n", m_execute->device().tag());
	}

	// enqueue the event
	if (event_index < std::size(m_queue))
	{
		if (vector == USE_STORED_VECTOR)
			vector = m_stored_vector;
		m_queue[event_index] = (state & 0xff) | (vector << 8);

		// if this is the first one, set the timer
		if (event_index == 0)
			m_execute->scheduler().synchronize(timer_expired_delegate(FUNC(device_execute_interface::device_input::empty_event_queue),this));
	}
}


//-------------------------------------------------
//  empty_event_queue - empty our event queue
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(device_execute_interface::device_input::empty_event_queue)
{
if (TEMPLOG) printf("empty_queue(%s,%d,%d)\n", m_execute->device().tag(), m_linenum, m_qindex);
	// loop over all events
	for (int curevent = 0; curevent < m_qindex; curevent++)
	{
		s32 input_event = m_queue[curevent];

		// set the input line state and vector
		m_curstate = input_event & 0xff;
		m_curvector = input_event >> 8;
if (TEMPLOG) printf(" (%d,%d)\n", m_curstate, m_curvector);

		assert(m_curstate == ASSERT_LINE || m_curstate == HOLD_LINE || m_curstate == CLEAR_LINE);

		// special case: RESET
		if (m_linenum == INPUT_LINE_RESET)
		{
			// if we're asserting the line, just halt the device
			// FIXME: outputs of onboard peripherals also need to be deactivated at this time
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
					m_execute->device().logerror("empty_event_queue device '%s', line %d, unknown state %d\n", m_execute->device().tag(), m_linenum, m_curstate);
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
	int const vector = m_curvector;

	// if the IRQ state is HOLD_LINE, clear it
	if (m_curstate == HOLD_LINE)
	{
		LOG(("->set_irq_line('%s',%d,%d)\n", m_execute->device().tag(), m_linenum, CLEAR_LINE));
		m_execute->execute_set_input(m_linenum, CLEAR_LINE);
		m_curstate = CLEAR_LINE;
	}
	return vector;
}
