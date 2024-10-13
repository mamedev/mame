// license:BSD-3-Clause
// copyright-holders:David Haywood,Bryan McPhail
#ifndef MAME_DATAEAST_TUMBLEB_H
#define MAME_DATAEAST_TUMBLEB_H

#pragma once

#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "decospr.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class tumbleb_state : public driver_device
{
public:
	tumbleb_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_mainram(*this, "mainram"),
		m_spriteram(*this, "spriteram"),
		m_pf1_data(*this, "pf1_data"),
		m_pf2_data(*this, "pf2_data"),
		m_control(*this, "control"),
		m_pf1_tilemap(nullptr),
		m_pf1_alt_tilemap(nullptr),
		m_pf2_tilemap(nullptr),
		m_pf2_alt_tilemap(nullptr),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_sprgen(*this, "spritegen"),
		m_screen(*this, "screen"),
		m_soundlatch(*this, "soundlatch"),
		m_protbase(0)
	{ }

	void tumblepb(machine_config &config);
	void tumblepba(machine_config &config);
	void bcstory(machine_config &config);
	void pangpang(machine_config &config);
	void semibase(machine_config &config);
	void tumbleb2(machine_config &config);
	void cookbib(machine_config &config);
	void metlsavr(machine_config &config);
	void fncywld(machine_config &config);
	void magipur(machine_config &config);
	void suprtrio(machine_config &config);
	void htchctch(machine_config &config);
	void htchctch_mcu(machine_config &config);
	void sdfight(machine_config &config);
	void chokchok(machine_config &config);
	void cookbib_mcu(machine_config &config);
	void jumpkids(machine_config &config);

	void init_dquizgo();
	void init_jumpkids();
	void init_htchctch();
	void init_wlstar();
	void init_suprtrio();
	void init_tumblepb();
	void init_tumblepba();
	void init_bcstory();
	void init_wondl96();
	void init_tumbleb2();
	void init_chokchok();
	void init_fncywld();
	void init_magipur();
	void init_carket();

	ioport_value suprtrio_prot_latch_r();

protected:
	/* memory pointers */
	optional_shared_ptr<uint16_t> m_mainram;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_pf1_data;
	required_shared_ptr<uint16_t> m_pf2_data;
	optional_shared_ptr<uint16_t> m_control;

	/* misc */
	int         m_music_command = 0;
	int         m_music_bank = 0;
	int         m_music_is_playing = 0;

	/* video-related */
	tilemap_t   *m_pf1_tilemap = nullptr;
	tilemap_t   *m_pf1_alt_tilemap = nullptr;
	tilemap_t   *m_pf2_tilemap = nullptr;
	tilemap_t   *m_pf2_alt_tilemap = nullptr;
	uint16_t      m_control_0[8]{};
	uint16_t      m_tilebank = 0;

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<decospr_device> m_sprgen;
	required_device<screen_device> m_screen;
	optional_device<generic_latch_8_device> m_soundlatch;

	uint8_t m_semicom_prot_offset = 0;
	uint16_t m_protbase;
	void tumblepb_oki_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t tumblepb_prot_r();
	uint16_t tumblepopb_controls_r(offs_t offset);
	uint16_t semibase_unknown_r();
	void jumpkids_sound_w(uint16_t data);
	void semicom_soundcmd_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void oki_sound_bank_w(uint8_t data);
	void jumpkids_oki_bank_w(uint8_t data);
	void prot_p0_w(uint8_t data);
	void prot_p1_w(uint8_t data);
	void prot_p2_w(uint8_t data);
	uint16_t bcstory_1a0_read();
	void bcstory_tilebank_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void chokchok_tilebank_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void wlstar_tilebank_w(uint16_t data);
	void suprtrio_tilebank_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void tumblepb_pf1_data_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void tumblepb_pf2_data_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fncywld_pf1_data_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fncywld_pf2_data_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void tumblepb_control_0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void pangpang_pf1_data_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void pangpang_pf2_data_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void tumbleb2_soundmcu_w(uint16_t data);

	TILEMAP_MAPPER_MEMBER(tumblep_scan);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_fncywld_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_fncywld_bg2_tile_info);
	TILE_GET_INFO_MEMBER(get_fncywld_fg_tile_info);
	TILE_GET_INFO_MEMBER(pangpang_get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(pangpang_get_bg2_tile_info);
	TILE_GET_INFO_MEMBER(pangpang_get_fg_tile_info);
	DECLARE_MACHINE_START(tumbleb);
	DECLARE_MACHINE_RESET(tumbleb);
	DECLARE_VIDEO_START(tumblepb);
	DECLARE_VIDEO_START(fncywld);
	DECLARE_MACHINE_RESET(htchctch);
	DECLARE_VIDEO_START(suprtrio);
	DECLARE_VIDEO_START(pangpang);
	DECLARE_VIDEO_START(sdfight);
	uint32_t screen_update_tumblepb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_jumpkids(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_fncywld(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_semicom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_suprtrio(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_pangpang(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_semicom_altoffsets(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_bcstory(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_semibase(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_sdfight(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(tumbleb2_interrupt);
	void tumbleb_tilemap_redraw();
	inline void get_bg_tile_info( tile_data &tileinfo, int tile_index, int gfx_bank, uint16_t *gfx_base);
	inline void get_fncywld_bg_tile_info( tile_data &tileinfo, int tile_index, int gfx_bank, uint16_t *gfx_base);
	inline void pangpang_get_bg_tile_info( tile_data &tileinfo, int tile_index, int gfx_bank, uint16_t *gfx_base );
	inline void pangpang_get_bg2x_tile_info( tile_data &tileinfo, int tile_index, int gfx_bank, uint16_t *gfx_base );
	void tumbleb_draw_common(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pf1x_offs, int pf1y_offs, int pf2x_offs, int pf2y_offs);
	void tumbleb2_set_music_bank( int bank );
	void tumbleb2_play_sound( okim6295_device *oki, int data );
	void tumbleb2_playmusic(okim6295_device *oki);
	void process_tumbleb2_music_command( okim6295_device *oki, int data );
	void tumblepb_gfx_rearrange(int rgn);
	void suprtrio_decrypt_code();
	void suprtrio_decrypt_gfx();

	void unico_base_map(address_map &map) ATTR_COLD;
	void fncywld_main_map(address_map &map) ATTR_COLD;
	void magipur_main_map(address_map &map) ATTR_COLD;
	void htchctch_main_map(address_map &map) ATTR_COLD;
	void jumpkids_main_map(address_map &map) ATTR_COLD;
	void jumpkids_sound_map(address_map &map) ATTR_COLD;
	void pangpang_main_map(address_map &map) ATTR_COLD;
	void semicom_sound_map(address_map &map) ATTR_COLD;
	void suprtrio_main_map(address_map &map) ATTR_COLD;
	void suprtrio_sound_map(address_map &map) ATTR_COLD;
	void tumblepopb_main_map(address_map &map) ATTR_COLD;
	void tumblepopba_main_map(address_map &map) ATTR_COLD;

	u8 m_suprtrio_prot_latch = 0;
};

class tumbleb_pic_state : public tumbleb_state
{
public:
	tumbleb_pic_state(const machine_config &mconfig, device_type type, const char *tag) :
		tumbleb_state(mconfig, type, tag),
		m_okibank(*this, "okibank")
	{ }

	void funkyjetb(machine_config &config);

protected:
	virtual void driver_start() override;

private:
	void oki_bank_w(uint8_t data);
	uint8_t pic_data_r();
	void pic_data_w(uint8_t data);
	void pic_ctrl_w(uint8_t data);

	void funkyjetb_map(address_map &map) ATTR_COLD;
	void funkyjetb_oki_map(address_map &map) ATTR_COLD;

	required_memory_bank m_okibank;

	uint8_t m_pic_data;
};

#endif // MAME_DATAEAST_TUMBLEB_H
