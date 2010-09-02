/*----------- defined in video/speedspn.c -----------*/

extern UINT8 *speedspn_attram;

VIDEO_START( speedspn );
VIDEO_UPDATE( speedspn );

WRITE8_HANDLER( speedspn_vidram_w );
WRITE8_HANDLER( speedspn_attram_w );
READ8_HANDLER( speedspn_vidram_r );
WRITE8_HANDLER( speedspn_banked_vidram_change );
WRITE8_HANDLER( speedspn_global_display_w );

