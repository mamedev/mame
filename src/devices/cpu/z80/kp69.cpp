// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Kawasaki Steel (Kawatetsu) KP69 Interrupt Controller

    This macro cell is the sole provider of maskable interrupts for
    KC80/KC82-based microcontrollers. It responds to the CPU's internal
    IACK and EOI outputs (the latter associated with the RETI instruction)
    with prioritized Mode 2 vectors and nested in-service lockouts. It
    offers no support for polled operation or daisy-chaining other
    interrupt controllers, but it does allow code to recognize spurious
    interrupts.

    Each of the 16 interrupt sources may be programmed either as level-
    triggered or edge-triggered, though the latter is required for
    interrupts that are internal timer/counter outputs. These and the
    interrupt vector register must be written first after a reset before
    the mask and priority group registers can be defined, with no way of
    returning to the initial mode once the vector has been set.

***************************************************************************/

#include "emu.h"
#include "kp69.h"

#define VERBOSE 0
#include "logmacro.h"

// device type definition
DEFINE_DEVICE_TYPE(KP69, kp69_device, "kp69", "Kawasaki Steel KP69 Interrupt Controller")


//-------------------------------------------------
//  kp69_base_device - constructor
//-------------------------------------------------

kp69_base_device::kp69_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_z80daisy_interface(mconfig, *this)
	, m_int_callback(*this)
	, m_input_levels(0)
	, m_irr(0)
	, m_isr(0)
	, m_illegal_state(false)
	, m_ivr(0)
	, m_imr(0xffff)
	, m_ler(0)
	, m_pgr(0)
	, m_int_active(false)
{
}


//-------------------------------------------------
//  kp69_device - constructor
//-------------------------------------------------

kp69_device::kp69_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: kp69_base_device(mconfig, KP69, tag, owner, clock)
	, m_ivr_written(false)
{
}


//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void kp69_base_device::device_resolve_objects()
{
	// Resolve output callback
	m_int_callback.resolve_safe();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void kp69_base_device::device_start()
{
	// Register state for saving
	save_item(NAME(m_input_levels));
	save_item(NAME(m_irr));
	save_item(NAME(m_isr));
	save_item(NAME(m_illegal_state));
	save_item(NAME(m_ivr));
	save_item(NAME(m_imr));
	save_item(NAME(m_ler));
	save_item(NAME(m_pgr));
	save_item(NAME(m_int_active));
}

void kp69_device::device_start()
{
	kp69_base_device::device_start();
	save_item(NAME(m_ivr_written));
}


//-------------------------------------------------
//  add_to_state - debug state interface for MCU
//-------------------------------------------------

void kp69_base_device::add_to_state(device_state_interface &state, int index)
{
	state.state_add(index, "IRR", m_irr, [this](u16 data) { set_irr(data); });
	state.state_add(index + 1, "ISR", m_isr, [this](u16 data) { set_isr(data); });
	state.state_add(index + 2, "IVR", m_ivr).mask(0xe0);
	state.state_add(index + 3, "LER", m_ler, [this](u16 data) { set_ler(data); });
	state.state_add(index + 4, "PGR", m_pgr, [this](u16 data) { set_pgr(data); });
	state.state_add(index + 5, "IMR", m_imr, [this](u16 data) { set_imr(data); });
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void kp69_base_device::device_reset()
{
	// Reset inputs to level mode
	m_ler = 0;

	// Mask all interrupts, cancel requests and end service
	m_imr = 0xffff;
	m_irr = 0;
	m_isr = 0;
	m_illegal_state = false;

	// Reset priority groups
	m_pgr = 0;

	// Deassert interrupt output
	set_int(false);
}

void kp69_device::device_reset()
{
	kp69_base_device::device_reset();

	// Allow LER and IVR to be written first
	m_ivr_written = false;
}


//-------------------------------------------------
//  isrl_r - read lower 8 bits of ISR
//-------------------------------------------------

u8 kp69_base_device::isrl_r()
{
	return m_isr & 0x00ff;
}


//-------------------------------------------------
//  isrh_r - read upper 8 bits of ISR
//-------------------------------------------------

u8 kp69_base_device::isrh_r()
{
	return (m_isr & 0xff00) >> 8;
}


//-------------------------------------------------
//  imrl_r - read lower 8 bits of IMR
//-------------------------------------------------

u8 kp69_base_device::imrl_r()
{
	return m_imr & 0x00ff;
}


//-------------------------------------------------
//  imrh_r - read upper 8 bits of IMR
//-------------------------------------------------

u8 kp69_base_device::imrh_r()
{
	return (m_imr & 0xff00) >> 8;
}


//-------------------------------------------------
//  lerl_pgrl_w - write lower 8 bits of LER or PGR
//-------------------------------------------------

void kp69_device::lerl_pgrl_w(u8 data)
{
	if (m_ivr_written)
		set_pgr((m_pgr & 0xff00) | data);
	else
		set_ler((m_ler & 0xff00) | data);
}


//-------------------------------------------------
//  lerh_pgrh_w - write upper 8 bits of LER or PGR
//-------------------------------------------------

void kp69_device::lerh_pgrh_w(u8 data)
{
	if (m_ivr_written)
		set_pgr(u16(data) << 8 | (m_pgr & 0x00ff));
	else
		set_ler(u16(data) << 8 | (m_ler & 0x00ff));
}


//-------------------------------------------------
//  imrl_w - write lower 8 bits of IMR
//-------------------------------------------------

void kp69_device::imrl_w(u8 data)
{
	if (!m_ivr_written)
		logerror("%s: IMRL written before IVR\n", machine().describe_context());

	set_imr((m_imr & 0xff00) | data);
}


//-------------------------------------------------
//  ivr_imrh_w - write IVR or upper 8 bits of IMR
//-------------------------------------------------

void kp69_device::ivr_imrh_w(u8 data)
{
	if (m_ivr_written)
		set_imr(u16(data) << 8 | (m_imr & 0x00ff));
	else
	{
		m_ivr = data & 0xe0;
		m_ivr_written = true;
	}
}


//-------------------------------------------------
//  int_active - determine whether or not the INT
//  output is currently active
//-------------------------------------------------

bool kp69_base_device::int_active() const
{
	if (m_illegal_state)
		return false;

	// Compare priority of pending interrupt request with any being serviced
	if ((m_irr & m_pgr) != 0 || (m_isr & m_pgr) != 0)
		return (m_irr & ~m_isr & m_pgr) > (m_isr & m_pgr);
	else
		return (m_irr & ~m_isr) > m_isr;
}


//-------------------------------------------------
//  set_int - update the INT output state
//-------------------------------------------------

void kp69_base_device::set_int(bool active)
{
	if (m_int_active != active)
	{
		m_int_active = active;
		m_int_callback(m_int_active ? ASSERT_LINE : CLEAR_LINE);
	}
}


//-------------------------------------------------
//  set_input_level - set the level state of one
//  out of 16 interrupt inputs
//-------------------------------------------------

void kp69_base_device::set_input_level(int level, bool state)
{
	if (!BIT(m_input_levels, level) && state)
	{
		m_input_levels |= 1 << level;

		// Masked-out interrupts cannot be requested
		if (!BIT(m_irr, level) && !BIT(m_imr, level))
		{
			u16 old_ints = m_irr | m_isr;
			m_irr |= 1 << level;
			LOG("IRR[%d] asserted\n", level);
			if (!m_illegal_state && (1 << level) > (BIT(m_pgr, level) ? old_ints & m_pgr : old_ints))
				set_int(true);
		}
	}
	else if (BIT(m_input_levels, level) && !state)
	{
		m_input_levels &= ~(1 << level);

		// Level-triggered interrupts may be deasserted
		if (!BIT(m_ler, level) && BIT(m_irr, level))
		{
			m_irr &= ~(1 << level);
			LOG("IRR[%d] cleared\n", level);
			if (!m_illegal_state)
				set_int(int_active());
		}
	}
}


//-------------------------------------------------
//  set_irr - write a new value to the Interrupt
//  Request Register (not accessible by software)
//-------------------------------------------------

void kp69_base_device::set_irr(u16 data)
{
	m_irr = (data & ~m_imr & m_ler) | (m_irr & ~m_ler);
	set_int(int_active());
}


//-------------------------------------------------
//  set_isr - write a new value to the In Service
//  Register (not writable by software)
//-------------------------------------------------

void kp69_base_device::set_isr(u16 data)
{
	m_isr = data;
	set_int(int_active());
}


//-------------------------------------------------
//  set_imr - write a new value to the Interrupt
//  Mask Register
//-------------------------------------------------

void kp69_base_device::set_imr(u16 data)
{
	u16 old_irr = m_irr;
	m_imr = data;
	m_irr = (m_irr & ~data & m_ler) | (m_input_levels & ~data & ~m_ler);
	if (m_irr != old_irr)
	{
		bool active = int_active();
		if (active != m_int_active)
			LOG("%s: INT %s (IRR = %04X, was %04X)\n", machine().describe_context(), active ? "unmasked" : "masked out", m_irr, old_irr);
		set_int(active);
	}
}


//-------------------------------------------------
//  set_ler - write a new value to the Level/Edge
//  Register
//-------------------------------------------------

void kp69_base_device::set_ler(u16 data)
{
	u16 old_irr = m_irr;
	m_irr = (m_input_levels & ~data) | (m_irr & m_ler & data);
	m_ler = data;
	if (m_irr != old_irr)
		set_int(int_active());
}


//-------------------------------------------------
//  set_pgr - write a new value to the Priority
//  Group Register
//-------------------------------------------------

void kp69_base_device::set_pgr(u16 data)
{
	if (m_pgr != data)
	{
		m_pgr = data;
		if (!m_illegal_state && m_isr != 0)
			set_int(int_active());
	}
}


//-------------------------------------------------
//  z80daisy_irq_state - return the overall IRQ
//  state for this device
//-------------------------------------------------

int kp69_base_device::z80daisy_irq_state()
{
	return m_int_active ? (Z80_DAISY_INT | Z80_DAISY_IEO) : Z80_DAISY_IEO;
}


//-------------------------------------------------
//  z80daisy_irq_ack - acknowledge an IRQ and
//  return the appropriate vector
//-------------------------------------------------

int kp69_base_device::z80daisy_irq_ack()
{
	int level = -1;

	// Restrict to high-priority interrupts if any of those are pending
	if ((m_irr & m_pgr) != 0)
	{
		level = 31 - count_leading_zeros_32(u32(m_irr & m_pgr));
		assert(level >= 0 && level < 16);
		if ((1 << level) < (m_isr & m_pgr))
			level = -1;
	}
	else if (m_irr != 0 && (m_isr & m_pgr) == 0)
	{
		level = 31 - count_leading_zeros_32(u32(m_irr));
		assert(level >= 0 && level < 16);
		if ((1 << level) < m_isr)
			level = -1;
	}

	if (level != -1)
	{
		u8 vector = m_ivr | (level << 1);
		if (BIT(m_ler, level))
		{
			LOG("%s: IRR[%d] acknowledged and cleared (vector = %02X)\n", machine().describe_context(), level, vector);
			m_irr &= ~(1 << level);
		}
		else
			LOG("%s: IR[%d] acknowledged (vector = %02X)\n", machine().describe_context(), level, vector);

		m_isr |= 1 << level;
		set_int(false);

		return vector;
	}

	// Illegal interrupt operation: same vector as IR[0] but ISR[0] not set
	LOG("%s: Illegal interrupt at IACK (vector = %02X)\n", machine().describe_context(), m_ivr);
	m_illegal_state = true;
	set_int(false);
	return m_ivr;
}


//-------------------------------------------------
//  z80daisy_irq_reti - clear the interrupt
//  pending state to allow other interrupts through
//-------------------------------------------------

void kp69_base_device::z80daisy_irq_reti()
{
	if (m_illegal_state)
	{
		LOG("%s: End of illegal interrupt\n", machine().describe_context());
		m_illegal_state = false;
	}
	else if (m_isr != 0)
	{
		int level = 31 - count_leading_zeros_32(u32((m_isr & m_pgr) != 0 ? (m_isr & m_pgr) : m_isr));
		assert(level >= 0 && level < 16);

		m_isr &= ~(1 << level);
		LOG("%s: EOI for ISR[%d]\n", machine().describe_context(), level);
	}
	else
	{
		logerror("%s: RETI before interrupt acknowledged\n", machine().describe_context());
		return;
	}

	set_int(int_active());
}
