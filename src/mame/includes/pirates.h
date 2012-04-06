class pirates_state : public driver_device
{
public:
	pirates_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_tx_tileram;
	UINT16 *m_spriteram;
	UINT16 *m_fg_tileram;
	UINT16 *m_bg_tileram;
	UINT16 *m_scroll;
	tilemap_t *m_tx_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	DECLARE_WRITE16_MEMBER(pirates_out_w);
	DECLARE_READ16_MEMBER(genix_prot_r);
	DECLARE_WRITE16_MEMBER(pirates_tx_tileram_w);
	DECLARE_WRITE16_MEMBER(pirates_fg_tileram_w);
	DECLARE_WRITE16_MEMBER(pirates_bg_tileram_w);
};


/*----------- defined in video/pirates.c -----------*/


VIDEO_START( pirates );
SCREEN_UPDATE_IND16( pirates );
