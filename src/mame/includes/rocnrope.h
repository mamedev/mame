// license:BSD-3-Clause
// copyright-holders:Chris Hardy
class rocnrope_state : public driver_device
{
public:
	rocnrope_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram2(*this, "spriteram2"),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram")
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	// memory pointers
	required_shared_ptr<UINT8> m_spriteram2;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_videoram;

	tilemap_t *m_bg_tilemap;
	UINT8 m_irq_mask;

	DECLARE_WRITE8_MEMBER(rocnrope_interrupt_vector_w);
	DECLARE_WRITE8_MEMBER(irq_mask_w);
	DECLARE_WRITE8_MEMBER(rocnrope_videoram_w);
	DECLARE_WRITE8_MEMBER(rocnrope_colorram_w);
	DECLARE_WRITE8_MEMBER(rocnrope_flipscreen_w);
	DECLARE_DRIVER_INIT(rocnrope);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start();
	DECLARE_PALETTE_INIT(rocnrope);
	UINT32 screen_update_rocnrope(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
};
