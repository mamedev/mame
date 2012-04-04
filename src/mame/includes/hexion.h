class hexion_state : public driver_device
{
public:
	hexion_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_vram[2];
	UINT8 *m_unkram;
	int m_bankctrl;
	int m_rambank;
	int m_pmcbank;
	int m_gfxrom_select;
	tilemap_t *m_bg_tilemap[2];
	DECLARE_WRITE8_MEMBER(coincntr_w);
};


/*----------- defined in video/hexion.c -----------*/

VIDEO_START( hexion );
SCREEN_UPDATE_IND16( hexion );

WRITE8_HANDLER( hexion_bankswitch_w );
READ8_HANDLER( hexion_bankedram_r );
WRITE8_HANDLER( hexion_bankedram_w );
WRITE8_HANDLER( hexion_bankctrl_w );
WRITE8_HANDLER( hexion_gfxrom_select_w );
