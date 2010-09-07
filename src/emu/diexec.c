/***************************************************************************

    diexec.c

    Device execution interfaces.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#include "emu.h"
#include "profiler.h"
#include "debugger.h"


//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)

#define TEMPLOG	0



//**************************************************************************
//  CONSTANTS
//**************************************************************************

const int TRIGGER_INT			= -2000;
const int TRIGGER_SUSPENDTIME	= -4000;



//**************************************************************************
//  EXECUTING DEVICE CONFIG
//**************************************************************************

//-------------------------------------------------
//  device_config_execute_interface - constructor
//-------------------------------------------------

device_config_execute_interface::device_config_execute_interface(const machine_config &mconfig, device_config &devconfig)
	: device_config_interface(mconfig, devconfig),
	  m_disabled(false),
	  m_vblank_interrupt(NULL),
	  m_vblank_interrupts_per_frame(0),
	  m_vblank_interrupt_screen(NULL),
	  m_timed_interrupt(NULL),
	  m_timed_interrupt_period(attotime_zero)
{
}


//-------------------------------------------------
//  device_config_execute_interface - destructor
//-------------------------------------------------

device_config_execute_interface::~device_config_execute_interface()
{
}


//-------------------------------------------------
//  static_set_disable - configuration helper to
//  set the disabled state of a device
//-------------------------------------------------

void device_config_execute_interface::static_set_disable(device_config *device)
{
	device_config_execute_interface *exec = dynamic_cast<device_config_execute_interface *>(device);
	if (exec == NULL)
		throw emu_fatalerror("MDRV_DEVICE_DISABLE called on device '%s' with no execute interface", device->tag());
	exec->m_disabled = true;
}


//-------------------------------------------------
//  static_set_vblank_int - configuration helper
//  to set up VBLANK interrupts on the device
//-------------------------------------------------

void device_config_execute_interface::static_set_vblank_int(device_config *device, device_interrupt_func function, const char *tag, int rate)
{
	device_config_execute_interface *exec = dynamic_cast<device_config_execute_interface *>(device);
	if (exec == NULL)
		throw emu_fatalerror("MDRV_DEVICE_VBLANK_INT called on device '%s' with no execute interface", device->tag());
	exec->m_vblank_interrupt = function;
	exec->m_vblank_interrupts_per_frame = rate;
	exec->m_vblank_interrupt_screen = tag;
}


//-------------------------------------------------
//  static_set_periodic_int - configuration helper
//  to set up periodic interrupts on the device
//-------------------------------------------------

void device_config_execute_interface::static_set_periodic_int(device_config *device, device_interrupt_func function, attotime rate)
{
	device_config_execute_interface *exec = dynamic_cast<device_config_execute_interface *>(device);
	if (exec == NULL)
		throw emu_fatalerror("MDRV_DEVICE_PERIODIC_INT called on device '%s' with no execute interface", device->tag());
	exec->m_timed_interrupt = function;
	exec->m_timed_interrupt_period = rate;
}


//-------------------------------------------------
//  execute_clocks_to_cycles - convert the number
//  of clocks to cycles, rounding down if necessary
//-------------------------------------------------

UINT64 device_config_execute_interface::execute_clocks_to_cycles(UINT64 clocks) const
{
	return clocks;
}


//-------------------------------------------------
//  execute_cycles_to_clocks - convert the number
//  of cycles to clocks, rounding down if necessary
//-------------------------------------------------

UINT64 device_config_execute_interface::execute_cycles_to_clocks(UINT64 cycles) const
{
	return cycles;
}


//-------------------------------------------------
//  execute_min_cycles - return the smallest number
//  of cycles that a single instruction or
//  operation can take
//-------------------------------------------------

UINT32 device_config_execute_interface::execute_min_cycles() const
{
	return 1;
}


//-------------------------------------------------
//  execute_max_cycles - return the maximum number
//  of cycles that a single instruction or
//  operation can take
//-------------------------------------------------

UINT32 device_config_execute_interface::execute_max_cycles() const
{
	return 1;
}


//-------------------------------------------------
//  execute_input_lines - return the total number
//  of input lines for the device
//-------------------------------------------------

UINT32 device_config_execute_interface::execute_input_lines() const
{
	return 0;
}


//-------------------------------------------------
//  execute_default_irq_vector - return the default
//  IRQ vector when an acknowledge is processed
//-------------------------------------------------

UINT32 device_config_execute_interface::execute_default_irq_vector() const
{
	return 0;
}


//-------------------------------------------------
//  interface_validity_check - validation for a
//  device after the configuration has been
//  constructed
//-------------------------------------------------

bool device_config_execute_interface::interface_validity_check(const game_driver &driver) const
{
	const device_config *devconfig = crosscast<const device_config *>(this);
	bool error = false;

	/* validate the interrupts */
	if (m_vblank_interrupt != NULL)
	{
		if (screen_count(m_machine_config) == 0)
		{
			mame_printf_error("%s: %s device '%s' has a VBLANK interrupt, but the driver is screenless!\n", driver.source_file, driver.name, devconfig->tag());
			error = true;
		}
		else if (m_vblank_interrupt_screen != NULL && m_vblank_interrupts_per_frame != 0)
		{
			mame_printf_error("%s: %s device '%s' has a new VBLANK interrupt handler with >1 interrupts!\n", driver.source_file, driver.name, devconfig->tag());
			error = true;
		}
		else if (m_vblank_interrupt_screen != NULL && m_machine_config.m_devicelist.find(m_vblank_interrupt_screen) == NULL)
		{
			mame_printf_error("%s: %s device '%s' VBLANK interrupt with a non-existant screen tag (%s)!\n", driver.source_file, driver.name, devconfig->tag(), m_vblank_interrupt_screen);
			error = true;
		}
		else if (m_vblank_interrupt_screen == NULL && m_vblank_interrupts_per_frame == 0)
		{
			mame_printf_error("%s: %s device '%s' has a VBLANK interrupt handler with 0 interrupts!\n", driver.source_file, driver.name, devconfig->tag());
			error = true;
		}
	}
	else if (m_vblank_interrupts_per_frame != 0)
	{
		mame_printf_error("%s: %s device '%s' has no VBLANK interrupt handler but a non-0 interrupt count is given!\n", driver.source_file, driver.name, devconfig->tag());
		error = true;
	}

	if (m_timed_interrupt != NULL && attotime_compare(m_timed_interrupt_period, attotime_zero) == 0)
	{
		mame_printf_error("%s: %s device '%s' has a timer interrupt handler with 0 period!\n", driver.source_file, driver.name, devconfig->tag());
		error = true;
	}
	else if (m_timed_interrupt == NULL && attotime_compare(m_timed_interrupt_period, attotime_zero) != 0)
	{
		mame_printf_error("%s: %s device '%s' has a no timer interrupt handler but has a non-0 period given!\n", driver.source_file, driver.name, devconfig->tag());
		error = true;
	}

	return error;
}



//**************************************************************************
//  EXECUTING DEVICE MANAGEMENT
//**************************************************************************

//-------------------------------------------------
//  device_execute_interface - constructor
//-------------------------------------------------

device_execute_interface::device_execute_interface(running_machine &machine, const device_config &config, device_t &device)
	: device_interface(machine, config, device),
	  m_machine(machine),
	  m_execute_config(dynamic_cast<const device_config_execute_interface &>(config)),
	  m_nextexec(NULL),
	  m_driver_irq(0),
	  m_timedint_timer(NULL),
	  m_iloops(0),
	  m_partial_frame_timer(NULL),
	  m_profiler(PROFILER_IDLE),
	  m_icountptr(NULL),
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
}


//-------------------------------------------------
//  ~device_execute_interface - destructor
//-------------------------------------------------

device_execute_interface::~device_execute_interface()
{
}


//-------------------------------------------------
//  executing - return true if this device is
//  within its execute function
//-------------------------------------------------

bool device_execute_interface::executing() const
{
	return (this == m_machine.scheduler().currently_executing());
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

	// aply the delta directly
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
	if (this != m_machine.scheduler().currently_executing())
		return;

	// swallow the remaining cycles
	if (m_icountptr != NULL)
	{
		int delta = *m_icountptr;
		m_cycles_stolen += delta;
		m_cycles_running -= delta;
		*m_icountptr -= delta;
	}
}


//-------------------------------------------------
//  set_irq_callback - install a driver-specific
//  callback for IRQ acknowledge
//-------------------------------------------------

void device_execute_interface::set_irq_callback(device_irq_callback callback)
{
	m_driver_irq = callback;
}


//-------------------------------------------------
//  suspend - set a suspend reason for this device
//-------------------------------------------------

void device_execute_interface::suspend(UINT32 reason, bool eatcycles)
{
if (TEMPLOG) printf("suspend %s (%X)\n", device().tag(), reason);
	// set the suspend reason and eat cycles flag
	m_nextsuspend |= reason;
	m_nexteatcycles = eatcycles;

	// if we're active, synchronize
	abort_timeslice();
}


//-------------------------------------------------
//  resume - clear a suspend reason for this
//  device
//-------------------------------------------------

void device_execute_interface::resume(UINT32 reason)
{
if (TEMPLOG) printf("resume %s (%X)\n", device().tag(), reason);
	// clear the suspend reason and eat cycles flag
	m_nextsuspend &= ~reason;

	// if we're active, synchronize
	abort_timeslice();
}


//-------------------------------------------------
//  spinuntil_time - burn cycles for a specific
//  period of time
//-------------------------------------------------

void device_execute_interface::spin_until_time(attotime duration)
{
	static int timetrig = 0;

	// suspend until the given trigger fires
	suspend_until_trigger(TRIGGER_SUSPENDTIME + timetrig, true);

	// then set a timer for it
	timer_set(&m_machine, duration, this, TRIGGER_SUSPENDTIME + timetrig, static_timed_trigger_callback);
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
	attotime result = m_localtime;
	if (executing())
	{
		assert(m_cycles_running >= *m_icountptr);
		int cycles = m_cycles_running - *m_icountptr;
		result = attotime_add(result, cycles_to_attotime(cycles));
	}
	return result;
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
//  interface_pre_start - work to be done prior to
//  actually starting a device
//-------------------------------------------------

void device_execute_interface::interface_pre_start()
{
	// fill in the initial states
	int index = m_machine.m_devicelist.index(&m_device);
	m_suspend = SUSPEND_REASON_RESET;
	m_profiler = profile_type(index + PROFILER_DEVICE_FIRST);
	m_inttrigger = index + TRIGGER_INT;

	// fill in the input states and IRQ callback information
	for (int line = 0; line < ARRAY_LENGTH(m_input); line++)
		m_input[line].start(this, line);

	// allocate timers if we need them
	if (m_execute_config.m_vblank_interrupts_per_frame > 1)
		m_partial_frame_timer = timer_alloc(&m_machine, static_trigger_partial_frame_interrupt, (void *)this);
	if (attotime_compare(m_execute_config.m_timed_interrupt_period, attotime_zero) != 0)
		m_timedint_timer = timer_alloc(&m_machine, static_trigger_periodic_interrupt, (void *)this);

	// register for save states
	state_save_register_device_item(&m_device, 0, m_suspend);
	state_save_register_device_item(&m_device, 0, m_nextsuspend);
	state_save_register_device_item(&m_device, 0, m_eatcycles);
	state_save_register_device_item(&m_device, 0, m_nexteatcycles);
	state_save_register_device_item(&m_device, 0, m_trigger);
	state_save_register_device_item(&m_device, 0, m_totalcycles);
	state_save_register_device_item(&m_device, 0, m_localtime.seconds);
	state_save_register_device_item(&m_device, 0, m_localtime.attoseconds);
	state_save_register_device_item(&m_device, 0, m_iloops);
}


//-------------------------------------------------
//  interface_post_start - work to be done after
//  actually starting a device
//-------------------------------------------------

void device_execute_interface::interface_post_start()
{
	// make sure somebody set us up the icount
	assert_always(m_icountptr != NULL, "m_icountptr never initialized!");
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
	if (!m_execute_config.disabled())
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
	for (int line = 0; line < ARRAY_LENGTH(m_input); line++)
		m_input[line].reset();

	// reconfingure VBLANK interrupts
	if (m_execute_config.m_vblank_interrupts_per_frame > 0 || m_execute_config.m_vblank_interrupt_screen != NULL)
	{
		// get the screen that will trigger the VBLANK

		// new style - use screen tag directly
		screen_device *screen;
		if (m_execute_config.m_vblank_interrupt_screen != NULL)
			screen = downcast<screen_device *>(m_machine.device(m_execute_config.m_vblank_interrupt_screen));

		// old style 'hack' setup - use screen #0
		else
			screen = screen_first(m_machine);

		assert(screen != NULL);
		screen->register_vblank_callback(static_on_vblank, NULL);
	}

	// reconfigure periodic interrupts
	if (attotime_compare(m_execute_config.m_timed_interrupt_period, attotime_zero) != 0)
	{
		attotime timedint_period = m_execute_config.m_timed_interrupt_period;
		assert(m_timedint_timer != NULL);
		timer_adjust_periodic(m_timedint_timer, timedint_period, 0, timedint_period);
	}
}


//-------------------------------------------------
//  interface_clock_changed - recomputes clock
//  information for this device
//-------------------------------------------------

void device_execute_interface::interface_clock_changed()
{
	// recompute cps and spc
	m_cycles_per_second = clocks_to_cycles(m_device.clock());
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
	m_machine.scheduler().compute_perfect_interleave();
}


//-------------------------------------------------
//  get_minimum_quantum - return the minimum
//  quantum required for this device
//-------------------------------------------------

attoseconds_t device_execute_interface::minimum_quantum() const
{
	// if we don't have that information, compute it
	attoseconds_t basetick = m_attoseconds_per_cycle;
	if (basetick == 0)
		basetick = HZ_TO_ATTOSECONDS(clocks_to_cycles(m_device.clock()));

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

void device_execute_interface::static_on_vblank(screen_device &screen, void *param, bool vblank_state)
{
	// VBLANK starting
	if (vblank_state)
	{
		device_execute_interface *exec = NULL;
		for (bool gotone = screen.machine->m_devicelist.first(exec); gotone; gotone = exec->next(exec))
			exec->on_vblank_start(screen);
	}
}

void device_execute_interface::on_vblank_start(screen_device &screen)
{
	// start the interrupt counter
	if (!suspended(SUSPEND_REASON_DISABLE))
		m_iloops = 0;
	else
		m_iloops = -1;

	// the hack style VBLANK decleration always uses the first screen
	bool interested = false;
	if (m_execute_config.m_vblank_interrupts_per_frame > 1)
		interested = true;

	// for new style declaration, we need to compare the tags
	else if (m_execute_config.m_vblank_interrupt_screen != NULL)
		interested = (strcmp(screen.tag(), m_execute_config.m_vblank_interrupt_screen) == 0);

	// if interested, call the interrupt handler
	if (interested)
	{
		if (!suspended(SUSPEND_REASON_HALT | SUSPEND_REASON_RESET | SUSPEND_REASON_DISABLE))
			(*m_execute_config.m_vblank_interrupt)(&m_device);

		// if we have more than one interrupt per frame, start the timer now to trigger the rest of them
		if (m_execute_config.m_vblank_interrupts_per_frame > 1 && !suspended(SUSPEND_REASON_DISABLE))
		{
			m_partial_frame_period = attotime_div(m_machine.primary_screen->frame_period(), m_execute_config.m_vblank_interrupts_per_frame);
			timer_adjust_oneshot(m_partial_frame_timer, m_partial_frame_period, 0);
		}
	}
}


//-------------------------------------------------
//  static_trigger_partial_frame_interrupt -
//  called to trigger a partial frame interrupt
//-------------------------------------------------

TIMER_CALLBACK( device_execute_interface::static_trigger_partial_frame_interrupt )
{
	reinterpret_cast<device_execute_interface *>(ptr)->trigger_partial_frame_interrupt();
}

void device_execute_interface::trigger_partial_frame_interrupt()
{
	// when we hit 0, reset to the total count
	if (m_iloops == 0)
		m_iloops = m_execute_config.m_vblank_interrupts_per_frame;

	// count one more "iloop"
	m_iloops--;

	// call the interrupt handler if we're not suspended
	if (!suspended(SUSPEND_REASON_HALT | SUSPEND_REASON_RESET | SUSPEND_REASON_DISABLE))
		(*m_execute_config.m_vblank_interrupt)(&m_device);

	// set up to retrigger if there's more interrupts to generate
	if (m_iloops > 1)
		timer_adjust_oneshot(m_partial_frame_timer, m_partial_frame_period, 0);
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
	if (m_execute_config.m_timed_interrupt != NULL && !suspended(SUSPEND_REASON_HALT | SUSPEND_REASON_RESET | SUSPEND_REASON_DISABLE))
		(*m_execute_config.m_timed_interrupt)(&m_device);
}


//-------------------------------------------------
//  static_standard_irq_callback - IRQ acknowledge
//  callback; handles HOLD_LINE case and signals
//  to the debugger
//-------------------------------------------------

IRQ_CALLBACK( device_execute_interface::static_standard_irq_callback )
{
	return device_execute(device)->standard_irq_callback(irqline);
}

int device_execute_interface::standard_irq_callback(int irqline)
{
	// get the default vector and acknowledge the interrupt if needed
	int vector = m_input[irqline].default_irq_callback();
	LOG(("static_standard_irq_callback('%s', %d) $%04x\n", m_device.tag(), irqline, vector));

	// if there's a driver callback, run it to get the vector
	if (m_driver_irq != NULL)
		vector = (*m_driver_irq)(&m_device, irqline);

	// notify the debugger
	debugger_interrupt_hook(&m_device, irqline);
	return vector;
}



//**************************************************************************
//  DEVICE INPUT
//**************************************************************************

//-------------------------------------------------
//  device_input - constructor
//-------------------------------------------------

device_execute_interface::device_input::device_input()
	: m_execute(NULL),
	  m_device(NULL),
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
	m_device = &m_execute->m_device;
	m_linenum = linenum;

	reset();

	state_save_register_device_item(m_device, m_linenum, m_stored_vector);
	state_save_register_device_item(m_device, m_linenum, m_curvector);
	state_save_register_device_item(m_device, m_linenum, m_curstate);
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
	LOG(("set_state_synced('%s',%d,%d,%02x)\n", m_device->tag(), m_linenum, state, vector));

if (TEMPLOG) printf("setline(%s,%d,%d,%d)\n", m_device->tag(), m_linenum, state, (vector == USE_STORED_VECTOR) ? 0 : vector);
	assert(state == ASSERT_LINE || state == HOLD_LINE || state == CLEAR_LINE || state == PULSE_LINE);

	// treat PULSE_LINE as ASSERT+CLEAR
	if (state == PULSE_LINE)
	{
		// catch errors where people use PULSE_LINE for devices that don't support it
		if (m_linenum != INPUT_LINE_NMI && m_linenum != INPUT_LINE_RESET)
			throw emu_fatalerror("device '%s': PULSE_LINE can only be used for NMI and RESET lines\n", m_device->tag());

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
		logerror("Exceeded pending input line event queue on device '%s'!\n", m_device->tag());
	}

	// enqueue the event
	if (event_index < ARRAY_LENGTH(m_queue))
	{
		if (vector == USE_STORED_VECTOR)
			vector = m_stored_vector;
		m_queue[event_index] = (state & 0xff) | (vector << 8);

		// if this is the first one, set the timer
		if (event_index == 0)
			timer_call_after_resynch(&m_execute->m_machine, (void *)this, 0, static_empty_event_queue);
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
if (TEMPLOG) printf("empty_queue(%s,%d,%d)\n", m_device->tag(), m_linenum, m_qindex);
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
				m_device->reset();
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
					logerror("empty_event_queue device '%s', line %d, unknown state %d\n", m_device->tag(), m_linenum, m_curstate);
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
		LOG(("->set_irq_line('%s',%d,%d)\n", m_device->tag(), m_linenum, CLEAR_LINE));
		m_execute->execute_set_input(m_linenum, CLEAR_LINE);
		m_curstate = CLEAR_LINE;
	}
	return vector;
}
