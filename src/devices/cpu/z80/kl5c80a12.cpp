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
    — USART (KP51), 8251-compatible
    — 16-bit timer/counters (KP64, KP63)
    — 16-level interrupt controller (KP69)
    — Parallel ports (KP65, KP66)
    — 512-byte high-speed RAM
    — External bus interface unit (unemulated)

    The system control registers (SCR0/SCR1) mostly select pin functions.
    Emulated here: SCR0 D0 (USART TXC/RXC from Timer B channel 1 output
    or from the TXC/RXC pins) and SCR0 D1 (DSR_/CTS_ pins, or tied low).

***************************************************************************/

#include "emu.h"
#include "kl5c80a12.h"
#include "kp63.h"
#include "kp64.h"

#define LOG_SCR (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_SCR)
#include "logmacro.h"

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
	, m_kp51(*this, "kp51")
	, m_porta_in_callback(*this, 0)
	, m_porta_out_callback(*this)
	, m_portb_in_callback(*this, 0)
	, m_portb_out_callback(*this)
	, m_txd_callback(*this)
	, m_porta_data{0, 0}
	, m_porta_direction{0, 0}
	, m_porta_3state{0, 0}
	, m_portb_data{0, 0, 0}
	, m_portb_direction(0)
	, m_portb_3state{0, 0, 0}
	, m_scr{0, 0}
	, m_cts_pin(1)
	, m_dsr_pin(1)
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
	map(0x38, 0x39).mirror(0xff00).rw(m_kp51, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x3a, 0x3b).mirror(0xff00).rw(FUNC(kl5c80a12_device::scr_r), FUNC(kl5c80a12_device::scr_w));
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
	save_item(NAME(m_scr));
	save_item(NAME(m_cts_pin));
	save_item(NAME(m_dsr_pin));
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

	// Reset system control registers
	m_scr[0] = 0x00;
	m_scr[1] = 0x00;
	update_serial_pins();
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
	timerb.outp_callback<1>().set(FUNC(kl5c80a12_device::timer_serial_clk_w)); // OUTBP1 can clock the USART (SCR0 D0 = 0)

	I8251(config, m_kp51, 0); // baud rate comes only from TXC/RXC edges, so the clock value is unused
	m_kp51->txd_handler().set(FUNC(kl5c80a12_device::txd_trampoline_w));
	m_kp51->txrdy_handler().set(m_kp69, FUNC(kp69_device::ir_w<8>));
	m_kp51->rxrdy_handler().set(m_kp69, FUNC(kp69_device::ir_w<9>));
	m_kp51->txempty_handler().set(m_kp69, FUNC(kp69_device::ir_w<10>));
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void kl5c80a12_device::device_config_complete()
{
	kc82_device::device_config_complete();

	set_daisy_config(pseudo_daisy_config);

	for (int i = 0; i < 2; i++)
	{
		if (m_porta_in_callback[i].isunset())
			m_porta_in_callback[i].bind().set_constant(m_porta_3state[i]);
	}
	for (int i = 0; i < 3; i++)
	{
		if (m_portb_in_callback[i].isunset())
			m_portb_in_callback[i].bind().set_constant(m_portb_3state[i]);
	}
}


//**************************************************************************
//  SYSTEM CONTROL REGISTERS / KP51 SERIAL PIN ROUTING
//**************************************************************************

//-------------------------------------------------
//  scr_r - read system control register
//-------------------------------------------------

u8 kl5c80a12_device::scr_r(offs_t offset)
{
	return m_scr[offset & 1];
}


//-------------------------------------------------
//  scr_w - write system control register
//
//  SCR0 (0x3a):
//    D0: USART TXC/RXC source, 1 = pins 73/74,
//        0 = Timer B channel 1 OUTBP (internal)
//    D1: 1 = DSR_/CTS_/DTR_/RTS_ pins,
//        0 = P41/P42/P45/P46 (DSR_/CTS_ tied low)
//    D2-D4: sync pins / timer output pins vs GPIO
//    D5/D6: Timer A GATE inputs, D7: Timer A cascade
//  SCR1 (0x3b): M1_/HALT_/BREQ_/NMI_ pins, wait states
//-------------------------------------------------

void kl5c80a12_device::scr_w(offs_t offset, u8 data)
{
	LOGMASKED(LOG_SCR, "%s: SCR%d = %02X\n", machine().describe_context(), offset & 1, data);

	m_scr[offset & 1] = data;
	if (!BIT(offset, 0))
		update_serial_pins();
}


//-------------------------------------------------
//  update_serial_pins - route DSR_/CTS_ to the
//  USART according to SCR0 D1
//-------------------------------------------------

void kl5c80a12_device::update_serial_pins()
{
	// when the pins are not selected, DSR_ and CTS_ are tied low (asserted)
	const bool pins = BIT(m_scr[0], 1);
	m_kp51->write_cts(pins ? m_cts_pin : 0);
	m_kp51->write_dsr(pins ? m_dsr_pin : 0);
}


//-------------------------------------------------
//  timer_serial_clk_w - Timer B channel 1 OUTBP
//  output, used as TXC/RXC when SCR0 D0 = 0
//-------------------------------------------------

void kl5c80a12_device::timer_serial_clk_w(int state)
{
	if (!BIT(m_scr[0], 0))
	{
		m_kp51->write_txc(state);
		m_kp51->write_rxc(state);
	}
}


//-------------------------------------------------
//  external serial pins
//-------------------------------------------------

void kl5c80a12_device::txc_w(int state)
{
	if (BIT(m_scr[0], 0))
		m_kp51->write_txc(state);
}

void kl5c80a12_device::rxc_w(int state)
{
	if (BIT(m_scr[0], 0))
		m_kp51->write_rxc(state);
}

void kl5c80a12_device::cts_w(int state)
{
	m_cts_pin = state ? 1 : 0;
	update_serial_pins();
}

void kl5c80a12_device::dsr_w(int state)
{
	m_dsr_pin = state ? 1 : 0;
	update_serial_pins();
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
