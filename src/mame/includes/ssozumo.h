class ssozumo_state : public driver_device
{
public:
	ssozumo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	size_t m_spriteram_size;
	UINT8 *m_spriteram;
	UINT8 *m_paletteram;
	UINT8 *m_videoram;
	UINT8 *m_colorram;
	UINT8 *m_videoram2;
	UINT8 *m_colorram2;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;

	UINT8 m_sound_nmi_mask;
};


/*----------- defined in video/ssozumo.c -----------*/

WRITE8_HANDLER( ssozumo_videoram_w );
WRITE8_HANDLER( ssozumo_colorram_w );
WRITE8_HANDLER( ssozumo_videoram2_w );
WRITE8_HANDLER( ssozumo_colorram2_w );
WRITE8_HANDLER( ssozumo_paletteram_w );
WRITE8_HANDLER( ssozumo_scroll_w );
WRITE8_HANDLER( ssozumo_flipscreen_w );

PALETTE_INIT( ssozumo );
VIDEO_START( ssozumo );
SCREEN_UPDATE( ssozumo );
