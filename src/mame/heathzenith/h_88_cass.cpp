// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit H-88-5 Cassette Interface Card

    Came standard on the H88 computer and was on option for other H89 class
    systems.

****************************************************************************/

#include "emu.h"

#include "h_88_cass.h"

#include "machine/clock.h"

#include "speaker.h"

#define LOG_REG   (1U << 1)
#define LOG_LINES (1U << 2)
#define LOG_CASS  (1U << 3)
#define LOG_FUNC  (1U << 4)
//#define VERBOSE (0xff)

#include "logmacro.h"

#define LOGREG(...)        LOGMASKED(LOG_REG, __VA_ARGS__)
#define LOGLINES(...)      LOGMASKED(LOG_LINES, __VA_ARGS__)
#define LOGCASS(...)       LOGMASKED(LOG_CASS, __VA_ARGS__)
#define LOGFUNC(...)       LOGMASKED(LOG_FUNC, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

DEFINE_DEVICE_TYPE(HEATH_H88_CASS, heath_h_88_cass_device, "heath_h_88_cass_device", "Heath H-88-5 Cassette Interface");

heath_h_88_cass_device::heath_h_88_cass_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock):
	device_t(mconfig, HEATH_H88_CASS, tag, owner, 0),
	m_uart(*this, "uart"),
	m_cass_player(*this, "cassette_player"),
	m_cass_recorder(*this, "cassette_recorder")
{
}

void heath_h_88_cass_device::write(offs_t reg, u8 val)
{
	LOGREG("%s: reg: %d val: 0x%02x\n", FUNCNAME, reg, val);

	m_uart->write(reg, val);
}

u8 heath_h_88_cass_device::read(offs_t reg)
{
	u8 val = m_uart->read(reg);

	LOGREG("%s: reg: %d val: 0x%02x\n", FUNCNAME, reg, val);

	return val;
}

TIMER_DEVICE_CALLBACK_MEMBER(heath_h_88_cass_device::kansas_w)
{
	m_cass_data[3]++;

	if (m_cassbit != m_cassold)
	{
		LOGCASS("%s: m_cassbit changed : %d\n", FUNCNAME, m_cassbit);
		m_cass_data[3] = 0;
		m_cassold = m_cassbit;
	}

	LOGCASS("%s: m_cassbit: %d\n", FUNCNAME, m_cassbit);
	// 2400Hz -> 0
	// 1200Hz -> 1
	const int bit_pos = m_cassbit ? 0 : 1;

	m_cass_recorder->output(BIT(m_cass_data[3], bit_pos) ? -1.0 : +1.0);
}

TIMER_DEVICE_CALLBACK_MEMBER(heath_h_88_cass_device::kansas_r)
{
	// cassette - turn 1200/2400Hz to a bit
	m_cass_data[1]++;
	u8 cass_ws = (m_cass_player->input() > +0.03) ? 1 : 0;

	LOGCASS("%s: cass_ws: %d\n", FUNCNAME, cass_ws);

	if (cass_ws != m_cass_data[0])
	{
		LOGCASS("%s: cass_ws has changed value\n", FUNCNAME);
		m_cass_data[0] = cass_ws;
		m_uart->write_rxd((m_cass_data[1] < 12) ? 1 : 0);
		m_cass_data[1] = 0;
	}
}

void heath_h_88_cass_device::uart_rts(u8 data)
{
	LOGLINES("%s: data: %d\n", FUNCNAME, data);

	m_cass_player->change_state(bool(data) ? CASSETTE_STOPPED : CASSETTE_PLAY, CASSETTE_MASK_UISTATE);
}

void heath_h_88_cass_device::uart_tx_empty(u8 data)
{
	LOGLINES("%s: data: %d\n", FUNCNAME, data);

	m_cass_recorder->change_state(bool(data) ? CASSETTE_STOPPED : CASSETTE_RECORD, CASSETTE_MASK_UISTATE);
}

void heath_h_88_cass_device::device_start()
{
	save_item(NAME(m_cass_data));
	save_item(NAME(m_cassbit));
	save_item(NAME(m_cassold));
}

void heath_h_88_cass_device::device_reset()
{
	LOGFUNC("%s\n", FUNCNAME);

	// cassette
	m_cassbit      = 1;
	m_cassold      = 0;
	m_cass_data[0] = 0;
	m_cass_data[1] = 0;
	m_cass_data[2] = 0;
	m_cass_data[3] = 0;

	m_uart->write_cts(0);
	m_uart->write_dsr(0);
	m_uart->write_rxd(0);
}

void heath_h_88_cass_device::device_add_mconfig(machine_config &config)
{
	I8251(config, m_uart, 0);
	m_uart->txd_handler().set([this] (bool state) { m_cassbit = state; });
	m_uart->rts_handler().set(FUNC(heath_h_88_cass_device::uart_rts));
	m_uart->txempty_handler().set(FUNC(heath_h_88_cass_device::uart_tx_empty));

	clock_device &cassette_clock(CLOCK(config, "cassette_clock", 4800));
	cassette_clock.signal_handler().set(m_uart, FUNC(i8251_device::write_txc));
	cassette_clock.signal_handler().append(m_uart, FUNC(i8251_device::write_rxc));

	SPEAKER(config, "mono").front_right();

	CASSETTE(config, m_cass_player);
	m_cass_player->set_formats(h8_cassette_formats);
	m_cass_player->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass_player->add_route(ALL_OUTPUTS, "mono", 0.15);
	m_cass_player->set_interface("h88_cass_player");

	CASSETTE(config, m_cass_recorder);
	m_cass_recorder->set_formats(h8_cassette_formats);
	m_cass_recorder->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass_recorder->add_route(ALL_OUTPUTS, "mono", 0.15);
	m_cass_recorder->set_interface("h88_cass_recorder");

	TIMER(config, "kansas_w").configure_periodic(FUNC(heath_h_88_cass_device::kansas_w), attotime::from_hz(4800));
	TIMER(config, "kansas_r").configure_periodic(FUNC(heath_h_88_cass_device::kansas_r), attotime::from_hz(40000));
}
