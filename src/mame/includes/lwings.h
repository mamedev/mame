
class lwings_state : public driver_device
{
public:
	lwings_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  fgvideoram;
	UINT8 *  bg1videoram;
	UINT8 *  soundlatch2;
//      UINT8 *  spriteram; // currently this uses generic buffered spriteram
//      UINT8 *  paletteram;    // currently this uses generic palette handling
//      UINT8 *  paletteram2;   // currently this uses generic palette handling

	/* video-related */
	tilemap_t  *fg_tilemap, *bg1_tilemap, *bg2_tilemap;
	UINT8    bg2_image;
	int      bg2_avenger_hw;
	UINT8    scroll_x[2], scroll_y[2];

	/* misc */
	UINT8    param[4];
	int      palette_pen;
	UINT8    soundstate;
	UINT8    adpcm;
};


/*----------- defined in video/lwings.c -----------*/

WRITE8_HANDLER( lwings_fgvideoram_w );
WRITE8_HANDLER( lwings_bg1videoram_w );
WRITE8_HANDLER( lwings_bg1_scrollx_w );
WRITE8_HANDLER( lwings_bg1_scrolly_w );
WRITE8_HANDLER( trojan_bg2_scrollx_w );
WRITE8_HANDLER( trojan_bg2_image_w );

VIDEO_START( lwings );
VIDEO_START( trojan );
VIDEO_START( avengers );
VIDEO_UPDATE( lwings );
VIDEO_UPDATE( trojan );
VIDEO_EOF( lwings );
