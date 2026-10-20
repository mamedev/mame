// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Robbbert
/**************************************************************************************************

PC-9801-03/PC-9801-13 cassette interface

i8251 + glue logic + cassette port

TODO:
- Subclass for PC-9801-13 support (adds a clock source select to control port,
  should be compatible with V30 at best);
- Test 600 baud select, test with actual SW once dumped;
- Interrupt enables, what's the INT pin this expects?
- Actual info about the cassette deck used, which should be compatible with non-PC-98 NEC computers
  at very least (to the point that multiplatform tapes are known to exist);

**************************************************************************************************/

#include "emu.h"

#include "pc9801_03.h"
#include "machine/clock.h"
#include "speaker.h"


#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

// device type definition
DEFINE_DEVICE_TYPE(PC9801_03, pc9801_03_device, "pc9801_03", "NEC PC-9801-03 CMT cassette interface card")

pc9801_03_device::pc9801_03_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC9801_03, tag, owner, clock)
	, device_pc98_cbus_slot_interface(mconfig, *this)
	, m_uart(*this, "uart")
	, m_cassette(*this, "cassette")
{
}

void pc9801_03_device::device_add_mconfig(machine_config &config)
{
	// assume mono speaker
	SPEAKER(config, "speaker").front_center();

	// TODO: clock, derived from C-Bus SCLK1?
	I8251(config, m_uart);
	m_uart->txd_handler().set([this] (bool state) { m_cassbit = state; });

	CASSETTE(config, m_cassette);
//  m_cassette->set_formats(pc98_cassette_formats);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "speaker", 0.05);

	clock_device &uart_clock(CLOCK(config, "uart_clock", 19200));
	uart_clock.signal_handler().set(FUNC(pc9801_03_device::kansas_w));
	TIMER(config, "kansas_r").configure_periodic(FUNC(pc9801_03_device::kansas_r), attotime::from_hz(40000));
}

void pc9801_03_device::device_validity_check(validity_checker &valid) const
{
	// TODO: check root passing the correct clock here
}

void pc9801_03_device::device_start()
{
	save_item(NAME(m_control));
	save_item(NAME(m_cassbit));
	save_item(NAME(m_cassold));
	save_item(NAME(m_cass_data));
	save_item(NAME(m_baud_select));
	save_item(NAME(m_record_latch));
}

void pc9801_03_device::device_reset()
{
	m_control = 0;
	m_cassbit = 0;
	m_cassold = 0;
	std::fill(std::begin(m_cass_data), std::begin(m_cass_data), 0U);
	m_baud_select = true;
	m_record_latch = false;

	m_uart->write_cts(0);
}

void pc9801_03_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_IO)
	{
		m_bus->install_device(0x0000, 0x3fff, *this, &pc9801_03_device::io_map);
	}
}

/*
 * $95/$97 Control Port
 *
 * x--- ---- BS Baud Rate Select (1 = 1200 bps, 0 = 600 bps)
 * -x-- ---- CINH write data disabled
 * --x- ---- CONT Motor ON
 * ---x ---- SCLK 8215A CLK switch (1 = 8 MHz, 0 = 5 MHz), possibly -13 only
 * ---- -x-- TxEMP interrupt enable
 * ---- --x- RxRDY interrupt enable
 * ---- ---x TxRDY interrupt enable
 *
 * CINH behaviour is guessed at how N88 Basic behaves
 */
void pc9801_03_device::control_port_w(offs_t offset, u8 data)
{
	const bool cinh = BIT(data, 6);
	const bool motor_on = BIT(data, 5);
	LOG("Control port %02x (CINH %d -> %d, Motor %s)\n", data, BIT(m_control, 6), cinh, motor_on ? "ON" : "OFF");

	m_cassette->change_state(motor_on ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);

	if (!motor_on)
	{
		m_cassette->change_state(CASSETTE_STOPPED, CASSETTE_MASK_UISTATE);
	}
	else if (!BIT(m_control, 6) && !cinh)
	{
		// double 0 -> 0 (0x80 -> 0xa0 -> 0xe0)
		m_cassette->change_state(CASSETTE_RECORD, CASSETTE_MASK_UISTATE);
		m_record_latch = true;
	}
	else if (!BIT(m_control, 6) && cinh)
	{
		// 0 -> 1
		if (m_record_latch)
			m_record_latch = false;
		else
		{
			// 0x80 -> 0xe0
			m_cassette->change_state(CASSETTE_PLAY, CASSETTE_MASK_UISTATE);
		}
	}

	m_baud_select = BIT(data, 7);

	if (data & 7)
		popmessage("pc9801_03.cpp: unemulated interrupt enable %02x", data);

	m_control = data;
}

void pc9801_03_device::io_map(address_map &map)
{
	map(0x0090, 0x0093).rw(m_uart, FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0xff00);
	map(0x0094, 0x0097).w(FUNC(pc9801_03_device::control_port_w)).umask16(0xff00);
}

void pc9801_03_device::kansas_w(int state)
{
	// incoming @19200Hz
	u8 twobit = m_cass_data[3] & 3;

	if (state)
	{
		if (twobit == 0)
			m_cassold = m_cassbit;

		if (m_cassold)
			m_cassette->output(BIT(m_cass_data[3], 2) ? -1.0 : +1.0); // 2400Hz
		else
			m_cassette->output(BIT(m_cass_data[3], 3) ? -1.0 : +1.0); // 1200Hz

		m_cass_data[3]++;
	}

	if (m_baud_select)
	{
		m_uart->write_txc(state);
		m_uart->write_rxc(state);
	}
	else
	{
		m_uart->write_txc(BIT(m_cass_data[3], 0));
		m_uart->write_rxc(BIT(m_cass_data[3], 0));
	}
}

TIMER_DEVICE_CALLBACK_MEMBER( pc9801_03_device::kansas_r )
{
	/* cassette - turn 1200/2400Hz to a bit */
	m_cass_data[1]++;
	uint8_t cass_ws = (m_cassette->input() > +0.04) ? 1 : 0;

	if (cass_ws != m_cass_data[0])
	{
		m_cass_data[0] = cass_ws;
		m_uart->write_rxd((m_cass_data[1] < 12) ? 1 : 0);
		m_cass_data[1] = 0;
	}
}
