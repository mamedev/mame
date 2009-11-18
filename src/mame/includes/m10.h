/***************************************************************************

    IREM M-10,M-11 and M-15 based hardware

****************************************************************************/


#define IREMM10_MASTER_CLOCK		(12500000)

#define IREMM10_CPU_CLOCK		(IREMM10_MASTER_CLOCK/16)
#define IREMM10_PIXEL_CLOCK		(IREMM10_MASTER_CLOCK/2)
#define IREMM10_HTOTAL			(360)	/* (0x100-0xd3)*8 */
#define IREMM10_HBSTART			(248)
#define IREMM10_HBEND			(8)
#define IREMM10_VTOTAL			(281)	/* (0x200-0xe7) */
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

typedef struct _m10_state m10_state;
struct _m10_state
{
	/* memory pointers */
	UINT8 *             chargen;
	UINT8 *             memory;
	UINT8 *             rom;
	UINT8 *             videoram;
	UINT8 *             colorram;

	/* video-related */
	tilemap *           tx_tilemap;
	gfx_element *       back_gfx;

	/* this is currently unused, because it is needed by gfx_layout (which has no machine) */
	UINT32              extyoffs[32 * 8];

	/* video state */
	UINT8	              bottomline;
	UINT8               flip;

	/* misc */
	int                 last;

	/* devices */
	const device_config *ic8j1;
	const device_config *ic8j2;
	const device_config *samples;
};


/*----------- defined in video/m10.c -----------*/


WRITE8_HANDLER( m10_colorram_w );
WRITE8_HANDLER( m10_chargen_w );
WRITE8_HANDLER( m15_chargen_w );

VIDEO_UPDATE( m10 );
VIDEO_UPDATE( m15 );

VIDEO_START( m10 );
VIDEO_START( m15 );
