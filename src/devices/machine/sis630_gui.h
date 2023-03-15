// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_MACHINE_SIS630_VGA_H
#define MAME_MACHINE_SIS630_VGA_H

#pragma once

#include "pci.h"
#include "video/pc_vga.h"

class sis630_svga_device : public svga_device
{
public:
	sis630_svga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t mem_r(offs_t offset) override;
	virtual void mem_w(offs_t offset, uint8_t data) override;

	virtual u8 port_03c0_r(offs_t offset) override;
	virtual void port_03c0_w(offs_t offset, uint8_t data) override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual uint8_t crtc_reg_read(uint8_t index) override;
	virtual void crtc_reg_write(uint8_t index, uint8_t data) override;
	virtual uint8_t seq_reg_read(uint8_t index) override;
	virtual void seq_reg_write(uint8_t index, uint8_t data) override;
	virtual uint16_t offset() override;
	virtual void recompute_params() override;

	u8 m_crtc_ext_regs[0x100]{};
	u8 m_seq_ext_regs[0x100]{};
	u8 m_ramdac_mode = 0;
	u8 m_ext_misc_ctrl_0 = 0;
	u8 m_ext_vert_overflow = 0;
	u8 m_ext_horz_overflow[2]{};
	u32 m_svga_bank_reg_w = 0;
	u32 m_svga_bank_reg_r = 0;
	bool m_unlock_reg = false;

	std::tuple<u8, u8> flush_true_color_mode();
//  bool m_dual_seg_mode = false;
};

DECLARE_DEVICE_TYPE(SIS630_SVGA, sis630_svga_device)


class sis630_gui_device : public pci_device
{
public:
	sis630_gui_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void legacy_memory_map(address_map &map);
	void legacy_io_map(address_map &map);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual const tiny_rom_entry *device_rom_region() const override;

//  virtual void reset_all_mappings() override;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override;

	void memory_map(address_map &map);
	void io_map(address_map &map);
	void space_io_map(address_map &map);

private:
	required_device<sis630_svga_device> m_svga;
	required_memory_region m_gui_rom;

	u8 vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);
	u32 vga_3b0_r(offs_t offset, uint32_t mem_mask = ~0);
	void vga_3b0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	u32 vga_3c0_r(offs_t offset, uint32_t mem_mask = ~0);
	void vga_3c0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	u32 vga_3d0_r(offs_t offset, uint32_t mem_mask = ~0);
	void vga_3d0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void subvendor_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	virtual u8 capptr_r() override;
	u32 agp_id_r();
	u32 agp_status_r();
	u32 agp_command_r(offs_t offset, uint32_t mem_mask);
	void agp_command_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	struct {
		bool enable = false;
		u8 data_rate = 0;
	} m_agp;

	u32 m_subsystem_logger_mask = 0;

	u8 unmap_log_r(offs_t offset);
	void unmap_log_w(offs_t offset, u8 data);
};

DECLARE_DEVICE_TYPE(SIS630_GUI, sis630_gui_device)

class sis630_bridge_device : public pci_bridge_device
{
public:
	template <typename T> sis630_bridge_device(
		const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock,
		T &&gui_tag
	) : sis630_bridge_device(mconfig, tag, owner, clock)
	{
		// either 0001 or 6001 as device ID
		set_ids_bridge(0x10396001, 0x00);
		//set_multifunction_device(true);
		m_vga.set_tag(std::forward<T>(gui_tag));
	}

	sis630_bridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

private:
	required_device<sis630_gui_device> m_vga;

	virtual void bridge_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;


};

DECLARE_DEVICE_TYPE(SIS630_BRIDGE, sis630_bridge_device)


#endif
