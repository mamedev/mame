// license:BSD-3-Clause
// copyright-holders: BlueRain
/***************************************************************************

    NEC PC-9801-96

***************************************************************************/
#ifndef MAME_BUS_PC98_CBUS_PC9801_96_H
#define MAME_BUS_PC98_CBUS_PC9801_96_H

#pragma once

#include "slot.h"
#include "video/pc_vga_cirrus.h"


class pc9801_96_device : public device_t,
                         public device_pc98_cbus_slot_interface,
                         public device_memory_interface
{
public:
	pc9801_96_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void remap(int space_id, offs_t start, offs_t end) override;

	virtual space_config_vector memory_space_config() const override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	virtual void wab_map(address_map &map) ATTR_COLD;
	virtual uint8_t board_id_r();

private:
	required_device<cirrus_gd5428_vga_device> m_vga;

	const address_space_config m_wab_space_config;

	uint8_t m_reg_index;
	uint8_t m_reg_data[8];
	uint32_t m_vram_window_addr;
	uint32_t m_prev_vram_window_addr;
	uint32_t m_linear_vram_addr;
	uint8_t m_relay_ctrl;
	uint8_t m_video_enable;

	void io_map(address_map &map) ATTR_COLD;

	uint8_t reg_index_r();
	void reg_index_w(uint8_t data);
	uint8_t reg_data_r();
	void reg_data_w(uint8_t data);

	uint8_t video_enable_r();
	void video_enable_w(uint8_t data);

	uint8_t window_r();
	void window_w(uint8_t data);
	uint8_t linear_addr_r();
	void linear_addr_w(uint8_t data);
	uint8_t relay_r();
	void relay_w(uint8_t data);
	uint8_t window_index_r();

	uint8_t vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);

	uint8_t vga_read_3c0(offs_t offset);
	void vga_write_3c0(offs_t offset, uint8_t data);
	uint8_t vga_read_3b4(offs_t offset);
	void vga_write_3b4(offs_t offset, uint8_t data);
	uint8_t vga_read_3ba();
	void vga_write_3ba(uint8_t data);
	uint8_t vga_read_3d4(offs_t offset);
	void vga_write_3d4(offs_t offset, uint8_t data);
	uint8_t vga_read_3da();
	void vga_write_3da(uint8_t data);
};

DECLARE_DEVICE_TYPE(PC9801_96, pc9801_96_device)

#endif // MAME_BUS_PC98_CBUS_PC9801_96_H
