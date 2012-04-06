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
	DECLARE_WRITE16_MEMBER(quizpani_bg_videoram_w);
	DECLARE_WRITE16_MEMBER(quizpani_txt_videoram_w);
	DECLARE_WRITE16_MEMBER(quizpani_tilesbank_w);
};


/*----------- defined in video/quizpani.c -----------*/


VIDEO_START( quizpani );
SCREEN_UPDATE_IND16( quizpani );
