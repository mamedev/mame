// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_MACHINE_SIS630_VGA_H
#define MAME_MACHINE_SIS630_VGA_H

#pragma once

#include "pci.h"
#include "video/pc_vga_sis.h"

class sis630_gui_device : public pci_device
{
public:
	sis630_gui_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void legacy_memory_map(address_map &map) ATTR_COLD;
	void legacy_io_map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

//  virtual void reset_all_mappings() override;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;

	void memory_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void space_io_map(address_map &map) ATTR_COLD;

private:
	required_device<sis630_vga_device> m_vga;
	required_memory_region m_gui_rom;

	u8 vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);

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
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

private:
	required_device<sis630_gui_device> m_vga;

	virtual void bridge_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
};

DECLARE_DEVICE_TYPE(SIS630_BRIDGE, sis630_bridge_device)


#endif
