// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_EDLC_H
#define MAME_MACHINE_EDLC_H

#pragma once

class seeq8003_device :
	public device_t,
	public device_network_interface
{
public:
	// callback configuration
	auto out_int_cb() { return m_out_int.bind(); }
	auto out_rxrdy_cb() { return m_out_rxrdy.bind(); }
	auto out_txrdy_cb() { return m_out_txrdy.bind(); }

	seeq8003_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	// command/status interface
	void map(address_map &map);
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);
	void reset_w(int state);

	// data interface
	u8 fifo_r();
	int rxeof_r();
	void fifo_w(u8 data);
	void txeof_w(int state);

protected:
	// device_t overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_network_interface overrides
	virtual int recv_start_cb(u8 *buf, int length) override;

	// command/status registers
	template <unsigned N> void station_address_w(u8 data) { m_station_address[N] = data; }
	u8 rx_status_r();
	u8 tx_status_r();
	void rx_command_w(u8 data) { m_rx_command = data; }
	void tx_command_w(u8 data) { m_tx_command = data & TXC_M; }

	// helpers
	void transmit(void *ptr, int param);
	int receive(u8 *buf, int length);
	void interrupt(void *ptr = nullptr, int param = 0);
	bool address_filter(u8 *address);
	void dump_bytes(u8 *buf, int length);

	// constants and masks
	static const unsigned MAX_FRAME_SIZE = 1518;
	static const u32 FCS_RESIDUE = 0xdebb20e3;

	enum rx_status_mask : u8
	{
		RXS_V = 0x01, // received frame with overflow error
		RXS_C = 0x02, // received frame with crc error
		RXS_D = 0x04, // received frame with dribble error
		RXS_S = 0x08, // received short frame
		RXS_E = 0x10, // received end of frame
		RXS_G = 0x20, // received good frame
		RXS_O = 0x80, // old/new status

		RXS_M = 0x3f, // interrupt mask
	};
	enum rx_command_mask : u8
	{
		RXC_V = 0x01, // interrupt on receive overflow error
		RXC_C = 0x02, // interrupt on receive crc error
		RXC_D = 0x04, // interrupt on receive dribble error
		RXC_S = 0x08, // interrupt on short frame
		RXC_E = 0x10, // interrupt on end of frame
		RXC_G = 0x20, // interrupt on good frame
		RXC_M = 0xc0, // match mode
	};
	enum rx_match_mask : u8
	{
		RXC_M0 = 0x00, // receiver disable
		RXC_M1 = 0x40, // receive all frames
		RXC_M2 = 0x80, // receive station or broadcast frames
		RXC_M3 = 0xc0, // receive station, broadcast/multicast frames
	};
	enum tx_status_mask : u8
	{
		TXS_U = 0x01, // transmit underflow
		TXS_C = 0x02, // transmit collision
		TXS_R = 0x04, // 16 transmission attempts
		TXS_S = 0x08, // transmission successful
		TXS_O = 0x80, // old/new status

		TXS_M = 0x0f, // interrupt mask
	};
	enum tx_command_mask : u8
	{
		TXC_U = 0x01, // interrupt on transmit underflow
		TXC_C = 0x02, // interrupt on transmit collision
		TXC_R = 0x04, // interrupt on 16 transmission attempts
		TXC_S = 0x08, // interrupt on transmit success

		TXC_M = 0x0f, // write mask
	};

private:
	emu_timer *m_tx_timer;
	emu_timer *m_int_timer;

	// device state
	devcb_write_line m_out_int;
	devcb_write_line m_out_rxrdy;
	devcb_write_line m_out_txrdy;

	int m_int_state;
	int m_reset_state;
	u8 m_station_address[6];
	u8 m_rx_status;
	u8 m_tx_status;
	u8 m_rx_command;
	u8 m_tx_command;

	util::fifo<u8, MAX_FRAME_SIZE> m_rx_fifo;
	util::fifo<u8, MAX_FRAME_SIZE> m_tx_fifo;
};

DECLARE_DEVICE_TYPE(SEEQ8003, seeq8003_device)

#endif // MAME_MACHINE_EDLC_H
