// license:BSD-3-Clause
// copyright-holders:Lee Taylor, Chris Moore
/*************************************************************************

    Cheeky Mouse

*************************************************************************/


class cheekyms_state : public driver_device
{
public:
	cheekyms_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dac(*this, "dac"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_port_80(*this, "port_80") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<dac_device> m_dac;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_port_80;

	/* video-related */
	tilemap_t        *m_cm_tilemap;
	std::unique_ptr<bitmap_ind16>       m_bitmap_buffer;

	UINT8          m_irq_mask;

	DECLARE_WRITE8_MEMBER(port_40_w);
	DECLARE_WRITE8_MEMBER(port_80_w);

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	INTERRUPT_GEN_MEMBER(vblank_irq);

	TILE_GET_INFO_MEMBER(get_tile_info);

	virtual void machine_start() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(cheekyms);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx, int flip );
};
