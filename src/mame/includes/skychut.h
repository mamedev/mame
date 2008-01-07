/***************************************************************************

    IREM M-10,M-11 and M-15 based hardware

****************************************************************************/


#define IREMM10_MASTER_CLOCK		(25000000)

#define IREMM10_CPU_CLOCK		(IREMM10_MASTER_CLOCK/20)
#define IREMM10_PIXEL_CLOCK		(IREMM10_MASTER_CLOCK/5)
#define IREMM10_HTOTAL			(318)
#define IREMM10_HBSTART			(248)
#define IREMM10_HBEND			(8)
#define IREMM10_VTOTAL			(262)
#define IREMM10_VBSTART			(240)
#define IREMM10_VBEND			(16)

#define IREMM15_MASTER_CLOCK	(11730000)

#define IREMM15_CPU_CLOCK		(IREMM15_MASTER_CLOCK/10)
#define IREMM15_PIXEL_CLOCK		(IREMM15_MASTER_CLOCK/2)
#define IREMM15_HTOTAL			(372)
#define IREMM15_HBSTART			(256)
#define IREMM15_HBEND			(0)
#define IREMM15_VTOTAL			(262)
#define IREMM15_VBSTART			(240)
#define IREMM15_VBEND			(16)

typedef struct _irem_state irem_state;
struct _irem_state
{
	/* memory pointers */
	UINT8 *			chargen;
	UINT8 *			memory;
	UINT8 *			rom;

	/* machine states */

	/* sound state */

	/* video state */
	UINT8			bottomline;
	UINT8 			flip;

	/* Specific states */
};


/*----------- defined in video/skychut.c -----------*/


WRITE8_HANDLER( skychut_colorram_w );
WRITE8_HANDLER( iremm15_chargen_w );

VIDEO_UPDATE( iremm10 );
VIDEO_UPDATE( iremm15 );

VIDEO_START( iremm10 );
VIDEO_START( iremm15 );

