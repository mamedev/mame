class bloodbro_state : public driver_device
{
public:
	bloodbro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_bgvideoram;
	UINT16 *m_fgvideoram;
	UINT16 *m_txvideoram;
	UINT16 *m_scroll;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_tx_tilemap;
	UINT16 *m_spriteram;
	size_t m_spriteram_size;
	DECLARE_WRITE16_MEMBER(bloodbro_bgvideoram_w);
	DECLARE_WRITE16_MEMBER(bloodbro_fgvideoram_w);
	DECLARE_WRITE16_MEMBER(bloodbro_txvideoram_w);
};


/*----------- defined in video/bloodbro.c -----------*/


SCREEN_UPDATE_IND16( bloodbro );
SCREEN_UPDATE_IND16( weststry );
SCREEN_UPDATE_IND16( skysmash );
VIDEO_START( bloodbro );
