class targeth_state : public driver_device
{
public:
	targeth_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *vregs;
	UINT16 *videoram;
	UINT16 *spriteram;
	tilemap_t *pant[2];
};


/*----------- defined in video/targeth.c -----------*/

WRITE16_HANDLER( targeth_vram_w );
VIDEO_START( targeth );
SCREEN_UPDATE( targeth );
