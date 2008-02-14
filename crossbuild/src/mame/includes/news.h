/*----------- defined in video/news.c -----------*/

extern UINT8 *news_fgram;
extern UINT8 *news_bgram;

WRITE8_HANDLER( news_fgram_w );
WRITE8_HANDLER( news_bgram_w );
WRITE8_HANDLER( news_bgpic_w );
VIDEO_START( news );
VIDEO_UPDATE( news );
