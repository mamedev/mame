/*************************************************************************

    Megazone

*************************************************************************/

typedef struct _megazone_state megazone_state;
struct _megazone_state
{
	/* memory pointers */
	UINT8 *       scrollx;
	UINT8 *       scrolly;
	UINT8 *       videoram;
	UINT8 *       colorram;
	UINT8 *       videoram2;
	UINT8 *       colorram2;
	UINT8 *       spriteram;
	size_t        spriteram_size;
	size_t        videoram_size;
	size_t        videoram2_size;

	/* video-related */
	bitmap_t      *tmpbitmap;
	int           flipscreen;

	/* misc */
	int           i8039_status;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *daccpu;
};



/*----------- defined in video/megazone.c -----------*/

WRITE8_HANDLER( megazone_flipscreen_w );

PALETTE_INIT( megazone );
VIDEO_START( megazone );
VIDEO_UPDATE( megazone );
