// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
 * te7774 - Tokyo Electron quad UART
 */
#ifndef MAME_MACHINE_TE7774_H
#define MAME_MACHINE_TE7774_H

#pragma once

#include "diserial.h"

#include <queue>

// forward declaration
class te7774_device;

class te7774_channel : public device_t, public device_serial_interface
{
public:
	te7774_channel(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_serial overrides
	virtual void rcv_complete() override;
	virtual void tra_complete() override;
	virtual void tra_callback() override;

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	void set_channel(int ch) { m_ch = ch; }

private:
	// receiver state
	bool m_rx_enabled;

	// transmitter state
	uint8_t m_tx_data, m_rx_data;
	uint8_t m_control1, m_control2, m_control3;
	uint8_t m_status;
	bool m_tx_data_in_buffer;
	bool m_tx_enabled;

	// channel global
	int m_ch;
	te7774_device *m_parent;
};

class te7774_device : public device_t
{
	friend class te7774_channel;

public:
	// construction/destruction
	te7774_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	template <int ch> auto rxrdy_handler() { return m_rxrdy_handler[ch].bind(); }
	template <int ch> auto txrdy_handler() { return m_txrdy_handler[ch].bind(); }
	template <int ch> auto txd_handler() { return m_txd_handler[ch].bind(); }

	template <int ch> void rx_w(int state);

	// this chip has 4 chip selects, one per channel.
	template <int ch> uint8_t read_cs(offs_t offset);
	template <int ch> void write_cs(offs_t offset, uint8_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	devcb_write_line m_rxrdy_handler[4], m_txrdy_handler[4];
	devcb_write_line m_txd_handler[4];

private:
	required_device_array<te7774_channel, 4> m_channels;

	devcb_read_line m_rxd_handler[4];
};

DECLARE_DEVICE_TYPE(TE7774, te7774_device)
DECLARE_DEVICE_TYPE(TE7774_CHANNEL, te7774_channel)

#endif // MAME_MACHINE_TE7774_H
