/*************************************************************************

    Mosaic

*************************************************************************/

class mosaic_state : public driver_device
{
public:
	mosaic_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *        m_fgvideoram;
	UINT8 *        m_bgvideoram;
//      UINT8 *        m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t      *m_bg_tilemap;
	tilemap_t      *m_fg_tilemap;

	/* misc */
	int            m_prot_val;
};


/*----------- defined in video/mosaic.c -----------*/

WRITE8_HANDLER( mosaic_fgvideoram_w );
WRITE8_HANDLER( mosaic_bgvideoram_w );

VIDEO_START( mosaic );
SCREEN_UPDATE( mosaic );
