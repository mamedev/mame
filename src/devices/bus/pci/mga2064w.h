// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Angelo Salese
#ifndef MAME_BUS_PCI_MGA2064W_H
#define MAME_BUS_PCI_MGA2064W_H

#pragma once

#include "pci_slot.h"

#include "video/pc_vga_matrox.h"


class mga2064w_device : public pci_card_device, public device_memory_interface {
public:
	mga2064w_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void legacy_memory_map(address_map &map) ATTR_COLD;
	void legacy_io_map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;
	virtual void config_map(address_map &map) override ATTR_COLD;

	void mgabase1_map(address_map &map) ATTR_COLD;
	void mgabase2_map(address_map &map) ATTR_COLD;

	void dwgreg_map(address_map &map) ATTR_COLD;

	u32 dmawin_idump_r(offs_t offset, u32 mem_mask = ~0);
	void dmawin_iload_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	required_device<matrox_vga_device> m_svga;
	required_memory_region m_vga_rom;
private:
	u8 vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);

	address_space_config m_mgabase1_real_space_config;
	u32 mga_index_r();
	void mga_index_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 mga_data_r(offs_t offset, u32 mem_mask = ~0);
	void mga_data_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 m_mgabase1_real_index = 0;

	u32 status_r();
	u32 fifo_status_r();

	// DWGREG section
	void dwgctl_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void maccess_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	enum drawing_state_t {
		DRAW_IDLE,
		DRAW_ILOAD,
		DRAW_IDUMP
	};

	struct {
		u32 src[4]{};
		u32 dr[16]{};
		u32 ar[7]{};
		u32 dwgctl = 0;
		u32 maccess = 0;
		u32 zorg = 0;
		u32 plnwt = 0;
		u16 pitch = 0;
		u16 len = 0;
		u32 cytop = 0;
		u32 cybot = 0;
		u16 cxleft = 0;
		u16 cxright = 0;
		s16 fxleft = 0;
		s16 fxright = 0;
		s16 xdst = 0;
		s32 ydst = 0;
		u32 ydstorg = 0;
		u32 bcol = 0;
		u32 fcol = 0;
		u8 sellin = 0;
		drawing_state_t state;
		s32 current_x, current_y;
	} m_dwgreg;

	void draw_trigger();
};

DECLARE_DEVICE_TYPE(MGA2064W, mga2064w_device);

#endif // MAME_BUS_PCI_MGA2064W_H
