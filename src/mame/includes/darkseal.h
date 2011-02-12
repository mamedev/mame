class darkseal_state : public driver_device
{
public:
	darkseal_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *ram;
	UINT16 *pf12_row;
	UINT16 *pf34_row;
	UINT16 *pf1_data;
	UINT16 *pf2_data;
	UINT16 *pf3_data;
	UINT16 control_0[8];
	UINT16 control_1[8];
	tilemap_t *pf1_tilemap;
	tilemap_t *pf2_tilemap;
	tilemap_t *pf3_tilemap;
	int flipscreen;
};


/*----------- defined in video/darkseal.c -----------*/

VIDEO_START( darkseal );
VIDEO_UPDATE( darkseal );

WRITE16_HANDLER( darkseal_pf1_data_w );
WRITE16_HANDLER( darkseal_pf2_data_w );
WRITE16_HANDLER( darkseal_pf3_data_w );
WRITE16_HANDLER( darkseal_pf3b_data_w );
WRITE16_HANDLER( darkseal_control_0_w );
WRITE16_HANDLER( darkseal_control_1_w );
WRITE16_HANDLER( darkseal_palette_24bit_rg_w );
WRITE16_HANDLER( darkseal_palette_24bit_b_w );
