// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev

#ifndef MAME_BUS_MULTIBUS_ROBOTRON_K7071_H
#define MAME_BUS_MULTIBUS_ROBOTRON_K7071_H

#pragma once

#include "multibus.h"

#include "cpu/z80/z80.h"
#include "machine/i8257.h"
#include "video/i8275.h"

#include "emupal.h"
#include "screen.h"


class robotron_k7071_device
	: public device_t
	, public device_multibus_interface
{
public:
	robotron_k7071_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	void cpu_mem(address_map &map) ATTR_COLD;
	virtual void cpu_pio(address_map &map) ATTR_COLD;

private:
	required_device<z80_device> m_cpu;
	required_device<i8257_device> m_dma;
	required_device<i8275_device> m_crtc;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_shared_ptr<u8> m_ram;
	required_region_ptr<u8> m_p_chargen;

	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_program;

	uint8_t m_kgs_datao, m_kgs_datai, m_kgs_ctrl;
	bool m_nmi_enable;

	I8275_DRAW_CHARACTER_MEMBER(display_pixels);
	void hrq_w(int state);

	uint8_t kgs_host_r(offs_t offset);
	void kgs_host_w(offs_t offset, uint8_t data);

	uint16_t io_r(offs_t offset, uint16_t mem_mask);
	void io_w(offs_t offset, uint16_t data, uint16_t mem_mask);
};

DECLARE_DEVICE_TYPE(ROBOTRON_K7071, robotron_k7071_device)

#endif // MAME_BUS_MULTIBUS_ROBOTRON_K7071_H
