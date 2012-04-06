class tbowl_state : public driver_device
{
public:
	tbowl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_adpcm_pos[2];
	int m_adpcm_end[2];
	int m_adpcm_data[2];
	UINT8 *m_shared_ram;
	UINT8 *m_txvideoram;
	UINT8 *m_bgvideoram;
	UINT8 *m_bg2videoram;
	UINT8 *m_spriteram;
	tilemap_t *m_tx_tilemap;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_bg2_tilemap;
	UINT16 m_xscroll;
	UINT16 m_yscroll;
	UINT16 m_bg2xscroll;
	UINT16 m_bg2yscroll;
	DECLARE_WRITE8_MEMBER(tbowl_coin_counter_w);
	DECLARE_WRITE8_MEMBER(tbowlb_bankswitch_w);
	DECLARE_WRITE8_MEMBER(tbowlc_bankswitch_w);
	DECLARE_READ8_MEMBER(shared_r);
	DECLARE_WRITE8_MEMBER(shared_w);
	DECLARE_WRITE8_MEMBER(tbowl_sound_command_w);
	DECLARE_WRITE8_MEMBER(tbowl_trigger_nmi);
	DECLARE_WRITE8_MEMBER(tbowl_adpcm_start_w);
	DECLARE_WRITE8_MEMBER(tbowl_adpcm_end_w);
	DECLARE_WRITE8_MEMBER(tbowl_adpcm_vol_w);
	DECLARE_WRITE8_MEMBER(tbowl_txvideoram_w);
	DECLARE_WRITE8_MEMBER(tbowl_bg2videoram_w);
	DECLARE_WRITE8_MEMBER(tbowl_bgxscroll_lo);
	DECLARE_WRITE8_MEMBER(tbowl_bgxscroll_hi);
	DECLARE_WRITE8_MEMBER(tbowl_bgyscroll_lo);
	DECLARE_WRITE8_MEMBER(tbowl_bgyscroll_hi);
	DECLARE_WRITE8_MEMBER(tbowl_bgvideoram_w);
	DECLARE_WRITE8_MEMBER(tbowl_bg2xscroll_lo);
	DECLARE_WRITE8_MEMBER(tbowl_bg2xscroll_hi);
	DECLARE_WRITE8_MEMBER(tbowl_bg2yscroll_lo);
	DECLARE_WRITE8_MEMBER(tbowl_bg2yscroll_hi);
};


/*----------- defined in video/tbowl.c -----------*/



VIDEO_START( tbowl );
SCREEN_UPDATE_IND16( tbowl_left );
SCREEN_UPDATE_IND16( tbowl_right );

