// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Phil Stroffolino, Mirko Buffoni
#ifndef MAME_INCLUDES_SYSTEM16_H
#define MAME_INCLUDES_SYSTEM16_H

#pragma once

#include "video/sega16sp.h"
#include "machine/74157.h"
#include "machine/gen_latch.h"
#include "machine/segaic16.h"
#include "sound/msm5205.h"
#include "sound/upd7759.h"
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
		, m_sprites(*this, "sprites")
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
	void astormbl(machine_config &config);
	void astormb2(machine_config &config);
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

	void init_passsht();
	void init_wb3bbl();
	void init_wb3bble();
	void init_fpointbl();
	void init_eswatbl();
	void init_astormbl();
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
	DECLARE_WRITE16_MEMBER(sound_command_nmi_w);
	DECLARE_WRITE16_MEMBER(sound_command_irq_w);
	DECLARE_READ8_MEMBER(sound_command_irq_r);
	DECLARE_WRITE8_MEMBER(soundbank_msm_w);
	DECLARE_WRITE16_MEMBER(sys16_coinctrl_w);
	DECLARE_READ16_MEMBER(passht4b_service_r);
	DECLARE_READ16_MEMBER(passht4b_io1_r);
	DECLARE_READ16_MEMBER(passht4b_io2_r);
	DECLARE_READ16_MEMBER(passht4b_io3_r);
	DECLARE_WRITE16_MEMBER(sys16_tilebank_w);
	DECLARE_WRITE16_MEMBER(ddcrewbl_spritebank_w);
	DECLARE_WRITE8_MEMBER(tturfbl_msm5205_data_w);
	DECLARE_READ8_MEMBER(tturfbl_soundbank_r);
	DECLARE_WRITE8_MEMBER(tturfbl_soundbank_w);
	DECLARE_WRITE16_MEMBER(s16bl_bgpage_w);
	DECLARE_WRITE16_MEMBER(s16bl_fgpage_w);
	DECLARE_WRITE16_MEMBER(s16bl_fgscrollx_bank_w);
	DECLARE_WRITE16_MEMBER(s16bl_fgscrollx_w);
	DECLARE_WRITE16_MEMBER(s16bl_fgscrolly_w);
	DECLARE_WRITE16_MEMBER(s16bl_bgscrollx_w);
	DECLARE_WRITE16_MEMBER(s16bl_bgscrolly_w);
	template<int Page> DECLARE_WRITE16_MEMBER(datsu_page_w);
	DECLARE_WRITE16_MEMBER(goldnaxeb2_fgscrollx_w);
	DECLARE_WRITE16_MEMBER(goldnaxeb2_bgscrollx_w);
	DECLARE_WRITE16_MEMBER(goldnaxeb2_fgscrolly_w);
	DECLARE_WRITE16_MEMBER(goldnaxeb2_bgscrolly_w);
	DECLARE_WRITE16_MEMBER(goldnaxeb2_fgpage_w);
	DECLARE_WRITE16_MEMBER(goldnaxeb2_bgpage_w);
	DECLARE_WRITE16_MEMBER(eswat_tilebank0_w);
	DECLARE_WRITE16_MEMBER(altbeastbl_gfx_w);
	DECLARE_READ16_MEMBER(beautyb_unkx_r);
	DECLARE_WRITE16_MEMBER(wb3bble_refreshenable_w);
	DECLARE_WRITE16_MEMBER(sys18_refreshenable_w);
	DECLARE_WRITE16_MEMBER(sys18_tilebank_w);
	DECLARE_READ8_MEMBER(system18_bank_r);
	DECLARE_WRITE8_MEMBER(sys18_soundbank_w);
	DECLARE_WRITE8_MEMBER(shdancbl_msm5205_data_w);
	DECLARE_READ8_MEMBER(shdancbl_soundbank_r);
	DECLARE_WRITE8_MEMBER(shdancbl_bankctrl_w);
	DECLARE_WRITE8_MEMBER(sys18bl_okibank_w);
	DECLARE_WRITE16_MEMBER(sys16_tileram_w);
	DECLARE_WRITE16_MEMBER(sys16_textram_w);
	DECLARE_WRITE16_MEMBER(s16a_bootleg_bgscrolly_w);
	DECLARE_WRITE16_MEMBER(s16a_bootleg_bgscrollx_w);
	DECLARE_WRITE16_MEMBER(s16a_bootleg_fgscrolly_w);
	DECLARE_WRITE16_MEMBER(s16a_bootleg_fgscrollx_w);
	DECLARE_WRITE16_MEMBER(s16a_bootleg_tilemapselect_w);
	DECLARE_WRITE8_MEMBER(upd7759_bank_w);

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
	DECLARE_WRITE_LINE_MEMBER(tturfbl_msm5205_callback);
	DECLARE_WRITE_LINE_MEMBER(datsu_msm5205_callback);
	DECLARE_WRITE_LINE_MEMBER(shdancbl_msm5205_callback);
	DECLARE_WRITE_LINE_MEMBER(sound_cause_nmi);

	void astormbl_map(address_map &map);
	void bayrouteb1_map(address_map &map);
	void bayrouteb2_map(address_map &map);
	void beautyb_map(address_map &map);
	void ddcrewbl_map(address_map &map);
	void dduxbl_map(address_map &map);
	void decrypted_opcodes_map(address_map &map);
	void eswatbl2_map(address_map &map);
	void eswatbl_map(address_map &map);
	void goldnaxeb1_map(address_map &map);
	void goldnaxeb2_map(address_map &map);
	void mwalkbl_map(address_map &map);
	void passht4b_map(address_map &map);
	void passshtb_map(address_map &map);
	void pcm_map(address_map &map);
	void shdancbl_map(address_map &map);
	void shdancbl_sound_io_map(address_map &map);
	void shdancbl_sound_map(address_map &map);
	void shdancbla_map(address_map &map);
	void shdancbla_sound_map(address_map &map);
	void shinobi_datsu_sound_map(address_map &map);
	void shinobib_map(address_map &map);
	void sound_18_io_map(address_map &map);
	void sound_18_map(address_map &map);
	void sound_7759_io_map(address_map &map);
	void sound_7759_map(address_map &map);
	void sound_io_map(address_map &map);
	void sound_map(address_map &map);
	void sys18bl_oki_map(address_map &map);
	void sys18bl_sound_map(address_map &map);
	void tetrisbl_map(address_map &map);
	void tturfbl_map(address_map &map);
	void tturfbl_sound_io_map(address_map &map);
	void tturfbl_sound_map(address_map &map);
	void wb3bbl_map(address_map &map);
	void wb3bble_map(address_map &map);
	void wb3bble_decrypted_opcodes_map(address_map &map);

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

	optional_device<sega_16bit_sprite_device> m_sprites;

	uint16_t m_coinctrl;

	/* game specific */
	int m_passht4b_io1_val;
	int m_passht4b_io2_val;
	int m_passht4b_io3_val;

	int m_beautyb_unkx;

	int m_shinobl_kludge;

	int m_eswat_tilebank0;


	/* video-related */
	tilemap_t *m_background[2];
	tilemap_t *m_foreground[2];
	tilemap_t *m_text_layer;
	tilemap_t *m_bg_tilemaps[2];
	tilemap_t *m_text_tilemap;
	double m_weights[2][3][6];

	int m_spritebank_type;
	int m_back_yscroll;
	int m_fore_yscroll;
	int m_text_yscroll;

	int m_bg1_trans; // alien syn + sys18

	int m_tile_bank[2];
	int m_bg_page[2][4];
	int m_fg_page[2][4];

	uint16_t m_datsu_page[4];

	int m_old_bg_page[2][4];
	int m_old_fg_page[2][4];
	int m_old_tile_bank[2];

	int m_bg_scrollx;
	int m_bg_scrolly;
	int m_fg_scrollx;
	int m_fg_scrolly;
	uint16_t m_tilemapselect;

	int m_textlayer_lo_min;
	int m_textlayer_lo_max;
	int m_textlayer_hi_min;
	int m_textlayer_hi_max;

	int m_tilebank_switch;


	/* sound-related */
	int m_sample_buffer;
	int m_sample_select;

	uint8_t *m_soundbank_ptr;     /* Pointer to currently selected portion of ROM */

	/* sys18 */
	uint8_t *m_sound_bank;
	uint16_t *m_splittab_bg_x;
	uint16_t *m_splittab_bg_y;
	uint16_t *m_splittab_fg_x;
	uint16_t *m_splittab_fg_y;
	int     m_sound_info[4*2];
	int     m_refreshenable;
	int     m_system18;

	uint8_t *m_decrypted_region;  // goldnaxeb1 & bayrouteb1

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

#endif // MAME_INCLUDES_SYSTEM16_H
