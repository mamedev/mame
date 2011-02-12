class tryout_state : public driver_device
{
public:
	tryout_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
	UINT8 *gfx_control;
	tilemap_t *fg_tilemap;
	tilemap_t *bg_tilemap;
	UINT8 vram_bank;
	UINT8 *vram;
	UINT8 *vram_gfx;
};


/*----------- defined in video/tryout.c -----------*/

READ8_HANDLER( tryout_vram_r );
WRITE8_HANDLER( tryout_videoram_w );
WRITE8_HANDLER( tryout_vram_w );
WRITE8_HANDLER( tryout_vram_bankswitch_w );
WRITE8_HANDLER( tryout_flipscreen_w );

PALETTE_INIT( tryout );
VIDEO_START( tryout );
VIDEO_UPDATE( tryout );
