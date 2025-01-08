// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Phil Stroffolino, Mirko Buffoni
#ifndef MAME_SEGA_SYSTEM16_H
#define MAME_SEGA_SYSTEM16_H

#pragma once

#include "machine/74157.h"
#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "sound/upd7759.h"
#include "sega16sp.h"
#include "segaic16.h"
#include "screen.h"
#include "tilemap.h"

class segas1x_bootleg_state : public sega_16bit_common_base
{
public:
	segas1x_bootleg_state(const machine_config &mconfig, device_type type, const char *tag)
		: sega_16bit_common_base(mconfig, type, tag)
		, m_textram(*this, "textram")
		, m_bg0_tileram(*this, "bg0_tileram")
		, m_bg1_tileram(*this, "bg1_tileram")
		, m_tileram(*this, "tileram")
		, m_goldnaxeb2_bgpage(*this, "gab2_bgpage")
		, m_goldnaxeb2_fgpage(*this, "gab2_fgpage")
		, m_sprites_region(*this, "sprites")
		, m_soundcpu_region(*this, "soundcpu")
		, m_soundbank(*this, "soundbank")
		, m_okibank(*this, "okibank")
		, m_screen(*this, "screen")
		, m_sprites(*this, "sprites")
		, m_shinobl_kludge(0)
		, m_maincpu(*this, "maincpu")
		, m_soundcpu(*this, "soundcpu")
		, m_msm(*this, "5205")
		, m_upd7759(*this, "7759")
		, m_gfxdecode(*this, "gfxdecode")
		, m_soundlatch(*this, "soundlatch")
		, m_adpcm_select(*this, "adpcm_select")
		, m_decrypted_opcodes(*this, "decrypted_opcodes")
		, m_leds(*this, "led%u", 0U)
	{ }

	void z80_ym2151(machine_config &config);
	void z80_ym2151_upd7759(machine_config &config);
	void datsu_ym2151_msm5205(machine_config &config);
	void datsu_2x_ym2203_msm5205(machine_config &config);
	void system16_base(machine_config &config);
	void goldnaxeb_base(machine_config &config);
	void passshtb(machine_config &config);
	void goldnaxeb2(machine_config &config);
	void beautyb(machine_config &config);
	void goldnaxeb1(machine_config &config);
	void mwalkbl(machine_config &config);
	void eswatbl2(machine_config &config);
	void ddcrewbl(machine_config &config);
	void shdancbla(machine_config &config);
	void passsht4b(machine_config &config);
	void wb3bb(machine_config &config);
	void wb3bble(machine_config &config);
	void shdancbl(machine_config &config);
	void shinobi_datsu(machine_config &config);
	void bayrouteb1(machine_config &config);
	void tetrisbl(machine_config &config);
	void eswatbl(machine_config &config);
	void dduxbl(machine_config &config);
	void bayrouteb2(machine_config &config);
	void tturfbl(machine_config &config);
	void altbeastbl(machine_config &config);
	void system18(machine_config &config);
	void bloxeedbl(machine_config &config);

	void init_passsht();
	void init_wb3bbl();
	void init_wb3bble();
	void init_fpointbl();
	void init_eswatbl();
	void init_sys18bl_oki();
	void init_astormb2();
	void init_shdancbl();
	void init_dduxbl();
	void init_altbeastbl();
	void init_goldnaxeb2();
	void init_bayrouteb1();
	void init_beautyb();
	void init_bayrouteb2();
	void init_shinobl();
	void init_tturfbl();
	void init_goldnaxeb1();
	void init_ddcrewbl();
	void init_common();

private:
	void sound_command_nmi_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sound_command_irq_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t sound_command_irq_r();
	void soundbank_msm_w(uint8_t data);
	void sys16_coinctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t passht4b_service_r();
	uint16_t passht4b_io1_r();
	uint16_t passht4b_io2_r();
	uint16_t passht4b_io3_r();
	void sys16_tilebank_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void ddcrewbl_spritebank_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void tturfbl_msm5205_data_w(uint8_t data);
	uint8_t tturfbl_soundbank_r(offs_t offset);
	void tturfbl_soundbank_w(uint8_t data);
	void s16bl_bgpage_w(uint16_t data);
	void s16bl_fgpage_w(uint16_t data);
	void s16bl_fgscrollx_bank_w(uint16_t data);
	void s16bl_fgscrollx_w(uint16_t data);
	void s16bl_fgscrolly_w(uint16_t data);
	void s16bl_bgscrollx_w(uint16_t data);
	void s16bl_bgscrolly_w(uint16_t data);
	template<int Page> void datsu_page_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void goldnaxeb2_fgscrollx_w(uint16_t data);
	void goldnaxeb2_bgscrollx_w(uint16_t data);
	void goldnaxeb2_fgscrolly_w(uint16_t data);
	void goldnaxeb2_bgscrolly_w(uint16_t data);
	void goldnaxeb2_fgpage_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void goldnaxeb2_bgpage_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void eswat_tilebank0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void altbeastbl_gfx_w(offs_t offset, uint16_t data);
	uint16_t beautyb_unkx_r();
	void wb3bble_refreshenable_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sys18_refreshenable_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sys18_tilebank_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t system18_bank_r(offs_t offset);
	void sys18_soundbank_w(uint8_t data);
	void shdancbl_msm5205_data_w(uint8_t data);
	uint8_t shdancbl_soundbank_r(offs_t offset);
	void shdancbl_bankctrl_w(uint8_t data);
	void sys18bl_okibank_w(uint8_t data);
	void sys16_tileram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sys16_textram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void s16a_bootleg_bgscrolly_w(uint16_t data);
	void s16a_bootleg_bgscrollx_w(uint16_t data);
	void s16a_bootleg_fgscrolly_w(uint16_t data);
	void s16a_bootleg_fgscrollx_w(uint16_t data);
	void s16a_bootleg_tilemapselect_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void upd7759_bank_w(uint8_t data);

	DECLARE_MACHINE_RESET(ddcrewbl);
	TILEMAP_MAPPER_MEMBER(sys16_bg_map);
	TILEMAP_MAPPER_MEMBER(sys16_text_map);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	TILE_GET_INFO_MEMBER(get_fg2_tile_info);
	TILE_GET_INFO_MEMBER(get_text_tile_info);
	TILE_GET_INFO_MEMBER(get_s16a_bootleg_tile_infotxt);
	TILE_GET_INFO_MEMBER(get_s16a_bootleg_tile_info0);
	TILE_GET_INFO_MEMBER(get_s16a_bootleg_tile_info1);
	DECLARE_VIDEO_START(system16);
	DECLARE_VIDEO_START(system18old);
	DECLARE_VIDEO_START(s16a_bootleg_shinobi);
	DECLARE_VIDEO_START(s16a_bootleg_passsht);
	DECLARE_VIDEO_START(s16a_bootleg_wb3bl);
	DECLARE_VIDEO_START(s16a_bootleg);
	uint32_t screen_update_system16(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_system18old(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_s16a_bootleg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_s16a_bootleg_passht4b(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void setup_system16_bootleg_spritebanking(  );
	void update_page(  );
	void set_tile_bank( int data );
	void set_fg_page( int data );
	void set_bg_page( int data );
	void datsu_set_pages(  );
	void tturfbl_msm5205_callback(int state);
	void datsu_msm5205_callback(int state);
	void shdancbl_msm5205_callback(int state);
	void sound_cause_nmi(int state);

	void bayrouteb1_map(address_map &map) ATTR_COLD;
	void bayrouteb2_map(address_map &map) ATTR_COLD;
	void beautyb_map(address_map &map) ATTR_COLD;
	void bloxeedbl_map(address_map &map) ATTR_COLD;
	void ddcrewbl_map(address_map &map) ATTR_COLD;
	void dduxbl_map(address_map &map) ATTR_COLD;
	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void eswatbl2_map(address_map &map) ATTR_COLD;
	void eswatbl_map(address_map &map) ATTR_COLD;
	void goldnaxeb1_map(address_map &map) ATTR_COLD;
	void goldnaxeb2_map(address_map &map) ATTR_COLD;
	void mwalkbl_map(address_map &map) ATTR_COLD;
	void passht4b_map(address_map &map) ATTR_COLD;
	void passshtb_map(address_map &map) ATTR_COLD;
	void pcm_map(address_map &map) ATTR_COLD;
	void shdancbl_map(address_map &map) ATTR_COLD;
	void shdancbl_sound_io_map(address_map &map) ATTR_COLD;
	void shdancbl_sound_map(address_map &map) ATTR_COLD;
	void shdancbla_map(address_map &map) ATTR_COLD;
	void shdancbla_sound_map(address_map &map) ATTR_COLD;
	void shinobi_datsu_sound_map(address_map &map) ATTR_COLD;
	void shinobib_map(address_map &map) ATTR_COLD;
	void sound_18_io_map(address_map &map) ATTR_COLD;
	void sound_18_map(address_map &map) ATTR_COLD;
	void sound_7759_io_map(address_map &map) ATTR_COLD;
	void sound_7759_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void sys18bl_oki_map(address_map &map) ATTR_COLD;
	void sys18bl_sound_map(address_map &map) ATTR_COLD;
	void tetrisbl_map(address_map &map) ATTR_COLD;
	void tturfbl_map(address_map &map) ATTR_COLD;
	void tturfbl_sound_io_map(address_map &map) ATTR_COLD;
	void tturfbl_sound_map(address_map &map) ATTR_COLD;
	void wb3bbl_map(address_map &map) ATTR_COLD;
	void wb3bble_map(address_map &map) ATTR_COLD;
	void wb3bble_decrypted_opcodes_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override { m_leds.resolve(); }

	required_shared_ptr<uint16_t> m_textram;
	optional_shared_ptr<uint16_t> m_bg0_tileram;
	optional_shared_ptr<uint16_t> m_bg1_tileram;
	optional_shared_ptr<uint16_t> m_tileram;
	optional_shared_ptr<uint16_t> m_goldnaxeb2_bgpage;
	optional_shared_ptr<uint16_t> m_goldnaxeb2_fgpage;

	optional_memory_region m_sprites_region;
	optional_memory_region m_soundcpu_region;
	optional_memory_bank m_soundbank;
	optional_memory_bank m_okibank;

	required_device<screen_device> m_screen;
	optional_device<sega_16bit_sprite_device> m_sprites;

	uint16_t m_coinctrl = 0;

	/* game specific */
	int m_passht4b_io1_val = 0;
	int m_passht4b_io2_val = 0;
	int m_passht4b_io3_val = 0;

	int m_beautyb_unkx = 0;

	int m_shinobl_kludge = 0; // TODO: this never gets set, causing unreachable code in get_text_tile_info

	int m_eswat_tilebank0 = 0;


	/* video-related */
	tilemap_t *m_background[2]{};
	tilemap_t *m_foreground[2]{};
	tilemap_t *m_text_layer = nullptr;
	tilemap_t *m_bg_tilemaps[2]{};
	tilemap_t *m_text_tilemap = nullptr;
	double m_weights[2][3][6]{};

	int m_spritebank_type = 0;
	int m_back_yscroll = 0;
	int m_fore_yscroll = 0;
	int m_text_yscroll = 0;

	int m_bg1_trans = 0; // alien syn + sys18

	int m_tile_bank[2]{};
	int m_bg_page[2][4]{};
	int m_fg_page[2][4]{};

	uint16_t m_datsu_page[4]{};

	int m_old_bg_page[2][4]{};
	int m_old_fg_page[2][4]{};
	int m_old_tile_bank[2]{};

	int m_bg_scrollx = 0;
	int m_bg_scrolly = 0;
	int m_fg_scrollx = 0;
	int m_fg_scrolly = 0;
	uint16_t m_tilemapselect = 0;

	int m_textlayer_lo_min = 0;
	int m_textlayer_lo_max = 0;
	int m_textlayer_hi_min = 0;
	int m_textlayer_hi_max = 0;

	int m_tilebank_switch = 0;


	/* sound-related */
	int m_sample_buffer = 0;
	int m_sample_select = 0;

	uint8_t *m_soundbank_ptr = nullptr;     /* Pointer to currently selected portion of ROM */

	/* sys18 */
	uint8_t *m_sound_bank = nullptr;
	uint16_t *m_splittab_bg_x = nullptr;
	uint16_t *m_splittab_bg_y = nullptr;
	uint16_t *m_splittab_fg_x = nullptr;
	uint16_t *m_splittab_fg_y = nullptr;
	int     m_sound_info[4*2]{};
	int     m_refreshenable = 0;
	int     m_system18 = 0;

	uint8_t *m_decrypted_region = nullptr;  // goldnaxeb1 & bayrouteb1

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_soundcpu;
	optional_device<msm5205_device> m_msm;
	optional_device<upd7759_device> m_upd7759;
	required_device<gfxdecode_device> m_gfxdecode;
	optional_device<generic_latch_8_device> m_soundlatch;
	optional_device<ls157_device> m_adpcm_select;
	optional_shared_ptr<uint16_t> m_decrypted_opcodes;
	output_finder<2> m_leds;
};

#endif // MAME_SEGA_SYSTEM16_H
