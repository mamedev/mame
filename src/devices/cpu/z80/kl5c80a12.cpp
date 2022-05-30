// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Kawasaki Steel (Kawatetsu) KL5C80A12 CPU

    This is based on KC82, an MMU-enhanced version of KC80 (KL5C8400),
    Kawasaki's Z80-compatible core with an internal 16-bit architecture
    and significantly faster opcode timings, operating at up to 10 MHz
    (CLK = XIN/2).

    Important functional blocks:
    — MMU
    — USART (KP51) (unemulated)
    — 16-bit timer/counters (KP64, KP63)
    — 16-level interrupt controller (KP69)
    — Parallel ports (KP65, KP66)
    — 512-byte high-speed RAM
    — External bus interface unit (unemulated)

***************************************************************************/

#include "emu.h"
#include "kl5c80a12.h"
#include "kp63.h"
#include "kp64.h"

// device type definition
DEFINE_DEVICE_TYPE(KL5C80A12, kl5c80a12_device, "kl5c80a12", "Kawasaki Steel KL5C80A12")

static const z80_daisy_config pseudo_daisy_config[] = { { "kp69" }, { nullptr } };


//-------------------------------------------------
//  kl5c80a12_device - constructor
//-------------------------------------------------

kl5c80a12_device::kl5c80a12_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: kc82_device(mconfig, KL5C80A12, tag, owner, clock,
					address_map_constructor(FUNC(kl5c80a12_device::internal_ram), this),
					address_map_constructor(FUNC(kl5c80a12_device::internal_io), this))
	, m_kp69(*this, "kp69")
	, m_porta_in_callback(*this)
	, m_porta_out_callback(*this)
	, m_portb_in_callback(*this)
	, m_portb_out_callback(*this)
	, m_porta_data{0, 0}
	, m_porta_direction{0, 0}
	, m_porta_3state{0, 0}
	, m_portb_data{0, 0, 0}
	, m_portb_direction(0)
	, m_portb_3state{0, 0, 0}
{
}


//-------------------------------------------------
//  internal_ram - map for high-speed internal RAM
//-------------------------------------------------

void kl5c80a12_device::internal_ram(address_map &map)
{
	map(0xffe00, 0xfffff).ram().share("ram");
}


//-------------------------------------------------
//  internal_io - map for internal I/O registers
//-------------------------------------------------

void kl5c80a12_device::internal_io(address_map &map)
{
	map(0x00, 0x07).mirror(0xff00).rw(FUNC(kl5c80a12_device::mmu_r), FUNC(kl5c80a12_device::mmu_w));
	map(0x20, 0x25).mirror(0xff00).rw("timerb", FUNC(kp63_3channel_device::read), FUNC(kp63_3channel_device::write));
	map(0x28, 0x28).mirror(0xff00).rw("timera0", FUNC(kp64_device::counter_r), FUNC(kp64_device::counter_w));
	map(0x29, 0x29).mirror(0xff00).rw("timera0", FUNC(kp64_device::status_r), FUNC(kp64_device::control_w));
	map(0x2a, 0x2a).mirror(0xff00).rw("timera1", FUNC(kp64_device::counter_r), FUNC(kp64_device::counter_w));
	map(0x2b, 0x2b).mirror(0xff00).rw("timera1", FUNC(kp64_device::status_r), FUNC(kp64_device::control_w));
	map(0x2c, 0x2f).mirror(0xff00).rw(FUNC(kl5c80a12_device::porta_r), FUNC(kl5c80a12_device::porta_w));
	map(0x30, 0x32).mirror(0xff00).rw(FUNC(kl5c80a12_device::portb_r), FUNC(kl5c80a12_device::portb_w));
	map(0x33, 0x33).mirror(0xff00).rw(FUNC(kl5c80a12_device::portb_control_r), FUNC(kl5c80a12_device::portb_control_w));
	map(0x34, 0x34).mirror(0xff00).rw(m_kp69, FUNC(kp69_device::isrl_r), FUNC(kp69_device::lerl_pgrl_w));
	map(0x35, 0x35).mirror(0xff00).rw(m_kp69, FUNC(kp69_device::isrh_r), FUNC(kp69_device::lerh_pgrh_w));
	map(0x36, 0x36).mirror(0xff00).rw(m_kp69, FUNC(kp69_device::imrl_r), FUNC(kp69_device::imrl_w));
	map(0x37, 0x37).mirror(0xff00).rw(m_kp69, FUNC(kp69_device::imrh_r), FUNC(kp69_device::ivr_imrh_w));
}


//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void kl5c80a12_device::device_resolve_objects()
{
	// Resolve parallel port callbacks
	for (int i = 0; i < 2; i++)
		m_porta_in_callback[i].resolve_safe(m_porta_3state[i]);
	m_porta_out_callback.resolve_all_safe();
	for (int i = 0; i < 3; i++)
		m_portb_in_callback[i].resolve_safe(m_portb_3state[i]);
	m_portb_out_callback.resolve_all_safe();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void kl5c80a12_device::device_start()
{
	kc82_device::device_start();

	m_kp69->add_to_state(*this, KP69_IRR);

	// Register save state
	save_item(NAME(m_porta_data));
	save_item(NAME(m_porta_direction));
	save_item(NAME(m_portb_data));
	save_item(NAME(m_portb_direction));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void kl5c80a12_device::device_reset()
{
	kc82_device::device_reset();

	// Reset parallel port registers
	std::fill(std::begin(m_porta_data), std::end(m_porta_data), 0x00);
	std::fill(std::begin(m_porta_direction), std::end(m_porta_direction), 0x00);
	std::fill(std::begin(m_portb_data), std::end(m_portb_data), 0x00);
	m_portb_direction = 0x00;

	// Reset parallel port outputs
	for (int i = 0; i < 2; i++)
		m_porta_out_callback[i](0, m_porta_3state[i], 0x00);
	for (int i = 0; i < 3; i++)
		m_portb_out_callback[i](0, m_portb_3state[i], 0x00);
}


//-------------------------------------------------
//  device_add_mconfig - add device-specific
//  machine configuration
//-------------------------------------------------

void kl5c80a12_device::device_add_mconfig(machine_config &config)
{
	KP69(config, m_kp69);
	m_kp69->int_callback().set_inputline(*this, INPUT_LINE_IRQ0);

	kp64_device &timera0(KP64(config, "timera0", DERIVED_CLOCK(1, 2)));
	timera0.out_callback().set(m_kp69, FUNC(kp69_device::ir_w<11>));

	kp64_device &timera1(KP64(config, "timera1", DERIVED_CLOCK(1, 2)));
	timera1.out_callback().set(m_kp69, FUNC(kp69_device::ir_w<12>));

	kp63_3channel_device &timerb(KP63_3CHANNEL(config, "timerb", DERIVED_CLOCK(1, 2)));
	timerb.outs_callback<0>().set(m_kp69, FUNC(kp69_device::ir_w<13>));
	timerb.outs_callback<1>().set(m_kp69, FUNC(kp69_device::ir_w<14>));
	timerb.outs_callback<2>().set(m_kp69, FUNC(kp69_device::ir_w<15>));
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void kl5c80a12_device::device_config_complete()
{
	set_daisy_config(pseudo_daisy_config);
}


//**************************************************************************
//  KP65 PARALLEL PORT
//**************************************************************************

//-------------------------------------------------
//  porta_r - read from parallel port A data or
//  direction register
//-------------------------------------------------

u8 kl5c80a12_device::porta_r(offs_t offset)
{
	const unsigned n = offset >> 1;
	if (BIT(offset, 0))
		return m_porta_direction[n];
	else
	{
		u8 data = m_porta_data[n];
		const u8 dir = m_porta_direction[n];
		if (dir != 0xff)
			data = (data & dir) | (m_porta_in_callback[n](0, u8(~dir)) & ~dir);
		return data;
	}
}


//-------------------------------------------------
//  porta_w - write to parallel port A data or
//  direction register
//-------------------------------------------------

void kl5c80a12_device::porta_w(offs_t offset, u8 data)
{
	const unsigned n = offset >> 1;
	u8 dir = m_porta_direction[n];
	if (BIT(offset, 0))
	{
		if (dir == data)
			return;
		m_porta_direction[n] = dir = data;
		data = m_porta_data[n];
	}
	else
	{
		u8 old_data = std::exchange(m_porta_data[n], data);
		if (((old_data ^ data) & dir) == 0)
			return;
	}

	// Update port output
	m_porta_out_callback[n](0, (data & dir) | (m_porta_3state[n] & ~dir), dir);
}


//**************************************************************************
//  KP66 PARALLEL PORT
//**************************************************************************

//-------------------------------------------------
//  portb_r - read from parallel port B data
//  register
//-------------------------------------------------

u8 kl5c80a12_device::portb_r(offs_t offset)
{
	u8 data = m_portb_data[offset];
	const u8 dir = std::array<u8, 4>{{0x00, 0x0f, 0xf0, 0xff}}[BIT(m_portb_direction, offset * 2, 2)];
	if (dir != 0xff)
		data = (data & dir) | (m_portb_in_callback[offset](0, u8(~dir)) & ~dir);
	return data;
}


//-------------------------------------------------
//  portb_update_output - update output for one
//  port in parallel port B
//-------------------------------------------------

void kl5c80a12_device::portb_update_output(unsigned n)
{
	const u8 dir = std::array<u8, 4>{{0x00, 0x0f, 0xf0, 0xff}}[BIT(m_portb_direction, n * 2, 2)];
	m_portb_out_callback[n](0, (m_portb_data[n] & dir) | (m_portb_3state[n] & ~dir), dir);
}


//-------------------------------------------------
//  portb_w - write to parallel port B data
//  register
//-------------------------------------------------

void kl5c80a12_device::portb_w(offs_t offset, u8 data)
{
	const u8 old_data = std::exchange(m_portb_data[offset], data);
	const u8 dir = std::array<u8, 4>{{0x00, 0x0f, 0xf0, 0xff}}[BIT(m_portb_direction, offset * 2, 2)];
	if (((old_data ^ data) & dir) != 0)
		portb_update_output(offset);
}


//-------------------------------------------------
//  portb_control_r - read from parallel port B
//  direction register
//-------------------------------------------------

u8 kl5c80a12_device::portb_control_r()
{
	return m_portb_direction;
}


//-------------------------------------------------
//  portb_control_w - write to parallel port B
//  direction register or bit command
//-------------------------------------------------

void kl5c80a12_device::portb_control_w(u8 data)
{
	if ((data & 0xc0) == 0xc0)
	{
		// Bit set/reset command
		const unsigned b = BIT(data, 1, 3);
		const unsigned n = BIT(data, 4, 2);
		if (n < 3)
		{
			if (BIT(m_portb_data[n], b) != BIT(data, 0))
			{
				m_portb_data[n] ^= 1 << b;
				if (BIT(m_portb_direction, n * 2 + (b < 4 ? 0 : 1)))
					portb_update_output(n);
			}
		}
		else
		{
			if ((data & 0x0d) == 0x0d)
				logerror("%s: Attempt to set bit %d in port B direction register\n", machine().describe_context(), BIT(data, 1, 3));
			else if (BIT(m_portb_direction, b) != BIT(data, 0))
			{
				m_portb_direction ^= 1 << b;
				portb_update_output(b >> 1);
			}
		}
	}
	else if ((data & 0xc0) == 0)
	{
		// Write direction register
		const u8 old_dir = std::exchange(m_portb_direction, data);
		if (((data ^ old_dir) & 0x03) != 0)
			portb_update_output(0);
		if (((data ^ old_dir) & 0x0c) != 0)
			portb_update_output(1);
		if (((data ^ old_dir) & 0x30) != 0)
			portb_update_output(2);
	}
	else
		logerror("%s: Writing %02X to port B command register\n", machine().describe_context(), data);
}
