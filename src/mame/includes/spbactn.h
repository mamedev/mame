typedef struct _spbactn_state spbactn_state;
struct _spbactn_state
{
	UINT16 *bgvideoram;
	UINT16 *fgvideoram;
	UINT16 *spvideoram;

	bitmap_t *tile_bitmap_bg;
	bitmap_t *tile_bitmap_fg;
};


/*----------- defined in video/spbactn.c -----------*/

VIDEO_START( spbactn );
VIDEO_UPDATE( spbactn );
