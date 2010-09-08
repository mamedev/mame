class timelimt_state : public driver_device
{
public:
	timelimt_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
};


/*----------- defined in video/timelimt.c -----------*/

extern UINT8 *timelimt_bg_videoram;
extern size_t timelimt_bg_videoram_size;

VIDEO_START( timelimt );
PALETTE_INIT( timelimt );
VIDEO_UPDATE( timelimt );

WRITE8_HANDLER( timelimt_videoram_w );
WRITE8_HANDLER( timelimt_bg_videoram_w );
WRITE8_HANDLER( timelimt_scroll_y_w );
WRITE8_HANDLER( timelimt_scroll_x_msb_w );
WRITE8_HANDLER( timelimt_scroll_x_lsb_w );
