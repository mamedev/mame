class namcofl_state : public driver_device
{
public:
	namcofl_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	emu_timer *raster_interrupt_timer;
	UINT32 *workram;
	UINT16 *shareram;
	UINT8 mcu_port6;
	UINT32 sprbank;
};


/*----------- defined in video/namcofl.c -----------*/

VIDEO_START( namcofl );
SCREEN_UPDATE( namcofl );

WRITE32_HANDLER( namcofl_spritebank_w );
