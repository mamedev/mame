/*************************************************************************

    Gumbo - Miss Bingo - Miss Puzzle

*************************************************************************/

class gumbo_state : public driver_device
{
public:
	gumbo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *    m_bg_videoram;
	UINT16 *    m_fg_videoram;
//  UINT16 *    paletteram; // currently this uses generic palette handling

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	tilemap_t    *m_fg_tilemap;
	DECLARE_WRITE16_MEMBER(gumbo_bg_videoram_w);
	DECLARE_WRITE16_MEMBER(gumbo_fg_videoram_w);
};


/*----------- defined in video/gumbo.c -----------*/


VIDEO_START( gumbo );
SCREEN_UPDATE_IND16( gumbo );
