// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit H-88-5 Cassette Interface Card

    Came standard on the H88 computer and was on option for other H89 class
    systems.

****************************************************************************/

#include "emu.h"

#include "h_88_5.h"

#include "formats/h8_cas.h"
#include "imagedev/cassette.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/timer.h"

#include "softlist_dev.h"
#include "speaker.h"

#define LOG_REG   (1U << 1)
#define LOG_LINES (1U << 2)
#define LOG_CASS  (1U << 3)
#define LOG_FUNC  (1U << 4)
#define VERBOSE (0)

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

namespace {

class h_88_5_device : public device_t, public device_h89bus_right_card_interface
{
public:
	h_88_5_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	virtual void write(u8 select_lines, u8 offset, u8 data) override;
	virtual u8 read(u8 select_lines, u8 offset) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void uart_rts(u8 data);
	void uart_tx_empty(u8 data);

	TIMER_DEVICE_CALLBACK_MEMBER(kansas_r);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_w);

	required_device<i8251_device> m_uart;
	required_device<cassette_image_device> m_cass_player;
	required_device<cassette_image_device> m_cass_recorder;

	u8 m_cass_data[4];
	bool m_cassbit;
	bool m_cassold;
};

h_88_5_device::h_88_5_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock):
	device_t(mconfig, H89BUS_H_88_5, tag, owner, 0),
	device_h89bus_right_card_interface(mconfig, *this),
	m_uart(*this, "uart"),
	m_cass_player(*this, "cassette_player"),
	m_cass_recorder(*this, "cassette_recorder")
{
}

void h_88_5_device::write(u8 select_lines, u8 reg, u8 val)
{
	if (select_lines & h89bus_device::H89_CASS)
	{
		LOGREG("%s: reg: %d val: 0x%02x\n", FUNCNAME, reg, val);

		m_uart->write(reg, val);
	}
}

u8 h_88_5_device::read(u8 select_lines, u8 reg)
{
	if (select_lines & h89bus_device::H89_CASS)
	{
		u8 val = m_uart->read(reg);

		LOGREG("%s: reg: %d val: 0x%02x\n", FUNCNAME, reg, val);

		return val;
	}

	return 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(h_88_5_device::kansas_w)
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

TIMER_DEVICE_CALLBACK_MEMBER(h_88_5_device::kansas_r)
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

void h_88_5_device::uart_rts(u8 data)
{
	LOGLINES("%s: data: %d\n", FUNCNAME, data);

	m_cass_player->change_state(bool(data) ? CASSETTE_STOPPED : CASSETTE_PLAY, CASSETTE_MASK_UISTATE);
}

void h_88_5_device::uart_tx_empty(u8 data)
{
	LOGLINES("%s: data: %d\n", FUNCNAME, data);

	m_cass_recorder->change_state(bool(data) ? CASSETTE_STOPPED : CASSETTE_RECORD, CASSETTE_MASK_UISTATE);
}

void h_88_5_device::device_start()
{
	save_item(NAME(m_cass_data));
	save_item(NAME(m_cassbit));
	save_item(NAME(m_cassold));
}

void h_88_5_device::device_reset()
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

void h_88_5_device::device_add_mconfig(machine_config &config)
{
	I8251(config, m_uart, 0);
	m_uart->txd_handler().set([this] (bool state) { m_cassbit = state; });
	m_uart->rts_handler().set(FUNC(h_88_5_device::uart_rts));
	m_uart->txempty_handler().set(FUNC(h_88_5_device::uart_tx_empty));

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

	SOFTWARE_LIST(config, "cass_list").set_original("h88_cass");

	TIMER(config, "kansas_w").configure_periodic(FUNC(h_88_5_device::kansas_w), attotime::from_hz(4800));
	TIMER(config, "kansas_r").configure_periodic(FUNC(h_88_5_device::kansas_r), attotime::from_hz(40000));
}

}

DEFINE_DEVICE_TYPE_PRIVATE(H89BUS_H_88_5, device_h89bus_right_card_interface, h_88_5_device, "h89h_88_5", "Heath H-88-5 Cassette Interface Board");
