class bloodbro_state : public driver_device
{
public:
	bloodbro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_txvideoram(*this, "txvideoram"),
		m_scroll(*this, "scroll"){ }

	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_bgvideoram;
	required_shared_ptr<UINT16> m_fgvideoram;
	required_shared_ptr<UINT16> m_txvideoram;
	optional_shared_ptr<UINT16> m_scroll;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_tx_tilemap;

	DECLARE_WRITE16_MEMBER(bloodbro_bgvideoram_w);
	DECLARE_WRITE16_MEMBER(bloodbro_fgvideoram_w);
	DECLARE_WRITE16_MEMBER(bloodbro_txvideoram_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
};


/*----------- defined in video/bloodbro.c -----------*/


SCREEN_UPDATE_IND16( bloodbro );
SCREEN_UPDATE_IND16( weststry );
SCREEN_UPDATE_IND16( skysmash );
VIDEO_START( bloodbro );
