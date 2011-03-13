class portrait_state : public driver_device
{
public:
	portrait_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *bgvideoram;
	UINT8 *fgvideoram;
	int scroll;
	tilemap_t *foreground;
	tilemap_t *background;
	UINT8 *spriteram;
	size_t spriteram_size;
};


/*----------- defined in video/portrait.c -----------*/

PALETTE_INIT( portrait );
VIDEO_START( portrait );
SCREEN_UPDATE( portrait );
WRITE8_HANDLER( portrait_bgvideo_write );
WRITE8_HANDLER( portrait_fgvideo_write );
