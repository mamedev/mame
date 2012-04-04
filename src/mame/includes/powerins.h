class powerins_state : public driver_device
{
public:
	powerins_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_oki_bank;
	UINT16 *m_vram_0;
	UINT16 *m_vctrl_0;
	UINT16 *m_vram_1;
	UINT16 *m_vctrl_1;
	tilemap_t *m_tilemap_0;
	tilemap_t *m_tilemap_1;
	int m_tile_bank;
	UINT16 *m_spriteram;
	DECLARE_WRITE16_MEMBER(powerins_okibank_w);
	DECLARE_WRITE16_MEMBER(powerins_soundlatch_w);
	DECLARE_READ8_MEMBER(powerinb_fake_ym2203_r);
};


/*----------- defined in video/powerins.c -----------*/

WRITE16_HANDLER( powerins_flipscreen_w );
WRITE16_HANDLER( powerins_tilebank_w );

WRITE16_HANDLER( powerins_paletteram16_w );

WRITE16_HANDLER( powerins_vram_0_w );
WRITE16_HANDLER( powerins_vram_1_w );

VIDEO_START( powerins );
SCREEN_UPDATE_IND16( powerins );
