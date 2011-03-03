typedef struct {
	UINT8 *context_ram;
	UINT8 bank;
	UINT8 *image_ram;
	UINT8 param[0x9];
} coprocessor_t;

class thief_state : public driver_device
{
public:
	thief_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
	UINT8 input_select;
	UINT8 read_mask;
	UINT8 write_mask;
	UINT8 video_control;
	coprocessor_t coprocessor;
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
SCREEN_UPDATE( thief );
