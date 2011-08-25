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
};


/*----------- defined in video/quizdna.c -----------*/

VIDEO_START( quizdna );
SCREEN_UPDATE( quizdna );

WRITE8_HANDLER( quizdna_fg_ram_w );
WRITE8_HANDLER( quizdna_bg_ram_w );
WRITE8_HANDLER( quizdna_bg_yscroll_w );
WRITE8_HANDLER( quizdna_bg_xscroll_w );
WRITE8_HANDLER( quizdna_screen_ctrl_w );

WRITE8_HANDLER( paletteram_xBGR_RRRR_GGGG_BBBB_w );
