// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
/***************************************************************************

  Capcom Vulgus hardware

***************************************************************************/

class vulgus_state : public driver_device
{
public:
	vulgus_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_scroll_low(*this, "scroll_low"),
		m_scroll_high(*this, "scroll_high"),
		m_spriteram(*this, "spriteram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_scroll_low;
	required_shared_ptr<UINT8> m_scroll_high;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_fgvideoram;
	required_shared_ptr<UINT8> m_bgvideoram;

	int m_palette_bank;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;

	DECLARE_WRITE8_MEMBER(fgvideoram_w);
	DECLARE_WRITE8_MEMBER(bgvideoram_w);
	DECLARE_WRITE8_MEMBER(c804_w);
	DECLARE_WRITE8_MEMBER(palette_bank_w);

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	virtual void video_start() override;
	DECLARE_PALETTE_INIT(vulgus);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(vblank_irq);
};
