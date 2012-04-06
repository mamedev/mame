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


class spacefb_state : public driver_device
{
public:
	spacefb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_sound_latch;
	emu_timer *m_interrupt_timer;
	UINT8 *m_videoram;
	size_t m_videoram_size;
	UINT8 *m_object_present_map;
	UINT8 m_port_0;
	UINT8 m_port_2;
	UINT32 m_star_shift_reg;
	double m_color_weights_rg[3];
	double m_color_weights_b[2];
	DECLARE_WRITE8_MEMBER(spacefb_port_0_w);
	DECLARE_WRITE8_MEMBER(spacefb_port_2_w);
};


/*----------- defined in audio/spacefb.c -----------*/

MACHINE_CONFIG_EXTERN( spacefb_audio );

READ8_HANDLER( spacefb_audio_p2_r );
READ8_HANDLER( spacefb_audio_t0_r );
READ8_HANDLER( spacefb_audio_t1_r );
WRITE8_HANDLER( spacefb_port_1_w );


/*----------- defined in video/spacefb.c -----------*/

VIDEO_START( spacefb );
SCREEN_UPDATE_RGB32( spacefb );

