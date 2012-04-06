/*************************************************************************

    Gun Dealer

*************************************************************************/

class gundealr_state : public driver_device
{
public:
	gundealr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_bg_videoram;
	UINT8 *    m_fg_videoram;
	UINT8 *    m_rambase;
	UINT8 *    m_paletteram;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	tilemap_t    *m_fg_tilemap;
	int        m_flipscreen;
	UINT8      m_scroll[4];

	/* misc */
	int        m_input_ports_hack;
	DECLARE_WRITE8_MEMBER(yamyam_bankswitch_w);
	DECLARE_WRITE8_MEMBER(gundealr_bg_videoram_w);
	DECLARE_WRITE8_MEMBER(gundealr_fg_videoram_w);
	DECLARE_WRITE8_MEMBER(gundealr_paletteram_w);
	DECLARE_WRITE8_MEMBER(gundealr_fg_scroll_w);
	DECLARE_WRITE8_MEMBER(yamyam_fg_scroll_w);
	DECLARE_WRITE8_MEMBER(gundealr_flipscreen_w);
};



/*----------- defined in video/gundealr.c -----------*/


SCREEN_UPDATE_IND16( gundealr );
VIDEO_START( gundealr );
