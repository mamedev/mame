#include "sound/msm5205.h"

class tbowl_state : public driver_device
{
public:
	tbowl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_shared_ram(*this, "shared_ram"),
		m_txvideoram(*this, "txvideoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_bg2videoram(*this, "bg2videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm1(*this, "msm1"),
		m_msm2(*this, "msm2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	int m_adpcm_pos[2];
	int m_adpcm_end[2];
	int m_adpcm_data[2];
	required_shared_ptr<UINT8> m_shared_ram;
	required_shared_ptr<UINT8> m_txvideoram;
	required_shared_ptr<UINT8> m_bgvideoram;
	required_shared_ptr<UINT8> m_bg2videoram;
	required_shared_ptr<UINT8> m_spriteram;
	tilemap_t *m_tx_tilemap;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_bg2_tilemap;
	UINT16 m_xscroll;
	UINT16 m_yscroll;
	UINT16 m_bg2xscroll;
	UINT16 m_bg2yscroll;
	DECLARE_WRITE8_MEMBER(tbowl_coin_counter_w);
	DECLARE_WRITE8_MEMBER(tbowlb_bankswitch_w);
	DECLARE_WRITE8_MEMBER(tbowlc_bankswitch_w);
	DECLARE_READ8_MEMBER(shared_r);
	DECLARE_WRITE8_MEMBER(shared_w);
	DECLARE_WRITE8_MEMBER(tbowl_sound_command_w);
	DECLARE_WRITE8_MEMBER(tbowl_trigger_nmi);
	DECLARE_WRITE8_MEMBER(tbowl_adpcm_start_w);
	DECLARE_WRITE8_MEMBER(tbowl_adpcm_end_w);
	DECLARE_WRITE8_MEMBER(tbowl_adpcm_vol_w);
	DECLARE_WRITE8_MEMBER(tbowl_txvideoram_w);
	DECLARE_WRITE8_MEMBER(tbowl_bg2videoram_w);
	DECLARE_WRITE8_MEMBER(tbowl_bgxscroll_lo);
	DECLARE_WRITE8_MEMBER(tbowl_bgxscroll_hi);
	DECLARE_WRITE8_MEMBER(tbowl_bgyscroll_lo);
	DECLARE_WRITE8_MEMBER(tbowl_bgyscroll_hi);
	DECLARE_WRITE8_MEMBER(tbowl_bgvideoram_w);
	DECLARE_WRITE8_MEMBER(tbowl_bg2xscroll_lo);
	DECLARE_WRITE8_MEMBER(tbowl_bg2xscroll_hi);
	DECLARE_WRITE8_MEMBER(tbowl_bg2yscroll_lo);
	DECLARE_WRITE8_MEMBER(tbowl_bg2yscroll_hi);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_tbowl_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_tbowl_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void tbowl_draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect, int xscroll, UINT8* spriteram);
	DECLARE_WRITE_LINE_MEMBER(irqhandler);
	void tbowl_adpcm_int(msm5205_device *device, int chip);
	DECLARE_WRITE_LINE_MEMBER(tbowl_adpcm_int_1);
	DECLARE_WRITE_LINE_MEMBER(tbowl_adpcm_int_2);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm1;
	required_device<msm5205_device> m_msm2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
