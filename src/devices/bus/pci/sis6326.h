// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_PCI_SIS6326_H
#define MAME_BUS_PCI_SIS6326_H

#pragma once

#include "pci_slot.h"
#include "video/pc_vga_sis.h"

class sis6326_pci_device : public pci_card_device
{
public:
	sis6326_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// Partial working 2d graphics, no Turbo Queue, no 3d graphics, no TV Out
	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

protected:
	sis6326_pci_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;

	void vram_aperture_map(address_map &map) ATTR_COLD;
	void mmio_map(address_map &map) ATTR_COLD;
	void vmi_map(address_map &map) ATTR_COLD;

	required_device<sis6326_vga_device> m_vga;
	required_memory_region m_bios;
private:
	void legacy_memory_map(address_map &map) ATTR_COLD;
	void legacy_io_map(address_map &map) ATTR_COLD;

	u8 vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);

	// BitBlt engine
	void trigger_2d_command();
	u32 m_src_start_addr;
	u32 m_dst_start_addr;
	u8 m_draw_sel;
	u16 m_src_pitch, m_dst_pitch;
	u16 m_rect_width, m_rect_height;
	u32 m_fg_color, m_bg_color;
	u8 m_fg_rop, m_bg_rop;
	u8 m_mask[8];
	u16 m_clip_top, m_clip_left, m_clip_bottom, m_clip_right;

	u16 m_draw_command;
	u8 m_pattern_data[128];

	typedef u8 (sis6326_pci_device::*get_src_func)(u32 offset_base, u32 x);
	static const get_src_func get_src_table[4];
	u8 get_src_bgcol(u32 offset_base, u32 x);
	u8 get_src_fgcol(u32 offset_base, u32 x);
	u8 get_src_mem(u32 offset_base, u32 x);
	u8 get_src_cpu(u32 offset_base, u32 x);

	typedef u8 (sis6326_pci_device::*get_pat_func)(u32 offset_base, u32 x);
	static const get_pat_func get_pat_table[4];
	u8 get_pat_bgcol(u32 y, u32 x);
	u8 get_pat_fgcol(u32 y, u32 x);
	u8 get_pat_regs(u32 y, u32 x);
	u8 get_pat_ddraw(u32 y, u32 x);
	u32 GetROP(u8 rop, u32 src, u32 dst, u32 pat);

};

class sis6326_agp_device : public sis6326_pci_device
{
public:
	sis6326_agp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void config_map(address_map &map) override ATTR_COLD;

	virtual u8 capptr_r() override;
private:
	struct {
		bool enable = false;
		u8 data_rate = 0;
	} m_agp;

	u32 agp_command_r(offs_t offset, uint32_t mem_mask);
	void agp_command_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
};

DECLARE_DEVICE_TYPE(SIS6326_PCI, sis6326_pci_device)
DECLARE_DEVICE_TYPE(SIS6326_AGP, sis6326_agp_device)

#endif // MAME_BUS_PCI_SIS6326_H
