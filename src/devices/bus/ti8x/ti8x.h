// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
 TI-8x calculator link port.

 2.5mm TRS connector on each device, tip and ring both used as I/O
 lines.  Each device has passive pull-ups and software-controlled active
 pull-downs.  In TI products, the tip is connected to a red wire and the
 ring is connected to a white wire.

 The link ports are only intended to allow linking two devices together,
 but there is open source software implementing multi-master protocols
 including I2C.  The number of devices that can be connected together is
 limited in practice by the fact that every device provides pull-ups and
 there's a limit to the amount of current any device can sink.

 TI's link port protocol uses 8-bit bytes transmitted LSB first with no
 markers for beginning/end of a byte or parity.  To transfer a bit, the
 transmitting device pulls down a line (tip for 0, ring for 1), waits
 for the receiving device to pull down the other line, releases its
 line, and waits for the receiver to release the other line.  This
 ensures software-based implementations don't drop bits.  The 6 MHz
 Z80-based calculators can manage up to about 50 kbps.

 In a TI-82, each line is tied to the supply rail via a 10kΩ resistor
 in series with a signal diode, and can be pulled low by an NPN
 transistor in series with a 470Ω resistor.

 This bus implementation works with logic levels (1 = pulled up,
 0 = driven down).  The port device just gives you the drive level from
 the opposite side of the port which you need to mix with your device's
 output.  This makes implementing things like the tee connector easier.
 */

#ifndef MAME_DEVICES_BUS_TI8X_TI8X_H
#define MAME_DEVICES_BUS_TI8X_TI8X_H

#pragma once



DECLARE_DEVICE_TYPE(TI8X_LINK_PORT, ti8x_link_port_device)


class device_ti8x_link_port_interface;


class ti8x_link_port_device : public device_t, public device_single_card_slot_interface<device_ti8x_link_port_interface>
{
public:
	template <typename T>
	ti8x_link_port_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: ti8x_link_port_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	ti8x_link_port_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	// static configuration helpers
	auto tip_handler() { return m_tip_handler.bind(); }
	auto ring_handler() { return m_ring_handler.bind(); }

	void tip_w(int state);
	void ring_w(int state);

	int tip_r() { return m_tip_in ? 1 : 0; }
	int ring_r() { return m_ring_in ? 1 : 0; }

protected:
	ti8x_link_port_device(
			machine_config const &mconfig,
			device_type type,
			char const *tag,
			device_t *owner,
			u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_config_complete() override;

	devcb_write_line m_tip_handler;
	devcb_write_line m_ring_handler;

private:
	friend class device_ti8x_link_port_interface;

	device_ti8x_link_port_interface *m_dev;

	bool m_tip_in, m_tip_out, m_ring_in, m_ring_out;
};


class device_ti8x_link_port_interface : public device_interface
{
public:
	void output_tip(int state)
	{ if (bool(state) != m_port->m_tip_in) m_port->m_tip_handler((m_port->m_tip_in = bool(state)) ? 1 : 0); }
	void output_ring(int state)
	{ if (bool(state) != m_port->m_ring_in) m_port->m_ring_handler((m_port->m_ring_in = bool(state)) ? 1 : 0); }

protected:
	device_ti8x_link_port_interface(machine_config const &mconfig, device_t &device);

	ti8x_link_port_device &port() { return *m_port; }

private:
	virtual void input_tip(int state) = 0;
	virtual void input_ring(int state) = 0;

	friend class ti8x_link_port_device;

	ti8x_link_port_device *m_port;
};


class device_ti8x_link_port_bit_interface : public device_ti8x_link_port_interface
{
protected:
	device_ti8x_link_port_bit_interface(machine_config const &mconfig, device_t &device);

	virtual void interface_pre_start() override;
	virtual void interface_post_start() override;
	virtual void interface_pre_reset() override;

	void send_bit(bool data);
	void accept_bit();

private:
	enum bit_phase
	{
		IDLE,
		WAIT_ACK_0,
		WAIT_ACK_1,
		WAIT_REL_0,
		WAIT_REL_1,
		ACK_0,
		ACK_1,
		HOLD_0,
		HOLD_1,
		WAIT_IDLE
	};

	enum bit_buffer
	{
		EMPTY,
		PENDING_0,
		PENDING_1
	};

	virtual void input_tip(int state) override;
	virtual void input_ring(int state) override;

	virtual void bit_collision() = 0;
	virtual void bit_send_timeout() = 0;
	virtual void bit_receive_timeout() = 0;
	virtual void bit_sent() = 0;
	virtual void bit_received(bool data) = 0;

	TIMER_CALLBACK_MEMBER(bit_timeout);

	void check_tx_bit_buffer();

	emu_timer * m_error_timer;
	u8          m_bit_phase;
	u8          m_tx_bit_buffer;
	bool        m_tip_in, m_ring_in;
};


class device_ti8x_link_port_byte_interface : public device_ti8x_link_port_bit_interface
{
protected:
	device_ti8x_link_port_byte_interface(machine_config const &mconfig, device_t &device);

	virtual void interface_pre_start() override;
	virtual void interface_post_start() override;
	virtual void interface_pre_reset() override;

	void send_byte(u8 data);
	void accept_byte();

private:
	virtual void bit_collision() override;
	virtual void bit_send_timeout() override;
	virtual void bit_receive_timeout() override;
	virtual void bit_sent() override;
	virtual void bit_received(bool data) override;

	virtual void byte_collision() = 0;
	virtual void byte_send_timeout() = 0;
	virtual void byte_receive_timeout() = 0;
	virtual void byte_sent() = 0;
	virtual void byte_received(u8 data) = 0;

	u16 m_tx_byte_buffer, m_rx_byte_buffer;
};


void default_ti8x_link_devices(device_slot_interface &device);

#endif // MAME_DEVICES_BUS_TI8X_TI8X_H
