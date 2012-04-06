class wc90_state : public driver_device
{
public:
	wc90_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_fgvideoram;
	UINT8 *m_bgvideoram;
	UINT8 *m_txvideoram;
	UINT8 *m_scroll0xlo;
	UINT8 *m_scroll0xhi;
	UINT8 *m_scroll1xlo;
	UINT8 *m_scroll1xhi;
	UINT8 *m_scroll2xlo;
	UINT8 *m_scroll2xhi;
	UINT8 *m_scroll0ylo;
	UINT8 *m_scroll0yhi;
	UINT8 *m_scroll1ylo;
	UINT8 *m_scroll1yhi;
	UINT8 *m_scroll2ylo;
	UINT8 *m_scroll2yhi;
	tilemap_t *m_tx_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
	DECLARE_WRITE8_MEMBER(wc90_bankswitch_w);
	DECLARE_WRITE8_MEMBER(wc90_bankswitch1_w);
	DECLARE_WRITE8_MEMBER(wc90_sound_command_w);
	DECLARE_WRITE8_MEMBER(wc90_bgvideoram_w);
	DECLARE_WRITE8_MEMBER(wc90_fgvideoram_w);
	DECLARE_WRITE8_MEMBER(wc90_txvideoram_w);
};


/*----------- defined in video/wc90.c -----------*/

VIDEO_START( wc90 );
VIDEO_START( wc90t );
SCREEN_UPDATE_IND16( wc90 );
