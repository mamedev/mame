/*************************************************************************

    Atari ThunderJaws hardware

*************************************************************************/

/*----------- defined in video/thunderj.c -----------*/

extern UINT8 thunderj_alpha_tile_bank;

VIDEO_START( thunderj );
VIDEO_UPDATE( thunderj );

void thunderj_mark_high_palette(mame_bitmap *bitmap, UINT16 *pf, UINT16 *mo, int x, int y);
