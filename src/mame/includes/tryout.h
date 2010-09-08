class tryout_state : public driver_device
{
public:
	tryout_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
};


/*----------- defined in video/tryout.c -----------*/

extern UINT8 *tryout_gfx_control;

READ8_HANDLER( tryout_vram_r );
WRITE8_HANDLER( tryout_videoram_w );
WRITE8_HANDLER( tryout_vram_w );
WRITE8_HANDLER( tryout_vram_bankswitch_w );
WRITE8_HANDLER( tryout_flipscreen_w );

PALETTE_INIT( tryout );
VIDEO_START( tryout );
VIDEO_UPDATE( tryout );
