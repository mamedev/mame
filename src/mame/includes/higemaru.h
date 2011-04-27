/*************************************************************************

    Pirate Ship Higemaru

*************************************************************************/

class higemaru_state : public driver_device
{
public:
	higemaru_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_colorram;
	UINT8 *    m_spriteram;
	size_t     m_spriteram_size;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
};


/*----------- defined in video/higemaru.c -----------*/

WRITE8_HANDLER( higemaru_videoram_w );
WRITE8_HANDLER( higemaru_colorram_w );
WRITE8_HANDLER( higemaru_c800_w );

PALETTE_INIT( higemaru );
VIDEO_START( higemaru );
SCREEN_UPDATE( higemaru );
