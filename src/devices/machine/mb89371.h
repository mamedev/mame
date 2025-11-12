// license:BSD-3-Clause
// copyright-holders:R. Belmont, smf
/*
 * Fujitsu MB89371 Dual Serial UART
 */

#ifndef MAME_MACHINE_MB89371_H
#define MAME_MACHINE_MB89371_H

#pragma once

#include "i8251.h"

#include "machine/clock.h"

class mb89371_device : public device_t
{
public:
	// construction/destruction
	mb89371_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<int ch> void map(address_map &map) ATTR_COLD;

	template<int ch> auto txd_handler() { return m_txd_w[ch].bind(); }
	template<int ch> auto dtr_handler() { return m_sio[ch].lookup()->dtr_handler(); }
	template<int ch> auto rts_handler() { return m_sio[ch].lookup()->rts_handler(); }
	template<int ch> auto rxrdy_handler() { return m_rxready_w[ch].bind(); }
	template<int ch> auto txrdy_handler() { return m_txready_w[ch].bind(); }
	template<int ch> auto txempty_handler() { return m_txempty_w[ch].bind(); }
	template<int ch> auto syndet_handler() { return m_syndet_w[ch].bind(); }

	template<int ch> void write_rxd(int state) { if (!BIT(m_mode[ch], MODE_LOOPBACK)) m_sio[ch]->write_rxd(state); }
	template<int ch> void write_cts(int state) { m_sio[ch]->write_cts(state); }
	template<int ch> void write_dsr(int state) { m_sio[ch]->write_dsr(state); }

	template<int ch> void write_txc(int state) { if (!BIT(m_mode[ch], MODE_USE_BRG)) m_sio[ch]->write_txc(state); }
	template<int ch> void write_rxc(int state) { if (!BIT(m_mode[ch], MODE_USE_BRG)) m_sio[ch]->write_rxc(state); }

	template<int ch> int txrdy_r() { return m_sio[ch]->txrdy_r(); }
	template<int ch> int rxrdy_r() { return m_sio[ch]->rxrdy_r(); }

	template<int ch> void write(offs_t offset, uint8_t data);
	template<int ch> uint8_t read(offs_t offset);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

private:
	enum
	{
		MODE_ST1_SEL = 0,
		MODE_USE_BRG,
		MODE_MODEM,
		MODE_LOOPBACK,
		MODE_IRQMASK_TXEMPTY,
		MODE_IRQMASK_TXREADY,
		MODE_IRQMASK_RXREADY,
		MODE_IRQMASK_SYNDET
	};

	required_device_array<i8251_device, 2> m_sio;
	required_device_array<clock_device, 2> m_brg;

	devcb_write_line m_txd_w[2], m_rxready_w[2], m_txready_w[2], m_txempty_w[2], m_syndet_w[2];

	uint8_t m_mode[2], m_baud[2];

	template<int ch> void brg_tick(int state);
	void recalc_brg(int channel);
	template<int ch> uint8_t baud_r();
	template<int ch> void baud_w(uint8_t data);
	template<int ch> uint8_t mode_r();
	template<int ch> void mode_w(uint8_t data);
	template<int ch> void tx_data_w(int state);
	template<int ch> void rx_ready_w(int state);
	template<int ch> void tx_ready_w(int state);
	template<int ch> void tx_empty_w(int state);
	template<int ch> void syndet_w(int state);
};

// device type definition
DECLARE_DEVICE_TYPE(MB89371, mb89371_device)

#endif // MAME_MACHINE_MB89371_H
