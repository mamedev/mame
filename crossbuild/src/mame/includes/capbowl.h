/*************************************************************************

    Coors Light Bowling/Bowl-O-Rama hardware

*************************************************************************/

/*----------- defined in video/capbowl.c -----------*/

VIDEO_START( capbowl );
VIDEO_UPDATE( capbowl );

extern UINT8 *capbowl_rowaddress;

WRITE8_HANDLER( bowlrama_blitter_w );
READ8_HANDLER( bowlrama_blitter_r );

WRITE8_HANDLER( capbowl_tms34061_w );
READ8_HANDLER( capbowl_tms34061_r );
