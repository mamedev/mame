// license:GPL-2.0+
// copyright-holders:Kevin Thacker,Sandro Ronco
/*****************************************************************************
 *
 * includes/z88.h
 *
 ****************************************************************************/

#ifndef MAME_ACORN_Z88_H
#define MAME_ACORN_Z88_H

#pragma once

#include "cpu/z80/z80.h"
#include "machine/bankdev.h"
#include "machine/nvram.h"
#include "machine/ram.h"
#include "upd65031.h"
#include "z88_impexp.h"
#include "sound/spkrdev.h"

#include "bus/rs232/rs232.h"
#include "bus/z88/flash.h"
#include "bus/z88/ram.h"
#include "bus/z88/rom.h"
#include "bus/z88/z88.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#define Z88_NUM_COLOURS 3

#define Z88_SCREEN_WIDTH        640
#define Z88_SCREEN_HEIGHT       64

#define Z88_SCR_HW_REV  (1<<4)
#define Z88_SCR_HW_HRS  (1<<5)
#define Z88_SCR_HW_UND  (1<<1)
#define Z88_SCR_HW_FLS  (1<<3)
#define Z88_SCR_HW_GRY  (1<<2)
#define Z88_SCR_HW_CURS (Z88_SCR_HW_HRS|Z88_SCR_HW_FLS|Z88_SCR_HW_REV)
#define Z88_SCR_HW_NULL (Z88_SCR_HW_HRS|Z88_SCR_HW_GRY|Z88_SCR_HW_REV)

class z88_state : public driver_device
{
public:
	z88_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_nvram(*this, "nvram")
		, m_ram(*this, RAM_TAG)
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_blink(*this, "blink")
		, m_lines(*this, "LINE%u", 0U)
		, m_banks(*this, "bank%u", 1U)
		, m_carts(*this, "slot%u", 0U)
		, m_boot_view(*this, "boot_view")
	{ }

	void z88(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	uint8_t kb_r(offs_t offset);
	UPD65031_MEMORY_UPDATE(bankswitch_update);
	UPD65031_SCREEN_UPDATE(lcd_update);

	// defined in video/z88.cpp
	inline void plot_pixel(bitmap_ind16 &bitmap, int x, int y, uint16_t color);
	void vh_render_8x8(address_space &space, bitmap_ind16 &bitmap, int x, int y, uint16_t pen0, uint16_t pen1, uint32_t offset);
	void vh_render_6x8(address_space &space, bitmap_ind16 &bitmap, int x, int y, uint16_t pen0, uint16_t pen1, uint32_t offset);
	void vh_render_line(bitmap_ind16 &bitmap, int x, int y, uint16_t pen);

	void z88_palette(palette_device &palette) const;

	void z88_io(address_map &map) ATTR_COLD;
	void z88_mem(address_map &map) ATTR_COLD;
	void z88_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<nvram_device> m_nvram;
	required_device<ram_device> m_ram;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<upd65031_device> m_blink;
	required_ioport_array<8> m_lines;
	required_device_array<address_map_bank_device, 4> m_banks;
	optional_device_array<z88cart_slot_device, 4> m_carts;
	memory_view m_boot_view;
};

#endif // MAME_ACORN_Z88_H
