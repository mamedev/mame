// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_MACHINE_DC7085_H
#define MAME_MACHINE_DC7085_H

#pragma once

#include "diserial.h"

#define DC7085_RX_FIFO_SIZE (64)

// forward declaration
class dc7085_device;

class dc7085_channel : public device_t, public device_serial_interface
{
public:
	dc7085_channel(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_serial overrides
	virtual void rcv_complete() override;    // Rx completed receiving byte
	virtual void tra_complete() override;    // Tx completed sending byte
	virtual void tra_callback() override;    // Tx send bit

	void set_baud_rate(int baud);
	void set_format(int data_bits, int parity, int stop_bits);
	void clear();
	void set_tx_enable(bool bEnabled);
	void set_rx_enable(bool bEnabled);
	void write_TX(uint8_t data);
	bool is_tx_ready()
	{
		if (!tx_enabled)
		{
			return false;
		}

		return tx_ready ? true : false;
	}

private:
	/* Receiver */
	u8 rx_enabled;

	/* Shared */
	int baud_rate;
	int m_ch;

	/* Transmitter */
	u8 tx_enabled;
	u8 tx_data;
	u8 tx_ready;

	dc7085_device *m_base;
};

class dc7085_device : public device_t
{
	friend class dc7085_channel;

public:
	required_device<dc7085_channel> m_chan0, m_chan1, m_chan2, m_chan3;

	dc7085_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void map(address_map &map);

	auto int_cb() { return m_int_cb.bind(); }
	auto ch0_tx_cb() { return write_0_tx.bind(); }
	auto ch1_tx_cb() { return write_1_tx.bind(); }
	auto ch2_tx_cb() { return write_2_tx.bind(); }
	auto ch3_tx_cb() { return write_3_tx.bind(); }

protected:
	// standard device_interface overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	void rx_fifo_push(uint16_t data, uint16_t errors);
	void recalc_irqs();

	devcb_write_line m_int_cb;
	devcb_write_line write_0_tx, write_1_tx, write_2_tx, write_3_tx;

	u16 m_status;

	u16 status_r();
	u16 rxbuffer_r();
	u16 txparams_r();
	u16 modem_status_r();
	void control_w(u16 data);
	void lineparams_w(u16 data);
	void txparams_w(u16 data);
	void txdata_w(u16 data);

	int get_ch(dc7085_channel *ch)
	{
		if (ch == m_chan0)
		{
			return 0;
		}
		else if (ch == m_chan1)
		{
			return 1;
		}
		else if (ch == m_chan2)
		{
			return 2;
		}

		return 3;
	}

private:
	u16 rx_fifo[DC7085_RX_FIFO_SIZE];
	int rx_fifo_read_ptr;
	int rx_fifo_write_ptr;
	int rx_fifo_num;

	enum control_status_mask : u16
	{
		CTRL_TRDY           = 0x8000,
		CTRL_TX_IRQ_ENABLE  = 0x4000,
		CTRL_LINE_MASK      = 0x0300,
		CTRL_RX_DONE        = 0x0080,
		CTRL_RX_IRQ_ENABLE  = 0x0040,
		CTRL_MASTER_SCAN    = 0x0020,
		CTRL_MASTER_CLEAR   = 0x0010,
		CTRL_LOOPBACK       = 0x0008
	};

	enum rx_buffer_mask : u16
	{
		RXMASK_DATA_VALID   = 0x8000,
		RXMASK_OVERRUN_ERR  = 0x4000,
		RXMASK_FRAMING_ERR  = 0x2000,
		RXMASK_PARITY_ERR   = 0x1000,
		RXMASK_LINE_MASK    = 0x0300,
		RXMASK_DATA_MASK    = 0x00ff
	};

	enum line_param_mask : u16
	{
		LPARAM_RX_ENABLE    = 0x1000,
		LPARAM_BAUD_MASK    = 0x0f00,
		LPARAM_ODD_PARITY   = 0x0080,
		LPARAM_PARITY_ENB   = 0x0040,
		LPARAM_STOP_BITS    = 0x0020,
		LPARAM_CHARLEN_MASK = 0x0018,
		LPARAM_LINE_MASK    = 0x0003
	};

	enum tx_control_mask : u16
	{
		TXCTRL_DTR2         = 0x0400,
		TXCTRL_LINE3_ENB    = 0x0008,
		TXCTRL_LINE2_ENB    = 0x0004,
		TXCTRL_LINE1_ENB    = 0x0002,
		TXCTRL_LINE0_ENB    = 0x0001
	};

	enum modem_status_mask : u16
	{
		MSTAT_DSR2          = 0x0400
	};

	enum tx_data_mask : u16
	{
		TXDATA_LINE3_BREAK  = 0x0800,
		TXDATA_LINE2_BREAK  = 0x0400,
		TXDATA_LINE1_BREAK  = 0x0200,
		TXDATA_LINE0_BREAK  = 0x0100,
		TXDATA_DATA_MASK    = 0x00ff
	};
};

DECLARE_DEVICE_TYPE(DC7085, dc7085_device)
DECLARE_DEVICE_TYPE(DC7085_CHANNEL, dc7085_channel)

#endif // MAME_MACHINE_DC7085_H
