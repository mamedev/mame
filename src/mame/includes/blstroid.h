/*************************************************************************

    Atari Blasteroids hardware

*************************************************************************/

/*----------- defined in video/blstroid.c -----------*/

VIDEO_START( blstroid );
VIDEO_UPDATE( blstroid );

void blstroid_scanline_update(running_machine *machine, int scrnum, int scanline);

extern UINT16 *blstroid_priorityram;
