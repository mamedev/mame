class quizpani_state : public driver_device
{
public:
	quizpani_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_bg_videoram;
	UINT16 *m_txt_videoram;
	UINT16 *m_scrollreg;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_txt_tilemap;
	int m_bgbank;
	int m_txtbank;
};


/*----------- defined in video/quizpani.c -----------*/

WRITE16_HANDLER( quizpani_bg_videoram_w );
WRITE16_HANDLER( quizpani_txt_videoram_w );
WRITE16_HANDLER( quizpani_tilesbank_w );

VIDEO_START( quizpani );
SCREEN_UPDATE( quizpani );
