// license:BSD-3-Clause
// copyright-holders:Luca Elia
#ifndef MAME_INCLUDES_CAVE_H
#define MAME_INCLUDES_CAVE_H

#pragma once

/***************************************************************************

    Cave hardware

***************************************************************************/

#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "machine/nmk112.h"
#include "machine/timer.h"
#include "sound/okim6295.h"
#include "screen.h"

class cave_state : public driver_device
{
public:
	enum
	{
		MAX_PRIORITY        = 4,
		MAX_SPRITE_NUM      = 0x400
	};

	struct sprite_cave
	{
		sprite_cave() { }

		int priority = 0, flags = 0;

		const uint8_t *pen_data = nullptr;  /* points to top left corner of tile data */
		int line_offset = 0;

		pen_t base_pen = 0;
		int tile_width = 0, tile_height = 0;
		int total_width = 0, total_height = 0;  /* in screen coordinates */
		int x = 0, y = 0, xcount0 = 0, ycount0 = 0;
		int zoomx_re = 0, zoomy_re = 0;
	};

	cave_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_videoregs(*this, "videoregs.%u", 0)
		, m_vram(*this, "vram.%u", 0)
		, m_vctrl(*this, "vctrl.%u", 0)
		, m_spriteram(*this, "spriteram.%u", 0)
		, m_spriteram_2(*this, "spriteram_2.%u", 0)
		, m_paletteram(*this, "paletteram.%u", 0)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_oki(*this, "oki")
		, m_int_timer(*this, "int_timer")
		, m_int_timer_left(*this, "int_timer_left")
		, m_int_timer_right(*this, "int_timer_right")
		, m_eeprom(*this, "eeprom")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch") { }

	/* memory pointers */
	optional_shared_ptr_array<uint16_t, 4> m_videoregs;
	optional_shared_ptr_array<uint16_t, 4> m_vram;
	optional_shared_ptr_array<uint16_t, 4> m_vctrl;
	optional_shared_ptr_array<uint16_t, 4> m_spriteram;
	optional_shared_ptr_array<uint16_t, 4> m_spriteram_2;
	optional_shared_ptr_array<uint16_t, 4> m_paletteram;

	/* video-related */
	std::unique_ptr<sprite_cave []> m_sprite[4];
	sprite_cave *m_sprite_table[4][MAX_PRIORITY][MAX_SPRITE_NUM + 1];

	struct
	{
		int    clip_left, clip_right, clip_top, clip_bottom;
		uint8_t  *baseaddr;
		int    line_offset;
		uint8_t  *baseaddr_zbuf;
		int    line_offset_zbuf;
	} m_blit;


	void (cave_state::*m_get_sprite_info)(int chip);
	void (cave_state::*m_sprite_draw)(int chip, int priority);

	tilemap_t    *m_tilemap[4];
	int          m_tiledim[4];
	int          m_old_tiledim[4];

	bitmap_ind16 m_sprite_zbuf;
	uint16_t       m_sprite_zbuf_baseval;

	int          m_num_sprites[4];

	int          m_spriteram_bank[4];
	int          m_spriteram_bank_delay[4];

	std::unique_ptr<uint16_t[]>      m_palette_map[4];

	int          m_layers_offs_x;
	int          m_layers_offs_y;
	int          m_row_effect_offs_n;
	int          m_row_effect_offs_f;
	int          m_background_pen;

	int          m_spritetype[2];
	int          m_kludge;
	emu_timer *m_vblank_end_timer;


	/* misc */
	int          m_time_vblank_irq;
	uint8_t        m_irq_level;
	uint8_t        m_vblank_irq;
	uint8_t        m_sound_irq;
	uint8_t        m_unknown_irq;
	uint8_t        m_agallet_vblank_irq;

	/* sound related */
	int          m_soundbuf_len;
	uint8_t        m_soundbuf_data[32];
	//uint8_t        m_sound_flag1;
	//uint8_t        m_sound_flag2;

	/* game specific */
	// sailormn
	int          m_sailormn_tilebank;
	// korokoro
	uint16_t       m_leds[2];
	int          m_hopper;
	// ppsatan
	uint16_t       m_ppsatan_io_mux;

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<okim6295_device> m_oki;
	required_device<timer_device> m_int_timer;
	optional_device<timer_device> m_int_timer_left;
	optional_device<timer_device> m_int_timer_right;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_16_device> m_soundlatch;

	int m_rasflag;
	int m_old_rasflag;
	DECLARE_READ16_MEMBER(cave_irq_cause_r);
	DECLARE_READ8_MEMBER(soundflags_r);
	DECLARE_READ16_MEMBER(soundflags_ack_r);
	DECLARE_WRITE16_MEMBER(sound_cmd_w);
	DECLARE_READ8_MEMBER(soundlatch_lo_r);
	DECLARE_READ8_MEMBER(soundlatch_hi_r);
	DECLARE_READ16_MEMBER(soundlatch_ack_r);
	DECLARE_WRITE8_MEMBER(soundlatch_ack_w);
	DECLARE_WRITE16_MEMBER(gaia_coin_lsb_w);
	DECLARE_READ16_MEMBER(donpachi_videoregs_r);
	DECLARE_WRITE16_MEMBER(korokoro_leds_w);
	DECLARE_WRITE16_MEMBER(pwrinst2_vctrl_0_w);
	DECLARE_WRITE16_MEMBER(pwrinst2_vctrl_1_w);
	DECLARE_WRITE16_MEMBER(pwrinst2_vctrl_2_w);
	DECLARE_WRITE16_MEMBER(pwrinst2_vctrl_3_w);
	DECLARE_READ16_MEMBER(sailormn_input0_r);
	DECLARE_WRITE16_MEMBER(tjumpman_leds_w);
	DECLARE_WRITE16_MEMBER(pacslot_leds_w);
	DECLARE_WRITE8_MEMBER(hotdogst_rombank_w);
	DECLARE_WRITE8_MEMBER(hotdogst_okibank_w);
	DECLARE_WRITE8_MEMBER(mazinger_rombank_w);
	DECLARE_WRITE8_MEMBER(metmqstr_rombank_w);
	DECLARE_WRITE8_MEMBER(metmqstr_okibank_w);
	DECLARE_WRITE8_MEMBER(metmqstr_oki2bank_w);
	DECLARE_WRITE8_MEMBER(pwrinst2_rombank_w);
	DECLARE_WRITE8_MEMBER(sailormn_rombank_w);
	DECLARE_WRITE8_MEMBER(sailormn_okibank_w);
	DECLARE_WRITE8_MEMBER(sailormn_oki2bank_w);
	DECLARE_WRITE16_MEMBER(cave_vram_0_w);
	DECLARE_WRITE16_MEMBER(cave_vram_1_w);
	DECLARE_WRITE16_MEMBER(cave_vram_2_w);
	DECLARE_WRITE16_MEMBER(cave_vram_3_w);
	DECLARE_WRITE16_MEMBER(cave_vram_0_8x8_w);
	DECLARE_WRITE16_MEMBER(cave_vram_1_8x8_w);
	DECLARE_WRITE16_MEMBER(cave_vram_2_8x8_w);
	DECLARE_WRITE16_MEMBER(cave_vram_3_8x8_w);
	DECLARE_CUSTOM_INPUT_MEMBER(korokoro_hopper_r);
	DECLARE_CUSTOM_INPUT_MEMBER(tjumpman_hopper_r);
	DECLARE_WRITE16_MEMBER(cave_eeprom_msb_w);
	DECLARE_WRITE16_MEMBER(sailormn_eeprom_msb_w);
	DECLARE_WRITE16_MEMBER(hotdogst_eeprom_msb_w);
	DECLARE_WRITE16_MEMBER(cave_eeprom_lsb_w);
	DECLARE_WRITE16_MEMBER(metmqstr_eeprom_msb_w);
	DECLARE_WRITE16_MEMBER(korokoro_eeprom_msb_w);
	DECLARE_READ16_MEMBER(pwrinst2_eeprom_r);
	DECLARE_WRITE16_MEMBER(tjumpman_eeprom_lsb_w);
	DECLARE_WRITE16_MEMBER(ppsatan_eeprom_msb_w);
	DECLARE_WRITE16_MEMBER(ppsatan_io_mux_w);
	DECLARE_READ16_MEMBER(ppsatan_touch1_r);
	DECLARE_READ16_MEMBER(ppsatan_touch2_r);
	DECLARE_WRITE16_MEMBER(ppsatan_out_w);
	uint16_t ppsatan_touch_r(int player);
	DECLARE_DRIVER_INIT(uopoko);
	DECLARE_DRIVER_INIT(donpachi);
	DECLARE_DRIVER_INIT(mazinger);
	DECLARE_DRIVER_INIT(gaia);
	DECLARE_DRIVER_INIT(pwrinst2);
	DECLARE_DRIVER_INIT(ddonpach);
	DECLARE_DRIVER_INIT(agallet);
	DECLARE_DRIVER_INIT(hotdogst);
	DECLARE_DRIVER_INIT(tjumpman);
	DECLARE_DRIVER_INIT(korokoro);
	DECLARE_DRIVER_INIT(esprade);
	DECLARE_DRIVER_INIT(pwrinst2j);
	DECLARE_DRIVER_INIT(guwange);
	DECLARE_DRIVER_INIT(feversos);
	DECLARE_DRIVER_INIT(sailormn);
	DECLARE_DRIVER_INIT(dfeveron);
	DECLARE_DRIVER_INIT(metmqstr);
	DECLARE_DRIVER_INIT(ppsatan);
	TILE_GET_INFO_MEMBER(sailormn_get_tile_info_2);
	TILE_GET_INFO_MEMBER(get_tile_info_0);
	TILE_GET_INFO_MEMBER(get_tile_info_1);
	TILE_GET_INFO_MEMBER(get_tile_info_2);
	TILE_GET_INFO_MEMBER(get_tile_info_3);
	DECLARE_MACHINE_START(cave);
	DECLARE_MACHINE_RESET(cave);
	DECLARE_MACHINE_RESET(sailormn);
	DECLARE_VIDEO_START(cave_2_layers);
	DECLARE_PALETTE_INIT(dfeveron);
	DECLARE_VIDEO_START(cave_3_layers);
	DECLARE_PALETTE_INIT(ddonpach);
	DECLARE_PALETTE_INIT(cave);
	DECLARE_VIDEO_START(cave_1_layer);
	DECLARE_PALETTE_INIT(korokoro);
	DECLARE_PALETTE_INIT(mazinger);
	DECLARE_VIDEO_START(cave_4_layers);
	DECLARE_PALETTE_INIT(pwrinst2);
	DECLARE_VIDEO_START(sailormn_3_layers);
	DECLARE_PALETTE_INIT(sailormn);
	DECLARE_PALETTE_INIT(ppsatan);
	uint32_t screen_update_cave(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_ppsatan_core (screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int chip);
	uint32_t screen_update_ppsatan_top  (screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_ppsatan_left (screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_ppsatan_right(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(cave_interrupt);
	INTERRUPT_GEN_MEMBER(cave_interrupt_ppsatan);
	TIMER_CALLBACK_MEMBER(cave_vblank_end);
	TIMER_DEVICE_CALLBACK_MEMBER(cave_vblank_start);
	TIMER_DEVICE_CALLBACK_MEMBER(cave_vblank_start_left);
	TIMER_DEVICE_CALLBACK_MEMBER(cave_vblank_start_right);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_lev2_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(sailormn_startup);
	void cave_get_sprite_info(int chip);
	void cave_get_sprite_info_all();
	void sailormn_tilebank_w(int bank);
	DECLARE_WRITE_LINE_MEMBER(sound_irq_gen);
	void update_irq_state();
	void unpack_sprites(const char *region);
	void ddonpach_unpack_sprites(const char *region);
	void esprade_unpack_sprites(const char *region);
	void sailormn_unpack_tiles(const char *region);

	void uopoko(machine_config &config);
	void sailormn(machine_config &config);
	void paceight(machine_config &config);
	void pacslot(machine_config &config);
	void hotdogst(machine_config &config);
	void crusherm(machine_config &config);
	void donpachi(machine_config &config);
	void tekkencw(machine_config &config);
	void korokoro(machine_config &config);
	void esprade(machine_config &config);
	void mazinger(machine_config &config);
	void tjumpman(machine_config &config);
	void tekkenbs(machine_config &config);
	void gaia(machine_config &config);
	void metmqstr(machine_config &config);
	void ppsatan(machine_config &config);
	void guwange(machine_config &config);
	void dfeveron(machine_config &config);
	void ddonpach(machine_config &config);
	void pwrinst2(machine_config &config);
	void crusherm_map(address_map &map);
	void ddonpach_map(address_map &map);
	void dfeveron_map(address_map &map);
	void donpachi_map(address_map &map);
	void esprade_map(address_map &map);
	void gaia_map(address_map &map);
	void guwange_map(address_map &map);
	void hotdogst_map(address_map &map);
	void hotdogst_sound_map(address_map &map);
	void hotdogst_sound_portmap(address_map &map);
	void korokoro_map(address_map &map);
	void mazinger_map(address_map &map);
	void mazinger_sound_map(address_map &map);
	void mazinger_sound_portmap(address_map &map);
	void metmqstr_map(address_map &map);
	void metmqstr_sound_map(address_map &map);
	void metmqstr_sound_portmap(address_map &map);
	void oki2_map(address_map &map);
	void oki_map(address_map &map);
	void paceight_map(address_map &map);
	void pacslot_map(address_map &map);
	void ppsatan_map(address_map &map);
	void pwrinst2_map(address_map &map);
	void pwrinst2_sound_map(address_map &map);
	void pwrinst2_sound_portmap(address_map &map);
	void sailormn_map(address_map &map);
	void sailormn_sound_map(address_map &map);
	void sailormn_sound_portmap(address_map &map);
	void tekkenbs_map(address_map &map);
	void tekkencw_map(address_map &map);
	void tjumpman_map(address_map &map);
	void uopoko_map(address_map &map);
private:
	inline void get_tile_info( tile_data &tileinfo, int tile_index, int GFX );
	inline void tilemap_draw( int chip, screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, uint32_t flags, uint32_t priority, uint32_t priority2, int GFX );
	inline void vram_w( address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED uint16_t data, ATTR_UNUSED uint16_t mem_mask, int GFX );
	inline void vram_8x8_w( address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED uint16_t data, ATTR_UNUSED uint16_t mem_mask, int GFX );
	inline void vctrl_w( address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask, int GFX );
	void set_pens(int chip);
	void cave_vh_start( int num );
	void get_sprite_info_cave(int chip);
	void get_sprite_info_donpachi(int chip);
	void sprite_init_cave();
	void cave_sprite_check(int chip, screen_device &screen, const rectangle &clip);
	void do_blit_zoom32_cave( int chip, const sprite_cave *sprite );
	void do_blit_zoom32_cave_zb( int chip, const sprite_cave *sprite );
	void do_blit_32_cave( int chip, const sprite_cave *sprite );
	void do_blit_32_cave_zb( int chip, const sprite_cave *sprite );
	void sprite_draw_cave( int chip, int priority );
	void sprite_draw_cave_zbuf( int chip, int priority );
	void sprite_draw_donpachi( int chip, int priority );
	void sprite_draw_donpachi_zbuf( int chip, int priority );
	void init_cave();
	void show_leds();
};

#endif // MAME_INCLUDES_CAVE_H
