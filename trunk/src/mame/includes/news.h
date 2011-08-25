
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
};


/*----------- defined in video/news.c -----------*/

WRITE8_HANDLER( news_fgram_w );
WRITE8_HANDLER( news_bgram_w );
WRITE8_HANDLER( news_bgpic_w );

VIDEO_START( news );
SCREEN_UPDATE( news );
