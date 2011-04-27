class stfight_state : public driver_device
{
public:
	stfight_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_text_char_ram;
	UINT8 *m_text_attr_ram;
	UINT8 *m_vh_latch_ram;
	UINT8 *m_sprite_ram;
	UINT8 *m_decrypt;
	int m_adpcm_data_offs;
	int m_adpcm_data_end;
	int m_toggle;
	UINT8 m_fm_data;
	int m_coin_mech_latch[2];
	int m_coin_mech_query_active;
	int m_coin_mech_query;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_tx_tilemap;
	int m_sprite_base;
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
