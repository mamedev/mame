// license:BSD-3-Clause
// copyright-holders:David Haywood, ???
class scotrsht_state : public driver_device
{
public:
	scotrsht_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_scroll(*this, "scroll")  { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_scroll;

	tilemap_t *m_bg_tilemap;

	int m_irq_enable;
	int m_charbank;
	int m_palette_bank;

	DECLARE_WRITE8_MEMBER(ctrl_w);
	DECLARE_WRITE8_MEMBER(soundlatch_w);
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(colorram_w);
	DECLARE_WRITE8_MEMBER(charbank_w);
	DECLARE_WRITE8_MEMBER(palettebank_w);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	INTERRUPT_GEN_MEMBER(interrupt);

	virtual void video_start() override;
	DECLARE_PALETTE_INIT(scotrsht);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
};
