class powerins_state : public driver_device
{
public:
	powerins_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int oki_bank;
	UINT16 *vram_0;
	UINT16 *vctrl_0;
	UINT16 *vram_1;
	UINT16 *vctrl_1;
	tilemap_t *tilemap_0;
	tilemap_t *tilemap_1;
	int tile_bank;
};


/*----------- defined in video/powerins.c -----------*/

WRITE16_HANDLER( powerins_flipscreen_w );
WRITE16_HANDLER( powerins_tilebank_w );

WRITE16_HANDLER( powerins_paletteram16_w );

WRITE16_HANDLER( powerins_vram_0_w );
WRITE16_HANDLER( powerins_vram_1_w );

VIDEO_START( powerins );
SCREEN_UPDATE( powerins );
