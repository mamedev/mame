// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Lee Taylor
/*************************************************************************

    Cosmic Guerilla & other Universal boards (in cosmic.c)

*************************************************************************/
#ifndef MAME_UNIVERSAL_COSMIC_H
#define MAME_UNIVERSAL_COSMIC_H

#pragma once

#include "machine/timer.h"
#include "sound/samples.h"
#include "sound/dac.h"
#include "emupal.h"
#include "screen.h"

#define Z80_MASTER_CLOCK         XTAL(10'816'000)


class cosmic_state : public driver_device
{
public:
	cosmic_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_sound_enabled(0),
		m_in_ports(*this, "IN%u", 0),
		m_dsw(*this, "DSW"),
		m_maincpu(*this, "maincpu"),
		m_samples(*this, "samples"),
		m_dac(*this, "dac"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	void cosmic(machine_config &config);
	void cosmica(machine_config &config);
	void nomnlnd(machine_config &config);
	void devzone(machine_config &config);
	void panic(machine_config &config);
	void magspot(machine_config &config);

	void init_devzone();
	void init_nomnlnd();
	void init_cosmica();
	void init_panic();

	void panic_coin_inserted(int state);
	DECLARE_INPUT_CHANGED_MEMBER(cosmica_coin_inserted);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted_irq0);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted_nmi);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	typedef pen_t (cosmic_state::*color_func)(uint8_t x, uint8_t y);
	color_func     m_map_color{};
	int            m_color_registers[3]{};
	int            m_background_enable = 0;
	int            m_magspot_pen_mask = 0;

	/* sound-related */
	int            m_sound_enabled;
	int            m_dive_bomb_b_select = 0;

	/* misc */
	optional_ioport_array<4> m_in_ports;
	optional_ioport m_dsw;

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<samples_device> m_samples;
	optional_device<dac_bit_interface> m_dac;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	void panic_sound_output_w(offs_t offset, uint8_t data);
	void panic_sound_output2_w(offs_t offset, uint8_t data);
	void cosmica_sound_output_w(offs_t offset, uint8_t data);
	void dac_w(uint8_t data);
	uint8_t cosmica_pixel_clock_r();
	uint8_t magspot_coinage_dip_r(offs_t offset);
	uint8_t nomnlnd_port_0_1_r(offs_t offset);
	void flip_screen_w(uint8_t data);
	void cosmic_color_register_w(offs_t offset, uint8_t data);
	void cosmic_background_enable_w(uint8_t data);
	void panic_palette(palette_device &palette);
	void cosmica_palette(palette_device &palette);
	void magspot_palette(palette_device &palette);
	void nomnlnd_palette(palette_device &palette);
	uint32_t screen_update_panic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_cosmica(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_magspot(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_devzone(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_nomnlnd(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(panic_scanline);
	void draw_bitmap(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int color_mask, int extra_sprites);
	void cosmica_draw_starfield(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void devzone_draw_grid(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void nomnlnd_draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	pen_t panic_map_color(uint8_t x, uint8_t y);
	pen_t cosmica_map_color(uint8_t x, uint8_t y);
	pen_t magspot_map_color(uint8_t x, uint8_t y);
	void cosmica_map(address_map &map) ATTR_COLD;
	void magspot_map(address_map &map) ATTR_COLD;
	void panic_map(address_map &map) ATTR_COLD;
};

#endif // MAME_UNIVERSAL_COSMIC_H
