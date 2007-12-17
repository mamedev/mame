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
#define SPACEFB_AUDIO_CPU_CLOCK			(6000000 / 15)	/* this goes to X2, pixel clock goes to X1 */
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

MACHINE_DRIVER_EXTERN( spacefb_audio );

READ8_HANDLER( spacefb_audio_p2_r );
READ8_HANDLER( spacefb_audio_t0_r );
READ8_HANDLER( spacefb_audio_t1_r );
WRITE8_HANDLER( spacefb_port_1_w );



/*----------- defined in video/spacefb.c -----------*/

extern UINT8 *spacefb_videoram;
extern size_t spacefb_videoram_size;

VIDEO_START( spacefb );
VIDEO_UPDATE( spacefb );

void spacefb_set_flip_screen(UINT8 data);
void spacefb_set_gfx_bank(UINT8 data);
void spacefb_set_palette_bank(UINT8 data);
void spacefb_set_background_red(UINT8 data);
void spacefb_set_background_blue(UINT8 data);
void spacefb_set_disable_star_field(UINT8 data);
void spacefb_set_color_contrast_r(UINT8 data);
void spacefb_set_color_contrast_g(UINT8 data);
void spacefb_set_color_contrast_b(UINT8 data);
