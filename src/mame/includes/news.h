
class news_state : public driver_device
{
public:
	news_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *  m_bgram;
	UINT8 *  m_fgram;
//  UINT8 *  m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	int      m_bgpic;
	DECLARE_WRITE8_MEMBER(news_fgram_w);
	DECLARE_WRITE8_MEMBER(news_bgram_w);
	DECLARE_WRITE8_MEMBER(news_bgpic_w);
};


/*----------- defined in video/news.c -----------*/


VIDEO_START( news );
SCREEN_UPDATE_IND16( news );
