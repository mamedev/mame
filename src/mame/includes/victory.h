/***************************************************************************

    Victory system

****************************************************************************/


#define VICTORY_MAIN_CPU_CLOCK		(XTAL_8MHz / 2)

#define VICTORY_PIXEL_CLOCK				(XTAL_11_289MHz / 2)
#define VICTORY_HTOTAL					(0x150)
#define VICTORY_HBEND						(0x000)
#define VICTORY_HBSTART					(0x100)
#define VICTORY_VTOTAL					(0x118)
#define VICTORY_VBEND						(0x000)
#define VICTORY_VBSTART					(0x100)


/*----------- defined in video/victory.c -----------*/

extern UINT8 *victory_videoram;
extern UINT8 *victory_charram;

VIDEO_START( victory );
SCREEN_UPDATE( victory );
INTERRUPT_GEN( victory_vblank_interrupt );

READ8_HANDLER( victory_video_control_r );
WRITE8_HANDLER( victory_video_control_w );
WRITE8_HANDLER( victory_paletteram_w );
