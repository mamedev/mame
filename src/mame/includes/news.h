
typedef struct _news_state news_state;
struct _news_state
{
	/* memory pointers */
	UINT8 *  bgram;
	UINT8 *  fgram;
//  UINT8 *  paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t *fg_tilemap, *bg_tilemap;
	int      bgpic;
};


/*----------- defined in video/news.c -----------*/

WRITE8_HANDLER( news_fgram_w );
WRITE8_HANDLER( news_bgram_w );
WRITE8_HANDLER( news_bgpic_w );

VIDEO_START( news );
VIDEO_UPDATE( news );
