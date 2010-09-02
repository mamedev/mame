class sbugger_state : public driver_device
{
public:
	sbugger_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
	UINT8 *videoram_attr;

	tilemap_t *tilemap;
};


/*----------- defined in video/sbugger.c -----------*/

PALETTE_INIT(sbugger);
VIDEO_UPDATE(sbugger);
VIDEO_START(sbugger);
WRITE8_HANDLER( sbugger_videoram_attr_w );
WRITE8_HANDLER( sbugger_videoram_w );
