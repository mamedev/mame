/*************************************************************************

    Atari Gauntlet hardware

*************************************************************************/

/*----------- defined in video/gauntlet.c -----------*/

extern UINT8 vindctr2_screen_refresh;

WRITE16_HANDLER( gauntlet_xscroll_w );
WRITE16_HANDLER( gauntlet_yscroll_w );

VIDEO_START( gauntlet );
VIDEO_UPDATE( gauntlet );
