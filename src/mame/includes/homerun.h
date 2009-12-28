/*************************************************************************

    Moero Pro Yakyuu Homerun & Dynamic Shooting

*************************************************************************/

typedef struct _homerun_state homerun_state;
struct _homerun_state
{
	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    spriteram;
	size_t     spriteram_size;

	/* video-related */
	tilemap_t    *tilemap;
	int        gfx_ctrl;

	/* misc */
	int        xpa, xpb, xpc;
	int        gc_up, gc_down;
};


/*----------- defined in video/homerun.c -----------*/

WRITE8_HANDLER( homerun_videoram_w );
WRITE8_HANDLER( homerun_color_w );
WRITE8_DEVICE_HANDLER( homerun_banking_w );

VIDEO_START(homerun);
VIDEO_UPDATE(homerun);
