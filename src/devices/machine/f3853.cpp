// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, hap
/**********************************************************************

Fairchild F3853 SMI, F3851 PSU, F3856 PSU, F38T56 PSU

This device only emulates the I/O, interrupt, timer functions. Not the
low-level ROMC signals.

F3853: Static memory interface with integrated interrupt controller and timer.

The timer is an 8-bit linear feedback shift register:
Feedback in0 = !((out3 ^ out4) ^ (out5 ^ out7))
Interrupts are at 0xfe
0xff stops the register (0xfe is never reached)

F3851: Program Storage Unit, same timer and interrupt controller as F3853,
but has 2 I/O ports instead of a programmable interrupt vector.

F3856/F38T56 Program Storage Unit: similar interrupt controller, timer
is more versatile, a simple downcounter instead of shift register.

TODO:
- emulate at lower level and place this stuff into devices/cpu/f8 folder
- interrupt priority pin
- 3856/38T56 timer pulse counter mode, event counter mode

**********************************************************************/

#include "emu.h"
#include "f3853.h"


// device type definition

DEFINE_DEVICE_TYPE(F3853, f3853_device, "f3853_smi", "Fairchild F3853 SMI")
DEFINE_DEVICE_TYPE(F3851, f3851_device, "f3851_psu", "Fairchild F3851 PSU")
DEFINE_DEVICE_TYPE(F3856, f3856_device, "f3856_psu", "Fairchild F3856 PSU")
DEFINE_DEVICE_TYPE(F38T56, f38t56_device, "f38t56_psu", "Fairchild F38T56 PSU")


//-------------------------------------------------
//  constructor
//-------------------------------------------------

f3853_device::f3853_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_int_req_callback(*this),
	m_pri_out_callback(*this),
	m_int_daisy_chain_callback(*this),
	m_int_vector(0),
	m_prescaler(31),
	m_priority_line(false),
	m_external_interrupt_line(false)
{ }

f3853_device::f3853_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	f3853_device(mconfig, F3853, tag, owner, clock)
{ }

f3851_device::f3851_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	f3853_device(mconfig, type, tag, owner, clock),
	m_read_port{{*this}, {*this}},
	m_write_port{{*this}, {*this}}
{ }

f3851_device::f3851_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	f3851_device(mconfig, F3851, tag, owner, clock)
{ }

f3856_device::f3856_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	f3851_device(mconfig, type, tag, owner, clock)
{ }

f3856_device::f3856_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	f3856_device(mconfig, F3856, tag, owner, clock)
{ }

f38t56_device::f38t56_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	f3856_device(mconfig, F38T56, tag, owner, clock)
{ }



//-------------------------------------------------
//  initialisation
//-------------------------------------------------

void f3853_device::device_resolve_objects()
{
	m_int_req_callback.resolve_safe();
	m_pri_out_callback.resolve_safe(); // TODO: not implemented
	m_int_daisy_chain_callback.resolve();
}

void f3851_device::device_resolve_objects()
{
	f3853_device::device_resolve_objects();

	// 2 I/O ports
	for (devcb_read8 &cb : m_read_port)
		cb.resolve_safe(0);
	for (devcb_write8 &cb : m_write_port)
		cb.resolve_safe();
}

void f3853_device::device_start()
{
	// lookup table for 3851/3853 lfsr timer
	m_value_to_cycle[0xff] = 0xff;
	uint8_t reg = 0xfe; // Known to get 0xfe after 255 cycles
	for (int i = reg; i >= 0; i--)
	{
		m_value_to_cycle[reg] = i;
		reg = reg << 1 | (BIT(reg,7) ^ BIT(reg,5) ^ BIT(reg,4) ^ BIT(reg,3) ^ 1);
	}

	m_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(f3853_device::timer_callback),this));

	// zerofill (what's not in constructor)
	m_external_int_enable = false;
	m_timer_int_enable = false;
	m_request_flipflop = false;

	// register for savestates
	save_item(NAME(m_int_vector));
	save_item(NAME(m_prescaler));
	save_item(NAME(m_external_int_enable));
	save_item(NAME(m_timer_int_enable));
	save_item(NAME(m_request_flipflop));
	save_item(NAME(m_priority_line));
	save_item(NAME(m_external_interrupt_line));
}

void f3856_device::device_start()
{
	f3853_device::device_start();

	m_timer_count = 0;
	m_timer_modulo = 0;
	m_timer_start = false;

	save_item(NAME(m_timer_count));
	save_item(NAME(m_timer_modulo));
	save_item(NAME(m_timer_start));
}

void f3853_device::device_reset()
{
	// note that standalone peripherals don't have a reset pin, but 3870 does

	// clear ports at power-on
	for (int i = 0; i < 4; i++)
		write(machine().dummy_space(), i, 0);
}


//-------------------------------------------------
//  implementation
//-------------------------------------------------

void f3853_device::set_interrupt_request_line()
{
	m_int_req_callback(m_request_flipflop && !m_priority_line ? ASSERT_LINE : CLEAR_LINE);
}

IRQ_CALLBACK_MEMBER(f3853_device::int_acknowledge)
{
	if (m_external_int_enable && !m_priority_line && m_request_flipflop)
	{
		m_request_flipflop = false;
		set_interrupt_request_line();
		return external_interrupt_vector();
	}
	else if (m_timer_int_enable && !m_priority_line && m_request_flipflop)
	{
		m_request_flipflop = false;
		set_interrupt_request_line();
		return timer_interrupt_vector();
	}
	else if (!m_int_daisy_chain_callback.isnull())
		return m_int_daisy_chain_callback(device, irqline);
	else
	{
		// should never happen
		logerror("%s: Spurious interrupt!\n", machine().describe_context());
		return 0;
	}
}


void f3853_device::timer_start(uint8_t value)
{
	attotime period = (value != 0xff) ? attotime::from_hz(clock()) * (m_prescaler * m_value_to_cycle[value]) : attotime::never;

	m_timer->adjust(period);
}

TIMER_CALLBACK_MEMBER(f3853_device::timer_callback)
{
	if (m_timer_int_enable)
	{
		m_request_flipflop = true;
		set_interrupt_request_line();
	}

	// next timeout after 255 timer counts (prescaler doesn't reset)
	m_timer->adjust(attotime::from_hz(clock()) * (m_prescaler * 0xff));
}


WRITE_LINE_MEMBER(f3853_device::ext_int_w)
{
	if (!m_external_interrupt_line && state && m_external_int_enable)
	{
		m_request_flipflop = true;
	}
	m_external_interrupt_line = bool(state);
	set_interrupt_request_line();
}

WRITE_LINE_MEMBER(f3853_device::pri_in_w)
{
	m_priority_line = bool(state);
	set_interrupt_request_line();
}


READ8_MEMBER(f3853_device::read)
{
	switch (offset & 3)
	{
	// interrupt vector
	case 0:
		return m_int_vector >> 8;
	case 1:
		return m_int_vector & 0xff;

	// interrupt control, timer: write-only
	default:
		return 0;
	}
}

WRITE8_MEMBER(f3853_device::write)
{
	switch (offset & 3)
	{
	// interrupt vector
	case 0:
		m_int_vector = (data << 8) | (m_int_vector & 0x00ff);
		break;
	case 1:
		m_int_vector = data | (m_int_vector & 0xff00);
		break;

	// interrupt control
	case 2:
		m_external_int_enable = (data & 3) == 1;
		m_timer_int_enable = (data & 3) == 3;
		set_interrupt_request_line();
		break;

	// set timer
	case 3:
		m_request_flipflop = false;
		set_interrupt_request_line();
		timer_start(data);
		break;
	}
}


//-------------------------------------------------
//  f3851_device-specific handlers
//-------------------------------------------------

READ8_MEMBER(f3851_device::read)
{
	switch (offset & 3)
	{
	// I/O ports
	case 0: case 1:
		return (m_read_port[offset & 1])(offs_t(offset & 1));

	// interrupt control, timer: write-only
	default:
		return 0;
	}
}

WRITE8_MEMBER(f3851_device::write)
{
	switch (offset & 3)
	{
	// I/O ports
	case 0: case 1:
		(m_write_port[offset & 1])(offs_t(offset & 1), data);
		break;

	// interrupt control, timer: same as 3853
	case 2: case 3:
		f3853_device::write(space, offset, data);
		break;
	}
}


//-------------------------------------------------
//  f3856_device-specific handlers
//-------------------------------------------------

void f3856_device::timer_start(uint8_t value)
{
	m_timer_count = value;
	attotime period = (m_timer_start) ? (attotime::from_hz(clock()) * m_prescaler) : attotime::never;

	m_timer->adjust(period);
}

TIMER_CALLBACK_MEMBER(f3856_device::timer_callback)
{
	if (--m_timer_count == 0)
	{
		m_timer_count = m_timer_modulo;
		if (m_timer_int_enable)
		{
			m_request_flipflop = true;
			set_interrupt_request_line();
		}
	}

	timer_start(m_timer_count);
}

READ8_MEMBER(f3856_device::read)
{
	switch (offset & 3)
	{
	// timer: active counter
	case 3:
		return m_timer_count;

	// other: same as 3851
	default:
		return f3851_device::read(space, offset);
	}
}

WRITE8_MEMBER(f3856_device::write)
{
	switch (offset & 3)
	{
	// I/O ports: same as 3851
	case 0: case 1:
		f3851_device::write(space, offset, data);
		break;

	// interrupt/timer control
	case 2:
	{
		// timer prescaler
		static const u8 prescaler[4] = { 32, 128, 8, 2 };
		m_prescaler = prescaler[data >> 2 & 3];

		// start/stop timer
		bool prev = m_timer_start;
		m_timer_start = bool(~data & 0x10);
		if (m_timer_start != prev)
			timer_start(m_timer_count);

		// enable interrupts
		m_external_int_enable = (data & 3) == 1 || (data & 3) == 2;
		m_timer_int_enable = bool(data & 2);
		set_interrupt_request_line();
		break;
	}

	// set timer
	case 3:
		f3853_device::write(space, offset, data);
		break;
	}
}


//-------------------------------------------------
//  f38t56_device-specific handlers
//-------------------------------------------------

READ8_MEMBER(f38t56_device::read)
{
	switch (offset & 3)
	{
	// interrupt: sense ext int pin
	case 2:
		return (m_external_interrupt_line) ? 0 : 0x80;

	// other: same as 3856
	default:
		return f3856_device::read(space, offset);
	}
}

WRITE8_MEMBER(f38t56_device::write)
{
	switch (offset & 3)
	{
	// I/O ports: same as 3851
	case 0: case 1:
		f3851_device::write(space, offset, data);
		break;

	// interrupt/timer control
	case 2:
	{
		// timer prescaler
		m_prescaler = 200;
		if (~data & 0x80) m_prescaler /= 20;
		if (~data & 0x40) m_prescaler /= 5;
		if (~data & 0x20) m_prescaler /= 2;

		// start/stop timer
		bool prev = m_timer_start;
		m_timer_start = bool(data & 8);
		if (m_timer_start != prev)
			timer_start(m_timer_count);

		// enable interrupts
		m_external_int_enable = bool(data & 1);
		m_timer_int_enable = bool(data & 2);
		set_interrupt_request_line();
		break;
	}

	// set timer
	case 3:
		m_timer_modulo = data;
		f3853_device::write(space, offset, data);
		break;
	}
}
