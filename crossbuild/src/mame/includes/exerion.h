/*************************************************************************

    Jaleco Exerion

*************************************************************************/


#define EXERION_MASTER_CLOCK	(20000000)
#define EXERION_CPU_CLOCK		(EXERION_MASTER_CLOCK / 6)
#define EXERION_AY8910_CLOCK	(EXERION_CPU_CLOCK / 2)
#define EXERION_PIXEL_CLOCK		(EXERION_MASTER_CLOCK / 3)
#define EXERION_HCOUNT_START	(0x58)
#define EXERION_HTOTAL			(512-EXERION_HCOUNT_START)
#define EXERION_HBEND			(12*8)	/* ?? */
#define EXERION_HBSTART			(52*8)	/* ?? */
#define EXERION_VTOTAL			(256)
#define EXERION_VBEND			(16)
#define EXERION_VBSTART			(240)


/*----------- defined in video/exerion.c -----------*/

PALETTE_INIT( exerion );
VIDEO_START( exerion );
VIDEO_UPDATE( exerion );

WRITE8_HANDLER( exerion_videoreg_w );
WRITE8_HANDLER( exerion_video_latch_w );
READ8_HANDLER( exerion_video_timing_r );

extern UINT8 exerion_cocktail_flip;
