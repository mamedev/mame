class powerins_state : public driver_device
{
public:
	powerins_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_vctrl_0(*this, "vctrl_0"),
		m_vram_0(*this, "vram_0"),
		m_vram_1(*this, "vram_1"),
		m_spriteram(*this, "spriteram"){ }

	int m_oki_bank;
	required_shared_ptr<UINT16> m_vctrl_0;
	required_shared_ptr<UINT16> m_vram_0;
	required_shared_ptr<UINT16> m_vram_1;
	required_shared_ptr<UINT16> m_spriteram;
	UINT16 *m_vctrl_1;
	tilemap_t *m_tilemap_0;
	tilemap_t *m_tilemap_1;
	int m_tile_bank;
	DECLARE_WRITE16_MEMBER(powerins_okibank_w);
	DECLARE_WRITE16_MEMBER(powerins_soundlatch_w);
	DECLARE_READ8_MEMBER(powerinb_fake_ym2203_r);
	DECLARE_WRITE16_MEMBER(powerins_flipscreen_w);
	DECLARE_WRITE16_MEMBER(powerins_tilebank_w);
	DECLARE_WRITE16_MEMBER(powerins_paletteram16_w);
	DECLARE_WRITE16_MEMBER(powerins_vram_0_w);
	DECLARE_WRITE16_MEMBER(powerins_vram_1_w);
	TILE_GET_INFO_MEMBER(get_tile_info_0);
	TILEMAP_MAPPER_MEMBER(powerins_get_memory_offset_0);
	TILE_GET_INFO_MEMBER(get_tile_info_1);
};


/*----------- defined in video/powerins.c -----------*/




VIDEO_START( powerins );
SCREEN_UPDATE_IND16( powerins );
