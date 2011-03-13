class thedeep_state : public driver_device
{
public:
	thedeep_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *spriteram;
	size_t spriteram_size;
};


/*----------- defined in video/thedeep.c -----------*/

extern UINT8 *thedeep_vram_0, *thedeep_vram_1;
extern UINT8 *thedeep_scroll, *thedeep_scroll2;

WRITE8_HANDLER( thedeep_vram_0_w );
WRITE8_HANDLER( thedeep_vram_1_w );

PALETTE_INIT( thedeep );
VIDEO_START( thedeep );
SCREEN_UPDATE( thedeep );

