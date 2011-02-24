class thoop2_state : public driver_device
{
public:
	thoop2_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *vregs;
	UINT16 *videoram;
	UINT16 *spriteram;
	int sprite_count[5];
	int *sprite_table[5];
	tilemap_t *pant[2];
};


/*----------- defined in video/thoop2.c -----------*/

WRITE16_HANDLER( thoop2_vram_w );
VIDEO_START( thoop2 );
SCREEN_UPDATE( thoop2 );
