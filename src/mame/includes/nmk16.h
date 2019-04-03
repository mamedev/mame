// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni,Nicola Salmoria,Bryan McPhail,David Haywood,R. Belmont,Alex Marshall,Angelo Salese,Luca Elia
// thanks-to:Richard Bush
#ifndef MAME_INCLUDES_NMK16_H
#define MAME_INCLUDES_NMK16_H

#pragma once

#include "audio/seibu.h"
#include "machine/gen_latch.h"
#include "machine/nmk004.h"
#include "machine/timer.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "screen.h"

class nmk16_state : public driver_device, public seibu_sound_common
{
public:
	nmk16_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki%u", 1U),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_nmk004(*this, "nmk004"),
		m_soundlatch(*this, "soundlatch"),
		m_nmk_bgvideoram(*this, "nmk_bgvideoram%u", 0U),
		m_nmk_txvideoram(*this, "nmk_txvideoram"),
		m_mainram(*this, "mainram"),
		m_gunnail_scrollram(*this, "scrollram"),
		m_spriteram(*this, "spriteram"),
		m_gunnail_scrollramy(*this, "scrollramy"),
		m_afega_scroll(*this, "afega_scroll_%u", 0U),
		m_tilemap_rom(*this, "tilerom"),
		m_audiobank(*this, "audiobank"),
		m_okibank(*this, "okibank%u", 1U),
		m_dsw_io(*this, "DSW%u", 1U),
		m_in_io(*this, "IN%u", 0U),
		m_sprdma_base(0x8000)
	{ }

	void vandyke(machine_config &config);
	void redhawkb(machine_config &config);
	void grdnstrm(machine_config &config);
	void tdragon_prot(machine_config &config);
	void tdragon2(machine_config &config);
	void tharrier(machine_config &config);
	void raphero(machine_config &config);
	void tdragon(machine_config &config);
	void tdragonb(machine_config &config);
	void twinactn(machine_config &config);
	void firehawk(machine_config &config);
	void gunnail(machine_config &config);
	void hachamf(machine_config &config);
	void redhawki(machine_config &config);
	void bjtwin(machine_config &config);
	void ssmissin(machine_config &config);
	void bioship(machine_config &config);
	void spec2k(machine_config &config);
	void macross2(machine_config &config);
	void blkheart(machine_config &config);
	void stagger1(machine_config &config);
	void manybloc(machine_config &config);
	void acrobatm(machine_config &config);
	void strahl(machine_config &config);
	void tdragon3h(machine_config &config);
	void atombjt(machine_config &config);
	void hachamf_prot(machine_config &config);
	void popspops(machine_config &config);
	void grdnstrmk(machine_config &config);
	void macross(machine_config &config);
	void mustangb(machine_config &config);
	void mustang(machine_config &config);
	void vandykeb(machine_config &config);

	void init_nmk();
	void init_tharrier();
	void init_vandykeb();
	void init_tdragonb();
	void init_ssmissin();
	void init_hachamf_prot();
	void init_redhawk();
	void init_redhawkg();
	void init_redhawki();
	void init_tdragon_prot();
	void init_bubl2000();
	void init_banked_audiocpu();
	void init_grdnstrm();
	void init_grdnstrmau();
	void init_spec2k();
	void init_redfoxwp2a();
	void init_grdnstrmg();
	void init_bjtwin();
	void init_atombjt();

	DECLARE_VIDEO_START(gunnail);
	TIMER_DEVICE_CALLBACK_MEMBER(nmk16_scanline);
	uint32_t screen_update_macross(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE16_MEMBER(nmk_txvideoram_w);
	DECLARE_WRITE8_MEMBER(nmk_flipscreen_w);
	DECLARE_WRITE8_MEMBER(nmk_tilebank_w);

	DECLARE_WRITE8_MEMBER(macross2_sound_bank_w);

protected:
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device_array<okim6295_device, 2> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<nmk004_device> m_nmk004;
	optional_device<generic_latch_8_device> m_soundlatch;

	optional_shared_ptr_array<uint16_t, 2> m_nmk_bgvideoram;
	optional_shared_ptr<uint16_t> m_nmk_txvideoram;
	required_shared_ptr<uint16_t> m_mainram;
	optional_shared_ptr<uint16_t> m_gunnail_scrollram;
	optional_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint16_t> m_gunnail_scrollramy;
	optional_shared_ptr_array<uint16_t, 2> m_afega_scroll;

	optional_region_ptr<uint16_t> m_tilemap_rom;
	optional_memory_bank m_audiobank;
	optional_memory_bank_array<2> m_okibank;

	optional_ioport_array<2> m_dsw_io;
	optional_ioport_array<3> m_in_io;

	int m_sprclk;
	int m_sprlimit;
	int m_tilerambank;
	int m_sprdma_base;
	int mask[4*2];
	std::unique_ptr<uint16_t[]> m_spriteram_old;
	std::unique_ptr<uint16_t[]> m_spriteram_old2;
	int m_bgbank;
	int m_videoshift;
	int m_bioship_background_bank;
	tilemap_t *m_bg_tilemap[2];
	tilemap_t *m_tx_tilemap;
	int m_mustang_bg_xscroll;
	uint8_t m_scroll[2][4];
	uint16_t m_vscroll[4];
	int m_prot_count;
	uint8_t m_input_pressed;
	uint8_t m_start_helper;
	uint8_t m_coin_count[2];
	uint8_t m_coin_count_frac[2];
	DECLARE_WRITE16_MEMBER(nmk16_mainram_strange_w);
	DECLARE_WRITE8_MEMBER(ssmissin_soundbank_w);
	DECLARE_WRITE16_MEMBER(tharrier_mcu_control_w);
	DECLARE_READ16_MEMBER(tharrier_mcu_r);
	DECLARE_WRITE16_MEMBER(macross2_sound_reset_w);
	template<int Chip> DECLARE_WRITE8_MEMBER(tharrier_oki6295_bankswitch_w);
	DECLARE_WRITE16_MEMBER(hachamf_mainram_w);
	DECLARE_WRITE16_MEMBER(tdragon_mainram_w);
	DECLARE_READ16_MEMBER(vandykeb_r);
	DECLARE_READ16_MEMBER(tdragonb_prot_r);
	DECLARE_READ16_MEMBER(afega_unknown_r);
	template<int Scroll> DECLARE_WRITE16_MEMBER(afega_scroll_w);
	template<int Layer> DECLARE_WRITE16_MEMBER(nmk_bgvideoram_w);
	DECLARE_WRITE16_MEMBER(mustang_scroll_w);
	DECLARE_WRITE16_MEMBER(raphero_scroll_w);
	template<int Layer> DECLARE_WRITE8_MEMBER(nmk_scroll_w);
	DECLARE_WRITE16_MEMBER(vandyke_scroll_w);
	DECLARE_WRITE16_MEMBER(vandykeb_scroll_w);
	DECLARE_WRITE16_MEMBER(manybloc_scroll_w);
	DECLARE_WRITE8_MEMBER(bioship_bank_w);
	DECLARE_WRITE8_MEMBER(spec2k_oki1_banking_w);
	DECLARE_WRITE8_MEMBER(twinactn_oki_bank_w);
	DECLARE_WRITE16_MEMBER(nmk16_x0016_w);
	DECLARE_WRITE16_MEMBER(nmk16_bioship_x0016_w);
	void save_protregs();

	void set_hacky_interrupt_timing(machine_config &config);
	void set_hacky_screen_lowres(machine_config &config);
	void set_hacky_screen_hires(machine_config &config);

	TILEMAP_MAPPER_MEMBER(tilemap_scan_pages);
	template<int Layer, int Gfx> TILE_GET_INFO_MEMBER(common_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(common_get_tx_tile_info);
	TILE_GET_INFO_MEMBER(bioship_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(bjtwin_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_tile_info_0_8bit);
	DECLARE_VIDEO_START(macross);
	DECLARE_VIDEO_START(bioship);
	DECLARE_VIDEO_START(strahl);
	DECLARE_VIDEO_START(macross2);
	DECLARE_VIDEO_START(bjtwin);
	DECLARE_VIDEO_START(afega);
	DECLARE_VIDEO_START(grdnstrm);
	uint32_t screen_update_tharrier(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_manybloc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_strahl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_tdragon2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_bjtwin(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_afega(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_firehawk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_redhawki(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_redhawkb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_bubl2000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(tdragon_mcu_sim);
	TIMER_DEVICE_CALLBACK_MEMBER(hachamf_mcu_sim);
	TIMER_DEVICE_CALLBACK_MEMBER(manybloc_scanline);
	void nmk16_video_init();
	inline void nmk16_draw_sprite(bitmap_ind16 &bitmap, const rectangle &cliprect, uint16_t *spr);
	inline void nmk16_draw_sprite_flipsupported(bitmap_ind16 &bitmap, const rectangle &cliprect, uint16_t *spr);
	void nmk16_draw_sprites_swap(bitmap_ind16 &bitmap, const rectangle &cliprect, int *bittbl);
	void nmk16_draw_sprites_swap_flipsupported(bitmap_ind16 &bitmap, const rectangle &cliprect, int *bittbl);
	void nmk16_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void nmk16_draw_sprites_flipsupported(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void nmk16_bg_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer = 0);
	void nmk16_tx_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void video_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect,int dsw_flipscreen,int xoffset, int yoffset,int attr_mask);
	void redhawki_video_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void mcu_run(uint8_t dsw_setting);
	uint8_t decode_byte(uint8_t src, const uint8_t *bitp);
	uint32_t bjtwin_address_map_bg0(uint32_t addr);
	uint16_t decode_word(uint16_t src, const uint8_t *bitp);
	uint32_t bjtwin_address_map_sprites(uint32_t addr);
	void decode_gfx();
	void decode_tdragonb();
	void decode_ssmissin();

	void acrobatm_map(address_map &map);
	void afega_map(address_map &map);
	void afega_sound_cpu(address_map &map);
	void atombjt_map(address_map &map);
	void bioship_map(address_map &map);
	void bjtwin_map(address_map &map);
	void firehawk_map(address_map &map);
	void firehawk_sound_cpu(address_map &map);
	void gunnail_map(address_map &map);
	void hachamf_map(address_map &map);
	void macross2_map(address_map &map);
	void macross2_sound_io_map(address_map &map);
	void macross2_sound_map(address_map &map);
	void macross_map(address_map &map);
	void manybloc_map(address_map &map);
	void mustang_map(address_map &map);
	void mustangb_map(address_map &map);
	void oki1_map(address_map &map);
	void oki2_map(address_map &map);
	void raphero_map(address_map &map);
	void raphero_sound_mem_map(address_map &map);
	void ssmissin_map(address_map &map);
	void ssmissin_sound_map(address_map &map);
	void strahl_map(address_map &map);
	void tdragon3h_map(address_map &map);
	void tdragon_map(address_map &map);
	void tdragonb_map(address_map &map);
	void tharrier_map(address_map &map);
	void tharrier_sound_io_map(address_map &map);
	void tharrier_sound_map(address_map &map);
	void twinactn_map(address_map &map);
	void twinactn_sound_cpu(address_map &map);
	void vandyke_map(address_map &map);
	void vandykeb_map(address_map &map);
};

class nmk16_tomagic_state : public nmk16_state
{
public:
	nmk16_tomagic_state(const machine_config &mconfig, device_type type, const char *tag) :
		nmk16_state(mconfig, type, tag)
	{}

	void tomagic(machine_config &config);
	void init_tomagic();

private:
	void tomagic_map(address_map &map);
	void tomagic_sound_map(address_map &map);
	void tomagic_sound_io_map(address_map &map);
};

#endif //MAME_INCLUDES_NMK16_H
