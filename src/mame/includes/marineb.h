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
};


/*----------- defined in video/marineb.c -----------*/

WRITE8_HANDLER( marineb_videoram_w );
WRITE8_HANDLER( marineb_colorram_w );
WRITE8_HANDLER( marineb_column_scroll_w );
WRITE8_HANDLER( marineb_palette_bank_0_w );
WRITE8_HANDLER( marineb_palette_bank_1_w );
WRITE8_HANDLER( marineb_flipscreen_x_w );
WRITE8_HANDLER( marineb_flipscreen_y_w );

PALETTE_INIT( marineb );
VIDEO_START( marineb );
SCREEN_UPDATE( marineb );
SCREEN_UPDATE( changes );
SCREEN_UPDATE( springer );
SCREEN_UPDATE( hoccer );
SCREEN_UPDATE( hopprobo );
