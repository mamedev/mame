// license:BSD-3-Clause
// copyright-holders:Ted Green
// 3dfx Voodoo Graphics SST-1/2 emulator.

#ifndef MAME_VIDEO_VOODOO_PCI_H
#define MAME_VIDEO_VOODOO_PCI_H

#pragma once

#include "machine/pci.h"
#include "voodoo.h"

#define MCFG_VOODOO_PCI_ADD(_tag,  _type, _cpu_tag) \
	voodoo_pci_device::set_type(_type); \
	MCFG_PCI_DEVICE_ADD(_tag, VOODOO_PCI, 0, 0, 0, 0) \
	downcast<voodoo_pci_device *>(device)->set_cpu_tag(_cpu_tag);

#define MCFG_VOODOO_PCI_FBMEM(_value) \
	downcast<voodoo_pci_device *>(device)->set_fbmem(_value);

#define MCFG_VOODOO_PCI_TMUMEM(_value1, _value2) \
	downcast<voodoo_pci_device *>(device)->set_tmumem(_value1, _value2);

class voodoo_pci_device : public pci_device {
public:
	voodoo_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	virtual void config_map(address_map &map) override;

	void postload(void);
	void set_cpu_tag(const char *tag);
	static void set_type(const int type) {m_type = type;}
	void set_fbmem(const int fbmem) {m_fbmem = fbmem;}
	void set_tmumem(const int tmumem0, const int tmumem1) {m_tmumem0 = tmumem0; m_tmumem1 = tmumem1;}

	DECLARE_READ32_MEMBER(pcictrl_r);
	DECLARE_WRITE32_MEMBER(pcictrl_w);

	DECLARE_READ32_MEMBER(vga_r);
	DECLARE_WRITE32_MEMBER(vga_w);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<voodoo_device> m_voodoo;
	static int m_type; // FIXME: all voodoo have to be the same?  really?
	int m_fbmem, m_tmumem0, m_tmumem1;
	const char *m_cpu_tag;

	uint32_t m_pcictrl_reg[0x20];
	void voodoo_reg_map(address_map &map);
	void banshee_reg_map(address_map &map);
	void lfb_map(address_map &map);
	void io_map(address_map &map);
};

DECLARE_DEVICE_TYPE(VOODOO_PCI, voodoo_pci_device)

#endif // MAME_VIDEO_VOODOO_PCI_H
