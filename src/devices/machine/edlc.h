// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_EDLC_H
#define MAME_MACHINE_EDLC_H

#pragma once

#include "machine/bankdev.h"
#include "dinetwork.h"

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

	// register interface
	virtual void map(address_map &map) ATTR_COLD;
	virtual u8 read(offs_t offset);
	virtual void write(offs_t offset, u8 data);

	// input and output lines
	int rxeof_r();
	void reset_w(int state);
	void txeof_w(int state);

	// fifo interface
	u8 fifo_r();
	void fifo_w(u8 data);

protected:
	seeq8003_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock = 0);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_network_interface implementation
	virtual int recv_start_cb(u8 *buf, int length) override;

	// register read handlers
	u8 unused_r() { return 0xff; }
	u8 rx_status_r();
	u8 tx_status_r();

	// register write handlers
	void station_address_w(offs_t offset, u8 data) { m_station_address[offset] = data; }
	void rx_command_w(u8 data);
	virtual void tx_command_w(u8 data);

	// helpers
	void transmit(s32 param);
	int receive(u8 *buf, int length);
	void interrupt(s32 param = 0);
	virtual bool address_filter(u8 *address);
	void dump_bytes(u8 *buf, int length);

	// 80c03 option helpers
	virtual bool mode_tx_pad() const { return false; }
	virtual bool mode_tx_crc() const { return true; }
	virtual bool mode_rx_crc() const { return false; }

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

		TXC_B = 0x60, // register bank select (80c03 only)
	};

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

class seeq80c03_device : public seeq8003_device
{
public:
	seeq80c03_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	// register interface
	virtual void map(address_map &map) override ATTR_COLD;
	virtual u8 read(offs_t offset) override;
	virtual void write(offs_t offset, u8 data) override;

protected:
	// device_t overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_network_interface overrides
	virtual void send_complete_cb(int result) override;

	// banked register map
	void map_reg(address_map &map) ATTR_COLD;

	// register read handlers
	u8 tx_ccl_r() { return u8(m_tx_cc >> 0); }
	u8 tx_cch_r() { return u8(m_tx_cc >> 8); }
	u8 ccl_r() { return u8(m_cc >> 0); }
	u8 cch_r() { return u8(m_cc >> 8); }
	u8 test_r() { return 0; }
	u8 flags_r() { return m_flags; }

	// register write handlers
	void multicast_filter_w(offs_t offset, u8 data);
	virtual void tx_command_w(u8 data) override;
	void control_w(u8 data);
	void config_w(u8 data);

	// helpers
	virtual bool address_filter(u8 *address) override;

	// 80c03 option helpers
	virtual bool mode_tx_pad() const override { return bool(m_config & CFG_TPA); }
	virtual bool mode_tx_crc() const override { return !bool(m_config & CFG_TNC); }
	virtual bool mode_rx_crc() const override { return bool(m_config & CFG_RXC); }

	required_device<address_map_bank_device> m_regbank;

	enum flags_mask : u8
	{
		FLAGS_SQE = 0x01,
		FLAGS_TNC = 0x02, // txen_no_carrier
	};
	enum control_mask : u8
	{
		CTL_TCC = 0x01, // transmit collision counter
		CTL_CC  = 0x02, // collision counter
		CTL_SQE = 0x04, // sqe function
		CTL_MHF = 0x08, // multicast hash filter
		CTL_DRF = 0x10, // discard short frames
		CTL_TNC = 0x20, // txen_no_carrier function
	};
	enum config_mask : u8
	{
		CFG_GAM = 0x01, // group address
		CFG_TPA = 0x02, // transmit packet autopad
		CFG_TNP = 0x04, // transmit no preamble
		CFG_RTD = 0x08, // receive own transmit disable
		CFG_TNC = 0x10, // transmit no crc
		CFG_FDX = 0x20, // full duplex
		CFG_RXC = 0x40, // receive crc
		CFG_FRD = 0x80, // fast receive discard
	};
	u16 m_tx_cc;
	u16 m_cc;
	u8 m_flags;

	u8 m_control;
	u8 m_config;

	u64 m_multicast_filter;
};

DECLARE_DEVICE_TYPE(SEEQ8003,  seeq8003_device)
DECLARE_DEVICE_TYPE(SEEQ80C03, seeq80c03_device)

#endif // MAME_MACHINE_EDLC_H
