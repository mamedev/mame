/***************************************************************************

    Space Firebird hardware

****************************************************************************/

/*
 *  SPACEFB_PIXEL_CLOCK clocks the star generator circuit.  The rest of
 *  the graphics use a clock half of SPACEFB_PIXEL_CLOCK, thus creating
 *  double width pixels.
 */

#define SPACEFB_MASTER_CLOCK			(20160000)
#define SPACEFB_MAIN_CPU_CLOCK			(6000000 / 2)
#define SPACEFB_AUDIO_CPU_CLOCK			(6000000)	/* this goes to X2, pixel clock goes to X1 */
#define SPACEFB_PIXEL_CLOCK				(SPACEFB_MASTER_CLOCK / 2)
#define SPACEFB_HTOTAL					(0x280)
#define SPACEFB_HBEND					(0x000)
#define SPACEFB_HBSTART					(0x200)
#define SPACEFB_VTOTAL					(0x100)
#define SPACEFB_VBEND					(0x010)
#define SPACEFB_VBSTART					(0x0f0)
#define SPACEFB_INT_TRIGGER_COUNT_1		(0x080)
#define SPACEFB_INT_TRIGGER_COUNT_2		(0x0f0)


/*----------- defined in audio/spacefb.c -----------*/

MACHINE_CONFIG_EXTERN( spacefb_audio );

READ8_HANDLER( spacefb_audio_p2_r );
READ8_HANDLER( spacefb_audio_t0_r );
READ8_HANDLER( spacefb_audio_t1_r );
WRITE8_HANDLER( spacefb_port_1_w );


/*----------- defined in video/spacefb.c -----------*/

extern UINT8 *spacefb_videoram;
extern size_t spacefb_videoram_size;

VIDEO_START( spacefb );
VIDEO_UPDATE( spacefb );

WRITE8_HANDLER( spacefb_port_0_w );
WRITE8_HANDLER( spacefb_port_2_w );
