// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn

#ifndef MAME_MACHINE_XAVIX_DMA_H
#define MAME_MACHINE_XAVIX_DMA_H

#pragma once

#include <array>

class xavix_dma_device : public device_t
{
public:
	xavix_dma_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// The device clock is the peripheral clock. Callbacks retain full byte
	// addresses; source decoding and banked destination writes belong to the bus.
	auto source_read_callback() { return m_source_read_cb.bind(); }
	auto destination_write_callback() { return m_destination_write_cb.bind(); }
	auto irq_callback() { return m_irq_cb.bind(); }

	void map(address_map &map) ATTR_COLD;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u8 ctrl_r();
	void ctrl_w(u8 data);
	u8 src_r(offs_t offset);
	void src_w(offs_t offset, u8 data);
	u8 dst_r(offs_t offset);
	void dst_w(offs_t offset, u8 data);
	u8 len_r(offs_t offset);
	void len_w(offs_t offset, u8 data);
	u8 mode_r();
	void mode_w(u8 data);
	void schedule_transfer();
	void complete_transfer(bool acknowledge = false);
	void update_irq();
	TIMER_CALLBACK_MEMBER(transfer_tick);

	devcb_read8 m_source_read_cb;
	devcb_write8 m_destination_write_cb;
	devcb_write_line m_irq_cb;

	std::array<u8, 3> m_src;
	std::array<u8, 2> m_dst;
	std::array<u8, 2> m_len;
	u8 m_ctrl;
	u8 m_mode;
	u8 m_pending_data;
	bool m_byte_pending;
	bool m_in_destination_callback;
	bool m_destination_written;
	emu_timer *m_transfer_timer;
};

DECLARE_DEVICE_TYPE(XAVIX_DMA, xavix_dma_device)

#endif // MAME_MACHINE_XAVIX_DMA_H
