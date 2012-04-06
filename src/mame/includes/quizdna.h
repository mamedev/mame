class quizdna_state : public driver_device
{
public:
	quizdna_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_bg_ram;
	UINT8 *m_fg_ram;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	UINT8 m_bg_xscroll[2];
	int m_flipscreen;
	int m_video_enable;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
	DECLARE_WRITE8_MEMBER(quizdna_rombank_w);
	DECLARE_WRITE8_MEMBER(gekiretu_rombank_w);
	DECLARE_WRITE8_MEMBER(quizdna_bg_ram_w);
	DECLARE_WRITE8_MEMBER(quizdna_fg_ram_w);
	DECLARE_WRITE8_MEMBER(quizdna_bg_yscroll_w);
	DECLARE_WRITE8_MEMBER(quizdna_bg_xscroll_w);
	DECLARE_WRITE8_MEMBER(quizdna_screen_ctrl_w);
	DECLARE_WRITE8_MEMBER(paletteram_xBGR_RRRR_GGGG_BBBB_w);
};


/*----------- defined in video/quizdna.c -----------*/

VIDEO_START( quizdna );
SCREEN_UPDATE_IND16( quizdna );


