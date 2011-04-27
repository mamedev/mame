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
};


/*----------- defined in video/tbowl.c -----------*/

WRITE8_HANDLER( tbowl_bg2videoram_w );
WRITE8_HANDLER( tbowl_bgvideoram_w );
WRITE8_HANDLER( tbowl_txvideoram_w );

WRITE8_HANDLER( tbowl_bg2xscroll_lo );
WRITE8_HANDLER( tbowl_bg2xscroll_hi );
WRITE8_HANDLER( tbowl_bg2yscroll_lo );
WRITE8_HANDLER( tbowl_bg2yscroll_hi );
WRITE8_HANDLER( tbowl_bgxscroll_lo );
WRITE8_HANDLER( tbowl_bgxscroll_hi );
WRITE8_HANDLER( tbowl_bgyscroll_lo );
WRITE8_HANDLER( tbowl_bgyscroll_hi );

VIDEO_START( tbowl );
SCREEN_UPDATE( tbowl );
