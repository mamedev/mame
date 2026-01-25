// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay, O. Galibert
#ifndef MAME_OMRON_LUNA_68K_IOC_H
#define MAME_OMRON_LUNA_68K_IOC_H

#pragma once

#include "cpu/m68000/m68000.h"
#include "machine/hd63450.h"
#include "machine/mb87030.h"
#include "machine/wd_fdc.h"
#include "imagedev/floppy.h"
#include "machine/z80scc.h"
#include "machine/z8536.h"

class luna_68k_ioc_device : public device_t {
public:
	luna_68k_ioc_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock=0);

	auto interrupt_cb() { return m_interrupt_cb.bind(); }
	auto interrupt_scsii_cb() { return m_interrupt_scsii_cb.bind(); }
	auto interrupt_scsie_cb() { return m_interrupt_scsie_cb.bind(); }
	auto interrupt_dma0_cb() { return m_interrupt_dma0_cb.bind(); }
	auto interrupt_dma1_cb() { return m_interrupt_dma1_cb.bind(); }

	void vme_map(address_map &map);
	u16 shared_ram_r(offs_t offset);
	void shared_ram_w(offs_t offset, u16 data, u16 mem_mask);
	u8 get_i2m() const { return m_i2m; }

public:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	required_device<m68000_device> m_cpu;
	required_device_array<hd63450_device, 2> m_dma;
	required_device_array<mb89352_device, 2> m_spc;
	required_device<mb8877_device> m_fdc;
	required_device<floppy_connector> m_floppy;
	required_device<z80scc_device> m_scc;
	required_device<z8536_device> m_cio;
	required_shared_ptr<u16> m_ram;
	memory_view m_boot;
	devcb_write_line m_interrupt_cb;
	devcb_write_line m_interrupt_scsii_cb;
	devcb_write_line m_interrupt_scsie_cb;
	devcb_write_line m_interrupt_dma0_cb;
	devcb_write_line m_interrupt_dma1_cb;

	u8 m_m2i;
	u8 m_i2m;
	u16 m_direction;
	int m_interrupt_hack;

	void cpu_map(address_map &map) ATTR_COLD;
	void cpuspace_map(address_map &map) ATTR_COLD;

	void direction_w(u16 data);
	u16 direction_r();
	void m2i_w(u32 data);
	u8 m2i_r();
	void m2i_int_clear(u8 data);
	void i2m_w(u8 data);
	u32 i2m_r();
	void i2m_int_clear(u32 data);
	u8 cio_vector_r();
	void reset_cb(int);

	static void scsi_devices(device_slot_interface &device);
	static void floppies(device_slot_interface &device);
};

DECLARE_DEVICE_TYPE(LUNA_68K_IOC, luna_68k_ioc_device)

#endif
