// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
#ifndef MAME_MACHINE_NRF2401_H
#define MAME_MACHINE_NRF2401_H

#pragma once

#include <array>

class nrf2401_device : public device_t
{
public:
	nrf2401_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void set_peer(nrf2401_device &peer) { m_peer.set_tag(peer); }
	auto dr1_callback() { return m_dr_cb[0].bind(); }
	auto dr2_callback() { return m_dr_cb[1].bind(); }

	void pwr_up_w(int state);
	void cs_w(int state);
	void ce_w(int state);
	void clock_w(int state);
	void clock2_w(int state);
	void data_w(int state) { m_data = bool(state); }
	int data_r() const;
	int data2_r() const;
	int dr1_r() const { return m_dr[0]; }
	int dr2_r() const { return m_dr[1]; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr unsigned CONFIG_BYTES = 18;
	static constexpr unsigned PACKET_BITS = 256;
	static constexpr u8 RESET_CONFIG[CONFIG_BYTES] =
	{
		0x04, 0x0f, 0x21, 0xe7, 0x00, 0x00, 0x00, 0x00, 0xe7,
		0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x1c, 0x08, 0x8e
	};

	static unsigned crc_bits(u8 config) { return BIT(config, 0) ? (BIT(config, 1) ? 16 : 8) : 0; }
	static int packet_bit(const u8 *data, unsigned bit) { return BIT(data[bit / 8], 7 - (bit & 7)); }
	static void set_packet_bit(u8 *data, unsigned bit, int state);
	bool receiving() const { return m_pwr_up && m_ce && !m_cs && !m_tx_busy && BIT(m_config[0], 0) && BIT(m_config[1], 6); }
	void set_dr(unsigned channel, bool state);
	void restart_receiver();
	void cancel_reception();
	void clock_rx(unsigned channel, bool state);
	void begin_transmit();
	void receive_packet(const u8 *data, unsigned bits, const u8 *config, const attotime &duration);
	TIMER_CALLBACK_MEMBER(transmit_tick);
	TIMER_CALLBACK_MEMBER(receive_tick);

	optional_device<nrf2401_device> m_peer;
	devcb_write_line::array<2> m_dr_cb;
	emu_timer *m_tx_timer;
	emu_timer *m_rx_timer[2];

	std::array<u8, CONFIG_BYTES> m_config;
	std::array<u8, CONFIG_BYTES> m_config_shift;
	u16 m_config_bits;
	std::array<u8, PACKET_BITS / 8> m_tx_buffer;
	u16 m_tx_bits;
	std::array<u8, PACKET_BITS / 8> m_packet;
	u16 m_packet_bits;
	u8 m_packet_config[3];
	bool m_tx_busy;
	u8 m_rx_buffer[2][PACKET_BITS / 8];
	u16 m_rx_bits[2];
	u16 m_rx_position[2];
	bool m_rx_pending[2];
	bool m_dr[2];
	bool m_pwr_up;
	bool m_cs;
	bool m_ce;
	bool m_clock[2];
	bool m_data;
	attotime m_power_ready;
	attotime m_rx_ready[2];
};

DECLARE_DEVICE_TYPE(NRF2401, nrf2401_device)

#endif // MAME_MACHINE_NRF2401_H
