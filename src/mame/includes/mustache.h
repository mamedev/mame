class mustache_state : public driver_device
{
public:
	mustache_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;

    emu_timer *clear_irq_timer;
};


/*----------- defined in video/mustache.c -----------*/

WRITE8_HANDLER( mustache_videoram_w );
WRITE8_HANDLER( mustache_scroll_w );
WRITE8_HANDLER( mustache_video_control_w );
VIDEO_START( mustache );
VIDEO_UPDATE( mustache );
PALETTE_INIT( mustache );
