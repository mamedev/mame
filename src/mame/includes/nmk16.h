// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni,Nicola Salmoria,Bryan McPhail,David Haywood,R. Belmont,Alex Marshall,Angelo Salese,Luca Elia
// thanks-to:Richard Bush

#include "machine/nmk112.h"
#include "sound/okim6295.h"
#include "audio/seibu.h"
#include "machine/nmk004.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"

class nmk16_state : public driver_device, protected seibu_sound_common
{
public:
	nmk16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki1(*this, "oki1"),
		m_oki2(*this, "oki2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_nmk004(*this, "nmk004"),
		m_soundlatch(*this, "soundlatch"),
		m_nmk_bgvideoram0(*this, "nmk_bgvideoram0"),
		m_nmk_txvideoram(*this, "nmk_txvideoram"),
		m_mainram(*this, "mainram"),
		m_gunnail_scrollram(*this, "scrollram"),
		m_spriteram(*this, "spriteram"),
		m_nmk_fgvideoram(*this, "nmk_fgvideoram"),
		m_gunnail_scrollramy(*this, "scrollramy"),
		m_nmk_bgvideoram1(*this, "nmk_bgvideoram1"),
		m_nmk_bgvideoram2(*this, "nmk_bgvideoram2"),
		m_nmk_bgvideoram3(*this, "nmk_bgvideoram3"),
		m_afega_scroll_0(*this, "afega_scroll_0"),
		m_afega_scroll_1(*this, "afega_scroll_1"),
		m_sprdma_base(0x8000)
	{}

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<okim6295_device> m_oki1;
	optional_device<okim6295_device> m_oki2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<nmk004_device> m_nmk004;
	optional_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint16_t> m_nmk_bgvideoram0;
	optional_shared_ptr<uint16_t> m_nmk_txvideoram;
	required_shared_ptr<uint16_t> m_mainram;
	optional_shared_ptr<uint16_t> m_gunnail_scrollram;
	optional_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint16_t> m_nmk_fgvideoram;
	optional_shared_ptr<uint16_t> m_gunnail_scrollramy;
	optional_shared_ptr<uint16_t> m_nmk_bgvideoram1;
	optional_shared_ptr<uint16_t> m_nmk_bgvideoram2;
	optional_shared_ptr<uint16_t> m_nmk_bgvideoram3;
	optional_shared_ptr<uint16_t> m_afega_scroll_0;
	optional_shared_ptr<uint16_t> m_afega_scroll_1;


	int m_sprdma_base;
	int mask[4*2];
	int m_simple_scroll;
	int m_redraw_bitmap;
	std::unique_ptr<uint16_t[]> m_spriteram_old;
	std::unique_ptr<uint16_t[]> m_spriteram_old2;
	int m_bgbank;
	int m_videoshift;
	int m_bioship_background_bank;
	uint8_t m_bioship_scroll[4];
	tilemap_t *m_bg_tilemap0;
	tilemap_t *m_bg_tilemap1;
	tilemap_t *m_bg_tilemap2;
	tilemap_t *m_bg_tilemap3;
	tilemap_t *m_tx_tilemap;
	tilemap_t *m_fg_tilemap;
	std::unique_ptr<bitmap_ind16> m_background_bitmap;
	int m_mustang_bg_xscroll;
	uint8_t m_scroll[4];
	uint8_t m_scroll_2[4];
	uint16_t m_vscroll[4];
	int m_prot_count;
	uint8_t m_input_pressed;
	uint8_t m_start_helper;
	uint8_t m_coin_count[2];
	uint8_t m_coin_count_frac[2];
	DECLARE_WRITE16_MEMBER(nmk16_mainram_strange_w);
	DECLARE_WRITE16_MEMBER(ssmissin_sound_w);
	DECLARE_WRITE8_MEMBER(ssmissin_soundbank_w);
	DECLARE_WRITE16_MEMBER(tharrier_mcu_control_w);
	DECLARE_READ16_MEMBER(tharrier_mcu_r);
	DECLARE_WRITE16_MEMBER(macross2_sound_reset_w);
	DECLARE_WRITE16_MEMBER(macross2_sound_command_w);
	DECLARE_WRITE8_MEMBER(macross2_sound_bank_w);
	DECLARE_WRITE8_MEMBER(tharrier_oki6295_bankswitch_0_w);
	DECLARE_WRITE8_MEMBER(tharrier_oki6295_bankswitch_1_w);
	DECLARE_WRITE16_MEMBER(afega_soundlatch_w);
	DECLARE_WRITE16_MEMBER(hachamf_mainram_w);
	DECLARE_WRITE16_MEMBER(tdragon_mainram_w);
	DECLARE_READ16_MEMBER(vandykeb_r);
	DECLARE_READ16_MEMBER(tdragonb_prot_r);
	DECLARE_READ16_MEMBER(afega_unknown_r);
	DECLARE_WRITE16_MEMBER(afega_scroll0_w);
	DECLARE_WRITE16_MEMBER(afega_scroll1_w);
	DECLARE_WRITE16_MEMBER(nmk_bgvideoram0_w);
	DECLARE_WRITE16_MEMBER(nmk_bgvideoram1_w);
	DECLARE_WRITE16_MEMBER(nmk_bgvideoram2_w);
	DECLARE_WRITE16_MEMBER(nmk_bgvideoram3_w);
	DECLARE_WRITE16_MEMBER(nmk_fgvideoram_w);
	DECLARE_WRITE16_MEMBER(nmk_txvideoram_w);
	DECLARE_WRITE16_MEMBER(mustang_scroll_w);
	DECLARE_WRITE16_MEMBER(bioshipbg_scroll_w);
	DECLARE_WRITE16_MEMBER(nmk_scroll_w);
	DECLARE_WRITE16_MEMBER(nmk_scroll_2_w);
	DECLARE_WRITE16_MEMBER(vandyke_scroll_w);
	DECLARE_WRITE16_MEMBER(vandykeb_scroll_w);
	DECLARE_WRITE16_MEMBER(manybloc_scroll_w);
	DECLARE_WRITE16_MEMBER(nmk_flipscreen_w);
	DECLARE_WRITE16_MEMBER(nmk_tilebank_w);
	DECLARE_WRITE16_MEMBER(bioship_scroll_w);
	DECLARE_WRITE16_MEMBER(bioship_bank_w);
	DECLARE_WRITE8_MEMBER(spec2k_oki1_banking_w);
	DECLARE_WRITE8_MEMBER(twinactn_oki_bank_w);
	DECLARE_READ16_MEMBER(atombjt_unkr_r) {return 0x0000;}
	DECLARE_WRITE16_MEMBER(nmk16_x0016_w);
	DECLARE_WRITE16_MEMBER(nmk16_bioship_x0016_w);
	DECLARE_DRIVER_INIT(nmk);
	DECLARE_DRIVER_INIT(tharrier);
	DECLARE_DRIVER_INIT(vandykeb);
	DECLARE_DRIVER_INIT(tdragonb);
	DECLARE_DRIVER_INIT(ssmissin);
	DECLARE_DRIVER_INIT(hachamf_prot);
	DECLARE_DRIVER_INIT(redhawk);
	DECLARE_DRIVER_INIT(tdragon_prot);
	DECLARE_DRIVER_INIT(bubl2000);
	DECLARE_DRIVER_INIT(banked_audiocpu);
	DECLARE_DRIVER_INIT(grdnstrm);
	DECLARE_DRIVER_INIT(spec2k);
	DECLARE_DRIVER_INIT(redfoxwp2a);
	DECLARE_DRIVER_INIT(grdnstrmg);
	DECLARE_DRIVER_INIT(bjtwin);
	TILEMAP_MAPPER_MEMBER(afega_tilemap_scan_pages);
	TILE_GET_INFO_MEMBER(macross_get_bg0_tile_info);
	TILE_GET_INFO_MEMBER(macross_get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(macross_get_bg2_tile_info);
	TILE_GET_INFO_MEMBER(macross_get_bg3_tile_info);
	TILE_GET_INFO_MEMBER(strahl_get_fg_tile_info);
	TILE_GET_INFO_MEMBER(macross_get_tx_tile_info);
	TILE_GET_INFO_MEMBER(bjtwin_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_tile_info_0_8bit);
	DECLARE_VIDEO_START(macross);
	DECLARE_VIDEO_START(bioship);
	DECLARE_VIDEO_START(strahl);
	DECLARE_VIDEO_START(gunnail);
	DECLARE_VIDEO_START(macross2);
	DECLARE_VIDEO_START(raphero);
	DECLARE_VIDEO_START(bjtwin);
	DECLARE_VIDEO_START(afega);
	DECLARE_VIDEO_START(firehawk);
	DECLARE_VIDEO_START(grdnstrm);
	uint32_t screen_update_tharrier(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_manybloc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_macross(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_bioship(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_strahl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_gunnail(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_tdragon2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_bjtwin(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_afega(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_firehawk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_redhawki(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_redhawkb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_bubl2000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(tdragon_mcu_sim);
	TIMER_DEVICE_CALLBACK_MEMBER(hachamf_mcu_sim);
	TIMER_DEVICE_CALLBACK_MEMBER(nmk16_scanline);
	TIMER_DEVICE_CALLBACK_MEMBER(manybloc_scanline);
	void nmk16_video_init();
	inline void nmk16_draw_sprite(bitmap_ind16 &bitmap, const rectangle &cliprect, uint16_t *spr);
	inline void nmk16_draw_sprite_flipsupported(bitmap_ind16 &bitmap, const rectangle &cliprect, uint16_t *spr);
	void nmk16_draw_sprites_swap(bitmap_ind16 &bitmap, const rectangle &cliprect, int *bittbl);
	void nmk16_draw_sprites_swap_flipsupported(bitmap_ind16 &bitmap, const rectangle &cliprect, int *bittbl);
	void nmk16_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void nmk16_draw_sprites_flipsupported(bitmap_ind16 &bitmap, const rectangle &cliprect);
	int nmk16_bg_spr_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	int nmk16_bg_fg_spr_tx_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	int nmk16_bg_spr_tx_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	int nmk16_bg_sprflip_tx_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	int nmk16_bioshipbg_sprflip_tx_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	int nmk16_bg_sprswap_tx_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int bittbl[8]);
	int nmk16_bg_sprswapflip_tx_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int bittbl[8]);
	int nmk16_complexbg_sprswap_tx_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int bittbl[8]);
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
