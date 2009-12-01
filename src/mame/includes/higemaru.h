/*************************************************************************

    Pirate Ship Higemaru

*************************************************************************/

typedef struct _higemaru_state higemaru_state;
struct _higemaru_state
{
	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    colorram;
	UINT8 *    spriteram;
	size_t     spriteram_size;

	/* video-related */
	tilemap    *bg_tilemap;
};


/*----------- defined in video/higemaru.c -----------*/

WRITE8_HANDLER( higemaru_videoram_w );
WRITE8_HANDLER( higemaru_colorram_w );
WRITE8_HANDLER( higemaru_c800_w );

PALETTE_INIT( higemaru );
VIDEO_START( higemaru );
VIDEO_UPDATE( higemaru );
