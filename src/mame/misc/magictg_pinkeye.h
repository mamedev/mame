// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_MISC_MAGICTG_PINKEYE_H
#define MAME_MISC_MAGICTG_PINKEYE_H

#pragma once

#include "machine/pci.h"
#include "video/zr36050.h"

#include <memory>


class acclaim_pinkeye_device : public pci_device
{
public:
	acclaim_pinkeye_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto irq_cb() { return m_irq_cb.bind(); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<zr36050_device> m_zr36050;
	required_region_ptr<uint8_t> m_flash;
	required_region_ptr<uint8_t> m_key;
	memory_share_creator<uint8_t> m_alpha;
	devcb_write_line m_irq_cb;

	emu_timer *m_done_timer;
	uint32_t m_regs[0x4c / 4];
	uint32_t m_po;
	uint8_t m_fpga[2][8];
	uint32_t m_flash_ptr;
	uint8_t m_zr050_addr;
	uint8_t m_zr016_regs[2];
	uint8_t m_zr016_iaddr;
	uint8_t m_zr016_iregs[0x10];
	std::unique_ptr<uint8_t []> m_mask;

	void regs_map(address_map &map) ATTR_COLD;
	uint32_t regs_r(offs_t offset);
	void regs_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint8_t guest_r(int guest, int reg);
	void guest_w(int guest, int reg, uint8_t data);
	void code_transfer();
	offs_t decode(offs_t dest);
	void decode_key(offs_t address, int width, int height);
	uint8_t code_r();
	TIMER_CALLBACK_MEMBER(transfer_done);
	void update_irq();
};

DECLARE_DEVICE_TYPE(ACCLAIM_PINKEYE, acclaim_pinkeye_device)

#endif // MAME_MISC_MAGICTG_PINKEYE_H
