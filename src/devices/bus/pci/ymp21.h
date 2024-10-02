// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// PCI interfacing gate array common to the sw1000xg and the ds2416

#ifndef MAME_BUS_PCI_YMP21_H
#define MAME_BUS_PCI_YMP21_H

#pragma once

#include "pci_slot.h"

#include <array>


class ymp21_device : public pci_card_device {
protected:
	devcb_write_line::array<2> m_tx_cb;

	ymp21_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	memory_share_creator<u32> m_dsp3_buffer;
	std::array<u32, 5> m_dsp3_ctrl1;
	std::array<u32, 5> m_dsp3_glob1, m_dsp3_glob2, m_dsp3_glob3;

	emu_timer *m_rx_timer[2], *m_tx_timer[2];

	void map(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(uart_rx);
	TIMER_CALLBACK_MEMBER(uart_tx);

	void uart_data_w(offs_t offset, u8 data);
	u8 uart_data_r(offs_t offset);
	void uart_ctrl_w(offs_t offset, u8 data);
	u8 uart_status_r(offs_t offset);

	void port0_w(u32 data);
	u32 port0_r();
	void port1_w(u32 data);
	void irq_w(u32 data);
	u32 status_r();

	void dsp3_buffer_w(offs_t offset, u32 data, u32 mem_mask);
	void dsp3_ctrl1_w(offs_t offset, u32 data);
	void dsp3_ctrl2_w(offs_t offset, u32 data);
	u32 dsp3_status_r(offs_t offset);

	void dsp3_glob1_w(offs_t offset, u32 data);
	void dsp3_glob2_w(offs_t offset, u32 data);
	void dsp3_glob3_w(offs_t offset, u32 data);
};

#endif
