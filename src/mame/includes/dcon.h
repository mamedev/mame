// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
class dcon_state : public driver_device
{
public:
	dcon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_back_data(*this, "back_data"),
		m_fore_data(*this, "fore_data"),
		m_mid_data(*this, "mid_data"),
		m_textram(*this, "textram"),
		m_spriteram(*this, "spriteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT16> m_back_data;
	required_shared_ptr<UINT16> m_fore_data;
	required_shared_ptr<UINT16> m_mid_data;
	required_shared_ptr<UINT16> m_textram;
	required_shared_ptr<UINT16> m_spriteram;

	tilemap_t *m_background_layer;
	tilemap_t *m_foreground_layer;
	tilemap_t *m_midground_layer;
	tilemap_t *m_text_layer;

	int m_gfx_bank_select;
	int m_last_gfx_bank;
	UINT16 m_scroll_ram[6];
	UINT16 m_layer_en;

	DECLARE_WRITE16_MEMBER(layer_en_w);
	DECLARE_WRITE16_MEMBER(layer_scroll_w);
	DECLARE_WRITE16_MEMBER(gfxbank_w);
	DECLARE_WRITE16_MEMBER(background_w);
	DECLARE_WRITE16_MEMBER(foreground_w);
	DECLARE_WRITE16_MEMBER(midground_w);
	DECLARE_WRITE16_MEMBER(text_w);

	TILE_GET_INFO_MEMBER(get_back_tile_info);
	TILE_GET_INFO_MEMBER(get_fore_tile_info);
	TILE_GET_INFO_MEMBER(get_mid_tile_info);
	TILE_GET_INFO_MEMBER(get_text_tile_info);

	DECLARE_DRIVER_INIT(sdgndmps);
	virtual void video_start() override;

	UINT32 screen_update_dcon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_sdgndmps(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect);
};
