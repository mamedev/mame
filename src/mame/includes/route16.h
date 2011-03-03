class route16_state : public driver_device
{
public:
	route16_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *sharedram;
	UINT8 ttmahjng_port_select;
	int speakres_vrx;
	UINT8 *videoram1;
	UINT8 *videoram2;
	size_t videoram_size;
	UINT8 flipscreen;
	UINT8 palette_1;
	UINT8 palette_2;
};


/*----------- defined in video/route16.c -----------*/

WRITE8_HANDLER( route16_out0_w );
WRITE8_HANDLER( route16_out1_w );
SCREEN_UPDATE( route16 );
SCREEN_UPDATE( stratvox );
SCREEN_UPDATE( ttmahjng );
