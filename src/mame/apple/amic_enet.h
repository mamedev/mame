// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_APPLE_AMIC_ENET_H
#define MAME_APPLE_AMIC_ENET_H

#pragma once

#include "machine/am79c940.h"

// Ethernet portion of the first Power Macs' AMIC DMA controller.
class amic_enet_device : public device_t
{
public:
	amic_enet_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	template <typename T> void set_space(T &&tag, int spacenum) { m_space.set_tag(std::forward<T>(tag), spacenum); }
	template <typename T> void set_mace_tag(T &&tag) { m_mace.set_tag(std::forward<T>(tag)); }
	auto rx_irq_out() { return m_rx_irq_out.bind(); }
	auto tx_irq_out() { return m_tx_irq_out.bind(); }
	void rx_drq_w(int state);
	void tx_drq_w(int state);
	void set_dma_base(u32 address) { m_dma_base = address & 0xfffc0000; }
	void map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

private:
	static constexpr u8 RESET = 0x01, RUN = 0x02, IE = 0x08, OVERRUN = 0x40, IF = 0x80;
	static constexpr unsigned RING_PAGES = 0xc0, RING_SIZE = RING_PAGES * 256;
	u8 rx_control_r() { return m_rx_control; }
	u8 tx_control_r();
	void rx_control_w(u8 data);
	void tx_control_w(u8 data);
	u8 rx_head_r() { return m_rx_head; }
	u8 rx_tail_r() { return m_rx_tail; }
	void rx_tail_w(u8 data);
	u8 tx_count_r(offs_t offset);
	void tx_count_w(offs_t offset, u8 data);
	void update_irqs();
	void kick();
	void receive_complete();
	void log_packet(const char *direction, const u8 *data, unsigned length);
	TIMER_CALLBACK_MEMBER(dma_tick);

	required_address_space m_space;
	required_device<am79c940_device> m_mace;
	devcb_write_line m_rx_irq_out, m_tx_irq_out;
	emu_timer *m_timer;
	u32 m_dma_base;
	u8 m_rx_control, m_tx_control, m_rx_head, m_rx_tail;
	bool m_rx_full, m_rx_drq, m_tx_drq;
	u16 m_tx_count[2], m_tx_position[2];
	u8 m_tx_set;
	u8 m_rx_packet[4095], m_rx_status[4];
	u16 m_rx_length;
	u8 m_rx_status_count;
};

DECLARE_DEVICE_TYPE(AMIC_ENET, amic_enet_device)

#endif // MAME_APPLE_AMIC_ENET_H
