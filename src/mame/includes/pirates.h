class pirates_state : public driver_device
{
public:
	pirates_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"),
		m_scroll(*this, "scroll"),
		m_tx_tileram(*this, "tx_tileram"),
		m_fg_tileram(*this, "fg_tileram"),
		m_bg_tileram(*this, "bg_tileram"){ }

	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_scroll;
	required_shared_ptr<UINT16> m_tx_tileram;
	required_shared_ptr<UINT16> m_fg_tileram;
	required_shared_ptr<UINT16> m_bg_tileram;
	tilemap_t *m_tx_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	DECLARE_WRITE16_MEMBER(pirates_out_w);
	DECLARE_READ16_MEMBER(genix_prot_r);
	DECLARE_WRITE16_MEMBER(pirates_tx_tileram_w);
	DECLARE_WRITE16_MEMBER(pirates_fg_tileram_w);
	DECLARE_WRITE16_MEMBER(pirates_bg_tileram_w);
	DECLARE_CUSTOM_INPUT_MEMBER(prot_r);
	DECLARE_DRIVER_INIT(pirates);
	DECLARE_DRIVER_INIT(genix);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
};


/*----------- defined in video/pirates.c -----------*/


VIDEO_START( pirates );
SCREEN_UPDATE_IND16( pirates );
