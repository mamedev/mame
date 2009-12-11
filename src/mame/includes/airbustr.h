/*************************************************************************

    Air Buster

*************************************************************************/

typedef struct _airbustr_state airbustr_state;
struct _airbustr_state
{
	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    videoram2;
	UINT8 *    colorram;
	UINT8 *    colorram2;
	UINT8 *    paletteram;
	UINT8 *    devram;

	/* video-related */
	tilemap    *bg_tilemap, *fg_tilemap;
	bitmap_t   *sprites_bitmap;
	int        bg_scrollx, bg_scrolly, fg_scrollx, fg_scrolly, highbits;

	/* misc */
	int        soundlatch_status, soundlatch2_status;
	int        master_addr;
	int        slave_addr;

	/* devices */
	const device_config *master;
	const device_config *slave;
	const device_config *audiocpu;
	const device_config *pandora;
};


/*----------- defined in video/airbustr.c -----------*/

WRITE8_HANDLER( airbustr_videoram_w );
WRITE8_HANDLER( airbustr_colorram_w );
WRITE8_HANDLER( airbustr_videoram2_w );
WRITE8_HANDLER( airbustr_colorram2_w );
WRITE8_HANDLER( airbustr_scrollregs_w );

VIDEO_START( airbustr );
VIDEO_UPDATE( airbustr );
VIDEO_EOF( airbustr );
