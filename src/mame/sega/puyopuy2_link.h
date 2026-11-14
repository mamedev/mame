// license:BSD-3-Clause
// copyright-holders:Damian Rogers
#ifndef MAME_SEGA_PUYOPUY2_LINK_H
#define MAME_SEGA_PUYOPUY2_LINK_H

#pragma once

#include "segac2_cn4.h"

#include <deque>
#include <memory>
#include <vector>


class puyopuy2_link_device : public device_t, public device_segac2_cn4_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::COMMS; }
	static constexpr flags_type emulation_flags() { return flags::SAVE_UNSUPPORTED; }

	puyopuy2_link_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);
	~puyopuy2_link_device();

	// device_segac2_cn4_interface implementation
	virtual u8 read(offs_t offset) override;
	virtual void write(offs_t offset, u8 data) override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	class context;

	u8 window_r(offs_t offset);
	void window_w(offs_t offset, u8 data);
	void reset_w(u8 data);

	void board_reset();
	bool link_ok() const { return m_local_ready && m_remote_ready; }
	void poll();
	void fill_rx();
	void transmit(u32 count);
	void send_state();
	void start_link() ATTR_COLD;

	std::unique_ptr<context> m_context;

	// board state
	u8 m_page;
	bool m_in_reset;
	bool m_local_ready;
	bool m_remote_ready;
	u8 m_node;
	u8 m_tx_read;
	u8 m_rx_write;
	u8 m_ctrl[8];
	u8 m_id_page[8];
	u8 m_tx_buf[256];
	u8 m_rx_buf[256];

	// host side (not emulation state)
	std::vector<u8> m_stream;
	std::deque<u8> m_rx_backlog;
	u32 m_seen_generation;
};


DECLARE_DEVICE_TYPE(PUYOPUY2_LINK, puyopuy2_link_device)

#endif // MAME_SEGA_PUYOPUY2_LINK_H
