/*************************************************************************

    Mr. Jong

*************************************************************************/

class mrjong_state : public driver_device
{
public:
	mrjong_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_colorram;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	DECLARE_READ8_MEMBER(io_0x03_r);
	DECLARE_WRITE8_MEMBER(mrjong_videoram_w);
	DECLARE_WRITE8_MEMBER(mrjong_colorram_w);
	DECLARE_WRITE8_MEMBER(mrjong_flipscreen_w);
};


/*----------- defined in video/mrjong.c -----------*/


PALETTE_INIT( mrjong );
VIDEO_START( mrjong );
SCREEN_UPDATE_IND16( mrjong );
