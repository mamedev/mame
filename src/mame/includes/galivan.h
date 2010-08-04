/***************************************************************************

    Galivan - Cosmo Police

***************************************************************************/

class galivan_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, galivan_state(machine)); }

	galivan_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT8 *     videoram;
	UINT8 *     colorram;
	UINT8 *     spriteram;
	size_t      videoram_size;
	size_t      spriteram_size;

	/* video-related */
	tilemap_t     *bg_tilemap, *tx_tilemap;
	UINT8       scrollx[2], scrolly[2];
	UINT8       flipscreen;
	UINT8       write_layers, layers;
	UINT8       ninjemak_dispdisable;
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
VIDEO_UPDATE( galivan );
VIDEO_UPDATE( ninjemak );
