

class funybubl_state : public driver_device
{
public:
	funybubl_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    banked_vram;
	UINT8 *    paletteram;

	/* devices */
	device_t *audiocpu;
};



/*----------- defined in video/funybubl.c -----------*/

WRITE8_HANDLER ( funybubl_paldatawrite );

VIDEO_START(funybubl);
VIDEO_UPDATE(funybubl);
