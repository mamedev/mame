/*************************************************************************

    Atari System 1 hardware

*************************************************************************/

/*----------- defined in video/atarisy1.c -----------*/

extern UINT16 *atarisy1_bankselect;

READ16_HANDLER( atarisy1_int3state_r );

WRITE16_HANDLER( atarisy1_spriteram_w );
WRITE16_HANDLER( atarisy1_bankselect_w );
WRITE16_HANDLER( atarisy1_xscroll_w );
WRITE16_HANDLER( atarisy1_yscroll_w );
WRITE16_HANDLER( atarisy1_priority_w );

VIDEO_START( atarisy1 );
VIDEO_UPDATE( atarisy1 );
