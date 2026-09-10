// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MACHINE_XN297L_H
#define MAME_MACHINE_XN297L_H

#pragma once


class xn297l_device : public device_t
{
public:
	xn297l_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	static constexpr feature_type unemulated_features() { return feature::COMMS; }

	// Byte-level interface for the chip's 3-wire SPI bus.
	u8 transfer(u8 data);
	void cs_w(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr unsigned MAX_PAYLOAD = 64;
	static constexpr unsigned FIFO_DEPTH = 2;

	TIMER_CALLBACK_MEMBER(transmit_complete);

	void reset_registers();
	u8 register_r(u8 reg, u8 offset) const;
	void register_w(u8 reg, u8 offset, u8 data);
	u8 status_r() const;
	u8 fifo_status_r() const;
	unsigned register_width(u8 reg) const;
	unsigned payload_limit() const;
	unsigned fifo_limit() const;
	void finish_command();
	void flush_tx();
	void flush_rx();
	void start_transmit();
	void pop_tx();
	void pop_rx();

	u8 m_registers[0x20][6];
	u8 m_tx_fifo[FIFO_DEPTH][MAX_PAYLOAD];
	u8 m_tx_length[FIFO_DEPTH];
	u8 m_tx_no_ack[FIFO_DEPTH];
	u8 m_tx_count;
	u8 m_rx_fifo[FIFO_DEPTH][MAX_PAYLOAD];
	u8 m_rx_length[FIFO_DEPTH];
	u8 m_rx_pipe[FIFO_DEPTH];
	u8 m_rx_count;

	u8 m_command;
	u8 m_command_pos;
	u8 m_payload[MAX_PAYLOAD];
	u8 m_payload_length;
	u8 m_payload_no_ack;
	bool m_cs;
	bool m_ce;
	bool m_reset_hold;
	bool m_features_active;
	bool m_tx_reuse;
	bool m_transmitting;
	emu_timer *m_transmit_timer;

	static constexpr u8 CMD_R_REGISTER           = 0x00;
	static constexpr u8 CMD_W_REGISTER           = 0x20;
	static constexpr u8 CMD_R_RX_PL_WID          = 0x60;
	static constexpr u8 CMD_R_RX_PAYLOAD         = 0x61;
	static constexpr u8 CMD_W_TX_PAYLOAD         = 0xa0;
	static constexpr u8 CMD_FLUSH_TX             = 0xe1;
	static constexpr u8 CMD_FLUSH_RX             = 0xe2;
	static constexpr u8 CMD_REUSE_TX_PL          = 0xe3;
	static constexpr u8 CMD_W_TX_PAYLOAD_NO_ACK  = 0xb0;
	static constexpr u8 CMD_ACTIVATE             = 0x50;
	static constexpr u8 CMD_RESET                = 0x53;
	static constexpr u8 CMD_CE_FSPI_OFF          = 0xfc;
	static constexpr u8 CMD_CE_FSPI_ON           = 0xfd;
	static constexpr u8 CMD_NOP                  = 0xff;

	static constexpr u8 REG_CONFIG       = 0x00;
	static constexpr u8 REG_EN_AA        = 0x01;
	static constexpr u8 REG_RF_CH        = 0x05;
	static constexpr u8 REG_STATUS       = 0x07;
	static constexpr u8 REG_OBSERVE_TX   = 0x08;
	static constexpr u8 REG_RX_ADDR_P0   = 0x0a;
	static constexpr u8 REG_RX_ADDR_P1   = 0x0b;
	static constexpr u8 REG_TX_ADDR      = 0x10;
	static constexpr u8 REG_FIFO_STATUS  = 0x17;
	static constexpr u8 REG_FEATURE      = 0x1d;
};

DECLARE_DEVICE_TYPE(XN297L, xn297l_device)

#endif // MAME_MACHINE_XN297L_H
