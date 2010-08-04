/*************************************************************************

    Tail to Nose / Super Formula

*************************************************************************/

class tail2nos_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, tail2nos_state(machine)); }

	tail2nos_state(running_machine &machine)
		: driver_data_t(machine) { }

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
	running_device *maincpu;
	running_device *audiocpu;
	running_device *k051316;
};


/*----------- defined in video/tail2nos.c -----------*/

extern void tail2nos_zoom_callback(running_machine *machine, int *code,int *color,int *flags);

WRITE16_HANDLER( tail2nos_bgvideoram_w );
READ16_HANDLER( tail2nos_zoomdata_r );
WRITE16_HANDLER( tail2nos_zoomdata_w );
WRITE16_HANDLER( tail2nos_gfxbank_w );

VIDEO_START( tail2nos );
VIDEO_UPDATE( tail2nos );
