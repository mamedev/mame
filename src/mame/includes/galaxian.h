// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Couriersud
/***************************************************************************

    Galaxian hardware family

***************************************************************************/
#ifndef MAME_INCLUDES_GALAXIAN_H
#define MAME_INCLUDES_GALAXIAN_H

#pragma once

#include "machine/gen_latch.h"
#include "machine/i8255.h"
#include "machine/timer.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/digitalk.h"
#include "sound/discrete.h"
#include "emupal.h"
#include "screen.h"

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
		, m_audio2(*this, "audio2")
		, m_dac(*this, "dac")
		, m_ay8910(*this, "8910.%u", 0)
		, m_ay8910_cclimber(*this, "cclimber_audio:aysnd")
		, m_digitalker(*this, "digitalker")
		, m_ppi8255(*this, "ppi8255_%u", 0)
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch")
		, m_discrete(*this, "konami")
		, m_fake_select(*this, "FAKE_SELECT")
		, m_tenspot_game_dsw(*this, {"IN2_GAME0", "IN2_GAME1", "IN2_GAME2", "IN2_GAME3", "IN2_GAME4", "IN2_GAME5", "IN2_GAME6", "IN2_GAME7", "IN2_GAME8", "IN2_GAME9"})
		, m_spriteram(*this, "spriteram")
		, m_videoram(*this, "videoram")
		, m_decrypted_opcodes(*this, "decrypted_opcodes")
		, m_lamps(*this, "lamp%u", 0U)
	{ }

	/* video extension callbacks */
	typedef void (galaxian_state::*galaxian_extend_tile_info_func)(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x);
	typedef void (galaxian_state::*galaxian_extend_sprite_info_func)(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);
	typedef void (galaxian_state::*galaxian_draw_bullet_func)(bitmap_rgb32 &bitmap, const rectangle &cliprect, int offs, int x, int y);
	typedef void (galaxian_state::*galaxian_draw_background_func)(bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE8_MEMBER(galaxian_videoram_w);
	DECLARE_WRITE8_MEMBER(galaxian_objram_w);
	DECLARE_WRITE8_MEMBER(galaxian_flip_screen_x_w);
	DECLARE_WRITE8_MEMBER(galaxian_flip_screen_y_w);
	DECLARE_WRITE8_MEMBER(galaxian_flip_screen_xy_w);
	DECLARE_WRITE8_MEMBER(galaxian_stars_enable_w);
	DECLARE_WRITE8_MEMBER(scramble_background_enable_w);
	DECLARE_WRITE8_MEMBER(scramble_background_red_w);
	DECLARE_WRITE8_MEMBER(scramble_background_green_w);
	DECLARE_WRITE8_MEMBER(scramble_background_blue_w);
	DECLARE_WRITE8_MEMBER(galaxian_gfxbank_w);
	DECLARE_CUSTOM_INPUT_MEMBER(gmgalax_port_r);
	DECLARE_CUSTOM_INPUT_MEMBER(azurian_port_r);
	DECLARE_CUSTOM_INPUT_MEMBER(kingball_muxbit_r);
	DECLARE_CUSTOM_INPUT_MEMBER(kingball_noise_r);
	DECLARE_CUSTOM_INPUT_MEMBER(moonwar_dial_r);
	DECLARE_WRITE8_MEMBER(irq_enable_w);
	DECLARE_WRITE8_MEMBER(start_lamp_w);
	DECLARE_WRITE8_MEMBER(coin_lock_w);
	DECLARE_WRITE8_MEMBER(coin_count_0_w);
	DECLARE_WRITE8_MEMBER(coin_count_1_w);
	DECLARE_READ8_MEMBER(konami_ay8910_r);
	DECLARE_WRITE8_MEMBER(konami_ay8910_w);
	DECLARE_WRITE8_MEMBER(konami_sound_filter_w);
	DECLARE_READ8_MEMBER(theend_ppi8255_r);
	DECLARE_WRITE8_MEMBER(theend_ppi8255_w);
	DECLARE_WRITE8_MEMBER(theend_protection_w);
	DECLARE_READ8_MEMBER(theend_protection_r);
	DECLARE_CUSTOM_INPUT_MEMBER(theend_protection_alt_r);
	DECLARE_WRITE8_MEMBER(explorer_sound_control_w);
	DECLARE_READ8_MEMBER(sfx_sample_io_r);
	DECLARE_WRITE8_MEMBER(sfx_sample_io_w);
	DECLARE_READ8_MEMBER(monsterz_protection_r);
	DECLARE_READ8_MEMBER(frogger_ppi8255_r);
	DECLARE_WRITE8_MEMBER(frogger_ppi8255_w);
	DECLARE_READ8_MEMBER(frogger_ay8910_r);
	DECLARE_WRITE8_MEMBER(frogger_ay8910_w);
	IRQ_CALLBACK_MEMBER(froggermc_audiocpu_irq_ack);
	DECLARE_WRITE8_MEMBER(froggermc_sound_control_w);
	DECLARE_READ8_MEMBER(frogf_ppi8255_r);
	DECLARE_WRITE8_MEMBER(frogf_ppi8255_w);
	DECLARE_READ8_MEMBER(turtles_ppi8255_0_r);
	DECLARE_READ8_MEMBER(turtles_ppi8255_1_r);
	DECLARE_WRITE8_MEMBER(turtles_ppi8255_0_w);
	DECLARE_WRITE8_MEMBER(turtles_ppi8255_1_w);
	DECLARE_READ8_MEMBER(scorpion_ay8910_r);
	DECLARE_WRITE8_MEMBER(scorpion_ay8910_w);
	DECLARE_READ8_MEMBER(scorpion_digitalker_intr_r);
	DECLARE_WRITE8_MEMBER(zigzag_bankswap_w);
	DECLARE_WRITE8_MEMBER(zigzag_ay8910_w);
	DECLARE_WRITE8_MEMBER(kingball_speech_dip_w);
	DECLARE_WRITE8_MEMBER(kingball_sound1_w);
	DECLARE_WRITE8_MEMBER(kingball_sound2_w);
	DECLARE_WRITE8_MEMBER(mshuttle_ay8910_cs_w);
	DECLARE_WRITE8_MEMBER(mshuttle_ay8910_control_w);
	DECLARE_WRITE8_MEMBER(mshuttle_ay8910_data_w);
	DECLARE_READ8_MEMBER(mshuttle_ay8910_data_r);
	DECLARE_READ8_MEMBER(jumpbug_protection_r);
	DECLARE_WRITE8_MEMBER(checkman_sound_command_w);
	DECLARE_READ8_MEMBER(checkmaj_protection_r);
	DECLARE_READ8_MEMBER(dingo_3000_r);
	DECLARE_READ8_MEMBER(dingo_3035_r);
	DECLARE_READ8_MEMBER(dingoe_3001_r);
	DECLARE_WRITE8_MEMBER(tenspot_unk_6000_w);
	DECLARE_WRITE8_MEMBER(tenspot_unk_8000_w);
	DECLARE_WRITE8_MEMBER(tenspot_unk_e000_w);
	DECLARE_READ8_MEMBER(froggeram_ppi8255_r);
	DECLARE_WRITE8_MEMBER(froggeram_ppi8255_w);
	DECLARE_WRITE8_MEMBER(artic_gfxbank_w);
	DECLARE_READ8_MEMBER(tenspot_dsw_read);
	DECLARE_INPUT_CHANGED_MEMBER(gmgalax_game_changed);
	DECLARE_WRITE8_MEMBER(konami_sound_control_w);
	DECLARE_READ8_MEMBER(konami_sound_timer_r);
	DECLARE_WRITE8_MEMBER(konami_portc_0_w);
	DECLARE_WRITE8_MEMBER(konami_portc_1_w);
	DECLARE_WRITE8_MEMBER(theend_coin_counter_w);
	DECLARE_READ8_MEMBER(explorer_sound_latch_r);
	DECLARE_WRITE8_MEMBER(sfx_sample_control_w);
	DECLARE_WRITE8_MEMBER(monsterz_porta_1_w);
	DECLARE_WRITE8_MEMBER(monsterz_portb_1_w);
	DECLARE_WRITE8_MEMBER(monsterz_portc_1_w);
	DECLARE_READ8_MEMBER(frogger_sound_timer_r);
	DECLARE_READ8_MEMBER(scorpion_protection_r);
	DECLARE_WRITE8_MEMBER(scorpion_protection_w);
	DECLARE_WRITE8_MEMBER(scorpion_digitalker_control_w);
	DECLARE_WRITE8_MEMBER(kingball_dac_w);
	DECLARE_WRITE8_MEMBER(moonwar_port_select_w);
	void init_fourplay();
	void init_videight();
	void init_galaxian();
	void init_nolock();
	void init_azurian();
	void init_gmgalax();
	void init_pisces();
	void init_batman2();
	void init_frogg();
	void init_mooncrst();
	void init_mooncrsu();
	void init_mooncrgx();
	void init_moonqsr();
	void init_pacmanbl();
	void init_tenspot();
	void init_devilfsg();
	void init_zigzag();
	void init_jumpbug();
	void init_checkman();
	void init_checkmaj();
	void init_dingo();
	void init_dingoe();
	void init_skybase();
	void init_kong();
	void init_mshuttle();
	void init_mshuttlj();
	void init_fantastc();
	void init_timefgtr();
	void init_kingball();
	void init_scorpnmc();
	void init_theend();
	void init_scramble();
	void init_sidam();
	void init_explorer();
	void init_amigo2();
	void init_mandinga();
	void init_sfx();
	void init_atlantis();
	void init_scobra();
	void init_scobrae();
	void init_losttomb();
	void init_frogger();
	void init_froggermc();
	void init_froggers();
	void init_quaak();
	void init_turtles();
	void init_scorpion();
	void init_anteater();
	void init_anteateruk();
	void init_superbon();
	void init_calipso();
	void init_moonwar();
	void init_ghostmun();
	void init_froggrs();
	void init_warofbugg();
	void init_jungsub();
	void init_victoryc();
	TILE_GET_INFO_MEMBER(bg_get_tile_info);
	void galaxian_palette(palette_device &palette);
	void moonwar_palette(palette_device &palette);
	void eagle_palette(palette_device &palette);
	void tenspot_set_game_bank(int bank, int from_game);
	uint32_t screen_update_galaxian(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_interrupt_w);
	DECLARE_WRITE_LINE_MEMBER(tenspot_interrupt_w);
	TIMER_DEVICE_CALLBACK_MEMBER(checkmaj_irq0_gen);
	TIMER_DEVICE_CALLBACK_MEMBER(scramble_stars_blink_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(timefgtr_scanline);
	void state_save_register();
	void sprites_draw(bitmap_rgb32 &bitmap, const rectangle &cliprect, const uint8_t *spritebase);
	void bullets_draw(bitmap_rgb32 &bitmap, const rectangle &cliprect, const uint8_t *base);
	void stars_init();
	void stars_update_origin();
	void stars_draw_row(bitmap_rgb32 &bitmap, int maxx, int y, uint32_t star_offs, uint8_t starmask);
	void null_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void galaxian_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void background_draw_colorsplit(bitmap_rgb32 &bitmap, const rectangle &cliprect, rgb_t color, int split, int split_flipped);
	void scramble_draw_stars(bitmap_rgb32 &bitmap, const rectangle &cliprect, int maxx);
	void scramble_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void anteater_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void jumpbug_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void turtles_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void sfx_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void frogger_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	inline void galaxian_draw_pixel(bitmap_rgb32 &bitmap, const rectangle &cliprect, int y, int x, rgb_t color);
	void galaxian_draw_bullet(bitmap_rgb32 &bitmap, const rectangle &cliprect, int offs, int x, int y);
	void mshuttle_draw_bullet(bitmap_rgb32 &bitmap, const rectangle &cliprect, int offs, int x, int y);
	void scramble_draw_bullet(bitmap_rgb32 &bitmap, const rectangle &cliprect, int offs, int x, int y);
	void theend_draw_bullet(bitmap_rgb32 &bitmap, const rectangle &cliprect, int offs, int x, int y);
	void upper_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x);
	void upper_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);
	void frogger_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x);
	void frogger_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);
	void gmgalax_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x);
	void gmgalax_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);
	void pisces_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x);
	void pisces_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);
	void batman2_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x);
	void mooncrst_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x);
	void mooncrst_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);
	void moonqsr_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x);
	void moonqsr_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);
	void mshuttle_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x);
	void mshuttle_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);
	void calipso_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);
	void jumpbug_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x);
	void jumpbug_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);
	void monsterz_set_latch();
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
	void mshuttle_decode(const uint8_t convtable[8][16]);
	void common_init(galaxian_draw_bullet_func draw_bullet,galaxian_draw_background_func draw_background,
					 galaxian_extend_tile_info_func extend_tile_info,galaxian_extend_sprite_info_func extend_sprite_info);
	void galaxian_base(machine_config &config);
	void sidam_bootleg_base(machine_config &config);
	void konami_base(machine_config &config);
	void konami_sound_1x_ay8910(machine_config &config);
	void konami_sound_2x_ay8910(machine_config &config);
	void scramble_base(machine_config &config);
	void timefgtr(machine_config &config);
	void moonqsr(machine_config &config);
	void frogger(machine_config &config);
	void anteatergg(machine_config &config);
	void theend(machine_config &config);
	void turtles(machine_config &config);
	void fantastc(machine_config &config);
	void jumpbug(machine_config &config);
	void checkmaj(machine_config &config);
	void pacmanbl(machine_config &config);
	void quaak(machine_config &config);
	void galaxian(machine_config &config);
	void gmgalax(machine_config &config);
	void tenspot(machine_config &config);
	void froggers(machine_config &config);
	void froggervd(machine_config &config);
	void mshuttle(machine_config &config);
	void anteateruk(machine_config &config);
	void monsterz(machine_config &config);
	void kingball(machine_config &config);
	void anteaterg(machine_config &config);
	void anteater(machine_config &config);
	void moonwar(machine_config &config);
	void turpins(machine_config &config);
	void explorer(machine_config &config);
	void scramble(machine_config &config);
	void scobra(machine_config &config);
	void froggermc(machine_config &config);
	void froggeram(machine_config &config);
	void spactrai(machine_config &config);
	void takeoff(machine_config &config);
	void sfx(machine_config &config);
	void mooncrst(machine_config &config);
	void eagle(machine_config &config);
	void scorpion(machine_config &config);
	void frogf(machine_config &config);
	void amigo2(machine_config &config);
	void zigzag(machine_config &config);
	void checkman(machine_config &config);
	void jungsub(machine_config &config);
	void victoryc(machine_config &config);
	void frogg(machine_config &config);
	void mandingarf(machine_config &config);
	void thepitm(machine_config &config);
	void skybase(machine_config &config);
	void kong(machine_config &config);
	void scorpnmc(machine_config &config);
	void fourplay(machine_config &config);
	void videight(machine_config &config);

protected:
	void amigo2_map(address_map &map);
	void anteaterg_map(address_map &map);
	void anteatergg_map(address_map &map);
	void anteateruk_map(address_map &map);
	void checkmaj_sound_map(address_map &map);
	void checkman_sound_map(address_map &map);
	void checkman_sound_portmap(address_map &map);
	void explorer_map(address_map &map);
	void fantastc_map(address_map &map);
	void frogf_map(address_map &map);
	void frogg_map(address_map &map);
	void frogger_map(address_map &map);
	void froggervd_map(address_map &map);
	void frogger_sound_map(address_map &map);
	void frogger_sound_portmap(address_map &map);
	void froggeram_map(address_map &map);
	void froggermc_map(address_map &map);
	void galaxian_map(address_map &map);
	void galaxian_map_base(address_map &map);
	void galaxian_map_discrete(address_map &map);
	void jumpbug_map(address_map &map);
	void jungsub_map(address_map &map);
	void jungsub_io_map(address_map &map);
	void kingball_sound_map(address_map &map);
	void kingball_sound_portmap(address_map &map);
	void konami_sound_map(address_map &map);
	void konami_sound_portmap(address_map &map);
	void kong_map(address_map &map);
	void mandingarf_map(address_map &map);
	void monsterz_map(address_map &map);
	void mooncrst_map(address_map &map);
	void mooncrst_map_base(address_map &map);
	void mooncrst_map_discrete(address_map &map);
	void moonqsr_decrypted_opcodes_map(address_map &map);
	void mshuttle_decrypted_opcodes_map(address_map &map);
	void mshuttle_map(address_map &map);
	void mshuttle_portmap(address_map &map);
	void scobra_map(address_map &map);
	void scorpion_map(address_map &map);
	void scorpion_sound_map(address_map &map);
	void scorpion_sound_portmap(address_map &map);
	void scorpnmc_map(address_map &map);
	void sfx_map(address_map &map);
	void sfx_sample_map(address_map &map);
	void sfx_sample_portmap(address_map &map);
	void skybase_map(address_map &map);
	void spactrai_map(address_map &map);
	void takeoff_sound_map(address_map &map);
	void takeoff_sound_portmap(address_map &map);
	void tenspot_select_map(address_map &map);
	void theend_map(address_map &map);
	void thepitm_map(address_map &map);
	void turpins_map(address_map &map);
	void turpins_sound_map(address_map &map);
	void turtles_map(address_map &map);
	void victoryc_map(address_map &map);
	void zigzag_map(address_map &map);

	virtual void machine_start() override { m_lamps.resolve(); }
	virtual void video_start() override;

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_audio2;
	optional_device<dac_byte_interface> m_dac;
	optional_device_array<ay8910_device, 3> m_ay8910;
	optional_device<ay8910_device> m_ay8910_cclimber;
	optional_device<digitalker_device> m_digitalker;
	optional_device_array<i8255_device, 3> m_ppi8255;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch;
	optional_device<discrete_device> m_discrete;

	optional_ioport m_fake_select;
	optional_ioport_array<10> m_tenspot_game_dsw;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;
	output_finder<2> m_lamps;

	int m_bullets_base;
	int m_sprites_base;
	int m_numspritegens;
	int m_counter_74ls161[2];
	int m_direction[2];
	uint8_t m_gmgalax_selected_game;
	uint8_t m_zigzag_ay8910_latch;
	uint8_t m_kingball_speech_dip;
	uint8_t m_kingball_sound;
	uint8_t m_mshuttle_ay8910_cs;
	uint16_t m_protection_state;
	uint8_t m_protection_result;
	uint8_t m_konami_sound_control;
	uint8_t m_sfx_sample_control;
	uint8_t m_moonwar_port_select;
	uint8_t m_irq_enabled;
	int m_irq_line;
	int m_tenspot_current_game;
	uint8_t m_frogger_adjust;
	uint8_t m_x_scale;
	uint8_t m_h0_start;
	uint8_t m_sfx_tilemap;

	galaxian_extend_tile_info_func m_extend_tile_info_ptr;
	galaxian_extend_sprite_info_func m_extend_sprite_info_ptr;
	galaxian_draw_bullet_func m_draw_bullet_ptr;
	galaxian_draw_background_func m_draw_background_ptr;

	tilemap_t *m_bg_tilemap;
	uint8_t m_flipscreen_x;
	uint8_t m_flipscreen_y;
	uint8_t m_background_enable;
	uint8_t m_background_red;
	uint8_t m_background_green;
	uint8_t m_background_blue;
	uint32_t m_star_rng_origin;
	uint32_t m_star_rng_origin_frame;
	rgb_t m_star_color[64];
	std::unique_ptr<uint8_t[]> m_stars;
	uint8_t m_stars_enabled;
	uint8_t m_stars_blink_state;
	rgb_t m_bullet_color[8];
	uint8_t m_gfxbank[5];

	DECLARE_WRITE8_MEMBER(fourplay_rombank_w);
	DECLARE_WRITE8_MEMBER(videight_rombank_w);
	DECLARE_WRITE8_MEMBER(videight_gfxbank_w);
	void videight_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x);
	void videight_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);
	void fourplay_map(address_map &map);
	void videight_map(address_map &map);
};

#endif // MAME_INCLUDES_GALAXIAN_H
