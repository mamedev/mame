// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  Galaxian hardware family (old)

  This include file is used by the following drivers:
    - dambustr.c
    - galaxold.c
    - scramble.c
    - scobra.c

***************************************************************************/

#ifndef __GALAXOLD_H__
#define __GALAXOLD_H__

#include "machine/7474.h"

/* star circuit */
#define STAR_COUNT  252
struct star_gold
{
	int x, y, color;
};

class galaxold_state : public driver_device
{
public:
	galaxold_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_audiocpu(*this, "audiocpu"),
			m_7474_9m_1(*this, "7474_9m_1"),
			m_7474_9m_2(*this, "7474_9m_2"),
			m_gfxdecode(*this, "gfxdecode"),
			m_screen(*this, "screen"),
			m_palette(*this, "palette"),
			m_videoram(*this,"videoram"),
			m_spriteram(*this,"spriteram"),
			m_spriteram2(*this,"spriteram2"),
			m_attributesram(*this,"attributesram"),
			m_bulletsram(*this,"bulletsram"),
			m_rockclim_videoram(*this,"rockclim_vram"),
			m_racknrol_tiles_bank(*this,"racknrol_tbank"),
			m_leftclip(2)
	{
	}

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
	optional_shared_ptr<uint8_t> m_rockclim_videoram;
	optional_shared_ptr<uint8_t> m_racknrol_tiles_bank;

	int m_irq_line;
	uint8_t m__4in1_bank;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_rockclim_tilemap;
	int m_spriteram2_present;
	uint8_t m_gfxbank[5];
	uint8_t m_flipscreen_x;
	uint8_t m_flipscreen_y;
	uint8_t m_color_mask;
	tilemap_t *m_dambustr_tilemap2;
	std::unique_ptr<uint8_t[]> m_dambustr_videoram2;
	int m_leftclip;

	void (galaxold_state::*m_modify_charcode)(uint16_t *code, uint8_t x);     /* function to call to do character banking */
	void (galaxold_state::*m_modify_spritecode)(uint8_t *spriteram, int*, int*, int*, int); /* function to call to do sprite banking */
	void (galaxold_state::*m_modify_color)(uint8_t *color);   /* function to call to do modify how the color codes map to the PROM */
	void (galaxold_state::*m_modify_ypos)(uint8_t*);  /* function to call to do modify how vertical positioning bits are connected */

	uint8_t m_timer_adjusted;
	uint8_t m_darkplnt_bullet_color;
	void (galaxold_state::*m_draw_bullets)(bitmap_ind16 &,const rectangle &, int, int, int);  /* function to call to draw a bullet */

	uint8_t m_background_enable;
	uint8_t m_background_red;
	uint8_t m_background_green;
	uint8_t m_background_blue;
	void (galaxold_state::*m_draw_background)(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);   /* function to call to draw the background */
	uint16_t m_rockclim_v;
	uint16_t m_rockclim_h;
	int m_dambustr_bg_split_line;
	int m_dambustr_bg_color_1;
	int m_dambustr_bg_color_2;
	int m_dambustr_bg_priority;
	int m_dambustr_char_bank;
	std::unique_ptr<bitmap_ind16> m_dambustr_tmpbitmap;

	void (galaxold_state::*m_draw_stars)(bitmap_ind16 &, const rectangle &);      /* function to call to draw the star layer */
	int m_stars_colors_start;
	int32_t m_stars_scrollpos;
	uint8_t m_stars_on;
	uint8_t m_stars_blink_state;
	emu_timer *m_stars_blink_timer;
	emu_timer *m_stars_scroll_timer;
	struct star_gold m_stars[STAR_COUNT];

	uint8_t drivfrcg_port0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t scrambler_protection_2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t scramb2_protection_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t scramb2_port0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t scramb2_port1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t scramb2_port2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t hexpoola_data_port_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t bullsdrtg_data_port_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void galaxold_nmi_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void galaxold_coin_lockout_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void galaxold_coin_counter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void galaxold_coin_counter_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void galaxold_coin_counter_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void galaxold_leds_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t scramblb_protection_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t scramblb_protection_2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void _4in1_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void racknrol_tiles_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void galaxold_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t galaxold_videoram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void galaxold_attributesram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void galaxold_flip_screen_x_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void galaxold_flip_screen_y_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hotshock_flip_screen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scrambold_background_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scrambold_background_red_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scrambold_background_green_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scrambold_background_blue_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void galaxold_stars_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void darkplnt_bullet_color_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void galaxold_gfxbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rockclim_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rockclim_scroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void guttang_rombank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t rockclim_videoram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dambustr_bg_split_line_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dambustr_bg_color_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void galaxold_7474_9m_2_q_callback(int state);
	void galaxold_7474_9m_1_callback(int state);
	uint8_t rescueb_a002_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) { return 0xfc; }
	ioport_value _4in1_fake_port_r(ioport_field &field, void *param);
	ioport_value vpool_lives_r(ioport_field &field, void *param);
	ioport_value ckongg_coinage_r(ioport_field &field, void *param);
	ioport_value dkongjrm_coinage_r(ioport_field &field, void *param);

	void init_bullsdrtg();
	void init_ladybugg();
	void init_4in1();
	void init_guttangt();
	void init_ckonggx();

	void drivfrcg_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void racknrol_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void dambustr_get_tile_info2(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void rockclim_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void harem_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	void machine_reset_galaxold();
	void machine_reset_devilfsg();
	void machine_reset_hunchbkg();

	void palette_init_galaxold(palette_device &palette);
	void palette_init_rockclim(palette_device &palette);
	void palette_init_scrambold(palette_device &palette);
	void palette_init_stratgyx(palette_device &palette);
	void palette_init_darkplnt(palette_device &palette);
	void palette_init_minefld(palette_device &palette);
	void palette_init_rescue(palette_device &palette);
	void palette_init_mariner(palette_device &palette);
	void palette_init_dambustr(palette_device &palette);
	void palette_init_turtles(palette_device &palette);

	void video_start_galaxold();
	void video_start_drivfrcg();
	void video_start_racknrol();
	void video_start_batman2();
	void video_start_mooncrst();
	void video_start_scrambold();
	void video_start_newsin7();
	void video_start_pisces();
	void video_start_dkongjrm();
	void video_start_rockclim();
	void video_start_galaxold_plain();
	void video_start_ozon1();
	void video_start_bongo();
	void video_start_ckongs();
	void video_start_darkplnt();
	void video_start_rescue();
	void video_start_minefld();
	void video_start_stratgyx();
	void video_start_mariner();
	void video_start_mimonkey();
	void video_start_scorpion();
	void video_start_ad2083();
	void video_start_dambustr();
	void video_start_harem();
	void video_start_bagmanmc();

	uint32_t screen_update_galaxold(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_dambustr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void hunchbks_vh_interrupt(device_t &device);
	void stars_blink_callback(void *ptr, int32_t param);
	void stars_scroll_callback(void *ptr, int32_t param);
	void galaxold_interrupt_timer(timer_device &timer, void *ptr, int32_t param);
	int hunchbkg_irq_callback(device_t &device, int irqline);

	void state_save_register();
	void video_start_common();
	void pisces_modify_spritecode(uint8_t *spriteram, int *code, int *flipx, int *flipy, int offs);
	void mooncrst_modify_spritecode(uint8_t *spriteram, int *code, int *flipx, int *flipy, int offs);
	void batman2_modify_charcode(uint16_t *code, uint8_t x);
	void rockclim_draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void rockclim_modify_spritecode(uint8_t *spriteram, int *code, int *flipx, int *flipy, int offs);
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
	void ad2083_modify_spritecode(uint8_t *spriteram, int *code, int *flipx, int *flipy, int offs);
	void dambustr_modify_spritecode(uint8_t *spriteram, int *code, int *flipx, int *flipy, int offs);
	void drivfrcg_modify_color(uint8_t *color);
	void galaxold_draw_bullets(bitmap_ind16 &bitmap, const rectangle &cliprect, int offs, int x, int y);
	void scrambold_draw_bullets(bitmap_ind16 &bitmap, const rectangle &cliprect, int offs, int x, int y);
	void darkplnt_draw_bullets(bitmap_ind16 &bitmap, const rectangle &cliprect, int offs, int x, int y);
	void dambustr_draw_bullets(bitmap_ind16 &bitmap, const rectangle &cliprect, int offs, int x, int y);
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
	void bagmanmc_modify_charcode(uint16_t *code, uint8_t x);
	void bagmanmc_modify_spritecode(uint8_t *spriteram, int *code, int *flipx, int *flipy, int offs);
	void machine_reset_common(int line);
};

#define galaxold_coin_counter_0_w galaxold_coin_counter_w

#endif
