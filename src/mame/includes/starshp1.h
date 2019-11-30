// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo, Stefan Jokisch
/*
 *  The schematics don't seem to make a lot of sense when it
 *  comes to the video timing chain::
 *
 *    * there are clear typos -- what is H132???
 *    * there are two HBLANK/HSYNC periods as the horizontal
 *      chain is drawn, which would give an alternating long
 *      line followed by a much shorter one.  This cannot be right
 *    * the carry-out/load circuit on LS161@J8 is superfluous
 *
 *  These values also give a frame rate of about 45Hz, which is
 *  probably too low.  I suspect that screen is not really
 *  512 pixels wide -- most likely 384, which would give 60Hz
 *
 *  Based on photographs of the PCB, and analysis of videos of
 *  actual gameplay, the horizontal screen really is 384 clocks.
 *
 *  However, some of the graphics, like the starfield, are
 *  clocked with the 12MHz signal.  This effectively doubles
 *  the horizontal resolution:
 *
 *                             6.048Mhz clocks     12.096Mhz clocks
 *  Horizontal Visible Area    384 (0x180)         768 (0x300)
 *  Horizontal Blanking Time   128 (0x080)         256 (0x100)
 */
#ifndef MAME_INCLUDES_STARSHP1_H
#define MAME_INCLUDES_STARSHP1_H

#pragma once

#include "sound/discrete.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


#define STARSHP1_MASTER_CLOCK       (12096000)
#define STARSHP1_CPU_CLOCK          (STARSHP1_MASTER_CLOCK / 16)
#define STARSHP1_PIXEL_CLOCK        (STARSHP1_MASTER_CLOCK)
#define STARSHP1_HTOTAL             (0x300)
#define STARSHP1_HBEND              (0x000)
#define STARSHP1_HBSTART            (0x200)
#define STARSHP1_VTOTAL             (0x106)
#define STARSHP1_VBEND              (0x000)
#define STARSHP1_VBSTART            (0x0f0)


class starshp1_state : public driver_device
{
public:
	starshp1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_playfield_ram(*this, "playfield_ram"),
		m_hpos_ram(*this, "hpos_ram"),
		m_vpos_ram(*this, "vpos_ram"),
		m_obj_ram(*this, "obj_ram"),
		m_maincpu(*this, "maincpu"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_led(*this, "led0")
	{ }

	void starshp1(machine_config &config);

	DECLARE_CUSTOM_INPUT_MEMBER(starshp1_analog_r);
	DECLARE_CUSTOM_INPUT_MEMBER(collision_latch_r);

private:
	virtual void machine_start() override;
	DECLARE_WRITE8_MEMBER(starshp1_collision_reset_w);
	DECLARE_WRITE8_MEMBER(starshp1_analog_in_w);
	DECLARE_WRITE_LINE_MEMBER(ship_explode_w);
	DECLARE_WRITE_LINE_MEMBER(circle_mod_w);
	DECLARE_WRITE_LINE_MEMBER(circle_kill_w);
	DECLARE_WRITE_LINE_MEMBER(starfield_kill_w);
	DECLARE_WRITE_LINE_MEMBER(inverse_w);
	DECLARE_WRITE_LINE_MEMBER(mux_w);
	DECLARE_WRITE_LINE_MEMBER(led_w);
	DECLARE_READ8_MEMBER(starshp1_rng_r);
	DECLARE_WRITE8_MEMBER(starshp1_ssadd_w);
	DECLARE_WRITE8_MEMBER(starshp1_sspic_w);
	DECLARE_WRITE8_MEMBER(starshp1_playfield_w);
	DECLARE_WRITE_LINE_MEMBER(attract_w);
	DECLARE_WRITE_LINE_MEMBER(phasor_w);
	DECLARE_WRITE8_MEMBER(starshp1_analog_out_w);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void video_start() override;
	void starshp1_palette(palette_device &palette) const;
	uint32_t screen_update_starshp1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_starshp1);
	INTERRUPT_GEN_MEMBER(starshp1_interrupt);
	void set_pens();
	void draw_starfield(bitmap_ind16 &bitmap);
	int get_sprite_hpos(int i);
	int get_sprite_vpos(int i);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_spaceship(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_phasor(bitmap_ind16 &bitmap);
	int get_radius();
	int get_circle_hpos();
	int get_circle_vpos();
	void draw_circle_line(bitmap_ind16 &bitmap, int x, int y, int l);
	void draw_circle(bitmap_ind16 &bitmap);
	int spaceship_collision(bitmap_ind16 &bitmap, const rectangle &rect);
	int point_in_circle(int x, int y, int center_x, int center_y, int r);
	int circle_collision(const rectangle &rect);
	void starshp1_map(address_map &map);

private:
	int m_analog_in_select;
	int m_attract;
	required_shared_ptr<uint8_t> m_playfield_ram;
	required_shared_ptr<uint8_t> m_hpos_ram;
	required_shared_ptr<uint8_t> m_vpos_ram;
	required_shared_ptr<uint8_t> m_obj_ram;
	int m_ship_explode;
	int m_ship_picture;
	int m_ship_hoffset;
	int m_ship_voffset;
	int m_ship_size;
	int m_circle_hpos;
	int m_circle_vpos;
	int m_circle_size;
	int m_circle_mod;
	int m_circle_kill;
	int m_phasor;
	int m_collision_latch;
	int m_starfield_kill;
	int m_mux;
	int m_inverse;
	std::unique_ptr<uint16_t[]> m_LSFR;
	bitmap_ind16 m_helper;
	tilemap_t *m_bg_tilemap;

	required_device<cpu_device> m_maincpu;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	output_finder<> m_led;
};

/*----------- defined in audio/starshp1.c -----------*/

DISCRETE_SOUND_EXTERN( starshp1_discrete );

/* Discrete Sound Input Nodes */
#define STARSHP1_NOISE_AMPLITUDE    NODE_01
#define STARSHP1_TONE_PITCH         NODE_02
#define STARSHP1_MOTOR_SPEED        NODE_03
#define STARSHP1_NOISE_FREQ         NODE_04
#define STARSHP1_MOLVL              NODE_05
#define STARSHP1_SL2                NODE_06
#define STARSHP1_SL1                NODE_07
#define STARSHP1_KICKER             NODE_08
#define STARSHP1_PHASOR_ON          NODE_09
#define STARSHP1_ATTRACT            NODE_10

#endif // MAME_INCLUDES_STARSHP1_H
