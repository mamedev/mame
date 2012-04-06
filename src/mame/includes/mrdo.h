/*************************************************************************

    Mr. Do

*************************************************************************/

class mrdo_state : public driver_device
{
public:
	mrdo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_bgvideoram;
	UINT8 *    m_fgvideoram;
	UINT8 *    m_spriteram;
	size_t     m_spriteram_size;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	int       m_flipscreen;
	DECLARE_READ8_MEMBER(mrdo_SECRE_r);
	DECLARE_WRITE8_MEMBER(mrdo_bgvideoram_w);
	DECLARE_WRITE8_MEMBER(mrdo_fgvideoram_w);
	DECLARE_WRITE8_MEMBER(mrdo_scrollx_w);
	DECLARE_WRITE8_MEMBER(mrdo_scrolly_w);
	DECLARE_WRITE8_MEMBER(mrdo_flipscreen_w);
};


/*----------- defined in video/mrdo.c -----------*/


PALETTE_INIT( mrdo );
VIDEO_START( mrdo );
SCREEN_UPDATE_IND16( mrdo );
