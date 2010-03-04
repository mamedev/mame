class spbactn_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, spbactn_state(machine)); }

	spbactn_state(running_machine &machine) { }

	UINT16 *bgvideoram;
	UINT16 *fgvideoram;
	UINT16 *spvideoram;

	bitmap_t *tile_bitmap_bg;
	bitmap_t *tile_bitmap_fg;
};


/*----------- defined in video/spbactn.c -----------*/

VIDEO_START( spbactn );
VIDEO_UPDATE( spbactn );
