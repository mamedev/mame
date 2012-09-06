/*************************************************************************

    Mr. Jong

*************************************************************************/

class mrjong_state : public driver_device
{
public:
	mrjong_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	DECLARE_READ8_MEMBER(io_0x03_r);
	DECLARE_WRITE8_MEMBER(mrjong_videoram_w);
	DECLARE_WRITE8_MEMBER(mrjong_colorram_w);
	DECLARE_WRITE8_MEMBER(mrjong_flipscreen_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
};


/*----------- defined in video/mrjong.c -----------*/


PALETTE_INIT( mrjong );
VIDEO_START( mrjong );
SCREEN_UPDATE_IND16( mrjong );
