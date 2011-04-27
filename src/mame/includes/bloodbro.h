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
};


/*----------- defined in video/bloodbro.c -----------*/

WRITE16_HANDLER( bloodbro_bgvideoram_w );
WRITE16_HANDLER( bloodbro_fgvideoram_w );
WRITE16_HANDLER( bloodbro_txvideoram_w );

SCREEN_UPDATE( bloodbro );
SCREEN_UPDATE( weststry );
SCREEN_UPDATE( skysmash );
VIDEO_START( bloodbro );
