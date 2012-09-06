
class news_state : public driver_device
{
public:
	news_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_bgram(*this, "bgram"),
		m_fgram(*this, "fgram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_bgram;
	required_shared_ptr<UINT8> m_fgram;
//  UINT8 *  m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	int      m_bgpic;
	DECLARE_WRITE8_MEMBER(news_fgram_w);
	DECLARE_WRITE8_MEMBER(news_bgram_w);
	DECLARE_WRITE8_MEMBER(news_bgpic_w);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
};


/*----------- defined in video/news.c -----------*/


VIDEO_START( news );
SCREEN_UPDATE_IND16( news );
