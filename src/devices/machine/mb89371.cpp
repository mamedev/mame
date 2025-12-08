// license:BSD-3-Clause
// copyright-holders:R. Belmont, smf
/*
 * Fujitsu MB89371 Dual Serial UART
 * Emulation by R. Belmont, based on a skeleton by smf
 *
 * Basically two 8251s plus a rudimentary interrupt controller and
 * a baud rate generator for each 8251.  You can use it just as a
 * plain dual 8251 though, and the MPC60 does.  The MPC2000 and some
 * PS1-based Konami games use it more fully.
 */

#include "emu.h"

#include "mb89371.h"

#define LOG_BAUD_RATE_GENERATOR (1U << 1)

#define VERBOSE (0)

//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

DEFINE_DEVICE_TYPE(MB89371, mb89371_device, "mb89371", "MB89371 Dual Serial UART")

void mb89371_device::device_add_mconfig(machine_config &config)
{
	I8251(config, m_sio[0], clock());
	m_sio[0]->txd_handler().set(FUNC(mb89371_device::tx_data_w<0>));
	m_sio[0]->rxrdy_handler().set(FUNC(mb89371_device::rx_ready_w<0>));
	m_sio[0]->txrdy_handler().set(FUNC(mb89371_device::tx_ready_w<0>));
	m_sio[0]->txempty_handler().set(FUNC(mb89371_device::tx_empty_w<0>));
	m_sio[0]->syndet_handler().set(FUNC(mb89371_device::syndet_w<0>));

	I8251(config, m_sio[1], clock());
	m_sio[1]->txd_handler().set(FUNC(mb89371_device::tx_data_w<1>));
	m_sio[1]->rxrdy_handler().set(FUNC(mb89371_device::rx_ready_w<1>));
	m_sio[1]->txrdy_handler().set(FUNC(mb89371_device::tx_ready_w<1>));
	m_sio[1]->txempty_handler().set(FUNC(mb89371_device::tx_empty_w<1>));
	m_sio[1]->syndet_handler().set(FUNC(mb89371_device::syndet_w<1>));

	CLOCK(config, m_brg[0]);
	m_brg[0]->signal_handler().set(FUNC(mb89371_device::brg_tick<0>));

	CLOCK(config, m_brg[1]);
	m_brg[1]->signal_handler().set(FUNC(mb89371_device::brg_tick<1>));
}

mb89371_device::mb89371_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MB89371, tag, owner, clock)
	, m_sio(*this, "i8251_%u", 0)
	, m_brg(*this, "brg_%u", 0)
	, m_txd_w { *this, *this }
	, m_rxready_w { *this, *this }
	, m_txready_w { *this, *this }
	, m_txempty_w { *this, *this }
	, m_syndet_w { *this, *this }
	, m_mode { 0xf0, 0xf0 }
	, m_baud { 0, 0 }
{
}

void mb89371_device::device_start()
{
	save_item(NAME(m_mode));
	save_item(NAME(m_baud));
}

void mb89371_device::device_post_load()
{
	recalc_brg(0);
	recalc_brg(1);
}

template<int ch> void mb89371_device::map(address_map &map)
{
	map(0x0, 0x0).rw(m_sio[ch], FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0x1, 0x1).rw(m_sio[ch], FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0x2, 0x2).rw(FUNC(mb89371_device::baud_r<ch>), FUNC(mb89371_device::baud_w<ch>));
	map(0x3, 0x3).rw(FUNC(mb89371_device::mode_r<ch>), FUNC(mb89371_device::mode_w<ch>));
}

template void mb89371_device::map<0>(address_map &map);
template void mb89371_device::map<1>(address_map &map);

template<int ch> void mb89371_device::write(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0:
		m_sio[ch]->data_w(data);
		break;

	case 1:
		m_sio[ch]->control_w(data);
		break;

	case 2:
		baud_w<ch>(data);
		break;

	case 3:
		mode_w<ch>(data);
		break;
	}
}

template void mb89371_device::write<0>(offs_t offset, uint8_t data);
template void mb89371_device::write<1>(offs_t offset, uint8_t data);

template<int ch> uint8_t mb89371_device::read(offs_t offset)
{
	switch (offset)
	{
	case 0x00:
		return m_sio[ch]->data_r();

	case 0x01:
		return m_sio[ch]->status_r();

	case 0x02:
		return baud_r<ch>();

	case 0x03:
		return mode_r<ch>();
	}
	return 0xff;
}

template uint8_t mb89371_device::read<0>(offs_t offset);
template uint8_t mb89371_device::read<1>(offs_t offset);

template<int ch> void mb89371_device::brg_tick(int state)
{
	m_sio[ch]->write_txc(state);
	m_sio[ch]->write_rxc(state);
}

template void mb89371_device::brg_tick<0>(int state);
template void mb89371_device::brg_tick<1>(int state);

template<int ch> uint8_t mb89371_device::baud_r()
{
	return m_baud[ch];
}

template uint8_t mb89371_device::baud_r<0>();
template uint8_t mb89371_device::baud_r<1>();

template<int ch> void mb89371_device::baud_w(uint8_t data)
{
	uint32_t divider = (2 << data);
	m_baud[ch] = data;
	LOG("baud %d = %02x, divider is %d\n", ch, data, divider);
	recalc_brg(ch);
}

template void mb89371_device::baud_w<0>(uint8_t data);
template void mb89371_device::baud_w<1>(uint8_t data);

template<int ch> uint8_t mb89371_device::mode_r()
{
	return m_mode[ch];
}

template uint8_t mb89371_device::mode_r<0>();
template uint8_t mb89371_device::mode_r<1>();

template <int ch> void mb89371_device::mode_w(uint8_t data)
{
	LOG("%02x to mode\n", data);
	LOG("loopback %d modem %d clock source %s trnemp/st1 %s\n",
		BIT(data, MODE_LOOPBACK),
		BIT(data, MODE_MODEM),
		BIT(data, MODE_USE_BRG) ? "Internal BRG" : "External TRNCLK/RVCLK",
		BIT(data, MODE_ST1_SEL) ? "ST1" : "TRNEMP");

	m_mode[ch] = data;
	recalc_brg(ch);
}

template void mb89371_device::mode_w<0>(uint8_t data);
template void mb89371_device::mode_w<1>(uint8_t data);

void mb89371_device::recalc_brg(int channel)
{
	if (BIT(m_mode[channel], MODE_USE_BRG))
	{
		uint32_t rate = clock() / (2 << m_baud[channel]);
		LOGMASKED(LOG_BAUD_RATE_GENERATOR, "BRG %d enabled at %d Hz\n", channel, rate);
		m_brg[channel]->set_period(attotime::from_hz(rate));
	}
	else
	{
		LOGMASKED(LOG_BAUD_RATE_GENERATOR, "BRG %d off\n", channel);
		m_brg[channel]->set_period(attotime::never);
	}
}

template <int ch> void mb89371_device::tx_data_w(int state)
{
	// handle loopback
	if (BIT(m_mode[ch], MODE_LOOPBACK))
	{
		m_sio[ch]->write_rxd(state);
	}
	else
	{
		m_txd_w[ch](state);
	}
}

template void mb89371_device::tx_data_w<0>(int state);
template void mb89371_device::tx_data_w<1>(int state);

template <int ch> void mb89371_device::rx_ready_w(int state)
{
	if (BIT(m_mode[ch], MODE_IRQMASK_RXREADY))
	{
		m_rxready_w[ch](state);
	}
}

template void mb89371_device::rx_ready_w<0>(int state);
template void mb89371_device::rx_ready_w<1>(int state);

template <int ch> void mb89371_device::tx_ready_w(int state)
{
	if (BIT(m_mode[ch], MODE_IRQMASK_TXREADY))
	{
		m_txready_w[ch](state);
	}
}

template void mb89371_device::tx_ready_w<0>(int state);
template void mb89371_device::tx_ready_w<1>(int state);

template <int ch> void mb89371_device::tx_empty_w(int state)
{
	if (BIT(m_mode[ch], MODE_IRQMASK_TXREADY))
	{
		m_txempty_w[ch](state);
	}
}

template void mb89371_device::tx_empty_w<0>(int state);
template void mb89371_device::tx_empty_w<1>(int state);

template <int ch> void mb89371_device::syndet_w(int state)
{
	if (BIT(m_mode[ch], MODE_IRQMASK_TXREADY))
	{
		m_syndet_w[ch](state);
	}
}

template void mb89371_device::syndet_w<0>(int state);
template void mb89371_device::syndet_w<1>(int state);
