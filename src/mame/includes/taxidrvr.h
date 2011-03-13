class taxidrvr_state : public driver_device
{
public:
	taxidrvr_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int s1;
	int s2;
	int s3;
	int s4;
	int latchA;
	int latchB;
	UINT8 *vram0;
	UINT8 *vram1;
	UINT8 *vram2;
	UINT8 *vram3;
	UINT8 *vram4;
	UINT8 *vram5;
	UINT8 *vram6;
	UINT8 *vram7;
	UINT8 *scroll;
	int bghide;
	int spritectrl[9];
};


/*----------- defined in video/taxidrvr.c -----------*/

WRITE8_DEVICE_HANDLER( taxidrvr_spritectrl_w );

SCREEN_UPDATE( taxidrvr );
