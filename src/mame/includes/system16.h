// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Phil Stroffolino, Mirko Buffoni

#include "video/sega16sp.h"
#include "machine/gen_latch.h"
#include "machine/segaic16.h"
#include "sound/msm5205.h"
#include "sound/upd7759.h"

class segas1x_bootleg_state : public sega_16bit_common_base
{
public:
	segas1x_bootleg_state(const machine_config &mconfig, device_type type, const char *tag)
		: sega_16bit_common_base(mconfig, type, tag) ,
		m_textram(*this, "textram"),
		m_bg0_tileram(*this, "bg0_tileram"),
		m_bg1_tileram(*this, "bg1_tileram"),
		m_tileram(*this, "tileram"),
		m_goldnaxeb2_bgpage(*this, "gab2_bgpage"),
		m_goldnaxeb2_fgpage(*this, "gab2_fgpage"),
		m_sprites(*this, "sprites"),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_msm(*this, "5205"),
		m_upd7759(*this, "7759"),
		m_gfxdecode(*this, "gfxdecode"),
		m_soundlatch(*this, "soundlatch"),
		m_decrypted_opcodes(*this, "decrypted_opcodes") { }

	required_shared_ptr<uint16_t> m_textram;
	optional_shared_ptr<uint16_t> m_bg0_tileram;
	optional_shared_ptr<uint16_t> m_bg1_tileram;
	optional_shared_ptr<uint16_t> m_tileram;
	optional_shared_ptr<uint16_t> m_goldnaxeb2_bgpage;
	optional_shared_ptr<uint16_t> m_goldnaxeb2_fgpage;

	required_device<sega_16bit_sprite_device> m_sprites;

	uint16_t m_coinctrl;

	/* game specific */
	int m_passht4b_io1_val;
	int m_passht4b_io2_val;
	int m_passht4b_io3_val;

	int m_beautyb_unkx;

	int m_shinobl_kludge;

	int m_eswat_tilebank0;


	/* video-related */
	tilemap_t *m_background;
	tilemap_t *m_foreground;
	tilemap_t *m_text_layer;
	tilemap_t *m_background2;
	tilemap_t *m_foreground2;
	tilemap_t *m_bg_tilemaps[2];
	tilemap_t *m_text_tilemap;
	double m_weights[2][3][6];

	int m_spritebank_type;
	int m_back_yscroll;
	int m_fore_yscroll;
	int m_text_yscroll;

	int m_bg1_trans; // alien syn + sys18

	int m_tile_bank1;
	int m_tile_bank0;
	int m_bg_page[4];
	int m_fg_page[4];

	uint16_t m_datsu_page[4];

	int m_bg2_page[4];
	int m_fg2_page[4];

	int m_old_bg_page[4];
	int m_old_fg_page[4];
	int m_old_tile_bank1;
	int m_old_tile_bank0;
	int m_old_bg2_page[4];
	int m_old_fg2_page[4];

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
	optional_shared_ptr<uint16_t> m_decrypted_opcodes;

	void sound_command_nmi_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sound_command_irq_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sys16_coinctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t passht4b_service_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t passht4b_io1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t passht4b_io2_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t passht4b_io3_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void sys16_tilebank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void ddcrewbl_spritebank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tturfbl_msm5205_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t tturfbl_soundbank_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void tturfbl_soundbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void s16bl_bgpage_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void s16bl_fgpage_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void s16bl_fgscrollx_bank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void s16bl_fgscrollx_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void s16bl_fgscrolly_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void s16bl_bgscrollx_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void s16bl_bgscrolly_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void datsu_page0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void datsu_page1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void datsu_page2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void datsu_page3_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void goldnaxeb2_fgscrollx_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void goldnaxeb2_bgscrollx_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void goldnaxeb2_fgscrolly_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void goldnaxeb2_bgscrolly_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void goldnaxeb2_fgpage_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void goldnaxeb2_bgpage_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void eswat_tilebank0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void altbeastbl_gfx_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t beautyb_unkx_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void sys18_refreshenable_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sys18_tilebank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t system18_bank_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sys18_soundbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void shdancbl_msm5205_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t shdancbl_soundbank_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void shdancbl_bankctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sys16_tileram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sys16_textram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void s16a_bootleg_bgscrolly_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void s16a_bootleg_bgscrollx_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void s16a_bootleg_fgscrolly_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void s16a_bootleg_fgscrollx_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void s16a_bootleg_tilemapselect_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void upd7759_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_passsht();
	void init_wb3bbl();
	void init_fpointbl();
	void init_eswatbl();
	void init_astormbl();
	void init_shdancbl();
	void init_dduxbl();
	void init_altbeastbl();
	void init_goldnaxeb2();
	void init_bayrouteb1();
	void init_beautyb();
	void init_mwalkbl();
	void init_bayrouteb2();
	void init_shinobl();
	void init_tturfbl();
	void init_goldnaxeb1();
	void init_ddcrewbl();
	void init_common();
	void machine_reset_ddcrewbl();
	tilemap_memory_index sys16_bg_map(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	tilemap_memory_index sys16_text_map(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg2_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg2_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_text_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_s16a_bootleg_tile_infotxt(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_s16a_bootleg_tile_info0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_s16a_bootleg_tile_info1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void video_start_system16();
	void video_start_system18old();
	void video_start_s16a_bootleg_shinobi();
	void video_start_s16a_bootleg_passsht();
	void video_start_s16a_bootleg_wb3bl();
	void video_start_s16a_bootleg();
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
	void shdancbl_msm5205_callback(int state);
	void sound_cause_nmi(int state);
};
