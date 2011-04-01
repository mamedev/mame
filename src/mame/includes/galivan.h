/***************************************************************************

    Galivan - Cosmo Police

***************************************************************************/

class galivan_state : public driver_device
{
public:
	galivan_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *     m_videoram;
	UINT8 *     m_colorram;
	UINT8 *     m_spriteram;
	size_t      m_videoram_size;
	size_t      m_spriteram_size;

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_tx_tilemap;
	UINT8       m_scrollx[2];
	UINT8       m_scrolly[2];
	UINT8       m_flipscreen;
	UINT8       m_write_layers;
	UINT8       m_layers;
	UINT8       m_ninjemak_dispdisable;
};



/*----------- defined in video/galivan.c -----------*/

WRITE8_HANDLER( galivan_scrollx_w );
WRITE8_HANDLER( galivan_scrolly_w );
WRITE8_HANDLER( galivan_videoram_w );
WRITE8_HANDLER( galivan_colorram_w );
WRITE8_HANDLER( galivan_gfxbank_w );
WRITE8_HANDLER( ninjemak_scrollx_w );
WRITE8_HANDLER( ninjemak_scrolly_w );
WRITE8_HANDLER( ninjemak_gfxbank_w );

PALETTE_INIT( galivan );

VIDEO_START( galivan );
VIDEO_START( ninjemak );
SCREEN_UPDATE( galivan );
SCREEN_UPDATE( ninjemak );
