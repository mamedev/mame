class marineb_state : public driver_device
{
public:
	marineb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *   m_videoram;
	UINT8 *   m_colorram;
	UINT8 *   m_spriteram;

	/* video-related */
	tilemap_t   *m_bg_tilemap;
	tilemap_t   *m_fg_tilemap;
	UINT8     m_palette_bank;
	UINT8     m_column_scroll;
	UINT8     m_flipscreen_x;
	UINT8     m_flipscreen_y;
	UINT8     m_marineb_active_low_flipscreen;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;

	UINT8     m_irq_mask;
	DECLARE_WRITE8_MEMBER(irq_mask_w);
	DECLARE_WRITE8_MEMBER(marineb_videoram_w);
	DECLARE_WRITE8_MEMBER(marineb_colorram_w);
	DECLARE_WRITE8_MEMBER(marineb_column_scroll_w);
	DECLARE_WRITE8_MEMBER(marineb_palette_bank_0_w);
	DECLARE_WRITE8_MEMBER(marineb_palette_bank_1_w);
	DECLARE_WRITE8_MEMBER(marineb_flipscreen_x_w);
	DECLARE_WRITE8_MEMBER(marineb_flipscreen_y_w);
};


/*----------- defined in video/marineb.c -----------*/


PALETTE_INIT( marineb );
VIDEO_START( marineb );
SCREEN_UPDATE_IND16( marineb );
SCREEN_UPDATE_IND16( changes );
SCREEN_UPDATE_IND16( springer );
SCREEN_UPDATE_IND16( hoccer );
SCREEN_UPDATE_IND16( hopprobo );
