// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Kawasaki Steel (Kawatetsu) KL5C80A16 CPU

    Important functional blocks:
    — MMU
    — DMA controller (KP27) (unemulated)
    — UART (KP61) (unemulated)
    — Synchronous serial port (KP62) (unemulated)
    — 16-bit timer/counters (KP63A)
    — 16-level interrupt controller (KP69)
    — Parallel ports (KP67)
    — External bus/DRAM interface unit (unemulated)

***************************************************************************/

#include "emu.h"
#include "kl5c80a16.h"
#include "kp63.h"

// device type definition
DEFINE_DEVICE_TYPE(KL5C80A16, kl5c80a16_device, "kl5c80a16", "Kawasaki Steel KL5C80A16")

static const z80_daisy_config pseudo_daisy_config[] = { { "kp69" }, { nullptr } };


//-------------------------------------------------
//  kl5c80a16_device - constructor
//-------------------------------------------------

kl5c80a16_device::kl5c80a16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: kc82_device(mconfig, KL5C80A16, tag, owner, clock,
					address_map_constructor(),
					address_map_constructor(FUNC(kl5c80a16_device::internal_io), this))
	, m_kp69(*this, "kp69")
	, m_port_in_callback(*this)
	, m_port_out_callback(*this)
	, m_port_data{0, 0, 0, 0}
	, m_port_direction{0x0f, 0, 0, 0}
	, m_port_3state{0, 0, 0, 0}
{
}


//-------------------------------------------------
//  internal_io - map for internal I/O registers
//-------------------------------------------------

void kl5c80a16_device::internal_io(address_map &map)
{
	map(0x00, 0x07).mirror(0xff00).rw(FUNC(kl5c80a16_device::mmu_r), FUNC(kl5c80a16_device::mmu_w));
	map(0x20, 0x27).mirror(0xff00).rw("timer", FUNC(kp63a_device::read), FUNC(kp63a_device::write));
	map(0x34, 0x34).mirror(0xff00).rw(m_kp69, FUNC(kp69_device::isrl_r), FUNC(kp69_device::lerl_pgrl_w));
	map(0x35, 0x35).mirror(0xff00).rw(m_kp69, FUNC(kp69_device::isrh_r), FUNC(kp69_device::lerh_pgrh_w));
	map(0x36, 0x36).mirror(0xff00).rw(m_kp69, FUNC(kp69_device::imrl_r), FUNC(kp69_device::imrl_w));
	map(0x37, 0x37).mirror(0xff00).rw(m_kp69, FUNC(kp69_device::imrh_r), FUNC(kp69_device::ivr_imrh_w));
	map(0x38, 0x3f).mirror(0xff00).rw(FUNC(kl5c80a16_device::port_r), FUNC(kl5c80a16_device::port_w));
}


//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void kl5c80a16_device::device_resolve_objects()
{
	// Resolve parallel port callbacks
	for (int i = 0; i < 4; i++)
		m_port_in_callback[i].resolve_safe(m_port_3state[i]);
	m_port_out_callback.resolve_all_safe();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void kl5c80a16_device::device_start()
{
	kc82_device::device_start();

	m_kp69->add_to_state(*this, KP69_IRR);

	// Register save state
	save_item(NAME(m_port_data));
	save_item(NAME(m_port_direction));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void kl5c80a16_device::device_reset()
{
	kc82_device::device_reset();

	// Reset parallel port registers (Port 0 direction is fixed to upper 4 bits input & lower 4 bits output)
	std::fill(std::begin(m_port_data), std::end(m_port_data), 0x00);
	std::fill(std::begin(m_port_direction) + 1, std::end(m_port_direction), 0x00);

	// Reset parallel port outputs
	for (int i = 1; i < 4; i++)
		m_port_out_callback[i](0, m_port_3state[i] & ~m_port_direction[i], m_port_direction[i]);
}


//-------------------------------------------------
//  device_add_mconfig - add device-specific
//  machine configuration
//-------------------------------------------------

void kl5c80a16_device::device_add_mconfig(machine_config &config)
{
	KP69(config, m_kp69);
	m_kp69->int_callback().set_inputline(*this, INPUT_LINE_IRQ0);

	kp63a_device &timer(KP63A(config, "timer", DERIVED_CLOCK(1, 2)));
	timer.outs_callback<0>().set(m_kp69, FUNC(kp69_device::ir_w<12>));
	timer.outs_callback<1>().set(m_kp69, FUNC(kp69_device::ir_w<13>));
	//timer.outs_callback<2>().set(m_kp69, FUNC(kp69_device::ir_w<0>)); // TODO: multiplexed with P20 input by SCR1 bit 4
	//timer.outs_callback<3>().set(m_kp69, FUNC(kp69_device::ir_w<1>)); // TODO: multiplexed with P21 input by SCR1 bit 5 (may also be NMI)
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void kl5c80a16_device::device_config_complete()
{
	set_daisy_config(pseudo_daisy_config);
}


//**************************************************************************
//  KP67 PARALLEL PORT
//**************************************************************************

//-------------------------------------------------
//  port_r - read from parallel port data or
//  direction register
//-------------------------------------------------

u8 kl5c80a16_device::port_r(offs_t offset)
{
	if (BIT(offset, 0))
		return m_port_direction[offset >> 1];
	else
	{
		u8 data = m_port_data[offset >> 1];
		const u8 dir = m_port_direction[offset >> 1];
		if (dir != 0xff)
			data = (data & dir) | (m_port_in_callback[offset >> 1](0, u8(~dir)) & ~dir);
		return data;
	}
}


//-------------------------------------------------
//  port_w - write to parallel port data or
//  direction register or bit control
//-------------------------------------------------

void kl5c80a16_device::port_w(offs_t offset, u8 data)
{
	unsigned p = offset >> 1;
	u8 dir = m_port_direction[p];
	if (offset == 1)
	{
		// Bit set/reset command
		const unsigned b = BIT(data, 1, 3);
		p = BIT(data, 4, 2);
		if (BIT(m_port_data[p], b) == BIT(data, 0))
			return;
		data = (m_port_data[p] ^= 1 << b);
		dir = m_port_direction[p];
		if (!BIT(dir, b))
			return;
	}
	else if (BIT(offset, 0))
	{
		if (dir == data)
			return;
		m_port_direction[p] = dir = data;
		data = m_port_data[p];
	}
	else
	{
		u8 old_data = std::exchange(m_port_data[p], data);
		if (((old_data ^ data) & dir) == 0)
			return;
	}

	// Update port output
	m_port_out_callback[offset >> 1](0, (data & dir) | (m_port_3state[offset >> 1] & ~dir), dir);
}
