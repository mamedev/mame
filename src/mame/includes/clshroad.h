class clshroad_state : public driver_device
{
public:
	clshroad_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *vram_0;
	UINT8 *vram_1;
	UINT8 *vregs;
	tilemap_t *tilemap_0a;
	tilemap_t *tilemap_0b;
	tilemap_t *tilemap_1;
};


/*----------- defined in video/clshroad.c -----------*/

WRITE8_HANDLER( clshroad_vram_0_w );
WRITE8_HANDLER( clshroad_vram_1_w );
WRITE8_HANDLER( clshroad_flipscreen_w );

PALETTE_INIT( firebatl );
PALETTE_INIT( clshroad );
VIDEO_START( firebatl );
VIDEO_START( clshroad );
SCREEN_UPDATE( clshroad );
