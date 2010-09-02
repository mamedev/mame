/*************************************************************************

    Mosaic

*************************************************************************/

class mosaic_state : public driver_device
{
public:
	mosaic_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *        fgvideoram;
	UINT8 *        bgvideoram;
//      UINT8 *        paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t      *bg_tilemap,*fg_tilemap;

	/* misc */
	int            prot_val;
};


/*----------- defined in video/mosaic.c -----------*/

WRITE8_HANDLER( mosaic_fgvideoram_w );
WRITE8_HANDLER( mosaic_bgvideoram_w );

VIDEO_START( mosaic );
VIDEO_UPDATE( mosaic );
