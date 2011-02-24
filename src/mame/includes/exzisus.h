class exzisus_state : public driver_device
{
public:
	exzisus_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *sharedram_ab;
	UINT8 *sharedram_ac;
	int cpua_bank;
	int cpub_bank;
	UINT8 *videoram0;
	UINT8 *videoram1;
	UINT8 *objectram0;
	UINT8 *objectram1;
	size_t objectram_size0;
	size_t objectram_size1;
};


/*----------- defined in video/exzisus.c -----------*/

READ8_HANDLER( exzisus_videoram_0_r );
READ8_HANDLER( exzisus_videoram_1_r );
READ8_HANDLER( exzisus_objectram_0_r );
READ8_HANDLER( exzisus_objectram_1_r );
WRITE8_HANDLER( exzisus_videoram_0_w );
WRITE8_HANDLER( exzisus_videoram_1_w );
WRITE8_HANDLER( exzisus_objectram_0_w );
WRITE8_HANDLER( exzisus_objectram_1_w );

SCREEN_UPDATE( exzisus );


