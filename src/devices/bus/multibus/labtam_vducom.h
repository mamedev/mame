// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_BUS_MULTIBUS_LABTAM_VDUCOM_H
#define MAME_BUS_MULTIBUS_LABTAM_VDUCOM_H

#pragma once

#include "multibus.h"

#include "cpu/i86/i86.h"
#include "machine/am9513.h"
#include "machine/pic8259.h"
#include "machine/x2212.h"
#include "machine/z80sio.h"
#include "video/mc6845.h"

#include "bus/rs232/rs232.h"

#include "emupal.h"
#include "screen.h"


class labtam_vducom_device_base
	: public device_t
	, public device_multibus_interface
{
protected:
	labtam_vducom_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void cpu_mem(address_map &map) ATTR_COLD;
	virtual void cpu_pio(address_map &map) ATTR_COLD;

	void bus_mem_w(offs_t offset, u16 data, u16 mem_mask);
	u16 bus_mem_r(offs_t offset, u16 mem_mask);
	void bus_pio_w(offs_t offset, u16 data, u16 mem_mask) { m_bus->space(AS_IO).write_word(offset << 1, data, mem_mask); }
	u16 bus_pio_r(offs_t offset, u16 mem_mask) { return m_bus->space(AS_IO).read_word(offset << 1, mem_mask); }

	u8 nvram_r(address_space &space, offs_t offset) { return m_nvram[1]->read(space, offset) << 4 | m_nvram[0]->read(space, offset); }
	void nvram_w(offs_t offset, u8 data) { m_nvram[0]->write(offset, data & 0x0f); m_nvram[1]->write(offset, data >> 4); }

	u8 u15_r(offs_t offset) { return BIT(m_e4->read(), offset); }
	void u7_w(offs_t offset, u8 data);

	u16 start() const { return m_start; }
	u8 u7() const { return m_u7; }

	u8 nvram_recall();
	void nvram_store(u8 data);

	required_device<i8086_cpu_device> m_cpu;
	required_device<pic8259_device> m_pic;
	required_device_array<am9513_device, 2> m_ctc;
	required_device_array<z80sio_device, 2> m_com;
	required_device_array<x2212_device, 2> m_nvram;

	required_device_array<rs232_port_device, 4> m_serial;

private:
	required_ioport m_e4;
	memory_view m_mbus;

	u16 m_start;
	u8 m_u7;

	bool m_installed;
};

class labtam_8086cpu_device : public labtam_vducom_device_base
{
public:
	labtam_8086cpu_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

class labtam_vducom_device : public labtam_vducom_device_base
{
public:
	labtam_vducom_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void cpu_pio(address_map &map) override ATTR_COLD;

private:
	void palette_init(palette_device &palette);
	MC6845_UPDATE_ROW(update_row);

	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	required_shared_ptr_array<u16, 2> m_ram;
};

DECLARE_DEVICE_TYPE(LABTAM_8086CPU, labtam_8086cpu_device)
DECLARE_DEVICE_TYPE(LABTAM_VDUCOM,  labtam_vducom_device)

#endif // MAME_BUS_MULTIBUS_LABTAM_VDUCOM_H
