/***************************************************************************

    Victory system

****************************************************************************/


#define VICTORY_MAIN_CPU_CLOCK		(XTAL_8MHz / 2)


/*----------- defined in video/victory.c -----------*/

extern UINT8 *victory_videoram;
extern UINT8 *victory_charram;

VIDEO_START( victory );
VIDEO_UPDATE( victory );
INTERRUPT_GEN( victory_vblank_interrupt );

READ8_HANDLER( victory_video_control_r );
WRITE8_HANDLER( victory_video_control_w );
WRITE8_HANDLER( victory_paletteram_w );
