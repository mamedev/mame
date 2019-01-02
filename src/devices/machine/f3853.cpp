// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    Fairchild F3853 SRAM interface with integrated interrupt
    controller and timer (SMI)

    This chip is a timer shift register, basically the same as in the
    F3851.

    Based on a datasheet obtained from www.freetradezone.com

    8-bit shift register:
    Feedback in0 = !((out3 ^ out4) ^ (out5 ^ out7))
    Interrupts are at 0xfe
    0xff stops the register (0xfe is never reached)

**********************************************************************/

#include "emu.h"
#include "f3853.h"



/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(F3853, f3853_device, "f3853_device", "F3853 SMI")

//-------------------------------------------------
//  f3853_device - constructor
//-------------------------------------------------

f3853_device::f3853_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, F3853, tag, owner, clock)
	, m_int_req_callback(*this)
	, m_pri_out_callback(*this)
	, m_priority_line(false)
	, m_external_interrupt_line(true)
{
}

void f3853_device::device_resolve_objects()
{
	m_int_req_callback.resolve_safe();
	m_pri_out_callback.resolve_safe(); // TODO: not implemented
	m_int_daisy_chain_callback.bind_relative_to(*owner());
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void f3853_device::device_start()
{
	uint8_t reg = 0xfe; // Known to get 0xfe after 255 cycles
	for(int i = reg; i >= 0; i--)
	{
		m_value_to_cycle[reg] = i;
		reg = reg << 1 | (BIT(reg,7) ^ BIT(reg,5) ^ BIT(reg,4) ^ BIT(reg,3) ^ 1);
	}

	m_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(f3853_device::timer_callback),this));

	save_item(NAME(m_int_vector));
	save_item(NAME(m_external_enable));
	save_item(NAME(m_timer_enable));
	save_item(NAME(m_request_flipflop));
	save_item(NAME(m_priority_line));
	save_item(NAME(m_external_interrupt_line));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void f3853_device::device_reset()
{
	m_int_vector = 0;
	m_external_enable = false;
	m_timer_enable = false;
	m_request_flipflop = false;
	m_external_interrupt_line = true;
	set_interrupt_request_line();

	m_timer->enable(false);
}


void f3853_device::set_interrupt_request_line()
{
	m_int_req_callback(m_request_flipflop && !m_priority_line ? ASSERT_LINE : CLEAR_LINE);
}


IRQ_CALLBACK_MEMBER(f3853_device::int_acknowledge)
{
	if (m_external_enable && !m_priority_line && m_request_flipflop)
	{
		m_request_flipflop = false;
		set_interrupt_request_line();
		return external_interrupt_vector();
	}
	else if (m_timer_enable && !m_priority_line && m_request_flipflop)
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
	attotime period = (value != 0xff) ? attotime::from_hz(clock()) * (m_value_to_cycle[value]*31) : attotime::never;

	m_timer->adjust(period);
}

TIMER_CALLBACK_MEMBER(f3853_device::timer_callback)
{
	if(m_timer_enable)
	{
		m_request_flipflop = true;
		set_interrupt_request_line();
	}
	timer_start(0xfe);
}

WRITE_LINE_MEMBER(f3853_device::ext_int_w)
{
	if(m_external_interrupt_line && !state && m_external_enable)
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
	uint8_t data = 0;

	switch (offset)
	{
	case 0:
		data = m_int_vector >> 8;
		break;

	case 1:
		data = m_int_vector & 0xff;
		break;

	case 2: // Interrupt control; not readable
	case 3: // Timer; not readable
		break;
	}

	return data;
}


WRITE8_MEMBER(f3853_device::write)
{
	switch(offset)
	{
	case 0:
		m_int_vector = (data << 8) | (m_int_vector & 0x00ff);
		break;

	case 1:
		m_int_vector = data | (m_int_vector & 0xff00);
		break;

	case 2: //interrupt control
		m_external_enable = ((data & 3) == 1);
		m_timer_enable = ((data & 3) == 3);
		set_interrupt_request_line();
		break;

	case 3: //timer
		m_request_flipflop = false;
		set_interrupt_request_line();
		timer_start(data);
		break;
	}
}
