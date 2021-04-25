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

device_execute_interface::device_execute_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "execute"),
	m_nextexec(nullptr),
	m_icountptr(nullptr),
	m_cycles_per_second(0),
	m_totalcycles(0),
	m_profiler(PROFILER_IDLE),
	m_scheduler(nullptr),
	m_suspend(0),
	m_nextsuspend(0),
	m_trigger(0),
	m_inttrigger(0),
	m_disabled(false),
	m_vblank_interrupt(device),
	m_vblank_interrupt_screen(nullptr),
	m_periodic_interrupt(device),
	m_periodic_interrupt_period(attotime::zero),
	m_driver_irq(device)
{
	m_cycles.combined = 0;

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
		m_set_input_line.call_at(local_time() + duration, irqline, CLEAR_LINE);
	}
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
		m_cycles.separate.stolen += delta;
		scheduler_assert(int(m_cycles.separate.running) >= delta);
		m_cycles.separate.running -= delta;
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

void device_execute_interface::suspend(u32 reason)
{
if (TEMPLOG) printf("suspend %s (%X)\n", device().tag(), reason);
	// set the suspend reason and eat cycles flag
	m_nextsuspend |= reason;
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
	m_timed_trigger.call_after(duration, TRIGGER_SUSPENDTIME + timetrig);
	timetrig = (timetrig + 1) % 256;
}


//-------------------------------------------------
//  suspend_until_trigger - suspend execution
//  until the given trigger fires
//-------------------------------------------------

void device_execute_interface::suspend_until_trigger(int trigid, bool eatcycles)
{
	// suspend the device immediately if it's not already
	suspend(eatcycles ? SUSPEND_REASON_SPIN_TRIGGER : SUSPEND_REASON_YIELD_TRIGGER);

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
	if ((m_nextsuspend & SUSPEND_TRIGGER_REASONS) != 0 && m_trigger == trigid)
	{
		resume(SUSPEND_TRIGGER_REASONS);
		m_trigger = 0;
	}
}


//-------------------------------------------------
//  local_time - returns the current local time
//  for a device
//-------------------------------------------------

attotime device_execute_interface::local_time() noexcept
{
	// if we're active, add in the time from the current slice
	if (executing())
	{
		scheduler_assert(int(m_cycles.separate.running) >= *m_icountptr);
		int cycles = m_cycles.separate.running - *m_icountptr;
		return m_localtime.absolute() + cycles_to_attotime(cycles);
	}
	return m_localtime.absolute();
}


//-------------------------------------------------
//  total_cycles - return the total number of
//  cycles executed on this device
//-------------------------------------------------

u64 device_execute_interface::total_cycles() const noexcept
{
	if (executing())
	{
		scheduler_assert(int(m_cycles.separate.running) >= *m_icountptr);
		return m_totalcycles + m_cycles.separate.running - *m_icountptr;
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
//  execute_input_lines - return the total number
//  of input lines for the device
//-------------------------------------------------

u32 device_execute_interface::execute_input_lines() const noexcept
{
	return 0;
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
//  execute_burn - called after we consume a bunch
//  of cycles for artifical reasons (such as
//  spinning devices for performance optimization)
//-------------------------------------------------

void device_execute_interface::execute_burn(s32 cycles)
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
		screen_device_enumerator iter(device().mconfig().root_device());
		if (iter.first() == nullptr)
			osd_printf_error("VBLANK interrupt specified, but the driver is screenless\n");
		else if (m_vblank_interrupt_screen != nullptr && device().siblingdevice(m_vblank_interrupt_screen) == nullptr)
			osd_printf_error("VBLANK interrupt references a nonexistent screen tag '%s'\n", m_vblank_interrupt_screen);
	}

	if (!m_periodic_interrupt.isnull() && m_periodic_interrupt_period == attotime::zero)
		osd_printf_error("Timed interrupt handler specified with 0 period\n");
	else if (m_periodic_interrupt.isnull() && m_periodic_interrupt_period != attotime::zero)
		osd_printf_error("No timer interrupt handler specified, but has a non-0 period given\n");
}


//-------------------------------------------------
//  interface_pre_start - work to be done prior to
//  actually starting a device
//-------------------------------------------------

void device_execute_interface::interface_pre_start()
{
	m_scheduler = &device().machine().scheduler();
	m_localtime.set_base(m_scheduler->m_basetime.absolute());

	// create execution delegates
	m_run_fast_delegate = execute_delegate(&device_execute_interface::execute_run, this);
	m_run_debug_delegate = execute_delegate(&device_execute_interface::run_debug, this);
	m_suspend_delegate = execute_delegate(&device_execute_interface::run_suspend, this);
	m_run_delegate = m_run_fast_delegate;

	// bind delegates
	m_vblank_interrupt.resolve();
	m_periodic_interrupt.resolve();
	m_driver_irq.resolve();

	// fill in the initial states
	int const index = device_enumerator(device().machine().root_device()).indexof(*this);
	suspend(SUSPEND_REASON_RESET);
	m_profiler = profile_type(index + PROFILER_DEVICE_FIRST);
	m_inttrigger = index + TRIGGER_INT;

	// allocate timers if we need them
	if (m_periodic_interrupt_period != attotime::zero)
		m_periodic_interrupt_timer.init(*this, FUNC(device_execute_interface::periodic_interrupt));

	m_set_input_line.init(*this, FUNC(device_execute_interface::set_input_line));
	m_timed_trigger.init(*this, FUNC(device_execute_interface::trigger));
	m_process_input_event.init(*this, FUNC(device_execute_interface::process_input_event));
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
	device().save_item(NAME(m_trigger));
	device().save_item(NAME(m_totalcycles));
	device().save_item(NAME(m_localtime.m_relative));
	device().save_item(NAME(m_localtime.m_absolute));
	device().save_item(NAME(m_localtime.m_base));

	// it's more efficient and causes less clutter to save these this way
	device().save_item(STRUCT_MEMBER(m_input, m_stored_vector));
	device().save_item(STRUCT_MEMBER(m_input, m_curvector));
	device().save_item(STRUCT_MEMBER(m_input, m_curstate));
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
		suspend(SUSPEND_REASON_DISABLE);
	else if (device().clock() != 0)
		resume(SUSPEND_ANY_REASON);
}


//-------------------------------------------------
//  interface_post_reset - work to be done after a
//  device is reset
//-------------------------------------------------

void device_execute_interface::interface_post_reset()
{
	// reset the interrupt vectors
	for (int linenum = 0; linenum < std::size(m_input); linenum++)
	{
		auto &input = m_input[linenum];
		input.m_curvector = input.m_stored_vector = default_irq_vector(linenum);
	}

	// reconfingure VBLANK interrupts
	if (m_vblank_interrupt_screen != nullptr)
	{
		// get the screen that will trigger the VBLANK
		screen_device * screen = device().siblingdevice<screen_device>(m_vblank_interrupt_screen);

		assert(screen != nullptr);
		screen->register_vblank_callback(vblank_state_delegate(&device_execute_interface::on_vblank, this));
	}

	// reconfigure periodic interrupts
	if (!m_periodic_interrupt_period.is_zero())
		m_periodic_interrupt_timer.adjust(m_periodic_interrupt_period, 0, m_periodic_interrupt_period);
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
		suspend(SUSPEND_REASON_CLOCK);
		m_subseconds_per_cycle = subseconds::max();
		m_cycles_per_second = 0;
		return;
	}

	// if we were suspended because we had no clock, enable us now
	if (suspended(SUSPEND_REASON_CLOCK))
		resume(SUSPEND_REASON_CLOCK);

	// recompute cps and spc
	m_cycles_per_second = clocks_to_cycles(device().clock());
	m_subseconds_per_cycle = subseconds::from_hz(m_cycles_per_second);

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
	auto &input = m_input[irqline];

	// get the default vector and acknowledge the interrupt if needed
	int vector = input.m_curvector;
	if (input.m_curstate == HOLD_LINE)
	{
		execute_set_input(irqline, CLEAR_LINE);
		input.m_curstate = CLEAR_LINE;
	}

	if (VERBOSE) device().logerror("standard_irq_callback('%s', %d) $%04x\n", device().tag(), irqline, vector);

	// if there's a driver callback, run it to get the vector
	if (!m_driver_irq.isnull())
		vector = m_driver_irq(device(), irqline);

	// notify the debugger
	if (debugger_enabled())
		device().debug()->interrupt_hook(irqline);

	return vector;
}


//-------------------------------------------------
//  minimum_quantum - return the minimum quantum
//  required for this device
//-------------------------------------------------

subseconds device_execute_interface::minimum_quantum() const
{
	// if we don't have a clock, return a huge factor
	if (device().clock() == 0)
		return subseconds::max();

	// if we don't have the quantum time, compute it
	subseconds basetick = m_subseconds_per_cycle;
	if (basetick.is_zero())
		basetick = subseconds::from_hz(clocks_to_cycles(device().clock()));

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
//  perodic_interrupt - timer callback for periodic
//  interrupts
//-------------------------------------------------

void device_execute_interface::periodic_interrupt(timer_instance const &timer)
{
	// bail if there is no routine
	if (!suspended(SUSPEND_REASON_HALT | SUSPEND_REASON_RESET | SUSPEND_REASON_DISABLE | SUSPEND_REASON_CLOCK))
		if (!m_periodic_interrupt.isnull())
			m_periodic_interrupt(device());
}


//-------------------------------------------------
//  process_input_event - timer callback to process
//  an input event
//-------------------------------------------------

void device_execute_interface::process_input_event(timer_instance const &timer)
{
	int linenum = timer.param(0);
	assert(linenum < MAX_INPUT_LINES);
	auto &input = m_input[linenum];

	int state = timer.param(1);
	assert(state == ASSERT_LINE || state == HOLD_LINE || state == CLEAR_LINE);

	input.m_curstate = state;
	input.m_curvector = timer.param(2);

	// special case: RESET
	if (linenum == INPUT_LINE_RESET)
	{
		// if we're asserting the line, just halt the device
		// FIXME: outputs of onboard peripherals also need to be deactivated at this time
		if (input.m_curstate == ASSERT_LINE)
			suspend(SUSPEND_REASON_RESET);

		// if we're clearing the line that was previously asserted, reset the device
		else if (suspended(SUSPEND_REASON_RESET))
		{
			device().reset();
			resume(SUSPEND_REASON_RESET);
		}
	}

	// special case: HALT
	else if (linenum == INPUT_LINE_HALT)
	{
		// if asserting, halt the device
		if (input.m_curstate == ASSERT_LINE)
			suspend(SUSPEND_REASON_HALT);

		// if clearing, unhalt the device
		else if (input.m_curstate == CLEAR_LINE)
			resume(SUSPEND_REASON_HALT);
	}

	// all other cases
	else
	{
		// switch off the requested state
		switch (input.m_curstate)
		{
			case HOLD_LINE:
			case ASSERT_LINE:
				execute_set_input(linenum, ASSERT_LINE);
				break;

			case CLEAR_LINE:
				execute_set_input(linenum, CLEAR_LINE);
				break;

			default:
				device().logerror("process_input_event device '%s', line %d, unknown state %d\n", device().tag(), linenum, input.m_curstate);
				break;
		}

		// generate a trigger to unsuspend any devices waiting on the interrupt
		if (input.m_curstate != CLEAR_LINE)
			signal_interrupt_trigger();
	}
}


//-------------------------------------------------
//  run_debug - landing pad for debugging
//-------------------------------------------------

void device_execute_interface::run_debug()
{
	device().debug()->start_hook(m_scheduler->basetime());
	execute_run();
	device().debug()->stop_hook();
}


//-------------------------------------------------
//  run_suspend - landing pad for suspended CPUs
//-------------------------------------------------

void device_execute_interface::run_suspend()
{
	// eat all the cycles unless we're yielding
	if ((m_suspend & SUSPEND_YIELD_REASONS) == 0)
		*m_icountptr = 0;
}
