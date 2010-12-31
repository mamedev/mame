/*************************************************************************

    Commando

*************************************************************************/

class commando_state : public driver_device
{
public:
	commando_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  videoram;
	UINT8 *  colorram;
	UINT8 *  videoram2;
	UINT8 *  colorram2;
//  UINT8 *  spriteram; // currently this uses generic buffered_spriteram

	/* video-related */
	tilemap_t  *bg_tilemap, *fg_tilemap;
	UINT8 scroll_x[2];
	UINT8 scroll_y[2];

	/* devices */
	device_t *audiocpu;
};



/*----------- defined in video/commando.c -----------*/

WRITE8_HANDLER( commando_videoram_w );
WRITE8_HANDLER( commando_colorram_w );
WRITE8_HANDLER( commando_videoram2_w );
WRITE8_HANDLER( commando_colorram2_w );
WRITE8_HANDLER( commando_scrollx_w );
WRITE8_HANDLER( commando_scrolly_w );
WRITE8_HANDLER( commando_c804_w );

VIDEO_START( commando );
VIDEO_UPDATE( commando );
VIDEO_EOF( commando );
