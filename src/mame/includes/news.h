
class news_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, news_state(machine)); }

	news_state(running_machine &machine) { }

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
