// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Bryan McPhail
/*************************************************************************

    SNK/Alpha 68000 based games

*************************************************************************/

#include "machine/gen_latch.h"

class alpha68k_state : public driver_device
{
public:
	alpha68k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_shared_ram(*this, "shared_ram"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_audiocpu(*this, "audiocpu"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch") { }

	/* memory pointers */
	optional_shared_ptr<uint16_t> m_shared_ram;
	required_shared_ptr<uint16_t> m_spriteram;
	optional_shared_ptr<uint16_t> m_videoram;

	/* video-related */
	tilemap_t     *m_fix_tilemap;
	int         m_bank_base;
	int         m_flipscreen;
	int         m_last_bank;
	int         m_buffer_28;
	int         m_buffer_60;
	int         m_buffer_68;

	/* misc */
	int         m_invert_controls;
	int         m_microcontroller_id;
	int         m_coin_id;
	unsigned    m_trigstate;
	unsigned    m_deposits1;
	unsigned    m_deposits2;
	unsigned    m_credits;
	unsigned    m_coinvalue;
	unsigned    m_microcontroller_data;
	int         m_latch;
	unsigned    m_game_id;  // see below

	/* devices */
	required_device<cpu_device> m_audiocpu;
	uint8_t       m_sound_nmi_mask;
	uint8_t       m_sound_pa_latch;
	void tnextspc_coin_counters_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tnextspc_unknown_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void alpha_microcontroller_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t kyros_dip_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t control_1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t control_2_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t control_2_V_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t control_3_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t control_4_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t jongbou_inputs_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void kyros_sound_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void alpha68k_II_sound_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void alpha68k_V_sound_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void paddlema_soundlatch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tnextspc_soundlatch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t kyros_alpha_trigger_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t alpha_II_trigger_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t alpha_V_trigger_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t sound_cpu_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void sound_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void alpha68k_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void alpha68k_II_video_bank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void alpha68k_V_video_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void init_paddlema();
	void init_btlfield();
	void init_jongbou();
	void init_goldmedl();
	void init_skyadvnt();
	void init_goldmedla();
	void init_gangwarsu();
	void init_gangwars();
	void init_tnextspc();
	void init_timesold1();
	void init_sbasebal();
	void init_sbasebalj();
	void init_skysoldr();
	void init_skyadvntu();
	void init_btlfieldb();
	void init_timesold();
	void init_kyros();
	void init_sstingry();
	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_common();
	void machine_reset_common();
	void palette_init_kyros(palette_device &palette);
	void palette_init_paddlem(palette_device &palette);
	void machine_start_alpha68k_II();
	void machine_reset_alpha68k_II();
	void video_start_alpha68k();
	void machine_start_alpha68k_V();
	void machine_reset_alpha68k_V();
	uint32_t screen_update_sstingry(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_kyros(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_alpha68k_I(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_alpha68k_II(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_alpha68k_V(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_alpha68k_V_sb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void alpha68k_sound_nmi(device_t &device);
	void alpha68k_flipscreen_w( int flip );
	void alpha68k_V_video_bank_w( int bank );
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int j, int s, int e );
	void draw_sprites_V( bitmap_ind16 &bitmap, const rectangle &cliprect, int j, int s, int e, int fx_mask, int fy_mask, int sprite_mask );
	void draw_sprites_I( bitmap_ind16 &bitmap, const rectangle &cliprect, int c, int d, int yshift );
	void kyros_video_banking(int *bank, int data);
	void jongbou_video_banking(int *bank, int data);
	void kyros_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int c, int d );
	void sstingry_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int c, int d );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
};

/* game_id - used to deal with a few game specific situations */
enum
{
	ALPHA68K_BTLFIELDB = 1,     // used in alpha_II_trigger_r
	ALPHA68K_JONGBOU,           // used in kyros_alpha_trigger_r & kyros_draw_sprites
	ALPHA68K_KYROS          // used in kyros_draw_sprites
};
