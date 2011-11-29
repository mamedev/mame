class sbasketb_state : public driver_device
{
public:
	sbasketb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *  m_videoram;
	UINT8 *  m_colorram;
	UINT8 *  m_scroll;
	UINT8 *  m_spriteram;
	UINT8 *  m_palettebank;
	UINT8 *  m_spriteram_select;

	/* video-related */
	tilemap_t  *m_bg_tilemap;

	UINT8    m_irq_mask;
};

/*----------- defined in video/sbasketb.c -----------*/

WRITE8_HANDLER( sbasketb_videoram_w );
WRITE8_HANDLER( sbasketb_colorram_w );
WRITE8_HANDLER( sbasketb_flipscreen_w );

PALETTE_INIT( sbasketb );
VIDEO_START( sbasketb );
SCREEN_UPDATE( sbasketb );
