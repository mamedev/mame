// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev

#ifndef MAME_BUS_MULTIBUS_ROBOTRON_K7070_H
#define MAME_BUS_MULTIBUS_ROBOTRON_K7070_H

#pragma once

#include "multibus.h"

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"

#include "emupal.h"
#include "screen.h"


class robotron_k7070_device
	: public device_t
	, public device_multibus_interface
{
public:
	robotron_k7070_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	void cpu_mem(address_map &map) ATTR_COLD;
	virtual void cpu_pio(address_map &map) ATTR_COLD;

private:
	required_device<z80_device> m_cpu;
	required_device<z80ctc_device> m_ctc;
	required_device<z80sio_device> m_sio;
	required_device_array<rs232_port_device, 2> m_serial;
	required_device<palette_device> m_palette;
	required_shared_ptr<u8> m_ram;
	required_ioport_array<2> m_dsel;

	memory_view m_view_lo;
	memory_view m_view_hi;

	u16 m_start;

	bool m_abg_msel, m_kgs_iml;
	uint8_t m_abg_func, m_abg_split;
	uint16_t m_abg_addr;
	uint8_t m_kgs_datao, m_kgs_datai, m_kgs_ctrl;
	bool m_nmi_enable;

	uint8_t kgs_host_r(offs_t offset);
	void kgs_host_w(offs_t offset, uint8_t data);
	void abg_addr_w(offs_t offset, uint8_t data);
	void abg_func_w(offs_t offset, uint8_t data);
	void abg_split_w(offs_t offset, uint8_t data);
	void abg_misc_w(offs_t offset, uint8_t data);
	void kgs_memory_remap();

	uint32_t screen_update_k7072(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void palette_init(palette_device &palette);

	uint16_t io_r(offs_t offset, uint16_t mem_mask);
	void io_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	void k7070_cpu_io(address_map &map) ATTR_COLD;
	void k7070_cpu_mem(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(ROBOTRON_K7070, robotron_k7070_device)

#endif // MAME_BUS_MULTIBUS_ROBOTRON_K7070_H
