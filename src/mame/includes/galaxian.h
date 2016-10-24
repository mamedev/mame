// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Couriersud
/***************************************************************************

    Galaxian hardware family

***************************************************************************/

#include "machine/gen_latch.h"
#include "machine/i8255.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/digitalk.h"

/* we scale horizontally by 3 to render stars correctly */
#define GALAXIAN_XSCALE         3

/* master clocks */
#define GALAXIAN_MASTER_CLOCK   (18432000)
#define GALAXIAN_PIXEL_CLOCK    (GALAXIAN_XSCALE*GALAXIAN_MASTER_CLOCK/3)

/* H counts from 128->511, HBLANK starts at 130 and ends at 250 */
/* we normalize this here so that we count 0->383 with HBLANK */
/* from 264-383 */
#define GALAXIAN_HTOTAL         (384*GALAXIAN_XSCALE)
#define GALAXIAN_HBEND          (0*GALAXIAN_XSCALE)
//#define GALAXIAN_H0START      (6*GALAXIAN_XSCALE)
//#define GALAXIAN_HBSTART      (264*GALAXIAN_XSCALE)
#define GALAXIAN_H0START        (0*GALAXIAN_XSCALE)
#define GALAXIAN_HBSTART        (256*GALAXIAN_XSCALE)

#define GALAXIAN_VTOTAL         (264)
#define GALAXIAN_VBEND          (16)
#define GALAXIAN_VBSTART        (224+16)


class galaxian_state : public driver_device
{
public:
	galaxian_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_audiocpu(*this, "audiocpu"),
			m_audio2(*this, "audio2"),
			m_dac(*this, "dac"),
			m_ay8910_0(*this, "8910.0"),
			m_ay8910_1(*this, "8910.1"),
			m_ay8910_2(*this, "8910.2"),
			m_ay8910_cclimber(*this, "cclimber_audio:aysnd"),
			m_digitalker(*this, "digitalker"),
			m_ppi8255_0(*this, "ppi8255_0"),
			m_ppi8255_1(*this, "ppi8255_1"),
			m_ppi8255_2(*this, "ppi8255_2"),
			m_gfxdecode(*this, "gfxdecode"),
			m_screen(*this, "screen"),
			m_palette(*this, "palette"),
			m_soundlatch(*this, "soundlatch"),
			m_fake_select(*this, "FAKE_SELECT"),
			m_tenspot_game_dsw(*this, {"IN2_GAME0", "IN2_GAME1", "IN2_GAME2", "IN2_GAME3", "IN2_GAME4", "IN2_GAME5", "IN2_GAME6", "IN2_GAME7", "IN2_GAME8", "IN2_GAME9"}),
			m_spriteram(*this, "spriteram"),
			m_videoram(*this, "videoram"),
			m_decrypted_opcodes(*this, "decrypted_opcodes") { }

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_audio2;
	optional_device<dac_byte_interface> m_dac;
	optional_device<ay8910_device> m_ay8910_0;
	optional_device<ay8910_device> m_ay8910_1;
	optional_device<ay8910_device> m_ay8910_2;
	optional_device<ay8910_device> m_ay8910_cclimber;
	optional_device<digitalker_device> m_digitalker;
	optional_device<i8255_device> m_ppi8255_0;
	optional_device<i8255_device> m_ppi8255_1;
	optional_device<i8255_device> m_ppi8255_2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch;

	optional_ioport m_fake_select;
	optional_ioport_array<10> m_tenspot_game_dsw;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;

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
	uint8_t m_sfx_tilemap;

	/* video extension callbacks */
	typedef void (galaxian_state::*galaxian_extend_tile_info_func)(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x);
	typedef void (galaxian_state::*galaxian_extend_sprite_info_func)(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);
	typedef void (galaxian_state::*galaxian_draw_bullet_func)(bitmap_rgb32 &bitmap, const rectangle &cliprect, int offs, int x, int y);
	typedef void (galaxian_state::*galaxian_draw_background_func)(bitmap_rgb32 &bitmap, const rectangle &cliprect);

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
	void galaxian_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void galaxian_objram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void galaxian_flip_screen_x_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void galaxian_flip_screen_y_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void galaxian_flip_screen_xy_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void galaxian_stars_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scramble_background_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scramble_background_red_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scramble_background_green_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scramble_background_blue_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void galaxian_gfxbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	ioport_value scramble_protection_alt_r(ioport_field &field, void *param);
	ioport_value gmgalax_port_r(ioport_field &field, void *param);
	ioport_value azurian_port_r(ioport_field &field, void *param);
	ioport_value kingball_muxbit_r(ioport_field &field, void *param);
	ioport_value kingball_noise_r(ioport_field &field, void *param);
	ioport_value moonwar_dial_r(ioport_field &field, void *param);
	void irq_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void start_lamp_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void coin_lock_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void coin_count_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void coin_count_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t konami_ay8910_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void konami_ay8910_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void konami_sound_filter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t theend_ppi8255_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void theend_ppi8255_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void explorer_sound_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t sfx_sample_io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sfx_sample_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t monsterz_protection_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t frogger_ppi8255_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void frogger_ppi8255_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t frogger_ay8910_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void frogger_ay8910_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	int froggermc_audiocpu_irq_ack(device_t &device, int irqline);
	void froggermc_sound_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t frogf_ppi8255_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void frogf_ppi8255_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t turtles_ppi8255_0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t turtles_ppi8255_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void turtles_ppi8255_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void turtles_ppi8255_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t scorpion_ay8910_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void scorpion_ay8910_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t scorpion_digitalker_intr_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void zigzag_bankswap_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void zigzag_ay8910_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kingball_speech_dip_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kingball_sound1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kingball_sound2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mshuttle_ay8910_cs_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mshuttle_ay8910_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mshuttle_ay8910_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mshuttle_ay8910_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t jumpbug_protection_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void checkman_sound_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t checkmaj_protection_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dingo_3000_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dingo_3035_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dingoe_3001_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void tenspot_unk_6000_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tenspot_unk_8000_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tenspot_unk_e000_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void artic_gfxbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t tenspot_dsw_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void gmgalax_game_changed(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void konami_sound_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t konami_sound_timer_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void konami_portc_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void konami_portc_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void theend_coin_counter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scramble_protection_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t scramble_protection_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t explorer_sound_latch_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sfx_sample_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void monsterz_porta_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void monsterz_portb_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void monsterz_portc_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t frogger_sound_timer_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t scorpion_protection_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void scorpion_protection_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scorpion_digitalker_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kingball_dac_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void moonwar_port_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
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
	void init_thepitm();
	void init_theend();
	void init_scramble();
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
	void bg_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void video_start() override;
	void palette_init_galaxian(palette_device &palette);
	void palette_init_moonwar(palette_device &palette);
	void tenspot_set_game_bank(int bank, int from_game);
	uint32_t screen_update_galaxian(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void interrupt_gen(device_t &device);
	void fakechange_interrupt_gen(device_t &device);
	void checkmaj_irq0_gen(timer_device &timer, void *ptr, int32_t param);
	void galaxian_stars_blink_timer(timer_device &timer, void *ptr, int32_t param);
	void timefgtr_scanline(timer_device &timer, void *ptr, int32_t param);
	void state_save_register();
	void sprites_draw(bitmap_rgb32 &bitmap, const rectangle &cliprect, const uint8_t *spritebase);
	void bullets_draw(bitmap_rgb32 &bitmap, const rectangle &cliprect, const uint8_t *base);
	void stars_init();
	void stars_update_origin();
	void stars_draw_row(bitmap_rgb32 &bitmap, int maxx, int y, uint32_t star_offs, uint8_t starmask);
	void galaxian_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void background_draw_colorsplit(bitmap_rgb32 &bitmap, const rectangle &cliprect, rgb_t color, int split, int split_flipped);
	void scramble_draw_stars(bitmap_rgb32 &bitmap, const rectangle &cliprect, int maxx);
	void scramble_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void anteater_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void jumpbug_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void turtles_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void frogger_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void quaak_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect);
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
	void unmap_galaxian_sound(offs_t base);
	void mshuttle_decode(const uint8_t convtable[8][16]);
	void common_init(galaxian_draw_bullet_func draw_bullet,galaxian_draw_background_func draw_background,
		galaxian_extend_tile_info_func extend_tile_info,galaxian_extend_sprite_info_func extend_sprite_info);
};
