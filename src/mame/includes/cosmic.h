// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Lee Taylor
/*************************************************************************

    Cosmic Guerilla & other Universal boards (in cosmic.c)

*************************************************************************/
#ifndef MAME_INCLUDES_COSMIC_H
#define MAME_INCLUDES_COSMIC_H

#pragma once

#include "machine/timer.h"
#include "sound/samples.h"
#include "sound/dac.h"
#include "emupal.h"
#include "screen.h"

#define COSMICG_MASTER_CLOCK     XTAL(9'828'000)
#define Z80_MASTER_CLOCK         XTAL(10'816'000)


class cosmic_state : public driver_device
{
public:
	cosmic_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_in_ports(*this, "IN%u", 0),
		m_dsw(*this, "DSW"),
		m_maincpu(*this, "maincpu"),
		m_samples(*this, "samples"),
		m_dac(*this, "dac"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	typedef pen_t (cosmic_state::*color_func)(uint8_t x, uint8_t y);
	color_func     m_map_color;
	int            m_color_registers[3];
	int            m_background_enable;
	int            m_magspot_pen_mask;

	/* sound-related */
	int            m_sound_enabled;
	int            m_march_select;
	int            m_gun_die_select;
	int            m_dive_bomb_b_select;

	/* misc */
	uint32_t         m_pixel_clock;
	int            m_ic_state;   // for 9980
	optional_ioport_array<4> m_in_ports;
	optional_ioport m_dsw;

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<samples_device> m_samples;
	optional_device<dac_bit_interface> m_dac;
	optional_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	DECLARE_WRITE8_MEMBER(panic_sound_output_w);
	DECLARE_WRITE8_MEMBER(panic_sound_output2_w);
	DECLARE_WRITE8_MEMBER(cosmicg_output_w);
	DECLARE_WRITE8_MEMBER(cosmica_sound_output_w);
	DECLARE_WRITE8_MEMBER(dac_w);
	DECLARE_READ8_MEMBER(cosmica_pixel_clock_r);
	DECLARE_READ8_MEMBER(cosmicg_port_0_r);
	DECLARE_READ8_MEMBER(magspot_coinage_dip_r);
	DECLARE_READ8_MEMBER(nomnlnd_port_0_1_r);
	DECLARE_WRITE8_MEMBER(flip_screen_w);
	DECLARE_WRITE8_MEMBER(cosmic_color_register_w);
	DECLARE_WRITE8_MEMBER(cosmic_background_enable_w);
	DECLARE_WRITE_LINE_MEMBER(panic_coin_inserted);
	DECLARE_INPUT_CHANGED_MEMBER(cosmica_coin_inserted);
	DECLARE_INPUT_CHANGED_MEMBER(cosmicg_coin_inserted);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted_irq0);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted_nmi);
	void init_devzone();
	void init_cosmicg();
	void init_nomnlnd();
	void init_cosmica();
	void init_panic();
	DECLARE_MACHINE_START(cosmic);
	DECLARE_MACHINE_RESET(cosmic);
	DECLARE_MACHINE_RESET(cosmicg);
	void panic_palette(palette_device &palette);
	void cosmica_palette(palette_device &palette);
	void cosmicg_palette(palette_device &palette);
	void magspot_palette(palette_device &palette);
	void nomnlnd_palette(palette_device &palette);
	uint32_t screen_update_cosmicg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
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
	pen_t cosmicg_map_color(uint8_t x, uint8_t y);
	pen_t magspot_map_color(uint8_t x, uint8_t y);
	void cosmic(machine_config &config);
	void cosmica(machine_config &config);
	void cosmicg(machine_config &config);
	void nomnlnd(machine_config &config);
	void devzone(machine_config &config);
	void panic(machine_config &config);
	void magspot(machine_config &config);
	void cosmica_map(address_map &map);
	void cosmicg_io_map(address_map &map);
	void cosmicg_map(address_map &map);
	void magspot_map(address_map &map);
	void panic_map(address_map &map);
};

#endif // MAME_INCLUDES_COSMIC_H
