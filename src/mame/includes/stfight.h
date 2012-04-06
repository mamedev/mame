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
	DECLARE_READ8_MEMBER(stfight_dsw_r);
	DECLARE_READ8_MEMBER(stfight_coin_r);
	DECLARE_WRITE8_MEMBER(stfight_coin_w);
	DECLARE_WRITE8_MEMBER(stfight_e800_w);
	DECLARE_WRITE8_MEMBER(stfight_fm_w);
	DECLARE_READ8_MEMBER(stfight_fm_r);
	DECLARE_WRITE8_MEMBER(stfight_bank_w);
	DECLARE_WRITE8_MEMBER(stfight_text_char_w);
	DECLARE_WRITE8_MEMBER(stfight_text_attr_w);
	DECLARE_WRITE8_MEMBER(stfight_sprite_bank_w);
	DECLARE_WRITE8_MEMBER(stfight_vh_latch_w);
};


/*----------- defined in machine/stfight.c -----------*/

DRIVER_INIT( empcity );
DRIVER_INIT( stfight );
MACHINE_RESET( stfight );
INTERRUPT_GEN( stfight_vb_interrupt );
void stfight_adpcm_int(device_t *device);
WRITE8_DEVICE_HANDLER( stfight_adpcm_control_w );


/*----------- defined in video/stfight.c -----------*/

PALETTE_INIT( stfight );
VIDEO_START( stfight );
SCREEN_UPDATE_IND16( stfight );
