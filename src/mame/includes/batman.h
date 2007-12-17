/*************************************************************************

    Atari Batman hardware

*************************************************************************/

/*----------- defined in video/batman.c -----------*/

extern UINT8 batman_alpha_tile_bank;

VIDEO_START( batman );
VIDEO_UPDATE( batman );

void batman_scanline_update(running_machine *machine, int scrnum, int scanline);
