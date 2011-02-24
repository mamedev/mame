class hexion_state : public driver_device
{
public:
	hexion_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *vram[2];
	UINT8 *unkram;
	int bankctrl;
	int rambank;
	int pmcbank;
	int gfxrom_select;
	tilemap_t *bg_tilemap[2];
};


/*----------- defined in video/hexion.c -----------*/

VIDEO_START( hexion );
SCREEN_UPDATE( hexion );

WRITE8_HANDLER( hexion_bankswitch_w );
READ8_HANDLER( hexion_bankedram_r );
WRITE8_HANDLER( hexion_bankedram_w );
WRITE8_HANDLER( hexion_bankctrl_w );
WRITE8_HANDLER( hexion_gfxrom_select_w );
