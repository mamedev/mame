// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_DEC_DC7085_H
#define MAME_DEC_DC7085_H

#pragma once

#include "diserial.h"

class dc7085_channel
	: public device_t
	, public device_serial_interface
{
	friend class dc7085_device;

public:
	auto tx_cb() { return m_tx_cb.bind(); }

	dc7085_channel(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_serial overrides
	virtual void rcv_complete() override;
	virtual void tra_complete() override;
	virtual void tra_callback() override;

	// parent interface
	auto rx_done() { return m_rx_done.bind(); }
	auto tx_done() { return m_tx_done.bind(); }

	void set_format(unsigned baud, unsigned data_bits, unsigned parity, unsigned stop_bits);
	void set_enable(bool enable) { m_rx_enabled = enable; }
	bool tx_ready() const { return is_transmit_register_empty(); }
	void tx_w(u8 data);

private:
	devcb_write_line m_tx_cb;
	devcb_write_line m_tx_done;
	devcb_write16 m_rx_done;

	bool m_rx_enabled;
};

class dc7085_device : public device_t
{
public:
	dc7085_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	void map(address_map &map) ATTR_COLD;

	auto int_cb() { return m_int_cb.bind(); }

	template <unsigned Ch> void rx_w(int state) { m_chan[Ch]->rx_w(state); }
	template <unsigned Ch> auto tx_cb() { return m_tx_cb[Ch].bind(); }
	template <unsigned Ch> auto dtr_cb() { return m_dtr_cb[Ch].bind(); }

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// register accessors
	u16 csr_r() { return m_csr; }
	u16 rbuf_r();
	u16 tcr_r() { return m_tcr; }
	u16 msr_r() { return m_msr; }
	void csr_w(u16 data);
	void lpr_w(u16 data);
	void tcr_w(u16 data);
	void tdr_w(u16 data);

	// helpers
	void rx_fifo_push(u16 data);
	void rx_done(u16 data);
	void tx_done(int state);
	void recalc_irqs();

	void set_int(bool state)
	{
		if (state != m_int_state)
		{
			m_int_state = state;
			m_int_cb(state);
		}
	}

private:
	required_device_array<dc7085_channel, 4> m_chan;

	devcb_write_line m_int_cb;
	devcb_write_line::array<4> m_tx_cb;
	devcb_write_line::array<4> m_dtr_cb;

	u16 m_csr;
	u16 m_tcr;
	u16 m_msr;

	util::fifo<u16, 64> m_fifo;
	u16 m_rx_buf;

	bool m_int_state;
};

DECLARE_DEVICE_TYPE(DC7085, dc7085_device)
DECLARE_DEVICE_TYPE(DC7085_CHANNEL, dc7085_channel)

#endif // MAME_DEC_DC7085_H
