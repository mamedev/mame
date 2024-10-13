// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Ms.Pac-Man/Galaga - 20 Year Reunion hardware

    driver by Nicola Salmoria

***************************************************************************/
#ifndef MAME_NAMCO_20PACGAL_H
#define MAME_NAMCO_20PACGAL_H

#pragma once

#include "cpu/z180/z180.h"
#include "machine/eepromser.h"
#include "machine/intelfsh.h"
#include "sound/dac.h"
#include "sound/namco.h"
#include "bus/rs232/rs232.h"
#include "emupal.h"

class _20pacgal_state : public driver_device
{
public:
	_20pacgal_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_video_ram(*this, "video_ram"),
		m_char_gfx_ram(*this, "char_gfx_ram"),
		m_stars_seed(*this, "stars_seed"),
		m_stars_ctrl(*this, "stars_ctrl"),
		m_flip(*this, "flip"),
		m_proms(*this, "proms"),
		m_mainbank(*this, "mainbank"),
		m_maincpu(*this, "maincpu"),
		m_eeprom(*this, "eeprom"),
		m_palette(*this, "palette"),
		m_dac(*this, "dac"),
		m_rs232(*this, "rs232")
	{ }

	void _20pacgal(machine_config &config);
	void _20pacgal_video(machine_config &config);

	void init_25pacman();
	void init_20pacgal();

protected:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_video_ram;
	required_shared_ptr<uint8_t> m_char_gfx_ram;
	required_shared_ptr<uint8_t> m_stars_seed;
	required_shared_ptr<uint8_t> m_stars_ctrl;
	required_shared_ptr<uint8_t> m_flip;

	optional_memory_region m_proms;
	optional_memory_bank m_mainbank;

	/* machine state */
	uint8_t m_game_selected = 0;  /* 0 = Ms. Pac-Man, 1 = Galaga */

	/* devices */
	required_device<z180_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<palette_device> m_palette;
	required_device<dac_8bit_r2r_device> m_dac;
	required_device<rs232_port_device> m_rs232;

	/* memory */
	std::unique_ptr<uint8_t[]> m_sprite_gfx_ram;
	std::unique_ptr<uint8_t[]> m_sprite_ram;
	std::unique_ptr<uint8_t[]> m_sprite_color_lookup;
	std::unique_ptr<uint8_t[]> m_ram_48000;

	/* 25pacman and 20pacgal store the sprite palette at a different address, this is a hardware difference and confirmed NOT to be a register */
	uint8_t m_sprite_pal_base = 0;

	uint8_t m_irq_mask = 0;
	void irqack_w(uint8_t data);
	void timer_pulse_w(uint8_t data);
	void _20pacgal_coin_counter_w(uint8_t data);
	void ram_bank_select_w(uint8_t data);
	void ram_48000_w(offs_t offset, uint8_t data);
	void sprite_gfx_w(offs_t offset, uint8_t data);
	void sprite_ram_w(offs_t offset, uint8_t data);
	void sprite_lookup_w(offs_t offset, uint8_t data);

	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update_20pacgal(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vblank_irq(int state);
	void starpal_init(palette_device &palette) const;
	void get_pens();
	void do_pen_lookup(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_chars(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_stars(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_sprite(bitmap_rgb32 &bitmap, const rectangle &cliprect, int y, int x,
						uint8_t code, uint8_t color, int flip_y, int flip_x);
	void common_save_state();

	void _20pacgal_io_map(address_map &map) ATTR_COLD;
	void _20pacgal_map(address_map &map) ATTR_COLD;
};


class _25pacman_state : public _20pacgal_state
{
public:
	_25pacman_state(const machine_config &mconfig, device_type type, const char *tag) :
		_20pacgal_state(mconfig, type, tag)
	{ }

	void _25pacman(machine_config &config);

private:
	uint8_t _25pacman_io_87_r();

	virtual void machine_start() override ATTR_COLD;

	void _25pacman_io_map(address_map &map) ATTR_COLD;
	void _25pacman_map(address_map &map) ATTR_COLD;
};

#endif // MAME_NAMCO_20PACGAL_H
