class solomon_state : public driver_device
{
public:
	solomon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	size_t m_spriteram_size;
	UINT8 *m_spriteram;
	UINT8 *m_videoram;
	UINT8 *m_colorram;
	UINT8 *m_videoram2;
	UINT8 *m_colorram2;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;

	UINT8 m_nmi_mask;
};


/*----------- defined in video/solomon.c -----------*/

WRITE8_HANDLER( solomon_videoram_w );
WRITE8_HANDLER( solomon_colorram_w );
WRITE8_HANDLER( solomon_videoram2_w );
WRITE8_HANDLER( solomon_colorram2_w );
WRITE8_HANDLER( solomon_flipscreen_w );

VIDEO_START( solomon );
SCREEN_UPDATE( solomon );
