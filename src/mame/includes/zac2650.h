class zac2650_state : public driver_device
{
public:
	zac2650_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
	UINT8 *s2636_0_ram;
	bitmap_t *spritebitmap;
	int CollisionBackground;
	int CollisionSprite;
	tilemap_t *bg_tilemap;
};


/*----------- defined in video/zac2650.c -----------*/

WRITE8_HANDLER( tinvader_videoram_w );
READ8_HANDLER( zac_s2636_r );
WRITE8_HANDLER( zac_s2636_w );
READ8_HANDLER( tinvader_port_0_r );

VIDEO_START( tinvader );
SCREEN_UPDATE( tinvader );

