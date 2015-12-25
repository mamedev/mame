// license:???
// copyright-holders:Paul Leaman
/***************************************************************************

    Battle Lane Vol. 5

***************************************************************************/

class battlane_state : public driver_device
{
public:
	battlane_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_tileram(*this, "tileram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_tileram;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	bitmap_ind8 m_screen_bitmap;
	int         m_video_ctrl;
	int         m_cpu_control;  /* CPU interrupt control register */

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	DECLARE_WRITE8_MEMBER(battlane_cpu_command_w);
	DECLARE_WRITE8_MEMBER(battlane_palette_w);
	DECLARE_WRITE8_MEMBER(battlane_scrollx_w);
	DECLARE_WRITE8_MEMBER(battlane_scrolly_w);
	DECLARE_WRITE8_MEMBER(battlane_tileram_w);
	DECLARE_WRITE8_MEMBER(battlane_spriteram_w);
	DECLARE_WRITE8_MEMBER(battlane_bitmap_w);
	DECLARE_WRITE8_MEMBER(battlane_video_ctrl_w);
	TILE_GET_INFO_MEMBER(get_tile_info_bg);
	TILEMAP_MAPPER_MEMBER(battlane_tilemap_scan_rows_2x2);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_battlane(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(battlane_cpu1_interrupt);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_fg_bitmap( bitmap_ind16 &bitmap );
};
