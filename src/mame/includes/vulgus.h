class vulgus_state : public driver_device
{
public:
	vulgus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_fgvideoram;
	UINT8 *m_bgvideoram;
	UINT8 *m_scroll_low;
	UINT8 *m_scroll_high;
	int m_palette_bank;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
	DECLARE_WRITE8_MEMBER(vulgus_fgvideoram_w);
	DECLARE_WRITE8_MEMBER(vulgus_bgvideoram_w);
	DECLARE_WRITE8_MEMBER(vulgus_c804_w);
	DECLARE_WRITE8_MEMBER(vulgus_palette_bank_w);
};


/*----------- defined in video/vulgus.c -----------*/


VIDEO_START( vulgus );
PALETTE_INIT( vulgus );
SCREEN_UPDATE_IND16( vulgus );
