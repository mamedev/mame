class stfight_state : public driver_device
{
public:
	stfight_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_text_char_ram(*this, "text_char_ram"),
		m_text_attr_ram(*this, "text_attr_ram"),
		m_vh_latch_ram(*this, "vh_latch_ram"),
		m_sprite_ram(*this, "sprite_ram"){ }

	required_shared_ptr<UINT8> m_text_char_ram;
	required_shared_ptr<UINT8> m_text_attr_ram;
	required_shared_ptr<UINT8> m_vh_latch_ram;
	required_shared_ptr<UINT8> m_sprite_ram;
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
	DECLARE_DRIVER_INIT(stfight);
	DECLARE_DRIVER_INIT(empcity);
	TILEMAP_MAPPER_MEMBER(fg_scan);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILEMAP_MAPPER_MEMBER(bg_scan);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_stfight(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(stfight_vb_interrupt);
	TIMER_CALLBACK_MEMBER(stfight_interrupt_1);
	DECLARE_WRITE8_MEMBER(stfight_adpcm_control_w);
};

/*----------- defined in machine/stfight.c -----------*/
void stfight_adpcm_int(device_t *device);
