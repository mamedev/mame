// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Couriersud
/***************************************************************************

    Galaxian hardware family

***************************************************************************/
#ifndef MAME_GALAXIAN_GALAXIAN_H
#define MAME_GALAXIAN_GALAXIAN_H

#pragma once

#include "cpu/mcs48/mcs48.h"
#include "machine/gen_latch.h"
#include "machine/i8255.h"
#include "machine/netlist.h"
#include "machine/timer.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/digitalk.h"
#include "sound/sn76496.h"
#include "sound/sp0250.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


namespace {

/* master clocks */
static constexpr XTAL GALAXIAN_MASTER_CLOCK(18.432_MHz_XTAL);
static constexpr XTAL KONAMI_SOUND_CLOCK(14.318181_MHz_XTAL);
static constexpr XTAL SIDAM_MASTER_CLOCK(12_MHz_XTAL);

/* we scale horizontally by 3 to render stars correctly */
static constexpr int GALAXIAN_XSCALE = 3;
/* the Sidam bootlegs have a 12 MHz XTAL instead */
static constexpr int SIDAM_XSCALE    = 2;

static constexpr XTAL GALAXIAN_PIXEL_CLOCK(GALAXIAN_XSCALE*GALAXIAN_MASTER_CLOCK / 3);
static constexpr XTAL SIDAM_PIXEL_CLOCK(SIDAM_XSCALE*SIDAM_MASTER_CLOCK / 2);

/* H counts from 128->511, HBLANK starts at 130 and ends at 250 */
/* we normalize this here so that we count 0->383 with HBLANK */
/* from 264-383 */
static constexpr int GALAXIAN_HTOTAL  = (384 * GALAXIAN_XSCALE);
static constexpr int GALAXIAN_HBEND   = (0 * GALAXIAN_XSCALE);
//static constexpr int GALAXIAN_H0START = (6*GALAXIAN_XSCALE)
//static constexpr int GALAXIAN_HBSTART = (264*GALAXIAN_XSCALE)
static constexpr int GALAXIAN_H0START = (0 * GALAXIAN_XSCALE);
static constexpr int GALAXIAN_HBSTART = (256 * GALAXIAN_XSCALE);

static constexpr int GALAXIAN_VTOTAL  = (264);
static constexpr int GALAXIAN_VBEND   = (16);
static constexpr int GALAXIAN_VBSTART = (224 + 16);

static constexpr int SIDAM_HTOTAL     = (384 * SIDAM_XSCALE);
static constexpr int SIDAM_HBEND      = (0 * SIDAM_XSCALE);
static constexpr int SIDAM_H0START    = (0 * SIDAM_XSCALE);
static constexpr int SIDAM_HBSTART    = (256 * SIDAM_XSCALE);

} // anonymous namespace

class galaxian_state : public driver_device
{
public:
	galaxian_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_ay8910(*this, "8910.%u", 0)
		, m_ay8910_cclimber(*this, "cclimber_audio:aysnd")
		, m_ppi8255(*this, "ppi8255_%u", 0)
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch")
		, m_netlist(*this, "konami")
		, m_filter_ctl(*this, "konami:ctl%u", 0)
		, m_ckong_coinage(*this, "COINAGE")
		, m_spriteram(*this, "spriteram")
		, m_videoram(*this, "videoram")
		, m_decrypted_opcodes(*this, "decrypted_opcodes")
		, m_lamps(*this, "lamp%u", 0U)
	{ }

	/* video extension callbacks */
	typedef void (galaxian_state::*extend_tile_info_func)(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x, uint8_t y);
	typedef void (galaxian_state::*extend_sprite_info_func)(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);
	typedef void (galaxian_state::*draw_bullet_func)(bitmap_rgb32 &bitmap, const rectangle &cliprect, int offs, int x, int y);
	typedef void (galaxian_state::*draw_background_func)(bitmap_rgb32 &bitmap, const rectangle &cliprect);

	typedef delegate<void (uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x, uint8_t y)> extend_tile_info_delegate;
	typedef delegate<void (const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color)> extend_sprite_info_delegate;
	typedef delegate<void (bitmap_rgb32 &bitmap, const rectangle &cliprect, int offs, int x, int y)> draw_bullet_delegate;
	typedef delegate<void (bitmap_rgb32 &bitmap, const rectangle &cliprect)> draw_background_delegate;

	void galaxian_videoram_w(offs_t offset, uint8_t data);
	void galaxian_objram_w(offs_t offset, uint8_t data);
	void galaxian_flip_screen_x_w(uint8_t data);
	void galaxian_flip_screen_y_w(uint8_t data);
	void galaxian_flip_screen_xy_w(uint8_t data);
	void galaxian_stars_enable_w(uint8_t data);
	void scramble_background_enable_w(uint8_t data);
	void scramble_background_red_w(uint8_t data);
	void scramble_background_green_w(uint8_t data);
	void scramble_background_blue_w(uint8_t data);
	void galaxian_gfxbank_w(offs_t offset, uint8_t data);
	template <int N> int azurian_port_r();
	void irq_enable_w(uint8_t data);
	void start_lamp_w(offs_t offset, uint8_t data);
	void coin_lock_w(uint8_t data);
	void coin_count_0_w(uint8_t data);
	void coin_count_1_w(uint8_t data);
	uint8_t konami_ay8910_r(offs_t offset);
	void konami_ay8910_w(offs_t offset, uint8_t data);
	void konami_sound_filter_w(offs_t offset, uint8_t data);
	uint8_t theend_ppi8255_r(offs_t offset);
	void theend_ppi8255_w(offs_t offset, uint8_t data);
	void theend_protection_w(uint8_t data);
	uint8_t theend_protection_r();
	template <int N> int theend_protection_alt_r();
	uint8_t scrammr_protection_r();
	void explorer_sound_control_w(uint8_t data);
	uint8_t frogger_ppi8255_r(offs_t offset);
	void frogger_ppi8255_w(offs_t offset, uint8_t data);
	uint8_t frogger_ay8910_r(offs_t offset);
	void frogger_ay8910_w(offs_t offset, uint8_t data);
	IRQ_CALLBACK_MEMBER(froggermc_audiocpu_irq_ack);
	void froggermc_sound_control_w(uint8_t data);
	uint8_t frogf_ppi8255_r(offs_t offset);
	void frogf_ppi8255_w(offs_t offset, uint8_t data);
	uint8_t turtles_ppi8255_0_r(offs_t offset);
	uint8_t turtles_ppi8255_1_r(offs_t offset);
	void turtles_ppi8255_0_w(offs_t offset, uint8_t data);
	void turtles_ppi8255_1_w(offs_t offset, uint8_t data);
	uint8_t jumpbug_protection_r(offs_t offset);
	void checkman_sound_command_w(uint8_t data);
	uint8_t checkmaj_protection_r();
	uint8_t dingo_3000_r();
	uint8_t dingo_3035_r();
	uint8_t dingoe_3001_r();
	uint8_t froggeram_ppi8255_r(offs_t offset);
	void froggeram_ppi8255_w(offs_t offset, uint8_t data);
	void artic_gfxbank_w(uint8_t data);
	void konami_sound_control_w(uint8_t data);
	uint8_t konami_sound_timer_r();
	void konami_portc_0_w(uint8_t data);
	void konami_portc_1_w(uint8_t data);
	void theend_coin_counter_w(uint8_t data);
	uint8_t explorer_sound_latch_r();
	uint8_t frogger_sound_timer_r();
	void init_galaxian() ATTR_COLD;
	void init_nolock() ATTR_COLD;
	void init_azurian() ATTR_COLD;
	void init_batman2() ATTR_COLD;
	void init_ladybugg2() ATTR_COLD;
	void init_highroll() ATTR_COLD;
	void init_frogg() ATTR_COLD;
	void init_mooncrst() ATTR_COLD;
	void init_mooncrsu() ATTR_COLD;
	void init_mooncrgx() ATTR_COLD;
	void init_moonqsr() ATTR_COLD;
	void init_pacmanbl() ATTR_COLD;
	void init_devilfshg() ATTR_COLD;
	void init_jumpbug() ATTR_COLD;
	void init_jumpbugbc() ATTR_COLD;
	void init_checkman() ATTR_COLD;
	void init_checkmaj() ATTR_COLD;
	void init_dingo() ATTR_COLD;
	void init_dingoe() ATTR_COLD;
	void init_kong() ATTR_COLD;
	void init_fantastc() ATTR_COLD;
	void init_timefgtr() ATTR_COLD;
	void init_theend() ATTR_COLD;
	void init_scramble() ATTR_COLD;
	void init_explorer() ATTR_COLD;
	void init_mandinga() ATTR_COLD;
	void init_mandingaeg() ATTR_COLD;
	void init_atlantis() ATTR_COLD;
	void init_scobra() ATTR_COLD;
	void init_scobrae() ATTR_COLD;
	void init_losttomb() ATTR_COLD;
	void init_frogger() ATTR_COLD;
	void init_froggermc() ATTR_COLD;
	void init_froggers() ATTR_COLD;
	void init_quaak() ATTR_COLD;
	void init_turtles() ATTR_COLD;
	void init_ckongs() ATTR_COLD;
	void init_ckonggx() ATTR_COLD;
	void init_anteater() ATTR_COLD;
	void init_anteateruk() ATTR_COLD;
	void init_superbon() ATTR_COLD;
	void init_calipso() ATTR_COLD;
	void init_ghostmun() ATTR_COLD;
	void init_froggrs() ATTR_COLD;
	void init_warofbugg() ATTR_COLD;
	void init_jungsub() ATTR_COLD;
	void init_mimonkey() ATTR_COLD;
	void init_mimonkeyb() ATTR_COLD;
	void init_victoryc() ATTR_COLD;
	void init_bigkonggx() ATTR_COLD;
	void init_crazym() ATTR_COLD;

	TILE_GET_INFO_MEMBER(bg_get_tile_info);
	void galaxian_palette(palette_device &palette);
	void eagle_palette(palette_device &palette);
	uint32_t screen_update_galaxian(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vblank_interrupt_w(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(checkmaj_irq0_gen);
	TIMER_DEVICE_CALLBACK_MEMBER(scramble_stars_blink_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(timefgtr_scanline);
	void state_save_register();
	void sprites_draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, const uint8_t *spritebase);
	void bullets_draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, const uint8_t *base);
	void stars_init();
	void stars_update_origin();
	void stars_draw_row(bitmap_rgb32 &bitmap, int maxx, int y, uint32_t star_offs, uint8_t starmask);
	void null_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void galaxian_draw_stars(bitmap_rgb32 &bitmap, const rectangle &cliprect, int maxx);
	void galaxian_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void background_draw_colorsplit(bitmap_rgb32 &bitmap, const rectangle &cliprect, rgb_t color, int split, int split_flipped);
	void scramble_draw_stars(bitmap_rgb32 &bitmap, const rectangle &cliprect, int maxx);
	void scramble_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void anteater_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void jumpbug_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void turtles_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void frogger_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	inline void galaxian_draw_pixel(bitmap_rgb32 &bitmap, const rectangle &cliprect, int y, int x, rgb_t color);
	void galaxian_draw_bullet(bitmap_rgb32 &bitmap, const rectangle &cliprect, int offs, int x, int y);
	void mshuttle_draw_bullet(bitmap_rgb32 &bitmap, const rectangle &cliprect, int offs, int x, int y);
	void scramble_draw_bullet(bitmap_rgb32 &bitmap, const rectangle &cliprect, int offs, int x, int y);
	void theend_draw_bullet(bitmap_rgb32 &bitmap, const rectangle &cliprect, int offs, int x, int y);
	void empty_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x, uint8_t y);
	void empty_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);
	void upper_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x, uint8_t y);
	void upper_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);
	void frogger_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x, uint8_t y);
	void frogger_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);
	void batman2_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x, uint8_t y);
	void mooncrst_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x, uint8_t y);
	void mooncrst_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);
	void moonqsr_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x, uint8_t y);
	void moonqsr_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);
	void mshuttle_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x, uint8_t y);
	void mshuttle_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);
	void calipso_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);
	void jumpbug_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x, uint8_t y);
	void jumpbug_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);
	void mimonkey_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x, uint8_t y);
	void mimonkey_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);
	void guttangt_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);
	void decode_mooncrst(int length, uint8_t *dest);
	void decode_checkman();
	void decode_dingoe();
	void decode_frogger_sound();
	void decode_froggermc_sound();
	void decode_frogger_gfx();
	void decode_anteater_gfx();
	void decode_losttomb_gfx();
	void decode_superbon();
	void decode_victoryc();
	void common_init(
			draw_bullet_func draw_bullet,
			draw_background_func draw_background,
			extend_tile_info_func extend_tile_info,
			extend_sprite_info_func extend_sprite_info);
	void galaxian_base(machine_config &config) ATTR_COLD;
	void sidam_bootleg_base(machine_config &config) ATTR_COLD;
	void konami_base(machine_config &config) ATTR_COLD;
	void konami_sound_1x_ay8910(machine_config &config) ATTR_COLD;
	void konami_sound_2x_ay8910(machine_config &config) ATTR_COLD;
	void scramble_base(machine_config &config) ATTR_COLD;
	void timefgtr(machine_config &config) ATTR_COLD;
	void moonqsr(machine_config &config) ATTR_COLD;
	void frogger(machine_config &config) ATTR_COLD;
	void anteatergg(machine_config &config) ATTR_COLD;
	void ozon1(machine_config &config) ATTR_COLD;
	void theend(machine_config &config) ATTR_COLD;
	void turtles(machine_config &config) ATTR_COLD;
	void fantastc(machine_config &config) ATTR_COLD;
	void jumpbug(machine_config &config) ATTR_COLD;
	void jumpbugbrf(machine_config &config) ATTR_COLD;
	void checkmaj(machine_config &config) ATTR_COLD;
	void pacmanbl(machine_config &config) ATTR_COLD;
	void quaak(machine_config &config) ATTR_COLD;
	void galaxian(machine_config &config) ATTR_COLD;
	void highroll(machine_config &config) ATTR_COLD;
	void devilfshg(machine_config &config) ATTR_COLD;
	void froggers(machine_config &config) ATTR_COLD;
	void froggervd(machine_config &config) ATTR_COLD;
	void anteateruk(machine_config &config) ATTR_COLD;
	void anteaterg(machine_config &config) ATTR_COLD;
	void anteater(machine_config &config) ATTR_COLD;
	void turpins(machine_config &config) ATTR_COLD;
	void explorer(machine_config &config) ATTR_COLD;
	void scramble(machine_config &config) ATTR_COLD;
	void scobra(machine_config &config) ATTR_COLD;
	void froggermc(machine_config &config) ATTR_COLD;
	void froggeram(machine_config &config) ATTR_COLD;
	void spactrai(machine_config &config) ATTR_COLD;
	void takeoff(machine_config &config) ATTR_COLD;
	void mooncrst(machine_config &config) ATTR_COLD;
	void eagle(machine_config &config) ATTR_COLD;
	void ckongs(machine_config &config) ATTR_COLD;
	void frogf(machine_config &config) ATTR_COLD;
	void amigo2(machine_config &config) ATTR_COLD;
	void checkman(machine_config &config) ATTR_COLD;
	void jungsub(machine_config &config) ATTR_COLD;
	void victoryc(machine_config &config) ATTR_COLD;
	void frogg(machine_config &config) ATTR_COLD;
	void mandingarf(machine_config &config) ATTR_COLD;
	void mandinka(machine_config &config) ATTR_COLD;
	void thepitm(machine_config &config) ATTR_COLD;
	void kong(machine_config &config) ATTR_COLD;
	void bongo(machine_config &config) ATTR_COLD;
	void bongog(machine_config &config) ATTR_COLD;
	void scorpnmc(machine_config &config) ATTR_COLD;
	void ckongg(machine_config &config) ATTR_COLD;
	void ckongmc(machine_config &config) ATTR_COLD;
	void astroamb(machine_config &config) ATTR_COLD;
	void mimonkey(machine_config &config) ATTR_COLD;
	void mimonscr(machine_config &config) ATTR_COLD;
	void galartic(machine_config &config) ATTR_COLD;
	void bigkonggx(machine_config &config) ATTR_COLD;
	void scrammr(machine_config &config) ATTR_COLD;
	void turpinnv(machine_config &config) ATTR_COLD;

	template <int Mask> ioport_value ckongg_coinage_r();
	template <int Mask> int ckongs_coinage_r();

protected:
	// machine configuration helpers
	void set_irq_line(int line) { m_irq_line = line; }
	void set_bullets_base(int base) { m_bullets_base = base; }
	void set_num_spritegens(int num) { m_numspritegens = num; }
	void set_x_scale(uint8_t scale) { m_x_scale = scale; }
	void set_h0_start(uint8_t start) { m_h0_start = start; }

	void amigo2_map(address_map &map) ATTR_COLD;
	void anteaterg_map(address_map &map) ATTR_COLD;
	void anteatergg_map(address_map &map) ATTR_COLD;
	void anteateruk_map(address_map &map) ATTR_COLD;
	void astroamb_map(address_map &map) ATTR_COLD;
	void bigkonggx_map(address_map &map) ATTR_COLD;
	void bongo_map(address_map &map) ATTR_COLD;
	void bongog_map(address_map &map) ATTR_COLD;
	void bongo_io_map(address_map &map) ATTR_COLD;
	void checkmaj_sound_map(address_map &map) ATTR_COLD;
	void checkman_sound_map(address_map &map) ATTR_COLD;
	void checkman_sound_portmap(address_map &map) ATTR_COLD;
	void ckongg_map(address_map &map) ATTR_COLD;
	void ckongg_map_base(address_map &map) ATTR_COLD;
	void ckongmc_map(address_map &map) ATTR_COLD;
	void ckongs_map(address_map &map) ATTR_COLD;
	void explorer_map(address_map &map) ATTR_COLD;
	void fantastc_map(address_map &map) ATTR_COLD;
	void frogf_map(address_map &map) ATTR_COLD;
	void frogg_map(address_map &map) ATTR_COLD;
	void frogger_map(address_map &map) ATTR_COLD;
	void froggervd_map(address_map &map) ATTR_COLD;
	void frogger_sound_map(address_map &map) ATTR_COLD;
	void frogger_sound_portmap(address_map &map) ATTR_COLD;
	void froggeram_map(address_map &map) ATTR_COLD;
	void froggermc_map(address_map &map) ATTR_COLD;
	void galartic_map(address_map &map) ATTR_COLD;
	void galaxian_map(address_map &map) ATTR_COLD;
	void galaxian_map_base(address_map &map) ATTR_COLD;
	void galaxian_map_discrete(address_map &map) ATTR_COLD;
	void highroll_map(address_map &map) ATTR_COLD;
	void jumpbug_map(address_map &map) ATTR_COLD;
	void jumpbugbrf_map(address_map &map) ATTR_COLD;
	void jungsub_map(address_map &map) ATTR_COLD;
	void jungsub_io_map(address_map &map) ATTR_COLD;
	void konami_sound_map(address_map &map) ATTR_COLD;
	void konami_sound_portmap(address_map &map) ATTR_COLD;
	void kong_map(address_map &map) ATTR_COLD;
	void mandingarf_map(address_map &map) ATTR_COLD;
	void mandinka_map(address_map &map) ATTR_COLD;
	void mimonkey_map(address_map &map) ATTR_COLD;
	void mimonscr_map(address_map &map) ATTR_COLD;
	void mooncrst_map(address_map &map) ATTR_COLD;
	void mooncrst_map_base(address_map &map) ATTR_COLD;
	void mooncrst_map_discrete(address_map &map) ATTR_COLD;
	void moonqsr_decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void ozon1_map(address_map &map) ATTR_COLD;
	void ozon1_io_map(address_map &map) ATTR_COLD;
	void scobra_map(address_map &map) ATTR_COLD;
	void scorpnmc_map(address_map &map) ATTR_COLD;
	void spactrai_map(address_map &map) ATTR_COLD;
	void takeoff_sound_map(address_map &map) ATTR_COLD;
	void takeoff_sound_portmap(address_map &map) ATTR_COLD;
	void theend_map(address_map &map) ATTR_COLD;
	void thepitm_map(address_map &map) ATTR_COLD;
	void turpinnv_map(address_map &map) ATTR_COLD;
	void turpins_map(address_map &map) ATTR_COLD;
	void turpins_sound_map(address_map &map) ATTR_COLD;
	void turtles_map(address_map &map) ATTR_COLD;
	void victoryc_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override
	{
		m_lamps.resolve();
		m_irq_enabled = 0;
	}
	virtual void video_start() override ATTR_COLD;
	virtual void sprites_clip(screen_device &screen, rectangle &cliprect);

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device_array<ay8910_device, 3> m_ay8910;
	optional_device<ay8910_device> m_ay8910_cclimber;
	optional_device_array<i8255_device, 3> m_ppi8255;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch;
	optional_device<netlist_mame_sound_device> m_netlist;
	optional_device_array<netlist_mame_logic_input_device, 12> m_filter_ctl;
	optional_ioport m_ckong_coinage;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;
	output_finder<2> m_lamps;

	int m_bullets_base = 0x60;
	int m_sprites_base = 0x40;
	int m_numspritegens = 1;
	uint16_t m_protection_state = 0;
	uint8_t m_protection_result = 0;
	uint8_t m_konami_sound_control = 0;
	uint8_t m_irq_enabled = 0;
	int m_irq_line = INPUT_LINE_NMI;
	bool m_frogger_adjust = false;
	uint8_t m_x_scale = GALAXIAN_XSCALE;
	uint8_t m_h0_start = GALAXIAN_H0START;
	bool m_sfx_adjust = false;

	extend_tile_info_delegate m_extend_tile_info_ptr;
	extend_sprite_info_delegate m_extend_sprite_info_ptr;
	draw_bullet_delegate m_draw_bullet_ptr;
	draw_background_delegate m_draw_background_ptr;

	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_flipscreen_x = 0;
	uint8_t m_flipscreen_y = 0;
	uint8_t m_background_enable = 0;
	uint8_t m_background_red = 0;
	uint8_t m_background_green = 0;
	uint8_t m_background_blue = 0;
	uint32_t m_star_rng_origin = 0;
	uint32_t m_star_rng_origin_frame = 0;
	rgb_t m_star_color[64];
	std::unique_ptr<uint8_t[]> m_stars;
	uint8_t m_stars_enabled = 0;
	uint8_t m_stars_blink_state = 0;
	rgb_t m_bullet_color[8];
	uint8_t m_gfxbank[5]{};
};


class bagmanmc_state : public galaxian_state
{
public:
	bagmanmc_state(const machine_config &mconfig, device_type type, const char *tag)
		: galaxian_state(mconfig, type, tag)
	{
	}

	void bagmanmc(machine_config &config) ATTR_COLD;
	void init_bagmanmc() ATTR_COLD;

protected:
	void bagmanmc_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x, uint8_t y);
	void bagmanmc_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);

private:
	void bagmanmc_map(address_map &map) ATTR_COLD;
	void bagmanmc_io_map(address_map &map) ATTR_COLD;
};


class gmgalax_state : public bagmanmc_state
{
public:
	gmgalax_state(const machine_config &mconfig, device_type type, const char *tag)
		: bagmanmc_state(mconfig, type, tag)
		, m_glin(*this, "GLIN%u", 0U)
		, m_gmin(*this, "GMIN%u", 0U)
		, m_gamesel(*this, "GAMESEL")
		, m_rombank(*this, "rombank")
	{ }

	void gmgalax(machine_config &config) ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(game_changed);
	template <int N> ioport_value port_r();

	void init_gmgalax() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void gmgalax_map(address_map &map) ATTR_COLD;

	required_ioport_array<3> m_glin;
	required_ioport_array<3> m_gmin;
	required_ioport m_gamesel;
	required_memory_bank m_rombank;

	uint8_t m_selected_game;
};


class pisces_state : public galaxian_state
{
public:
	pisces_state(const machine_config &mconfig, device_type type, const char *tag)
		: galaxian_state(mconfig, type, tag)
	{
	}

	void ladybugg2(machine_config &config) ATTR_COLD;
	void pisces(machine_config &config) ATTR_COLD;
	void porter(machine_config &config) ATTR_COLD;
	void skybase(machine_config &config) ATTR_COLD;

	void init_pisces() ATTR_COLD;

protected:
	void pisces_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x, uint8_t y);
	void pisces_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);

private:
	void pisces_map(address_map &map) ATTR_COLD;
	void skybase_map(address_map &map) ATTR_COLD;
	void porter_map(address_map &map) ATTR_COLD;
	void ladybugg2_opcodes_map (address_map &map) ATTR_COLD;
};


class fourplay_state : public pisces_state
{
public:
	fourplay_state(const machine_config &mconfig, device_type type, const char *tag)
		: pisces_state(mconfig, type, tag)
		, m_rombank(*this, "rombank")
	{
	}

	void fourplay(machine_config &config) ATTR_COLD;
	void init_fourplay() ATTR_COLD;

private:
	void fourplay_rombank_w(offs_t offset, uint8_t data);
	void fourplay_map(address_map &map) ATTR_COLD;

	required_memory_bank m_rombank;
};


class mshuttle_state : public galaxian_state
{
public:
	mshuttle_state(const machine_config &mconfig, device_type type, const char *tag)
		: galaxian_state(mconfig, type, tag)
	{
	}

	void mshuttle(machine_config &config) ATTR_COLD;

	void init_mshuttle() ATTR_COLD;
	void init_mshuttlj() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void ay8910_cs_w(uint8_t data);
	void ay8910_control_w(uint8_t data);
	void ay8910_data_w(uint8_t data);
	uint8_t ay8910_data_r();

	void mshuttle_map(address_map &map) ATTR_COLD;
	void mshuttle_decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void mshuttle_portmap(address_map &map) ATTR_COLD;

	void mshuttle_decode(const uint8_t convtable[8][16]);

	uint8_t m_ay8910_cs = 0U;
};


class kingball_state : public galaxian_state
{
public:
	kingball_state(const machine_config &mconfig, device_type type, const char *tag)
		: galaxian_state(mconfig, type, tag)
		, m_dac(*this, "dac")
		, m_mux_port(*this, "FAKE")
	{
	}

	int muxbit_r();
	int noise_r();

	void kingball(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void speech_dip_w(uint8_t data);
	void sound1_w(uint8_t data);
	void sound2_w(uint8_t data);
	void dac_w(uint8_t data);

	void kingball_map(address_map &map) ATTR_COLD;
	void kingball_sound_map(address_map &map) ATTR_COLD;
	void kingball_sound_portmap(address_map &map) ATTR_COLD;

	required_device<dac_byte_interface> m_dac;
	required_ioport m_mux_port;

	uint8_t m_speech_dip = 0;
	uint8_t m_sound = 0;
};


class namenayo_state : public galaxian_state
{
public:
	namenayo_state(const machine_config &mconfig, device_type type, const char *tag)
		: galaxian_state(mconfig, type, tag)
		, m_exattrram(*this, "extattrram")
	{
	}

	void namenayo(machine_config &config) ATTR_COLD;
	void init_namenayo() ATTR_COLD;

protected:
	virtual void sprites_clip(screen_device &screen, rectangle &cliprect) override { }

private:
	void namenayo_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void namenayo_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x, uint8_t y);
	void namenayo_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);
	void namenayo_map(address_map &map) ATTR_COLD;
	void namenayo_extattr_w(offs_t offset, uint8_t data);
	void namenayo_unk_d800_w(uint8_t data);

	required_shared_ptr<uint8_t> m_exattrram;
};


class tenspot_state : public galaxian_state
{
public:
	tenspot_state(const machine_config &mconfig, device_type type, const char *tag)
		: galaxian_state(mconfig, type, tag)
		, m_game_dsw(*this, "IN2_GAME%u", 0U)
		, m_mainbank(*this, "mainbank")
	{
	}

	DECLARE_INPUT_CHANGED_MEMBER(tenspot_fake);

	void tenspot(machine_config &config) ATTR_COLD;
	void init_tenspot() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void unk_6000_w(uint8_t data);
	void unk_8000_w(uint8_t data);
	void unk_e000_w(uint8_t data);
	uint8_t dsw_read();

	void set_game_bank(int bank, bool invalidate_gfx);

	void tenspot_map(address_map &map) ATTR_COLD;
	void tenspot_select_map(address_map &map) ATTR_COLD;

	required_ioport_array<10> m_game_dsw;
	required_memory_bank m_mainbank;

	uint8_t m_current_game = 0;
};


class zigzagb_state : public galaxian_state
{
public:
	zigzagb_state(const machine_config &mconfig, device_type type, const char *tag)
		: galaxian_state(mconfig, type, tag)
		, m_rombanks(*this, "bank%u", 1U)
	{
	}

	void zigzag(machine_config &config) ATTR_COLD;
	void init_zigzag() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void bankswap_w(uint8_t data);
	void ay8910_w(offs_t offset, uint8_t data);
	void zigzag_map(address_map &map) ATTR_COLD;

	required_memory_bank_array<2> m_rombanks;
	uint8_t m_ay8910_latch = 0U;
};


class videight_state : public galaxian_state
{
public:
	videight_state(const machine_config &mconfig, device_type type, const char *tag)
		: galaxian_state(mconfig, type, tag)
		, m_rombank(*this, "rombank")
	{
	}

	void videight(machine_config &config) ATTR_COLD;
	void init_videight() ATTR_COLD;

private:
	void videight_rombank_w(offs_t offset, uint8_t data);
	void videight_gfxbank_w(offs_t offset, uint8_t data);
	void videight_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x, uint8_t y);
	void videight_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);
	void videight_map(address_map &map) ATTR_COLD;

	required_memory_bank m_rombank;
};


class guttangt_state : public galaxian_state
{
public:
	guttangt_state(const machine_config &mconfig, device_type type, const char *tag)
		: galaxian_state(mconfig, type, tag)
		, m_rombank(*this, "rombank")
	{
	}

	void guttangt(machine_config &config) ATTR_COLD;
	void guttangts3(machine_config &config) ATTR_COLD;
	void init_guttangt() ATTR_COLD;
	void init_guttangts3() ATTR_COLD;

private:
	void guttangt_map(address_map &map) ATTR_COLD;
	void guttangts3_map(address_map &map) ATTR_COLD;
	void guttangt_rombank_w(uint8_t data);

	optional_memory_bank m_rombank;
};


class zac_scorpion_state : public galaxian_state
{
public:
	zac_scorpion_state(const machine_config &mconfig, device_type type, const char *tag)
		: galaxian_state(mconfig, type, tag)
		, m_digitalker(*this, "digitalker")
	{
	}

	void scorpion(machine_config &config) ATTR_COLD;
	void init_scorpion() ATTR_COLD;

private:
	uint8_t ay8910_r(offs_t offset);
	void ay8910_w(offs_t offset, uint8_t data);
	uint8_t digitalker_intr_r();
	void digitalker_control_w(uint8_t data);
	uint8_t protection_r();
	void protection_w(uint8_t data);

	void scorpion_map(address_map &map) ATTR_COLD;
	void scorpion_sound_map(address_map &map) ATTR_COLD;
	void scorpion_sound_portmap(address_map &map) ATTR_COLD;

	required_device<digitalker_device> m_digitalker;
};


class nihon_sfx_state : public galaxian_state
{
public:
	nihon_sfx_state(const machine_config &mconfig, device_type type, const char *tag)
		: galaxian_state(mconfig, type, tag)
		, m_audio2(*this, "audio2")
		, m_dac(*this, "dac")
	{
	}

	void sfx(machine_config &config) ATTR_COLD;
	void init_sfx() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

	uint8_t sample_io_r(offs_t offset);
	void sample_io_w(offs_t offset, uint8_t data);
	void sample_control_w(uint8_t data);

	void sfx_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void sfx_draw_bullet(bitmap_rgb32 &bitmap, const rectangle &cliprect, int offs, int x, int y);

	void sfx_map(address_map &map) ATTR_COLD;
	void sfx_sample_map(address_map &map) ATTR_COLD;
	void sfx_sample_portmap(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_audio2;
	required_device<dac_byte_interface> m_dac;

	uint8_t m_sample_control = 0U;
};


class monsterz_state : public nihon_sfx_state
{
public:
	monsterz_state(const machine_config& mconfig, device_type type, const char* tag)
		: nihon_sfx_state(mconfig, type, tag)
		, m_dac2(*this, "dac2")
	{
	}

	void monsterz(machine_config& config);
	void init_monsterz() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void monsterz_map(address_map &map) ATTR_COLD;
	void monsterz_sound_map(address_map &map) ATTR_COLD;
	void monsterz_sound_portmap(address_map &map) ATTR_COLD;
	void monsterz_sample_map(address_map &map) ATTR_COLD;
	void monsterz_ay8910_w(offs_t offset, uint8_t data);

	required_device<dac_byte_interface> m_dac2;

	uint32_t m_monsterz_shift = 0U;
	uint32_t m_monsterz_shift2 = 0U;
	uint8_t m_monsterz_audio_portb = 0U;
	uint8_t m_monsterz_sample_portc = 0U;
};


class moonwar_state : public galaxian_state
{
public:
	moonwar_state(const machine_config &mconfig, device_type type, const char *tag)
		: galaxian_state(mconfig, type, tag)
		, m_dials(*this, "P%u_DIAL", 1U)
	{
	}

	ioport_value dial_r();

	void moonwar(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void port_select_w(uint8_t data);
	void moonwar_palette(palette_device &palette);

	required_ioport_array<2> m_dials;

	uint8_t m_port_select = 0U;
	uint8_t m_direction[2]{};
	uint8_t m_counter_74ls161[2]{};
	uint8_t m_last_dialread[2]{};
};


class sbhoei_state : public galaxian_state
{
public:
	sbhoei_state(const machine_config &mconfig, device_type type, const char *tag)
		: galaxian_state(mconfig, type, tag)
		, m_8039(*this, "i8039")
		, m_sp0250(*this, "sp0250")
		, m_soundbank(*this, "soundbank")
	{
	}

	void sbhoei(machine_config &config) ATTR_COLD;
	void init_sbhoei() ATTR_COLD;
	void sbhoei_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x, uint8_t y);
	void sbhoei_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void sbhoei_map(address_map &map) ATTR_COLD;
	void sbhoei_map_discrete(address_map &map) ATTR_COLD;
	void sbhoei_sound_map(address_map &map) ATTR_COLD;
	void sbhoei_sound_io_map(address_map &map) ATTR_COLD;

	void sbhoei_palette(palette_device &palette);
	void sbhoei_soundlatch_w(uint8_t data);
	void p2_w(uint8_t data);
	uint8_t p1_r();

	required_device<i8039_device> m_8039;
	required_device<sp0250_device> m_sp0250;
	required_memory_bank m_soundbank;

	uint8_t m_p2 = 0;
};


class bmxstunts_state : public galaxian_state
{
public:
	bmxstunts_state(const machine_config &mconfig, device_type type, const char *tag)
		: galaxian_state(mconfig, type, tag)
		, m_snsnd(*this, "snsnd")
	{
	}

	void bmxstunts(machine_config &config) ATTR_COLD;
	void init_bmxstunts() ATTR_COLD;
	void bmxstunts_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);

private:
	required_device<sn76489a_device> m_snsnd;

	void bmxstunts_map(address_map &map) ATTR_COLD;
	void snsnd_w(uint8_t data) { m_snsnd->write(bitswap<8>(data,0,1,2,3,4,5,6,7)); }
};


#endif // MAME_GALAXIAN_GALAXIAN_H
