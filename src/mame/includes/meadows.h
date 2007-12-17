/*************************************************************************

    Meadows S2650 hardware

*************************************************************************/

/*----------- defined in audio/meadows.c -----------*/

void meadows_sh_start(void);
void meadows_sh_dac_w(int data);
void meadows_sh_update(void);
extern UINT8 meadows_0c00;
extern UINT8 meadows_0c01;
extern UINT8 meadows_0c02;
extern UINT8 meadows_0c03;


/*----------- defined in video/meadows.c -----------*/

VIDEO_START( meadows );
VIDEO_UPDATE( meadows );
WRITE8_HANDLER( meadows_videoram_w );
WRITE8_HANDLER( meadows_spriteram_w );

