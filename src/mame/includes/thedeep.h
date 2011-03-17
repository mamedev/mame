class thedeep_state : public driver_device
{
public:
	thedeep_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *spriteram;
	size_t spriteram_size;
	int nmi_enable;
	UINT8 protection_command;
	UINT8 protection_data;
	int protection_index;
	int protection_irq;
	int rombank;
	UINT8 *vram_0;
	UINT8 *vram_1;
	UINT8 *scroll;
	UINT8 *scroll2;
	tilemap_t *tilemap_0;
	tilemap_t *tilemap_1;
};


/*----------- defined in video/thedeep.c -----------*/

WRITE8_HANDLER( thedeep_vram_0_w );
WRITE8_HANDLER( thedeep_vram_1_w );

PALETTE_INIT( thedeep );
VIDEO_START( thedeep );
SCREEN_UPDATE( thedeep );

