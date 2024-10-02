// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni,Nicola Salmoria,Bryan McPhail,David Haywood,R. Belmont,Alex Marshall,Angelo Salese,Luca Elia
// thanks-to:Richard Bush
#ifndef MAME_NMK_NMK16_H
#define MAME_NMK_NMK16_H

#pragma once

#include "nmk004.h"
#include "nmk214.h"
#include "nmk16spr.h"

#include "seibusound.h"

#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


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
		m_spritegen(*this, "spritegen"),
		m_nmk004(*this, "nmk004"),
		m_soundlatch(*this, "soundlatch"),
		m_bgvideoram(*this, "bgvideoram%u", 0U),
		m_txvideoram(*this, "txvideoram"),
		m_mainram(*this, "mainram"),
		m_gunnail_scrollram(*this, "scrollram"),
		m_spriteram(*this, "spriteram"),
		m_gunnail_scrollramy(*this, "scrollramy"),
		m_tilemap_rom(*this, "tilerom"),
		m_audiobank(*this, "audiobank"),
		m_okibank(*this, "okibank%u", 1U),
		m_vtiming_prom(*this, "vtiming"),
		m_dsw_io(*this, "DSW%u", 1U),
		m_in_io(*this, "IN%u", 0U),
		m_sprdma_base(0x8000)
	{ }

	void vandyke(machine_config &config);
	void tdragon2(machine_config &config);
	void tharrier(machine_config &config);
	void raphero(machine_config &config);
	void tdragon(machine_config &config);
	void tdragonb(machine_config &config);
	void tdragonb2(machine_config &config);
	void tdragonb3(machine_config &config);
	void gunnail(machine_config &config);
	void gunnailb(machine_config &config);
	void hachamf(machine_config &config);
	void bjtwin(machine_config &config);
	void ssmissin(machine_config &config);
	void bioship(machine_config &config);
	void macross2(machine_config &config);
	void blkheart(machine_config &config);
	void manybloc(machine_config &config);
	void acrobatm(machine_config &config);
	void acrobatmbl(machine_config &config);
	void strahl(machine_config &config);
	void strahljbl(machine_config &config);
	void tdragon3h(machine_config &config);
	void macross(machine_config &config);
	void mustang(machine_config &config);
	void mustangb(machine_config &config);
	void mustangb3(machine_config &config);
	void twinactn(machine_config &config);
	void vandykeb(machine_config &config);
	void powerins(machine_config &config);
	void powerinsj(machine_config &config);
	void powerinspu(machine_config &config);
	void powerinspj(machine_config &config);
	void powerinsa(machine_config &config);
	void powerinsb(machine_config &config);
	void powerinsc(machine_config &config);

	void init_nmk();
	void init_tharrier();
	void init_vandykeb();
	void init_tdragonb();
	void init_tdragonb2();
	void init_ssmissin();
	void init_twinactn();
	void init_banked_audiocpu();
	void init_gunnailb();
	void init_bjtwin();
	void init_powerinsa();
	void init_acrobatmbl();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	TIMER_DEVICE_CALLBACK_MEMBER(nmk16_scanline);
	TIMER_DEVICE_CALLBACK_MEMBER(nmk16_hacky_scanline);
	u32 screen_update_macross(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void txvideoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void flipscreen_w(u8 data);
	void vandyke_flipscreen_w(u8 data);
	void tilebank_w(u8 data);

	void macross2_sound_reset_w(u16 data);
	void macross2_audiobank_w(u8 data);
	void ssmissin_okibank_w(u8 data);
	void powerinsa_okibank_w(u8 data);
	template<unsigned Chip> void tharrier_okibank_w(u8 data);
	u8 powerins_bootleg_fake_ym2203_r();

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device_array<okim6295_device, 2> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<nmk_16bit_sprite_device> m_spritegen;
	optional_device<nmk004_device> m_nmk004;
	optional_device<generic_latch_8_device> m_soundlatch;

	optional_shared_ptr_array<u16, 2> m_bgvideoram;
	optional_shared_ptr<u16> m_txvideoram;
	required_shared_ptr<u16> m_mainram;
	optional_shared_ptr<u16> m_gunnail_scrollram;
	optional_shared_ptr<u8> m_spriteram;
	optional_shared_ptr<u16> m_gunnail_scrollramy;

	optional_region_ptr<u16> m_tilemap_rom;
	optional_memory_bank m_audiobank;
	optional_memory_bank_array<2> m_okibank;
	optional_memory_region m_vtiming_prom;

	optional_ioport_array<2> m_dsw_io;
	optional_ioport_array<3> m_in_io;

	int m_tilerambank = 0;
	int m_sprdma_base = 0;
	int mask[4*2]{};
	std::unique_ptr<u16[]> m_spriteram_old;
	std::unique_ptr<u16[]> m_spriteram_old2;
	int m_bgbank = 0;
	int m_bioship_background_bank = 0;
	tilemap_t *m_bg_tilemap[2]{};
	tilemap_t *m_tx_tilemap = nullptr;
	int m_mustang_bg_xscroll = 0;
	u8 m_scroll[2][4]{};
	u16 m_vscroll[4]{};
	int m_prot_count = 0;
	u8 m_vtiming_val = 0;

	void mainram_strange_w(offs_t offset, u16 data/*, u16 mem_mask = ~0*/);
	u16 mainram_swapped_r(offs_t offset);
	void mainram_swapped_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void tharrier_mcu_control_w(u16 data);
	u16 tharrier_mcu_r(offs_t offset, u16 mem_mask = ~0);
	u16 vandykeb_r();
	u16 tdragonb_prot_r();
	template<unsigned Layer> void bgvideoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void mustang_scroll_w(u16 data);
	void raphero_scroll_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	template<unsigned Layer> void scroll_w(offs_t offset, u8 data);
	void bjtwin_scroll_w(offs_t offset, u8 data);
	void vandyke_scroll_w(offs_t offset, u16 data);
	void vandykeb_scroll_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void manybloc_scroll_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void bioship_bank_w(u8 data);
	void nmk004_x0016_w(u16 data);
	void nmk004_bioship_x0016_w(u16 data);

	void set_interrupt_timing(machine_config &config);
	void set_hacky_interrupt_timing(machine_config &config);
	void set_screen_lowres(machine_config &config);
	void set_screen_midres(machine_config &config);
	void set_screen_hires(machine_config &config);

	TILEMAP_MAPPER_MEMBER(tilemap_scan_pages);
	template<unsigned Layer, unsigned Gfx> TILE_GET_INFO_MEMBER(common_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(common_get_tx_tile_info);
	TILE_GET_INFO_MEMBER(bioship_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(bjtwin_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(powerins_get_bg_tile_info);
	DECLARE_VIDEO_START(macross);
	DECLARE_VIDEO_START(bioship);
	DECLARE_VIDEO_START(strahl);
	DECLARE_VIDEO_START(macross2);
	DECLARE_VIDEO_START(gunnail);
	DECLARE_VIDEO_START(bjtwin);
	DECLARE_VIDEO_START(powerins);
	void get_colour_4bit(u32 &colour, u32 &pri_mask);
	void get_colour_5bit(u32 &colour, u32 &pri_mask);
	void get_colour_6bit(u32 &colour, u32 &pri_mask);
	void get_sprite_flip(u16 attr, int &flipx, int &flipy, int &code);
	void get_flip_extcode_powerins(u16 attr, int &flipx, int &flipy, int &code);
	u32 screen_update_tharrier(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_strahl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_bjtwin(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank_powerins_bootleg(int state);
	void sprite_dma();
	TIMER_DEVICE_CALLBACK_MEMBER(manybloc_scanline);
	void video_init();
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, u16 *src);
	void bg_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer = 0);
	void tx_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u8 decode_byte(u8 src, const u8 *bitp);
	u32 bjtwin_address_map_bg0(u32 addr);
	u16 decode_word(u16 src, const u8 *bitp);
	u32 bjtwin_address_map_sprites(u32 addr);
	void decode_gfx();
	void decode_tdragonb();
	void decode_ssmissin();

	void acrobatm_map(address_map &map) ATTR_COLD;
	void acrobatmbl_map(address_map &map) ATTR_COLD;
	void bioship_map(address_map &map) ATTR_COLD;
	void bjtwin_map(address_map &map) ATTR_COLD;
	void gunnail_map(address_map &map) ATTR_COLD;
	void gunnailb_map(address_map &map) ATTR_COLD;
	void gunnailb_sound_map(address_map &map) ATTR_COLD;
	void gunnailb_sound_io_map(address_map &map) ATTR_COLD;
	void hachamf_map(address_map &map) ATTR_COLD;
	void macross2_map(address_map &map) ATTR_COLD;
	void macross2_sound_io_map(address_map &map) ATTR_COLD;
	void macross2_sound_map(address_map &map) ATTR_COLD;
	void macross_map(address_map &map) ATTR_COLD;
	void manybloc_map(address_map &map) ATTR_COLD;
	void mustang_map(address_map &map) ATTR_COLD;
	void mustangb_map(address_map &map) ATTR_COLD;
	void mustangb3_map(address_map &map) ATTR_COLD;
	void mustangb3_sound_map(address_map &map) ATTR_COLD;
	void oki1_map(address_map &map) ATTR_COLD;
	void oki2_map(address_map &map) ATTR_COLD;
	void powerins_map(address_map &map) ATTR_COLD;
	void powerins_sound_map(address_map &map) ATTR_COLD;
	void powerins_bootleg_audio_io_map(address_map &map) ATTR_COLD;
	void powerinsa_map(address_map &map) ATTR_COLD;
	void powerinsa_oki_map(address_map &map) ATTR_COLD;
	void raphero_map(address_map &map) ATTR_COLD;
	void raphero_sound_mem_map(address_map &map) ATTR_COLD;
	void ssmissin_map(address_map &map) ATTR_COLD;
	void ssmissin_sound_map(address_map &map) ATTR_COLD;
	void strahl_map(address_map &map) ATTR_COLD;
	void strahljbl_map(address_map &map) ATTR_COLD;
	void tdragon2_map(address_map &map) ATTR_COLD;
	void tdragon3h_map(address_map &map) ATTR_COLD;
	void tdragon3h_sound_io_map(address_map &map) ATTR_COLD;
	void tdragon_map(address_map &map) ATTR_COLD;
	void tdragonb_map(address_map &map) ATTR_COLD;
	void tdragonb2_map(address_map &map) ATTR_COLD;
	void tdragonb2_oki_map(address_map &map) ATTR_COLD;
	void tdragonb3_map(address_map &map) ATTR_COLD;
	void tharrier_map(address_map &map) ATTR_COLD;
	void tharrier_sound_io_map(address_map &map) ATTR_COLD;
	void tharrier_sound_map(address_map &map) ATTR_COLD;
	void twinactn_map(address_map &map) ATTR_COLD;
	void vandyke_map(address_map &map) ATTR_COLD;
	void vandykeb_map(address_map &map) ATTR_COLD;
};

class tdragon_prot_state : public nmk16_state
{
public:
	tdragon_prot_state(const machine_config &mconfig, device_type type, const char *tag) :
		nmk16_state(mconfig, type, tag),
		m_protcpu(*this, "protcpu")
	{
	}

	void tdragon_prot(machine_config &config);
	void hachamf_prot(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	optional_device<tlcs90_device> m_protcpu;

	void tdragon_prot_map(address_map &map) ATTR_COLD;

	void mcu_side_shared_w(offs_t offset, u8 data);
	u8 mcu_side_shared_r(offs_t offset);
	void mcu_port6_w(u8 data);
	u8 mcu_port5_r();
	u8 mcu_port6_r();

	u8 m_bus_status;
};

class macross_prot_state : public tdragon_prot_state
{
public:
	macross_prot_state(const machine_config &mconfig, device_type type, const char *tag) :
		tdragon_prot_state(mconfig, type, tag),
		m_nmk214(*this, "nmk214_%u", 0U),
		m_init_data_nmk214(0),
		m_init_clock_nmk214(0),
		m_gfx_unscramble_enabled(false),
		m_gfx_decoded(false)
	{
	}

	void bjtwin_prot(machine_config &config);
	void gunnail_prot(machine_config &config);
	void macross_prot(machine_config &config);

protected:
	virtual void device_post_load() override;
	virtual void machine_start() override ATTR_COLD;

private:
	void base_nmk214_215(machine_config &config);

	void decode_nmk214();

	void mcu_port3_to_214_w(u8 data);
	void mcu_port7_to_214_w(u8 data);

	required_device_array<nmk214_device, 2> m_nmk214;

	u8 m_init_data_nmk214;
	u8 m_init_clock_nmk214;
	bool m_gfx_unscramble_enabled;
	bool m_gfx_decoded; // excluded from save states
};

class afega_state : public nmk16_state
{
public:
	afega_state(const machine_config &mconfig, device_type type, const char *tag) :
		nmk16_state(mconfig, type, tag),
		m_afega_scroll(*this, "afega_scroll_%u", 0U)
	{}

	void firehawk(machine_config &config);
	void grdnstrm(machine_config &config);
	void grdnstrmk(machine_config &config);
	void popspops(machine_config &config);
	void redhawki(machine_config &config);
	void redhawkb(machine_config &config);
	void stagger1(machine_config &config);
	void spec2k(machine_config &config);

	void init_bubl2000();
	void init_grdnstrm();
	void init_grdnstrmau();
	void init_redfoxwp2a();
	void init_grdnstrmg();
	void init_redhawk();
	void init_redhawkg();
	void init_redhawki();
	void init_redhawksa();
	void init_spec2k();

private:
	optional_shared_ptr_array<u16, 2> m_afega_scroll;

	u16 afega_unknown_r();
	void spec2k_oki1_banking_w(u8 data);
	template<unsigned Scroll> void afega_scroll_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	TILE_GET_INFO_MEMBER(get_bg_tile_info_8bit);
	DECLARE_VIDEO_START(grdnstrm);
	u32 screen_update_afega(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_firehawk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_redhawki(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_redhawkb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_bubl2000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void video_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int dsw_flipscreen, int xoffset, int yoffset, int attr_mask);
	void redhawki_video_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void afega_map(address_map &map) ATTR_COLD;
	void afega_sound_cpu(address_map &map) ATTR_COLD;
	void firehawk_map(address_map &map) ATTR_COLD;
	void firehawk_sound_cpu(address_map &map) ATTR_COLD;
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
	void tomagic_map(address_map &map) ATTR_COLD;
	void tomagic_sound_map(address_map &map) ATTR_COLD;
	void tomagic_sound_io_map(address_map &map) ATTR_COLD;
};

#endif //MAME_NMK_NMK16_H
