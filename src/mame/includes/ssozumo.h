// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
class ssozumo_state : public driver_device
{
public:
	ssozumo_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_videoram2(*this, "videoram2"),
		m_colorram2(*this, "colorram2") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_paletteram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_videoram2;
	required_shared_ptr<UINT8> m_colorram2;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	UINT8 m_sound_nmi_mask;

	DECLARE_WRITE8_MEMBER(sh_command_w);
	DECLARE_WRITE8_MEMBER(sound_nmi_mask_w);
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(colorram_w);
	DECLARE_WRITE8_MEMBER(videoram2_w);
	DECLARE_WRITE8_MEMBER(colorram2_w);
	DECLARE_WRITE8_MEMBER(paletteram_w);
	DECLARE_WRITE8_MEMBER(scroll_w);
	DECLARE_WRITE8_MEMBER(flipscreen_w);

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

	INTERRUPT_GEN_MEMBER(sound_timer_irq);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	virtual void machine_start() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(ssozumo);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};
