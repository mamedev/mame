class system1_state : public driver_device
{
public:
	system1_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
};


/*----------- defined in video/system1.c -----------*/

VIDEO_START( system1 );
VIDEO_START( system2 );

WRITE8_HANDLER( system1_videomode_w );
WRITE8_HANDLER( system1_paletteram_w );

READ8_HANDLER( system1_videoram_r );
WRITE8_HANDLER( system1_videoram_w );
WRITE8_DEVICE_HANDLER( system1_videoram_bank_w );

READ8_HANDLER( system1_mixer_collision_r );
WRITE8_HANDLER( system1_mixer_collision_w );
WRITE8_HANDLER( system1_mixer_collision_reset_w );

READ8_HANDLER( system1_sprite_collision_r );
WRITE8_HANDLER( system1_sprite_collision_w );
WRITE8_HANDLER( system1_sprite_collision_reset_w );

VIDEO_UPDATE( system1 );
VIDEO_UPDATE( system2 );
VIDEO_UPDATE( system2_rowscroll );
