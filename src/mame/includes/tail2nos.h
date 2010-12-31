/*************************************************************************

    Tail to Nose / Super Formula

*************************************************************************/

class tail2nos_state : public driver_device
{
public:
	tail2nos_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *    bgvideoram;
	UINT16 *    spriteram;
	UINT16 *    zoomdata;
//  UINT16 *    paletteram;    // currently this uses generic palette handling
	size_t      spriteram_size;

	/* video-related */
	tilemap_t   *bg_tilemap;
	int         charbank, charpalette, video_enable;

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
	device_t *k051316;
};


/*----------- defined in video/tail2nos.c -----------*/

extern void tail2nos_zoom_callback(running_machine *machine, int *code,int *color,int *flags);

WRITE16_HANDLER( tail2nos_bgvideoram_w );
READ16_HANDLER( tail2nos_zoomdata_r );
WRITE16_HANDLER( tail2nos_zoomdata_w );
WRITE16_HANDLER( tail2nos_gfxbank_w );

VIDEO_START( tail2nos );
VIDEO_UPDATE( tail2nos );
