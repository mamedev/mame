// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// PCI interfacing gate array common to the sw1000xg and the ds2416

#ifndef MAME_SOUND_YMP21_H
#define MAME_SOUND_YMP21_H

#pragma once

#include "pci_slot.h"

class ymp21_device : public pci_card_device {
protected:
	ymp21_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual void device_start() override;
	virtual void device_reset() override;

private:
	memory_share_creator<u32> m_mailbox_buffer;
	std::array<u32, 5> m_mailbox_size;

	void map(address_map &map);

	void uart_data_w(offs_t offset, u8 data);
	u8 uart_data_r(offs_t offset);
	void uart_ctrl_w(offs_t offset, u8 data);
	u8 uart_status_r(offs_t offset);

	void port0_w(u32 data);
	u32 port0_r();
	void port1_w(u32 data);
	void irq_w(u32 data);
	u32 status_r();

	void mailbox_w(offs_t offset, u32 data, u32 mem_mask);
	void mailbox_size_w(offs_t offset, u32 data);
	u32 mailbox_size_r(offs_t offset);
	void mailbox_address_w(offs_t offset, u32 data);
};

#endif
