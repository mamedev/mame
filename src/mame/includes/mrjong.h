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
};


/*----------- defined in video/mrjong.c -----------*/

WRITE8_HANDLER( mrjong_videoram_w );
WRITE8_HANDLER( mrjong_colorram_w );
WRITE8_HANDLER( mrjong_flipscreen_w );

PALETTE_INIT( mrjong );
VIDEO_START( mrjong );
SCREEN_UPDATE_IND16( mrjong );
