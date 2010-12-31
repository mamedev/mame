
class inufuku_state : public driver_device
{
public:
	inufuku_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *  bg_videoram;
	UINT16 *  bg_rasterram;
	UINT16 *  tx_videoram;
	UINT16 *  spriteram1;
	UINT16 *  spriteram2;
//      UINT16 *  paletteram;    // currently this uses generic palette handling
	size_t    spriteram1_size;

	/* video-related */
	tilemap_t  *bg_tilemap,*tx_tilemap;
	int       bg_scrollx, bg_scrolly;
	int       tx_scrollx, tx_scrolly;
	int       bg_raster;
	int       bg_palettebank, tx_palettebank;

	/* misc */
	UINT16    pending_command;

	/* devices */
	device_t *audiocpu;
};


/*----------- defined in video/inufuku.c -----------*/

READ16_HANDLER( inufuku_bg_videoram_r );
WRITE16_HANDLER( inufuku_bg_videoram_w );
READ16_HANDLER( inufuku_tx_videoram_r );
WRITE16_HANDLER( inufuku_tx_videoram_w );
WRITE16_HANDLER( inufuku_palettereg_w );
WRITE16_HANDLER( inufuku_scrollreg_w );

VIDEO_UPDATE( inufuku );
VIDEO_START( inufuku );
