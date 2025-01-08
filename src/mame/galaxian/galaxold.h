// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  Galaxian hardware family (old)

  This include file is used by the following drivers:
    - dambustr.cpp
    - galaxold.cpp
    - scramble.cpp
    - scobra.cpp

***************************************************************************/
#ifndef MAME_GALAXIAN_GALAXOLD_H
#define MAME_GALAXIAN_GALAXOLD_H

#pragma once

#include "machine/7474.h"
#include "machine/timer.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

/* star circuit */
#define STAR_COUNT  252
struct star_gold
{
	int x = 0, y = 0, color = 0;
};

class galaxold_state : public driver_device
{
public:
	galaxold_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_7474_9m_1(*this, "7474_9m_1")
		, m_7474_9m_2(*this, "7474_9m_2")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_videoram(*this,"videoram")
		, m_spriteram(*this,"spriteram")
		, m_spriteram2(*this,"spriteram2")
		, m_attributesram(*this,"attributesram")
		, m_bulletsram(*this,"bulletsram")
		, m_racknrol_tiles_bank(*this,"racknrol_tbank")
		, m_leds(*this, "led%u", 0U)
		, m_leftclip(2)
	{ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<ttl7474_device> m_7474_9m_1;
	optional_device<ttl7474_device> m_7474_9m_2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint8_t> m_spriteram2;
	required_shared_ptr<uint8_t> m_attributesram;
	optional_shared_ptr<uint8_t> m_bulletsram;
	optional_shared_ptr<uint8_t> m_racknrol_tiles_bank;
	output_finder<2> m_leds;

	int m_irq_line = 0;
	uint8_t m__4in1_bank = 0U;
	tilemap_t *m_bg_tilemap = nullptr;
	int m_spriteram2_present = 0;
	uint8_t m_gfxbank[5]{};
	uint8_t m_flipscreen_x = 0U;
	uint8_t m_flipscreen_y = 0U;
	uint8_t m_color_mask = 0U;
	tilemap_t *m_dambustr_tilemap2 = nullptr;
	std::unique_ptr<uint8_t[]> m_dambustr_videoram2{};
	int m_leftclip;

	void (galaxold_state::*m_modify_charcode)(uint16_t *code, uint8_t x);     /* function to call to do character banking */
	void (galaxold_state::*m_modify_spritecode)(uint8_t *spriteram, int*, int*, int*, int); /* function to call to do sprite banking */
	void (galaxold_state::*m_modify_color)(uint8_t *color);   /* function to call to do modify how the color codes map to the PROM */
	void (galaxold_state::*m_modify_ypos)(uint8_t*);  /* function to call to do modify how vertical positioning bits are connected */

	uint8_t m_timer_adjusted = 0U;
	uint8_t m_darkplnt_bullet_color = 0U;
	void (galaxold_state::*m_draw_bullets)(bitmap_ind16 &,const rectangle &, int, int, int);  /* function to call to draw a bullet */

	uint8_t m_background_enable = 0U;
	uint8_t m_background_red = 0U;
	uint8_t m_background_green = 0U;
	uint8_t m_background_blue = 0U;
	void (galaxold_state::*m_draw_background)(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);   /* function to call to draw the background */
	int m_dambustr_bg_split_line = 0;
	int m_dambustr_bg_color_1 = 0;
	int m_dambustr_bg_color_2 = 0;
	int m_dambustr_bg_priority = 0;
	int m_dambustr_char_bank = 0;
	std::unique_ptr<bitmap_ind16> m_dambustr_tmpbitmap{};

	void (galaxold_state::*m_draw_stars)(bitmap_ind16 &, const rectangle &);      /* function to call to draw the star layer */
	int m_stars_colors_start = 0;
	int32_t m_stars_scrollpos = 0U;
	uint8_t m_stars_on = 0U;
	uint8_t m_stars_blink_state = 0U;
	emu_timer *m_stars_blink_timer = nullptr;
	emu_timer *m_stars_scroll_timer = nullptr;
	struct star_gold m_stars[STAR_COUNT]{};

	uint8_t drivfrcg_port0_r();
	uint8_t scrambler_protection_2_r();
	uint8_t scramb2_protection_r();
	uint8_t scramb2_port0_r(offs_t offset);
	uint8_t scramb2_port1_r(offs_t offset);
	uint8_t scramb2_port2_r(offs_t offset);
	uint8_t hexpoola_data_port_r();
	uint8_t bullsdrtg_data_port_r();
	void galaxold_nmi_enable_w(uint8_t data);
	void galaxold_coin_lockout_w(uint8_t data);
	void galaxold_coin_counter_w(offs_t offset, uint8_t data);
	void galaxold_coin_counter_1_w(uint8_t data);
	void galaxold_coin_counter_2_w(uint8_t data);
	void galaxold_leds_w(offs_t offset, uint8_t data);
	uint8_t scramblb_protection_1_r();
	uint8_t scramblb_protection_2_r();
	void _4in1_bank_w(uint8_t data);
	void racknrol_tiles_bank_w(offs_t offset, uint8_t data);
	void galaxold_videoram_w(offs_t offset, uint8_t data);
	uint8_t galaxold_videoram_r(offs_t offset);
	void galaxold_attributesram_w(offs_t offset, uint8_t data);
	void galaxold_flip_screen_x_w(uint8_t data);
	void galaxold_flip_screen_y_w(uint8_t data);
	void hotshock_flip_screen_w(uint8_t data);
	void scrambold_background_enable_w(uint8_t data);
	void scrambold_background_red_w(uint8_t data);
	void scrambold_background_green_w(uint8_t data);
	void scrambold_background_blue_w(uint8_t data);
	void galaxold_stars_enable_w(uint8_t data);
	void darkplnt_bullet_color_w(uint8_t data);
	void galaxold_gfxbank_w(offs_t offset, uint8_t data);
	void dambustr_bg_split_line_w(uint8_t data);
	void dambustr_bg_color_w(uint8_t data);
	void galaxold_7474_9m_2_q_callback(int state);
	void galaxold_7474_9m_1_callback(int state);
	uint8_t rescueb_a002_r() { return 0xfc; }
	template <int Mask> int _4in1_fake_port_r();
	template <int Mask> int vpool_lives_r();
	template <int Mask> ioport_value dkongjrm_coinage_r();

	void init_bullsdrtg();
	void init_ladybugg();
	void init_4in1();
	void init_superbikg();

	TILE_GET_INFO_MEMBER(drivfrcg_get_tile_info);
	TILE_GET_INFO_MEMBER(racknrol_get_tile_info);
	TILE_GET_INFO_MEMBER(dambustr_get_tile_info2);
	TILE_GET_INFO_MEMBER(get_tile_info);
	TILE_GET_INFO_MEMBER(harem_get_tile_info);

	DECLARE_MACHINE_RESET(galaxold);
	DECLARE_MACHINE_RESET(hunchbkg);

	void galaxold_palette(palette_device &palette);
	void s2650_palette(palette_device &palette) const;
	void scrambold_palette(palette_device &palette);
	void stratgyx_palette(palette_device &palette);
	void darkplnt_palette(palette_device &palette) const;
	void minefld_palette(palette_device &palette);
	void rescue_palette(palette_device &palette);
	void mariner_palette(palette_device &palette);
	void dambustr_palette(palette_device &palette);
	void turtles_palette(palette_device &palette);

	DECLARE_VIDEO_START(galaxold);
	DECLARE_VIDEO_START(drivfrcg);
	DECLARE_VIDEO_START(racknrol);
	DECLARE_VIDEO_START(mooncrst);
	DECLARE_VIDEO_START(scrambold);
	DECLARE_VIDEO_START(newsin7);
	DECLARE_VIDEO_START(pisces);
	DECLARE_VIDEO_START(dkongjrm);
	DECLARE_VIDEO_START(dkongjrmc);
	DECLARE_VIDEO_START(galaxold_plain);
	DECLARE_VIDEO_START(ckongs);
	DECLARE_VIDEO_START(darkplnt);
	DECLARE_VIDEO_START(rescue);
	DECLARE_VIDEO_START(minefld);
	DECLARE_VIDEO_START(stratgyx);
	DECLARE_VIDEO_START(mariner);
	DECLARE_VIDEO_START(mimonkey);
	DECLARE_VIDEO_START(scorpion);
	DECLARE_VIDEO_START(ad2083);
	DECLARE_VIDEO_START(dambustr);
	DECLARE_VIDEO_START(harem);

	uint32_t screen_update_galaxold(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_dambustr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_CALLBACK_MEMBER(stars_blink_callback);
	TIMER_CALLBACK_MEMBER(stars_scroll_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(galaxold_interrupt_timer);
	uint8_t hunchbkg_intack();

	void state_save_register();
	void video_start_common();
	void pisces_modify_spritecode(uint8_t *spriteram, int *code, int *flipx, int *flipy, int offs);
	void mooncrst_modify_spritecode(uint8_t *spriteram, int *code, int *flipx, int *flipy, int offs);
	void harem_modify_spritecode(uint8_t *spriteram, int *code, int *flipx, int *flipy, int offs);
	void mooncrst_modify_charcode(uint16_t *code, uint8_t x);
	void pisces_modify_charcode(uint16_t *code, uint8_t x);
	void mimonkey_modify_charcode(uint16_t *code, uint8_t x);
	void mariner_modify_charcode(uint16_t *code, uint8_t x);
	void dambustr_modify_charcode(uint16_t *code, uint8_t x);
	void mshuttle_modify_spritecode(uint8_t *spriteram, int *code, int *flipx, int *flipy, int offs);
	void mimonkey_modify_spritecode(uint8_t *spriteram, int *code, int *flipx, int *flipy, int offs);
	void batman2_modify_spritecode(uint8_t *spriteram, int *code, int *flipx, int *flipy, int offs);
	void dkongjrm_modify_spritecode(uint8_t *spriteram, int *code, int *flipx, int *flipy, int offs);
	void dkongjrmc_modify_spritecode(uint8_t *spriteram, int *code, int *flipx, int *flipy, int offs);
	void ad2083_modify_spritecode(uint8_t *spriteram, int *code, int *flipx, int *flipy, int offs);
	void dambustr_modify_spritecode(uint8_t *spriteram, int *code, int *flipx, int *flipy, int offs);
	void drivfrcg_modify_color(uint8_t *color);
	void galaxold_draw_bullets(bitmap_ind16 &bitmap, const rectangle &cliprect, int offs, int x, int y);
	void scrambold_draw_bullets(bitmap_ind16 &bitmap, const rectangle &cliprect, int offs, int x, int y);
	void darkplnt_draw_bullets(bitmap_ind16 &bitmap, const rectangle &cliprect, int offs, int x, int y);
	void dambustr_draw_bullets(bitmap_ind16 &bitmap, const rectangle &cliprect, int offs, int x, int y);
	void rescue_draw_bullets(bitmap_ind16 &bitmap, const rectangle &cliprect, int offs, int x, int y);
	void galaxold_draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void scrambold_draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void ad2083_draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void stratgyx_draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void minefld_draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void rescue_draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void mariner_draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void dambustr_draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void dambustr_draw_upper_background(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void galaxold_init_stars(int colors_offset);
	void plot_star(bitmap_ind16 &bitmap, int x, int y, int color, const rectangle &cliprect);
	void noop_draw_stars(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void galaxold_draw_stars(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void scrambold_draw_stars(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void rescue_draw_stars(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void mariner_draw_stars(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void start_stars_blink_timer(double ra, double rb, double c);
	void start_stars_scroll_timer();
	void draw_bullets_common(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, uint8_t *spriteram, size_t spriteram_size);
	void machine_reset_common(int line);
	void galaxian(machine_config &config);
	void galaxold_base(machine_config &config);
	void _4in1(machine_config &config);
	void racknrol(machine_config &config);
	void hunchbkg(machine_config &config);
	void videotron(machine_config &config);
	void hexpoola(machine_config &config);
	void dkongjrm(machine_config &config);
	void tazzmang(machine_config &config);
	void scrambleo(machine_config &config);
	void scrambler(machine_config &config);
	void spcwarp(machine_config &config);
	void superbikg(machine_config &config);
	void dkongjrmc(machine_config &config);
	void bullsdrtg(machine_config &config);
	void drivfrcg(machine_config &config);
	void scramblb(machine_config &config);
	void scramb2(machine_config &config);
	void scramb3(machine_config &config);
	void mooncrst(machine_config &config);
	void galaxian_audio(machine_config &config);
	void mooncrst_audio(machine_config &config);
	void _4in1_map(address_map &map) ATTR_COLD;
	void bullsdrtg_data_map(address_map &map) ATTR_COLD;
	void dkongjrm_map(address_map &map) ATTR_COLD;
	void dkongjrmc_map(address_map &map) ATTR_COLD;
	void drivfrcg_program(address_map &map) ATTR_COLD;
	void drivfrcg_io(address_map &map) ATTR_COLD;
	void galaxold_map(address_map &map) ATTR_COLD;
	void hexpoola_data(address_map &map) ATTR_COLD;
	void hexpoola_io(address_map &map) ATTR_COLD;
	void hunchbkg_map(address_map &map) ATTR_COLD;
	void hunchbkg_data(address_map &map) ATTR_COLD;
	void hustlerb3_map(address_map &map) ATTR_COLD;
	void mooncrst_map(address_map &map) ATTR_COLD;
	void racknrol_map(address_map &map) ATTR_COLD;
	void racknrol_io(address_map &map) ATTR_COLD;
	void scramb_common_map(address_map &map) ATTR_COLD;
	void scramb2_map(address_map &map) ATTR_COLD;
	void scramb3_map(address_map &map) ATTR_COLD;
	void scramblb_map(address_map &map) ATTR_COLD;
	void scrambleo_map(address_map &map) ATTR_COLD;
	void scrambler_map(address_map &map) ATTR_COLD;
	void spcwarp_map(address_map &map) ATTR_COLD;
	void superbikg_data(address_map &map) ATTR_COLD;
	void superbikg_io(address_map &map) ATTR_COLD;
	void superbikg_map(address_map &map) ATTR_COLD;
	void tazzmang_map(address_map &map) ATTR_COLD;

protected:
	virtual void machine_start() override { m_leds.resolve(); }

private:
	uint8_t m_superbikg_latch = 0;
};

#define galaxold_coin_counter_0_w galaxold_coin_counter_w

#endif // MAME_GALAXIAN_GALAXOLD_H
