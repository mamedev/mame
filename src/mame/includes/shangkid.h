class shangkid_state : public driver_device
{
public:
	shangkid_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
	UINT8 *spriteram;
	UINT8 bbx_sound_enable;
	UINT8 sound_latch;
	UINT8 *videoreg;
	int gfx_type;
	tilemap_t *background;
};


/*----------- defined in video/shangkid.c -----------*/

VIDEO_START( shangkid );
SCREEN_UPDATE( shangkid );
WRITE8_HANDLER( shangkid_videoram_w );

PALETTE_INIT( dynamski );
SCREEN_UPDATE( dynamski );

