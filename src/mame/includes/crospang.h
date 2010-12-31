/*************************************************************************

    Cross Pang

*************************************************************************/

class crospang_state : public driver_device
{
public:
	crospang_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *  bg_videoram;
	UINT16 *  fg_videoram;
	UINT16 *  spriteram;
//  UINT16 *  paletteram;       // currently this uses generic palette handling
	size_t    spriteram_size;

	/* video-related */
	tilemap_t   *bg_layer,*fg_layer;
	int       xsproff, ysproff;
	int       bestri_tilebank;

	/* devices */
	device_t *audiocpu;
};


/*----------- defined in video/crospang.c -----------*/

VIDEO_START( crospang );
VIDEO_UPDATE( crospang );

WRITE16_HANDLER ( crospang_fg_scrolly_w );
WRITE16_HANDLER ( crospang_bg_scrolly_w );
WRITE16_HANDLER ( crospang_fg_scrollx_w );
WRITE16_HANDLER ( crospang_bg_scrollx_w );

WRITE16_HANDLER ( bestri_fg_scrolly_w );
WRITE16_HANDLER ( bestri_bg_scrolly_w );
WRITE16_HANDLER ( bestri_fg_scrollx_w );
WRITE16_HANDLER ( bestri_bg_scrollx_w );

WRITE16_HANDLER ( crospang_fg_videoram_w );
WRITE16_HANDLER ( crospang_bg_videoram_w );
WRITE16_HANDLER ( bestri_tilebank_w );
