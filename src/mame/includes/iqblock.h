class iqblock_state : public driver_device
{
public:
	iqblock_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *m_rambase;
	UINT8 *m_bgvideoram;
	UINT8 *m_fgvideoram;
	int m_videoenable;
	int m_video_type;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
};


/*----------- defined in video/iqblock.c -----------*/

WRITE8_HANDLER( iqblock_fgvideoram_w );
WRITE8_HANDLER( iqblock_bgvideoram_w );
READ8_HANDLER( iqblock_bgvideoram_r );
WRITE8_HANDLER( iqblock_fgscroll_w );

VIDEO_START( iqblock );
SCREEN_UPDATE( iqblock );
