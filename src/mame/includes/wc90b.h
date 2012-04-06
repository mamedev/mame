class wc90b_state : public driver_device
{
public:
	wc90b_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_msm5205next;
	int m_toggle;
	UINT8 *m_fgvideoram;
	UINT8 *m_bgvideoram;
	UINT8 *m_txvideoram;
	UINT8 *m_scroll1x;
	UINT8 *m_scroll2x;
	UINT8 *m_scroll1y;
	UINT8 *m_scroll2y;
	UINT8 *m_scroll_x_lo;
	tilemap_t *m_tx_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
	DECLARE_WRITE8_MEMBER(wc90b_bankswitch_w);
	DECLARE_WRITE8_MEMBER(wc90b_bankswitch1_w);
	DECLARE_WRITE8_MEMBER(wc90b_sound_command_w);
	DECLARE_WRITE8_MEMBER(adpcm_data_w);
	DECLARE_WRITE8_MEMBER(wc90b_bgvideoram_w);
	DECLARE_WRITE8_MEMBER(wc90b_fgvideoram_w);
	DECLARE_WRITE8_MEMBER(wc90b_txvideoram_w);
};


/*----------- defined in video/wc90b.c -----------*/

VIDEO_START( wc90b );
SCREEN_UPDATE_IND16( wc90b );

