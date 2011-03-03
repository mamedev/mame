class stfight_state : public driver_device
{
public:
	stfight_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *text_char_ram;
	UINT8 *text_attr_ram;
	UINT8 *vh_latch_ram;
	UINT8 *sprite_ram;
	UINT8 *decrypt;
	int adpcm_data_offs;
	int adpcm_data_end;
	int toggle;
	UINT8 fm_data;
	int coin_mech_latch[2];
	int coin_mech_query_active;
	int coin_mech_query;
	tilemap_t *fg_tilemap;
	tilemap_t *bg_tilemap;
	tilemap_t *tx_tilemap;
	int sprite_base;
};


/*----------- defined in machine/stfight.c -----------*/

DRIVER_INIT( empcity );
DRIVER_INIT( stfight );
MACHINE_RESET( stfight );
INTERRUPT_GEN( stfight_vb_interrupt );
READ8_HANDLER( stfight_dsw_r );
WRITE8_HANDLER( stfight_fm_w );
READ8_HANDLER( stfight_coin_r );
WRITE8_HANDLER( stfight_coin_w );
WRITE8_HANDLER( stfight_e800_w );
READ8_HANDLER( stfight_fm_r );
void stfight_adpcm_int(device_t *device);
WRITE8_DEVICE_HANDLER( stfight_adpcm_control_w );


/*----------- defined in video/stfight.c -----------*/

PALETTE_INIT( stfight );
WRITE8_HANDLER( stfight_text_char_w );
WRITE8_HANDLER( stfight_text_attr_w );
WRITE8_HANDLER( stfight_vh_latch_w );
WRITE8_HANDLER( stfight_sprite_bank_w );
VIDEO_START( stfight );
SCREEN_UPDATE( stfight );
