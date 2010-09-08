class thief_state : public driver_device
{
public:
	thief_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
};


/*----------- defined in video/thief.c -----------*/

READ8_HANDLER( thief_context_ram_r );
WRITE8_HANDLER( thief_context_ram_w );
WRITE8_HANDLER( thief_context_bank_w );
WRITE8_HANDLER( thief_video_control_w );
WRITE8_HANDLER( thief_color_map_w );
WRITE8_HANDLER( thief_color_plane_w );
READ8_HANDLER( thief_videoram_r );
WRITE8_HANDLER( thief_videoram_w );
WRITE8_HANDLER( thief_blit_w );
READ8_HANDLER( thief_coprocessor_r );
WRITE8_HANDLER( thief_coprocessor_w );

VIDEO_START( thief );
VIDEO_UPDATE( thief );
