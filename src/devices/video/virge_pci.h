// license:BSD-3-Clause
// copyright-holders:Barry Rodewald

/*
 *   S3 ViRGE 3D accelerator card
 */

#ifndef MAME_VIDEO_VIRGE_PCI_H
#define MAME_VIDEO_VIRGE_PCI_H

#pragma once

#include "machine/pci.h"
#include "bus/isa/s3virge.h"

class virge_pci_device : public pci_device
{
public:
	template <typename T>
	virge_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&screen_tag)
		: virge_pci_device(mconfig, tag, owner, clock)
	{
		set_screen_tag(std::forward<T>(screen_tag));
	}

	enum
	{
		LAW_64K = 0,
		LAW_1MB,
		LAW_2MB,
		LAW_4MB
	};

	virge_pci_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virge_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void lfb_map(address_map &map);
	void mmio_map(address_map& map);

	DECLARE_READ32_MEMBER(vga_3b0_r);
	DECLARE_WRITE32_MEMBER(vga_3b0_w);
	DECLARE_READ32_MEMBER(vga_3c0_r);
	DECLARE_WRITE32_MEMBER(vga_3c0_w);
	DECLARE_READ32_MEMBER(vga_3d0_r);
	DECLARE_WRITE32_MEMBER(vga_3d0_w);
	DECLARE_READ8_MEMBER(vram_r);
	DECLARE_WRITE8_MEMBER(vram_w);
	DECLARE_READ32_MEMBER(base_address_r);
	DECLARE_WRITE32_MEMBER(base_address_w);

protected:
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
		uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;
	virtual void config_map(address_map &map) override;

	void postload(void);

	required_device<s3virge_vga_device> m_vga;
	required_memory_region m_bios;
	optional_device<screen_device> m_screen;

private:
	void refresh_linear_window();

	uint8_t m_current_crtc_reg;
};

class virgedx_pci_device : public virge_pci_device
{
public:
	template <typename T>
	virgedx_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&screen_tag)
		: virgedx_pci_device(mconfig, tag, owner, clock)
	{
		set_screen_tag(std::forward<T>(screen_tag));
	}
	virgedx_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

//  required_device<s3virge_vga_device> m_vga;
//  required_memory_region m_bios;
//  optional_device<screen_device> m_screen;
};

DECLARE_DEVICE_TYPE(VIRGE_PCI, virge_pci_device)
DECLARE_DEVICE_TYPE(VIRGEDX_PCI, virgedx_pci_device)

#endif
