class sprcros2_state : public driver_device
{
public:
	sprcros2_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 s_port3;
	UINT8 m_port7;
	tilemap_t *bgtilemap;
	tilemap_t *fgtilemap;
	UINT8 *fgvideoram;
	UINT8 *bgvideoram;
	UINT8 *spriteram;
	size_t spriteram_size;
};


/*----------- defined in video/sprcros2.c -----------*/

WRITE8_HANDLER( sprcros2_fgvideoram_w );
WRITE8_HANDLER( sprcros2_bgvideoram_w );
WRITE8_HANDLER( sprcros2_bgscrollx_w );
WRITE8_HANDLER( sprcros2_bgscrolly_w );

PALETTE_INIT( sprcros2 );
VIDEO_START( sprcros2 );
VIDEO_UPDATE( sprcros2 );
