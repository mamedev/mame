

typedef struct _freekick_state freekick_state;
struct _freekick_state
{
	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    spriteram;
	size_t     spriteram_size;

	/* video-related */
	tilemap_t    *freek_tilemap;

	/* misc */
	int        inval, outval, cnt;	// used by oigas
	int        romaddr;
	int        spinner;
	int        nmi_en;
	int        ff_data;
};


/*----------- defined in video/freekick.c -----------*/

VIDEO_START(freekick);
VIDEO_UPDATE(gigas);
VIDEO_UPDATE(pbillrd);
VIDEO_UPDATE(freekick);
WRITE8_HANDLER( freek_videoram_w );
