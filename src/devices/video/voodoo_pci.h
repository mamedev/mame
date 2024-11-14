// license:BSD-3-Clause
// copyright-holders:Ted Green
// 3dfx Voodoo Graphics SST-1/2 emulator.

#ifndef MAME_VIDEO_VOODOO_PCI_H
#define MAME_VIDEO_VOODOO_PCI_H

#pragma once

#include "machine/pci.h"
#include "voodoo.h"
#include "voodoo_banshee.h"

class voodoo_pci_device : public pci_device
{
public:
	template <typename T> void set_cpu(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_screen(T &&tag) { m_screen.set_tag(std::forward<T>(tag)); }
	void set_fbmem(int fbmem) { m_fbmem = fbmem; }
	void set_tmumem(int tmumem0, int tmumem1) { m_tmumem0 = tmumem0; m_tmumem1 = tmumem1; }
	void set_status_cycles(u32 cycles) { m_status_cycles = cycles; }
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u32 vga_r(offs_t offset, u32 mem_mask = ~0);
	void vga_w(offs_t offset, u32 data, u32 mem_mask = ~0);

protected:
	voodoo_pci_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void map_extra(u64 memory_window_start, u64 memory_window_end, u64 memory_offset, address_space *memory_space,
							u64 io_window_start, u64 io_window_end, u64 io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;

	void postload(void);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	required_device<generic_voodoo_device> m_generic_voodoo;
	optional_device<cpu_device> m_cpu;
	optional_device<screen_device> m_screen;
	int m_fbmem, m_tmumem0, m_tmumem1;
	u32 m_status_cycles;

	u32 m_pcictrl_reg[0x20];

	u32 pcictrl_r(offs_t offset, u32 mem_mask = ~0);
	void pcictrl_w(offs_t offset, u32 data, u32 mem_mask = ~0);
};

class voodoo_1_pci_device : public voodoo_pci_device
{
public:
	template <typename T, typename U>
	voodoo_1_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&cpu_tag, U &&screen_tag)
		: voodoo_1_pci_device(mconfig, tag, owner, clock)
	{
		set_cpu(std::forward<T>(cpu_tag));
		set_screen(std::forward<U>(screen_tag));
	}

	voodoo_1_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	required_device<voodoo_1_device> m_voodoo;
};

class voodoo_2_pci_device : public voodoo_pci_device
{
public:
	template <typename T, typename U>
	voodoo_2_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&cpu_tag, U &&screen_tag)
		: voodoo_2_pci_device(mconfig, tag, owner, clock)
	{
		set_cpu(std::forward<T>(cpu_tag));
		set_screen(std::forward<U>(screen_tag));
	}

	voodoo_2_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	required_device<voodoo_2_device> m_voodoo;
};

class voodoo_banshee_pci_device : public voodoo_pci_device
{
public:
	template <typename T, typename U>
	voodoo_banshee_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&cpu_tag, U &&screen_tag)
		: voodoo_banshee_pci_device(mconfig, tag, owner, clock)
	{
		set_cpu(std::forward<T>(cpu_tag));
		set_screen(std::forward<U>(screen_tag));
	}

	voodoo_banshee_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	voodoo_banshee_pci_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void map_extra(u64 memory_window_start, u64 memory_window_end, u64 memory_offset, address_space *memory_space,
							u64 io_window_start, u64 io_window_end, u64 io_offset, address_space *io_space) override;
	required_device<voodoo_banshee_device> m_voodoo;
};

class voodoo_banshee_x86_pci_device : public voodoo_banshee_pci_device
{
public:
	template <typename T, typename U>
	voodoo_banshee_x86_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&cpu_tag, U &&screen_tag)
		: voodoo_banshee_x86_pci_device(mconfig, tag, owner, clock)
	{
		set_cpu(std::forward<T>(cpu_tag));
		set_screen(std::forward<U>(screen_tag));
	}

	voodoo_banshee_x86_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	required_memory_region m_vga_rom;
};

class voodoo_3_pci_device : public voodoo_pci_device
{
public:
	template <typename T, typename U>
	voodoo_3_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&cpu_tag, U &&screen_tag)
		: voodoo_3_pci_device(mconfig, tag, owner, clock)
	{
		set_cpu(std::forward<T>(cpu_tag));
		set_screen(std::forward<U>(screen_tag));
	}

	voodoo_3_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void map_extra(u64 memory_window_start, u64 memory_window_end, u64 memory_offset, address_space *memory_space,
							u64 io_window_start, u64 io_window_end, u64 io_offset, address_space *io_space) override;
	required_device<voodoo_3_device> m_voodoo;
};

DECLARE_DEVICE_TYPE(VOODOO_1_PCI, voodoo_1_pci_device)
DECLARE_DEVICE_TYPE(VOODOO_2_PCI, voodoo_2_pci_device)
DECLARE_DEVICE_TYPE(VOODOO_BANSHEE_PCI, voodoo_banshee_pci_device)
DECLARE_DEVICE_TYPE(VOODOO_BANSHEE_X86_PCI, voodoo_banshee_x86_pci_device)
DECLARE_DEVICE_TYPE(VOODOO_3_PCI, voodoo_3_pci_device)

#endif // MAME_VIDEO_VOODOO_PCI_H
