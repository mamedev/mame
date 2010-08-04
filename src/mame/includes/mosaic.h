/*************************************************************************

    Mosaic

*************************************************************************/

class mosaic_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, mosaic_state(machine)); }

	mosaic_state(running_machine &machine)
		: driver_data_t(machine) { }

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
